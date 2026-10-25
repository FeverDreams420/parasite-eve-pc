/* Mods panel: F1 opens a paused, in-window list of every mod's rows.
 *
 * It sits beside the game's Escape settings panel without touching it: the
 * PsyZ overlay callbacks and the event filter all return the previous one, so
 * this panel chains in front (events) and behind (drawing) of what was
 * installed before. Like the settings panel it is modal: opening pauses the
 * host clock and audio and runs its own present loop until it closes, so the
 * game never sees its input. Drawing copies the settings panel's approach: the
 * panel is rasterised on the CPU (stb_truetype, IBM Plex Sans) and drawn as
 * one textured quad over a dimmed frame. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <psyz/audio.h>
#include <psyz/overlay.h>
#include <psyz/overlay_sdl3.h>
#include <psyz/overlay_sdl3_gpu.h>
#include <psyz/video.h>

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../src/main/boot/stb_truetype.h"
#include "../../src/main/boot/fonts/ibm_plex_sans.ttf.inc"

#include "../../third_party/psyz/external/SDL/src/render/gpu/shaders/tri_texture.vert.spv.h"
#include "../../third_party/psyz/external/SDL/src/render/gpu/shaders/tri_texture.vert.msl.h"
#include "../../third_party/psyz/external/SDL/src/render/gpu/shaders/tri_texture.vert.dxil.h"
#include "../../third_party/psyz/external/SDL/src/render/gpu/shaders/texture_rgba.frag.spv.h"
#include "../../third_party/psyz/external/SDL/src/render/gpu/shaders/texture_rgba.frag.msl.h"
#include "../../third_party/psyz/external/SDL/src/render/gpu/shaders/texture_rgba.frag.dxil.h"

#include "pe_mod_host.h"
#include "pe_mod_menu.h"

/* ---- layout, in design units scaled with the window ---- */

enum {
    PANEL_W = 680,
    PANEL_PAD = 30,
    HEADER_H = 96,
    ROW_H = 42,
    VISIBLE_ROWS = 12,
    FOOTER_H = 70,
    VALUE_W = 280,
    REFERENCE_H = 820,
    MAX_ENTRIES = 256
};

typedef struct { float r, g, b, a; } Color;
typedef struct { uint8_t *pixels; int w, h; } Surface;
typedef struct { float scale; int x, y, w, h; } Layout;
typedef struct { float x, y, r, g, b, a, u, v; } Vertex;

static const Color kPanel = {0.075f, 0.082f, 0.102f, 0.96f};
static const Color kBorder = {1.0f, 1.0f, 1.0f, 0.10f};
static const Color kAccent = {0.86f, 0.36f, 0.30f, 1.0f}; /* red, so it never looks like Settings */
static const Color kText = {0.93f, 0.95f, 0.98f, 1.0f};
static const Color kMuted = {0.60f, 0.65f, 0.73f, 1.0f};

static Color WithAlpha(Color c, float a) { c.a = a; return c; }

/* ---- page model ---- */

typedef struct {
    int row;          /* >= 0: a registry row; -1: a sub-page */
    char page[96];    /* sub-page: its full path */
    const char *name; /* sub-page: its last segment (points into page) */
} Entry;

static struct {
    int open, dirty;
    char path[96]; /* current page, "" = list of mods */
    int cursor, scroll;
    Entry entries[MAX_ENTRIES];
    int count;
    const char *status;
} s_panel;

static int pe_has_entry_page(const char *page) {
    int i;
    for (i = 0; i < s_panel.count; i++)
        if (s_panel.entries[i].row < 0 && strcmp(s_panel.entries[i].page, page) == 0) return 1;
    return 0;
}

static void pe_build_entries(void) {
    size_t plen = strlen(s_panel.path);
    int i;
    s_panel.count = 0;
    for (i = 0; i < pe_menu_row_count() && s_panel.count < MAX_ENTRIES; i++) {
        PeMenuRow *row = pe_menu_row(i);
        const char *rest;
        Entry *e;
        if (strcmp(row->page, s_panel.path) == 0) {
            s_panel.entries[s_panel.count++] = (Entry){i, "", NULL};
            continue;
        }
        if (plen == 0) rest = row->page;
        else if (strncmp(row->page, s_panel.path, plen) == 0 && row->page[plen] == '/')
            rest = row->page + plen + 1;
        else continue;
        e = &s_panel.entries[s_panel.count];
        e->row = -1;
        snprintf(e->page, sizeof(e->page), "%.*s", (int)(rest - row->page + strcspn(rest, "/")),
                 row->page);
        if (pe_has_entry_page(e->page)) continue;
        e->name = e->page + (rest - row->page);
        s_panel.count++;
    }
    if (s_panel.cursor >= s_panel.count) s_panel.cursor = s_panel.count ? s_panel.count - 1 : 0;
}

static void pe_go(const char *path, int cursor) {
    snprintf(s_panel.path, sizeof(s_panel.path), "%s", path);
    s_panel.cursor = cursor;
    s_panel.scroll = 0;
    pe_build_entries();
}

static void pe_go_up(void) {
    char parent[96], *slash;
    const char *from = s_panel.path;
    int i, cursor = 0;
    snprintf(parent, sizeof(parent), "%s", from);
    slash = strrchr(parent, '/');
    if (slash) *slash = '\0';
    else parent[0] = '\0';
    {
        char came_from[96];
        snprintf(came_from, sizeof(came_from), "%s", from);
        pe_go(parent, 0);
        for (i = 0; i < s_panel.count; i++)
            if (s_panel.entries[i].row < 0 && strcmp(s_panel.entries[i].page, came_from) == 0) cursor = i;
    }
    s_panel.cursor = cursor;
}

/* ---- raster ---- */

static Surface s_surface;
static stbtt_fontinfo s_font;
static int s_font_state;

static int pe_font_ready(void) {
    int offset;
    if (s_font_state) return s_font_state > 0;
    offset = stbtt_GetFontOffsetForIndex(ibm_plex_sans_ttf, 0);
    s_font_state = offset >= 0 && stbtt_InitFont(&s_font, ibm_plex_sans_ttf, offset) ? 1 : -1;
    return s_font_state > 0;
}

static int pe_surface_resize(int w, int h) {
    if (s_surface.pixels && s_surface.w == w && s_surface.h == h + 1) return 1;
    free(s_surface.pixels);
    s_surface.pixels = (uint8_t *)calloc((size_t)w * (size_t)(h + 1), 4u);
    s_surface.w = s_surface.pixels ? w : 0;
    s_surface.h = s_surface.pixels ? h + 1 : 0;
    return s_surface.pixels != NULL;
}

static float Clamp01(float v) { return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v; }

static void BlendPixel(int x, int y, Color c, float coverage) {
    Surface *s = &s_surface;
    float a = c.a * coverage, keep = 1.0f - a;
    uint8_t *p;
    if (x < 0 || y < 0 || x >= s->w || y >= s->h - 1 || a <= 0.0f) return; /* last row: solid texel */
    p = s->pixels + ((size_t)y * (size_t)s->w + (size_t)x) * 4u;
    p[0] = (uint8_t)(c.r * a * 255.0f + p[0] * keep + 0.5f);
    p[1] = (uint8_t)(c.g * a * 255.0f + p[1] * keep + 0.5f);
    p[2] = (uint8_t)(c.b * a * 255.0f + p[2] * keep + 0.5f);
    p[3] = (uint8_t)(a * 255.0f + p[3] * keep + 0.5f);
}

static void FillRoundRect(float x, float y, float w, float h, float r, Color c) {
    int ix, iy;
    for (iy = (int)floorf(y) - 1; iy < (int)ceilf(y + h) + 1; iy++) {
        for (ix = (int)floorf(x) - 1; ix < (int)ceilf(x + w) + 1; ix++) {
            float px = (float)ix + 0.5f, py = (float)iy + 0.5f;
            float qx = fabsf(px - (x + w * 0.5f)) - (w * 0.5f - r);
            float qy = fabsf(py - (y + h * 0.5f)) - (h * 0.5f - r);
            float ox = qx > 0.0f ? qx : 0.0f, oy = qy > 0.0f ? qy : 0.0f;
            float inside = qx > qy ? qx : qy;
            float d = sqrtf(ox * ox + oy * oy) + (inside < 0.0f ? inside : 0.0f) - r;
            BlendPixel(ix, iy, c, Clamp01(0.5f - d));
        }
    }
}

typedef enum { ALIGN_LEFT, ALIGN_CENTRE, ALIGN_RIGHT } Align;

static float TextWidth(const char *text, float em) {
    float scale = stbtt_ScaleForMappingEmToPixels(&s_font, em), width = 0.0f;
    for (; *text; text++) {
        int advance, bearing;
        stbtt_GetCodepointHMetrics(&s_font, (unsigned char)*text, &advance, &bearing);
        width += (float)advance * scale;
    }
    return width;
}

static void DrawText(float x, float baseline, float em, const char *text, Color c, Align align) {
    enum { GLYPH_MAX = 256 };
    static uint8_t glyph[GLYPH_MAX * GLYPH_MAX];
    float scale = stbtt_ScaleForMappingEmToPixels(&s_font, em);
    float width = TextWidth(text, em);
    x -= align == ALIGN_CENTRE ? width * 0.5f : align == ALIGN_RIGHT ? width : 0.0f;
    for (; *text; text++) {
        int cp = (unsigned char)*text, advance, bearing, x0, y0, x1, y1, gx, gy;
        stbtt_GetCodepointHMetrics(&s_font, cp, &advance, &bearing);
        stbtt_GetCodepointBitmapBox(&s_font, cp, scale, scale, &x0, &y0, &x1, &y1);
        if (x1 > x0 && y1 > y0 && x1 - x0 <= GLYPH_MAX && y1 - y0 <= GLYPH_MAX) {
            int gw = x1 - x0, gh = y1 - y0;
            int ox = (int)floorf(x + 0.5f) + x0, oy = (int)floorf(baseline + 0.5f) + y0;
            stbtt_MakeCodepointBitmap(&s_font, glyph, gw, gh, gw, scale, scale, cp);
            for (gy = 0; gy < gh; gy++)
                for (gx = 0; gx < gw; gx++)
                    if (glyph[gy * gw + gx]) BlendPixel(ox + gx, oy + gy, c, glyph[gy * gw + gx] / 255.0f);
        }
        x += (float)advance * scale;
    }
}

/* ---- paint ---- */

static int pe_visible_rows(void) {
    int n = s_panel.count < VISIBLE_ROWS ? s_panel.count : VISIBLE_ROWS;
    return n > 0 ? n : 1;
}

static int pe_design_h(void) { return HEADER_H + pe_visible_rows() * ROW_H + FOOTER_H; }

static void pe_layout(int win_w, int win_h, Layout *l) {
    float scale = (float)win_h / (float)REFERENCE_H;
    float wl = (float)win_w * 0.94f / (float)PANEL_W, hl = (float)win_h * 0.94f / (float)pe_design_h();
    if (scale > wl) scale = wl;
    if (scale > hl) scale = hl;
    if (scale > 2.5f) scale = 2.5f;
    if (scale < 0.5f) scale = 0.5f;
    scale = floorf(scale * 8.0f) / 8.0f;
    l->scale = scale;
    l->w = (int)ceilf((float)PANEL_W * scale);
    l->h = (int)ceilf((float)pe_design_h() * scale);
    l->x = (win_w - l->w) / 2;
    l->y = (win_h - l->h) / 2;
}

static const char *pe_entry_help(void) {
    const Entry *e;
    if (s_panel.status) return s_panel.status;
    if (s_panel.count == 0) return "No mod has registered any rows.";
    e = &s_panel.entries[s_panel.cursor];
    return e->row < 0 ? "Open this page." : pe_menu_row(e->row)->description;
}

static void pe_paint(const Layout *l) {
    const float s = l->scale;
    int visible = pe_visible_rows(), i;
    float footer = (float)(HEADER_H + visible * ROW_H);

    memset(s_surface.pixels, 0, (size_t)s_surface.w * (size_t)s_surface.h * 4u);
    FillRoundRect(0, 0, PANEL_W * s, pe_design_h() * s, 16 * s, kBorder);
    FillRoundRect(1, 1, PANEL_W * s - 2, pe_design_h() * s - 2, 15 * s, kPanel);
    DrawText(PANEL_PAD * s, 42 * s, 26 * s, "Mods", kText, ALIGN_LEFT);
    DrawText((PANEL_W - PANEL_PAD) * s, 40 * s, 14 * s, "GAME PAUSED", kMuted, ALIGN_RIGHT);
    DrawText(PANEL_PAD * s, 74 * s, 15 * s, s_panel.path[0] ? s_panel.path : "All mods",
             kMuted, ALIGN_LEFT);
    FillRoundRect(PANEL_PAD * s, (HEADER_H - 8) * s, (PANEL_W - 2 * PANEL_PAD) * s, 1, 0,
                  WithAlpha(kText, 0.10f));

    for (i = 0; i < visible && s_panel.scroll + i < s_panel.count; i++) {
        int index = s_panel.scroll + i, selected = index == s_panel.cursor;
        const Entry *e = &s_panel.entries[index];
        float top = (HEADER_H + i * ROW_H) * s, mid = top + ROW_H * 0.5f * s, base = mid + 6 * s;
        Color label = selected ? kText : WithAlpha(kText, 0.82f);
        char value[96];

        if (selected) {
            FillRoundRect(PANEL_PAD / 2 * s, top + 3 * s, (PANEL_W - PANEL_PAD) * s, (ROW_H - 6) * s,
                          9 * s, WithAlpha(kAccent, 0.16f));
            FillRoundRect(PANEL_PAD / 2 * s + 4 * s, mid - 10 * s, 3 * s, 20 * s, 1.5f * s, kAccent);
        }
        if (e->row < 0) {
            DrawText(PANEL_PAD * s, base, 17 * s, e->name, label, ALIGN_LEFT);
            DrawText((PANEL_W - PANEL_PAD) * s, base, 17 * s, ">", selected ? kAccent : kMuted, ALIGN_RIGHT);
            continue;
        }
        PeMenuRow *row = pe_menu_row(e->row);
        pe_menu_row_value_text(row, value, sizeof(value));
        DrawText(PANEL_PAD * s, base, 17 * s, row->label, label, ALIGN_LEFT);
        DrawText((PANEL_W - PANEL_PAD - VALUE_W * 0.5f) * s, base, 17 * s,
                 row->kind == PE_ROW_ACTION ? (selected ? "Press Enter" : "") : value,
                 selected ? kAccent : WithAlpha(kText, 0.9f), ALIGN_CENTRE);
    }
    if (s_panel.count > visible) {
        float track_top = (HEADER_H + 4) * s, track_h = (visible * ROW_H - 8) * s;
        float thumb_h = track_h * (float)visible / (float)s_panel.count;
        float thumb_top = track_top + (track_h - thumb_h) * (float)s_panel.scroll /
                                          (float)(s_panel.count - visible);
        FillRoundRect((PANEL_W - 12) * s, track_top, 4 * s, track_h, 2 * s, WithAlpha(kText, 0.10f));
        FillRoundRect((PANEL_W - 12) * s, thumb_top, 4 * s, thumb_h, 2 * s, WithAlpha(kAccent, 0.65f));
    }
    FillRoundRect(PANEL_PAD * s, footer * s, (PANEL_W - 2 * PANEL_PAD) * s, 1, 0, WithAlpha(kText, 0.10f));
    DrawText(PANEL_PAD * s, (footer + 28) * s, 14 * s, pe_entry_help(), kMuted, ALIGN_LEFT);
    DrawText(PANEL_PAD * s, (footer + 54) * s, 13 * s,
             "Up/Down select   Left/Right change   Enter open/run   Backspace back   F1 close",
             WithAlpha(kMuted, 0.7f), ALIGN_LEFT);
    s_surface.pixels[((size_t)(s_surface.h - 1) * (size_t)s_surface.w) * 4u + 3] = 255;
}

/* ---- SDL_GPU (same pipeline as the settings panel) ---- */

static SDL_Window *s_window;
static SDL_GPUDevice *s_device;
static SDL_GPUGraphicsPipeline *s_pipeline;
static SDL_GPUSampler *s_sampler;
static SDL_GPUTexture *s_texture;
static SDL_GPUBuffer *s_vertices;
static int s_tex_w, s_tex_h, s_win_w, s_win_h;
static Layout s_layout;
static PsyzOverlayInitCB_SDL3GPU s_prev_init;
static PsyzOverlayRenderCB_SDL3GPU s_prev_render;
static PsyzOverlayDestroyCB s_prev_destroy;
static PsyzOverlayEventFilter_SDL3 s_prev_filter;

static void pe_gpu_release(void) {
    if (s_device) {
        if (s_texture) SDL_ReleaseGPUTexture(s_device, s_texture);
        if (s_vertices) SDL_ReleaseGPUBuffer(s_device, s_vertices);
        if (s_sampler) SDL_ReleaseGPUSampler(s_device, s_sampler);
        if (s_pipeline) SDL_ReleaseGPUGraphicsPipeline(s_device, s_pipeline);
    }
    s_texture = NULL;
    s_vertices = NULL;
    s_sampler = NULL;
    s_pipeline = NULL;
    s_tex_w = s_tex_h = s_win_w = s_win_h = 0;
}

static SDL_GPUShader *pe_shader(SDL_GPUShaderStage stage, const unsigned char *spv, size_t spv_size,
                                const unsigned char *msl, size_t msl_size, const unsigned char *dxil,
                                size_t dxil_size, Uint32 samplers) {
    SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(s_device);
    SDL_GPUShaderCreateInfo info;
    SDL_zero(info);
    info.stage = stage;
    info.num_samplers = samplers;
    info.num_uniform_buffers = 1;
    info.entrypoint = "main";
    if (formats & SDL_GPU_SHADERFORMAT_SPIRV) {
        info.code = spv; info.code_size = spv_size; info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    } else if (formats & SDL_GPU_SHADERFORMAT_MSL) {
        info.code = msl; info.code_size = msl_size; info.format = SDL_GPU_SHADERFORMAT_MSL;
        info.entrypoint = "main0";
    } else if (formats & SDL_GPU_SHADERFORMAT_DXIL) {
        info.code = dxil; info.code_size = dxil_size; info.format = SDL_GPU_SHADERFORMAT_DXIL;
    } else {
        return NULL;
    }
    return SDL_CreateGPUShader(s_device, &info);
}

static int pe_gpu_pipeline(SDL_GPUTextureFormat format) {
    SDL_GPUShader *vs, *fs;
    SDL_GPUVertexBufferDescription vbd;
    SDL_GPUVertexAttribute attr[3];
    SDL_GPUColorTargetDescription color;
    SDL_GPUGraphicsPipelineCreateInfo info;
    SDL_GPUSamplerCreateInfo sampler;
    SDL_GPUBufferCreateInfo buffer;

    if (format == SDL_GPU_TEXTUREFORMAT_INVALID) return 0;
    if (s_pipeline && s_sampler && s_vertices) return 1;
    vs = pe_shader(SDL_GPU_SHADERSTAGE_VERTEX, tri_texture_vert_spv, sizeof(tri_texture_vert_spv),
                   tri_texture_vert_msl, sizeof(tri_texture_vert_msl), tri_texture_vert_dxil,
                   sizeof(tri_texture_vert_dxil), 0);
    fs = pe_shader(SDL_GPU_SHADERSTAGE_FRAGMENT, texture_rgba_frag_spv, sizeof(texture_rgba_frag_spv),
                   texture_rgba_frag_msl, sizeof(texture_rgba_frag_msl), texture_rgba_frag_dxil,
                   sizeof(texture_rgba_frag_dxil), 1);
    if (!vs || !fs) {
        if (vs) SDL_ReleaseGPUShader(s_device, vs);
        if (fs) SDL_ReleaseGPUShader(s_device, fs);
        return 0;
    }
    SDL_zero(vbd);
    vbd.pitch = sizeof(Vertex);
    vbd.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    SDL_zero(attr);
    attr[0] = (SDL_GPUVertexAttribute){0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 0};
    attr[1] = (SDL_GPUVertexAttribute){1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, 2 * sizeof(float)};
    attr[2] = (SDL_GPUVertexAttribute){2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 6 * sizeof(float)};
    SDL_zero(color);
    color.format = format;
    color.blend_state.enable_blend = true;
    color.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    color.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    color.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    color.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    SDL_zero(info);
    info.vertex_shader = vs;
    info.fragment_shader = fs;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    info.vertex_input_state.num_vertex_buffers = 1;
    info.vertex_input_state.vertex_buffer_descriptions = &vbd;
    info.vertex_input_state.num_vertex_attributes = 3;
    info.vertex_input_state.vertex_attributes = attr;
    info.target_info.color_target_descriptions = &color;
    info.target_info.num_color_targets = 1;
    s_pipeline = SDL_CreateGPUGraphicsPipeline(s_device, &info);
    SDL_ReleaseGPUShader(s_device, vs);
    SDL_ReleaseGPUShader(s_device, fs);
    SDL_zero(sampler);
    sampler.min_filter = sampler.mag_filter = SDL_GPU_FILTER_NEAREST;
    sampler.address_mode_u = sampler.address_mode_v = sampler.address_mode_w =
        SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    s_sampler = SDL_CreateGPUSampler(s_device, &sampler);
    SDL_zero(buffer);
    buffer.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    buffer.size = sizeof(Vertex) * 12;
    s_vertices = SDL_CreateGPUBuffer(s_device, &buffer);
    if (!s_pipeline || !s_sampler || !s_vertices) {
        pe_gpu_release();
        return 0;
    }
    return 1;
}

static void pe_quad(Vertex *v, float x0, float y0, float x1, float y1, float u0, float v0, float u1,
                    float v1, float a) {
    v[0] = (Vertex){x0, y0, 1, 1, 1, a, u0, v0};
    v[1] = (Vertex){x1, y0, 1, 1, 1, a, u1, v0};
    v[2] = (Vertex){x0, y1, 1, 1, 1, a, u0, v1};
    v[3] = v[2];
    v[4] = v[1];
    v[5] = (Vertex){x1, y1, 1, 1, 1, a, u1, v1};
}

static void pe_gpu_upload(void) {
    Vertex verts[12];
    float w = (float)s_win_w, h = (float)s_win_h, su, sv;
    Uint32 tex_bytes, vtx_bytes = sizeof(verts);
    SDL_GPUTransferBufferCreateInfo ti;
    SDL_GPUTransferBuffer *transfer;
    SDL_GPUCommandBuffer *cmd;
    SDL_GPUCopyPass *copy;
    SDL_GPUTextureTransferInfo tsrc;
    SDL_GPUTextureRegion tdst;
    SDL_GPUTransferBufferLocation vsrc;
    SDL_GPUBufferRegion vdst;
    uint8_t *mapped;

    if (!s_texture || s_tex_w != s_surface.w || s_tex_h != s_surface.h) {
        SDL_GPUTextureCreateInfo info;
        if (s_texture) SDL_ReleaseGPUTexture(s_device, s_texture);
        SDL_zero(info);
        info.type = SDL_GPU_TEXTURETYPE_2D;
        info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
        info.width = (Uint32)s_surface.w;
        info.height = (Uint32)s_surface.h;
        info.layer_count_or_depth = 1;
        info.num_levels = 1;
        s_texture = SDL_CreateGPUTexture(s_device, &info);
        s_tex_w = s_surface.w;
        s_tex_h = s_surface.h;
        if (!s_texture) return;
    }
    su = 0.5f / (float)s_tex_w;
    sv = ((float)s_tex_h - 0.5f) / (float)s_tex_h;
    pe_quad(verts, -1, 1, 1, -1, su, sv, su, sv, 0.62f);
    pe_quad(verts + 6, (float)s_layout.x / w * 2 - 1, 1 - (float)s_layout.y / h * 2,
            (float)(s_layout.x + s_layout.w) / w * 2 - 1, 1 - (float)(s_layout.y + s_layout.h) / h * 2,
            0, 0, 1, (float)(s_tex_h - 1) / (float)s_tex_h, 1);

    tex_bytes = (Uint32)((size_t)s_surface.w * (size_t)s_surface.h * 4u);
    SDL_zero(ti);
    ti.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    ti.size = tex_bytes + vtx_bytes;
    if (!(transfer = SDL_CreateGPUTransferBuffer(s_device, &ti))) return;
    if (!(mapped = (uint8_t *)SDL_MapGPUTransferBuffer(s_device, transfer, false))) {
        SDL_ReleaseGPUTransferBuffer(s_device, transfer);
        return;
    }
    memcpy(mapped, s_surface.pixels, tex_bytes);
    memcpy(mapped + tex_bytes, verts, vtx_bytes);
    SDL_UnmapGPUTransferBuffer(s_device, transfer);
    if (!(cmd = SDL_AcquireGPUCommandBuffer(s_device))) {
        SDL_ReleaseGPUTransferBuffer(s_device, transfer);
        return;
    }
    copy = SDL_BeginGPUCopyPass(cmd);
    SDL_zero(tsrc);
    tsrc.transfer_buffer = transfer;
    tsrc.pixels_per_row = (Uint32)s_surface.w;
    tsrc.rows_per_layer = (Uint32)s_surface.h;
    SDL_zero(tdst);
    tdst.texture = s_texture;
    tdst.w = (Uint32)s_surface.w;
    tdst.h = (Uint32)s_surface.h;
    tdst.d = 1;
    SDL_UploadToGPUTexture(copy, &tsrc, &tdst, false);
    SDL_zero(vsrc);
    vsrc.transfer_buffer = transfer;
    vsrc.offset = tex_bytes;
    SDL_zero(vdst);
    vdst.buffer = s_vertices;
    vdst.size = vtx_bytes;
    SDL_UploadToGPUBuffer(copy, &vsrc, &vdst, false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(s_device, transfer);
}

static void pe_refresh(void) {
    int w = 0, h = 0;
    Layout l;
    if (!s_device || !s_window) return;
    SDL_GetWindowSizeInPixels(s_window, &w, &h);
    if (w <= 0 || h <= 0 || !pe_font_ready()) return;
    pe_layout(w, h, &l);
    if (w != s_win_w || h != s_win_h || l.w != s_layout.w || l.h != s_layout.h) s_panel.dirty = 1;
    s_win_w = w;
    s_win_h = h;
    s_layout = l;
    if (!s_panel.dirty || !pe_surface_resize(l.w, l.h)) return;
    pe_paint(&l);
    if (pe_gpu_pipeline(SDL_GetGPUSwapchainTextureFormat(s_device, s_window))) pe_gpu_upload();
    s_panel.dirty = 0;
}

static void pe_render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    static const float kIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    static const float kColorScale[4] = {1, 0, 0, 0};
    SDL_GPUViewport viewport;
    SDL_Rect scissor;
    SDL_GPUTextureSamplerBinding tex;
    SDL_GPUBufferBinding vtx;

    if (s_prev_render) s_prev_render(cmd, pass);
    if (!s_panel.open || !s_pipeline || !s_texture || !s_vertices || s_win_w <= 0 || s_win_h <= 0) return;
    viewport = (SDL_GPUViewport){0, 0, (float)s_win_w, (float)s_win_h, 0, 1};
    scissor = (SDL_Rect){0, 0, s_win_w, s_win_h};
    SDL_SetGPUViewport(pass, &viewport);
    SDL_SetGPUScissor(pass, &scissor);
    SDL_BindGPUGraphicsPipeline(pass, s_pipeline);
    SDL_zero(tex);
    tex.texture = s_texture;
    tex.sampler = s_sampler;
    SDL_BindGPUFragmentSamplers(pass, 0, &tex, 1);
    SDL_zero(vtx);
    vtx.buffer = s_vertices;
    SDL_BindGPUVertexBuffers(pass, 0, &vtx, 1);
    SDL_PushGPUVertexUniformData(cmd, 0, kIdentity, sizeof(kIdentity));
    SDL_PushGPUFragmentUniformData(cmd, 0, kColorScale, sizeof(kColorScale));
    SDL_DrawGPUPrimitives(pass, 12, 1, 0, 0);
}

static void pe_init_gpu(SDL_Window *window, SDL_GPUDevice *device) {
    if (s_prev_init) s_prev_init(window, device);
    if (s_device && s_device != device) pe_gpu_release();
    s_window = window;
    s_device = device;
}

static void pe_destroy_gpu(void) {
    if (s_prev_destroy) s_prev_destroy();
    pe_gpu_release();
    free(s_surface.pixels);
    memset(&s_surface, 0, sizeof(s_surface));
    s_device = NULL;
    s_window = NULL;
}

/* ---- input ---- */

static void pe_move(int delta) {
    int visible = pe_visible_rows();
    if (s_panel.count == 0) return;
    s_panel.cursor = (s_panel.cursor + delta + s_panel.count) % s_panel.count;
    if (s_panel.cursor < s_panel.scroll) s_panel.scroll = s_panel.cursor;
    if (s_panel.cursor >= s_panel.scroll + visible) s_panel.scroll = s_panel.cursor - visible + 1;
}

static void pe_back(void) {
    if (s_panel.path[0]) pe_go_up();
    else s_panel.open = 0;
}

static void pe_accept(void) {
    Entry *e;
    if (s_panel.count == 0) return;
    e = &s_panel.entries[s_panel.cursor];
    if (e->row < 0) {
        char page[96];
        snprintf(page, sizeof(page), "%s", e->page);
        pe_go(page, 0);
    } else {
        pe_menu_row_accept(pe_menu_row(e->row));
    }
}

static void pe_step(int direction) {
    Entry *e;
    if (s_panel.count == 0) return;
    e = &s_panel.entries[s_panel.cursor];
    if (e->row >= 0) pe_menu_row_step(pe_menu_row(e->row), direction);
}

static void pe_panel_event(const SDL_Event *ev) {
    if (ev->type == SDL_EVENT_QUIT ||
        (ev->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && s_window &&
         ev->window.windowID == SDL_GetWindowID(s_window))) {
        s_panel.open = 0;
        SDL_PushEvent((SDL_Event *)ev);
        return;
    }
    if (ev->type == SDL_EVENT_KEY_DOWN) {
        switch (ev->key.scancode) {
        case SDL_SCANCODE_F1: s_panel.open = 0; break;
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_BACKSPACE: if (!ev->key.repeat) pe_back(); break;
        case SDL_SCANCODE_UP: pe_move(-1); break;
        case SDL_SCANCODE_DOWN: pe_move(1); break;
        case SDL_SCANCODE_PAGEUP: pe_move(-VISIBLE_ROWS); break;
        case SDL_SCANCODE_PAGEDOWN: pe_move(VISIBLE_ROWS); break;
        case SDL_SCANCODE_LEFT: pe_step(-1); break;
        case SDL_SCANCODE_RIGHT: pe_step(1); break;
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_KP_ENTER:
        case SDL_SCANCODE_SPACE: if (!ev->key.repeat) pe_accept(); break;
        default: return;
        }
    } else if (ev->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        switch (ev->gbutton.button) {
        case SDL_GAMEPAD_BUTTON_DPAD_UP: pe_move(-1); break;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN: pe_move(1); break;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT: pe_step(-1); break;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: pe_step(1); break;
        case SDL_GAMEPAD_BUTTON_SOUTH: pe_accept(); break;
        case SDL_GAMEPAD_BUTTON_EAST: pe_back(); break;
        case SDL_GAMEPAD_BUTTON_GUIDE: s_panel.open = 0; break;
        default: return;
        }
    } else {
        return;
    }
    s_panel.dirty = 1;
}

static void pe_panel_run(void) {
    int audio_paused = Psyz_AudioIsPaused();
    if (!s_device) return; /* no in-window GPU overlay (fallback renderer): no panel yet */
    Psyz_PauseHostClock_SDL3(true);
    Psyz_AudioPause();
    s_panel.open = 1;
    s_panel.dirty = 1;
    s_panel.status = NULL;
    pe_build_entries();
    while (s_panel.open) {
        pe_refresh();
        Psyz_VideoVSync(0);
    }
    pe_menu_save_settings();
    SDL_ResetKeyboard(); /* a held key must not reach the resumed pad poll */
    Psyz_PauseHostClock_SDL3(false);
    if (!audio_paused) Psyz_AudioUnpause();
}

static bool pe_filter(const SDL_Event *ev) {
    if (s_panel.open) {
        if (ev->type == SDL_EVENT_WINDOW_RESIZED || ev->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED ||
            ev->type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
            s_panel.dirty = 1;
            return false;
        }
        pe_panel_event(ev);
        return true;
    }
    if ((ev->type == SDL_EVENT_KEY_DOWN && ev->key.scancode == SDL_SCANCODE_F1 && !ev->key.repeat) ||
        (ev->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && ev->gbutton.button == SDL_GAMEPAD_BUTTON_GUIDE)) {
        pe_panel_run();
        return true;
    }
    return s_prev_filter ? s_prev_filter(ev) : false;
}

void pe_panel_install(void) {
    s_prev_init = Psyz_OverlayInit_SDL3GPU(pe_init_gpu);
    s_prev_render = Psyz_OverlayRender_SDL3GPU(pe_render);
    s_prev_destroy = Psyz_OverlayDestroyCB(pe_destroy_gpu);
    s_prev_filter = Psyz_SetOverlayEventFilter_SDL3(pe_filter);
    pe_mod_log(NULL, "Mods panel on F1");
}
