/*
 * Viewer test: F1 > Debug > Viewer test. A throwaway test for the Bestiary's model viewer
 * (bridge findings pe-bestiary-viewer-look-options-2026-10-26.md, sections 8.5 and 9). Fork only.
 *
 *   - Test card: a 128 x 128 card shown through the image presenter (host/pe_present.c), as a box
 *     or as strips, at a chosen place. A 1-pixel border, colour bars, gradients, a 1-pixel
 *     checkerboard, red corner squares and a see-through hole and corners, so position, scaling,
 *     filtering and transparency show at a glance. Open Aya's menu to see it over the game's own.
 *   - Dump VRAM: the native VRAM copy to pe_vram_dump_mNNN_K.bmp beside the preferences (uploaded
 *     textures and palettes are exact; at internal resolution > 1 anything the GPU drew -- the
 *     framebuffers, the menu's text cache at x 320 y 0 -- is stale there).
 *   - Info rows: the buffer pasted into, video settings, the paste cost.
 *   - Creature: any Bestiary creature's model (form, silhouette) through the model view and the
 *     presenter, turning slowly with its idle animation, at the card's place.
 *   - PE_VIEWER_FORMS=<file.bmp> (timing / check runs): every creature's every form drawn into one
 *     contact sheet at the first frame drawn (the title screens too), then nothing else changes.
 * (A pink test of VRAM pages was removed after its run: restoring a page from a read-back corrupted
 * the menu's text cache at 2x, since PsyZ keeps GPU-drawn texture areas only in its upscaled copy.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psyz/video.h>

#include "pe_game.h"
#include "pe_image.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_model.h"
#include "pe_model_view.h"
#include "pe_pages.h"
#include "pe_present.h"
#include "pe_services.h"
#include "pe1/display_env.h"
#include "pe1/draw_buffers.h"
#include "pe1/psyq_gpu.h"

#define PE_CARD        128
#define PE_SLOT_TEST   0
#define PE_SLOT_MODEL  1
#define PE_TURN_STEP   12           /* model turn a frame, 4096 a full turn */
#define PE_SILHOUETTE  0x101018u
#define PE_BOX_RGB     0x182038u    /* a dark viewer-screen blue */

static int s_on, s_method, s_x = 176, s_y = 48;
static int s_shown_method = -1, s_shown_x = -1, s_shown_y = -1;
static uint8_t s_card_rgba[PE_CARD * PE_CARD * 4];
static int s_card_ready, s_logged, s_dumps;
static const char *const kMethods[] = {"Box", "Strips"};

/* ---- the test card ---- */
static void pe_card_pixel(int x, int y, uint8_t *p) {
    static const uint8_t bars[8][3] = {{255,255,255},{255,255,0},{0,255,255},{0,255,0},
                                       {255,0,255},{255,0,0},{0,0,255},{0,0,0}};
    const int c = PE_CARD / 2, dx = x - c, dy = y - c, r = 14;
    int cx = x < r ? r : x > PE_CARD - 1 - r ? PE_CARD - 1 - r : x;
    int cy = y < r ? r : y > PE_CARD - 1 - r ? PE_CARD - 1 - r : y;
    p[3] = 255;
    if ((x - cx) * (x - cx) + (y - cy) * (y - cy) > r * r || dx * dx + dy * dy < 22 * 22) {
        p[0] = p[1] = p[2] = p[3] = 0;                                 /* see-through */
        return;
    }
    if (x == 0 || y == 0 || x == PE_CARD - 1 || y == PE_CARD - 1 ||
        (x - cx) * (x - cx) + (y - cy) * (y - cy) > (r - 1) * (r - 1)) {
        p[0] = p[1] = p[2] = 255;                                       /* border */
    } else if ((x < 20 || x >= PE_CARD - 20) && (y < 20 || y >= PE_CARD - 20) &&
               (x >= 14 && x < PE_CARD - 14) && (y >= 14 && y < PE_CARD - 14)) {
        p[0] = 255; p[1] = p[2] = 0;                                    /* corner squares */
    } else if (y < 44) {
        memcpy(p, bars[x * 8 / PE_CARD], 3);                            /* colour bars */
    } else if (y < 88) {
        if (x < c) { p[0] = (uint8_t)(x * 255 / (c - 1)); p[1] = (uint8_t)((y - 44) * 255 / 43); p[2] = 64; }
        else p[0] = p[1] = p[2] = (uint8_t)(((x ^ y) & 1) ? 255 : 0);   /* 1-pixel checkerboard */
    } else {
        p[0] = p[1] = p[2] = (uint8_t)((x / 4) * 255 / 31);             /* 32-step grey ramp */
    }
}

static void pe_card_build(void) {
    int x, y;
    if (s_card_ready) return;
    for (y = 0; y < PE_CARD; ++y)
        for (x = 0; x < PE_CARD; ++x) pe_card_pixel(x, y, s_card_rgba + 4 * (y * PE_CARD + x));
    s_card_ready = 1;
}

static const char *pe_aspect_name(void) {
    return Psyz_VideoGetAspectMode() == PSYZ_ASPECT_SQUARE ? "Square" : "Display";
}

static void pe_log_setup(void) {
    const RECT *a = &g_RenderDrawEnvArray[0].clip, *b = &g_RenderDrawEnvArray[1].clip;
    pe_log("viewer", "draw0=%d,%d %dx%d draw1=%d,%d %dx%d slot=%d res=%ux aa=%d geom=%d filter=%d aspect=%s",
           a->x, a->y, a->w, a->h, b->x, b->y, b->w, b->h, g_ActiveDrawSlot, Psyz_VideoGetInternalResolution(),
           Psyz_VideoGetAntialiasing(), Psyz_VideoGetGeometryCorrection(), Psyz_VideoGetModelFiltering(),
           pe_aspect_name());
}

static void pe_card_tick(void) {
    PeImage card = {PE_CARD, PE_CARD, s_card_rgba};
    if (!s_on) {
        if (s_shown_method >= 0) { pe_present_hide(PE_SLOT_TEST); s_shown_method = -1; }
        return;
    }
    if (s_method == s_shown_method && s_x == s_shown_x && s_y == s_shown_y) return;
    pe_card_build();
    if (pe_present_show(PE_SLOT_TEST, s_x, s_y, &card, s_method ? PE_PRESENT_STRIPS : PE_PRESENT_BOX, PE_BOX_RGB) == 0) {
        s_shown_method = s_method;
        s_shown_x = s_x;
        s_shown_y = s_y;
        pe_log("viewer", "test card %s at %d,%d", kMethods[s_method], s_x, s_y);
        if (!s_logged) { pe_log_setup(); s_logged = 1; }
    }
}

/* ---- VRAM dump ---- */
static void pe_put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

static void pe_dump_vram(void) {
    enum { W = 1024, H = 512 };
    char name[64], path[1200], msg[96];
    RECT all = {0, 0, W, H};
    uint16_t *vram = malloc((size_t)W * H * 2);
    uint8_t *row = malloc((size_t)W * 3), header[54] = {'B', 'M'};
    FILE *f = NULL;
    int x, y;
    snprintf(name, sizeof name, "pe_vram_dump_m%03d_%d.bmp", pe_room_number(), ++s_dumps);
    if (!vram || !row || pe_mod_path(path, sizeof path, name) != 0 || !(f = fopen(path, "wb"))) {
        free(vram); free(row); pe_toast("VRAM dump failed"); return;
    }
    StoreImage(&all, vram);
    pe_put32(header + 2, 54 + W * H * 3); pe_put32(header + 10, 54); pe_put32(header + 14, 40);
    pe_put32(header + 18, W); pe_put32(header + 22, H); header[26] = 1; header[28] = 24;
    pe_put32(header + 34, W * H * 3);
    fwrite(header, 1, sizeof header, f);
    for (y = H - 1; y >= 0; --y) {                         /* BMP rows bottom up, BGR */
        for (x = 0; x < W; ++x) {
            const uint16_t c = vram[y * W + x];
            row[3 * x + 0] = (uint8_t)(((c >> 10) & 31) << 3);
            row[3 * x + 1] = (uint8_t)(((c >> 5) & 31) << 3);
            row[3 * x + 2] = (uint8_t)((c & 31) << 3);
        }
        fwrite(row, 1, (size_t)W * 3, f);
    }
    fclose(f);
    free(vram);
    free(row);
    pe_log("viewer", "VRAM dumped to %s", name);
    snprintf(msg, sizeof msg, "VRAM dumped to %s", name);
    pe_toast(msg);
}

/* ---- creature models ---- */
static int s_model_on, s_model_pick, s_model_form, s_model_silhouette;
static int s_shown_pick = -1, s_shown_form = -1, s_shown_silhouette = -1;
static const PeModel *s_model;
static PeViewFit s_fit;
static int s_yaw, s_frame_tick;
static uint8_t s_model_rgba[PE_CARD * PE_CARD * 4];
static char s_model_status[96] = "-";
static char s_creature_names[512][40];

static const PeBestiaryApi *pe_bestiary(void) { return (const PeBestiaryApi *)pe_service_get("bestiary", 1); }

static unsigned pe_creature_count(void) {
    const PeBestiaryApi *b = pe_bestiary();
    unsigned n = 0;
    while (b && n < 512 && b->info((int)n)) ++n;
    return n;
}

static const char *pe_creature_name(unsigned i) {
    const PeBestiaryApi *b = pe_bestiary();
    const PeCreatureInfo *c = b && i < 512 ? b->info((int)i) : NULL;
    if (!c) return "";
    snprintf(s_creature_names[i], sizeof s_creature_names[i], "%s%s", c->name, c->ex ? " (EX)" : "");
    return s_creature_names[i];
}

static void pe_model_tick(void) {
    const PeBestiaryApi *b = pe_bestiary();
    PeImage img = {PE_CARD, PE_CARD, s_model_rgba};
    PeView view = {PE_CARD, PE_CARD, 0, 0, 0, PE_SILHOUETTE};
    if (!s_model_on || !b) {
        if (s_shown_pick >= 0) { pe_present_hide(PE_SLOT_MODEL); s_shown_pick = -1; s_model = NULL; }
        return;
    }
    if (s_model_pick != s_shown_pick || s_model_form != s_shown_form || s_model_silhouette != s_shown_silhouette) {
        PeModelSource src;
        s_shown_pick = s_model_pick;
        s_shown_form = s_model_form;
        s_shown_silhouette = s_model_silhouette;
        s_yaw = 0;
        s_frame_tick = 0;
        s_model = b->model(s_model_pick, s_model_form, &src) ? pe_model_load(&src) : NULL;
        if (!s_model || !pe_model_fit(s_model, &s_fit)) {
            s_model = NULL;
            snprintf(s_model_status, sizeof s_model_status, "%s: no form %d", pe_creature_name((unsigned)s_model_pick), s_model_form);
            pe_present_hide(PE_SLOT_MODEL);
            return;
        }
        snprintf(s_model_status, sizeof s_model_status, "M%03d slot %d: %d parts, %d faces, %d frames",
                 src.map, src.actor, s_model->parts, s_model->faces, pe_model_frames(s_model));
        pe_log("viewer", "creature %s form %d: %s", pe_creature_name((unsigned)s_model_pick), s_model_form, s_model_status);
    }
    if (!s_model) return;
    view.yaw = s_yaw;
    view.frame = s_frame_tick / 2;               /* the game animates at 30 frames a second */
    view.silhouette = s_model_silhouette;
    s_yaw = (s_yaw + PE_TURN_STEP) & 4095;
    ++s_frame_tick;
    if (pe_model_render(s_model, &s_fit, &view, &img))
        pe_present_show(PE_SLOT_MODEL, s_x, s_y, &img, PE_PRESENT_BOX, PE_BOX_RGB);
}

static void pe_model_info(char *out, unsigned size) { snprintf(out, size, "%s", s_model_status); }

/* PE_VIEWER_FORMS=<file.bmp>: every form of every creature into one contact sheet, once. */
static int pe_on_scene(void *ctx, void *user) {
    enum { CELL = 96, COLS = 12 };
    static int done;
    const char *path = getenv("PE_VIEWER_FORMS");
    const PeBestiaryApi *b = pe_bestiary();
    static uint8_t cell[CELL * CELL * 4];
    uint8_t *sheet, header[54] = {'B', 'M'};
    int h, f, n = 0, ok = 0, rows, x, y;
    FILE *out;
    (void)ctx;
    (void)user;
    if (done || !path || !b) return 0;
    done = 1;
    for (h = 0; b->info(h); ++h) for (f = 0; f < PE_BESTIARY_MAX_FORMS; ++f) n += b->model(h, f, NULL);
    rows = (n + COLS - 1) / COLS;
    if (!(sheet = calloc((size_t)COLS * CELL * rows * CELL, 3))) return 0;
    for (h = 0, n = 0; b->info(h); ++h)
        for (f = 0; f < PE_BESTIARY_MAX_FORMS; ++f) {
            PeModelSource src;
            PeImage img = {CELL, CELL, cell};
            PeView view = {CELL, CELL, 0, 0, 0, 0};
            const PeModel *m;
            if (!b->model(h, f, &src)) continue;
            m = pe_model_load(&src);
            if (m && pe_model_fit(m, &s_fit) && pe_model_render(m, &s_fit, &view, &img)) {
                ++ok;
                for (y = 0; y < CELL; ++y)
                    for (x = 0; x < CELL; ++x) {
                        const uint8_t *p = cell + 4 * (y * CELL + x);
                        uint8_t *q = sheet + 3 * (((size_t)(n / COLS) * CELL + y) * COLS * CELL + (size_t)(n % COLS) * CELL + x);
                        q[0] = p[3] ? p[0] : 40; q[1] = p[3] ? p[1] : 40; q[2] = p[3] ? p[2] : 48;
                    }
            }
            pe_log("viewer", "form %s %d: %s", pe_creature_name((unsigned)h), f, m ? "drawn" : "FAILED");
            ++n;
        }
    if ((out = fopen(path, "wb"))) {                      /* 24-bit BMP, rows bottom up, BGR */
        const int w = COLS * CELL, hh = rows * CELL;
        pe_put32(header + 2, (uint32_t)(54 + w * hh * 3)); pe_put32(header + 10, 54); pe_put32(header + 14, 40);
        pe_put32(header + 18, (uint32_t)w); pe_put32(header + 22, (uint32_t)hh); header[26] = 1; header[28] = 24;
        fwrite(header, 1, sizeof header, out);
        for (y = hh - 1; y >= 0; --y)
            for (x = 0; x < w; ++x) { const uint8_t *q = sheet + 3 * ((size_t)y * w + x); const uint8_t bgr[3] = {q[2], q[1], q[0]}; fwrite(bgr, 1, 3, out); }
        fclose(out);
    }
    free(sheet);
    pe_log("viewer", "forms sheet: %d of %d forms drawn to %s", ok, n, path);
    return 0;
}

/* ---- info rows ---- */
static void pe_frame_info(char *out, unsigned size) {
    PePresentStats st;
    pe_present_stats(&st);
    if (!st.frame_w) { snprintf(out, size, "-"); return; }
    snprintf(out, size, "slot %d: %dx%d at %d,%d", st.draw_slot, st.frame_w, st.frame_h, st.frame_x, st.frame_y);
}

static void pe_video_info(char *out, unsigned size) {
    snprintf(out, size, "%ux, AA %s, geometry %s, filter %s, %s", Psyz_VideoGetInternalResolution(),
             Psyz_VideoGetAntialiasing() ? "on" : "off", Psyz_VideoGetGeometryCorrection() ? "on" : "off",
             Psyz_VideoGetModelFiltering() ? "on" : "off", pe_aspect_name());
}

static void pe_cost_info(char *out, unsigned size) {
    PePresentStats st;
    pe_present_stats(&st);
    snprintf(out, size, "%.1f us a frame, %u pastes", st.avg_us, st.pastes);
}

static void pe_tick(void) {
    static int started;
    if (!started) {                            /* a saved row: the test always starts off ... */
        const char *env = getenv("PE_VIEWER_TEST");   /* ... unless a timing run asks: box / strips */
        started = 1;
        s_model_on = 0;
        s_on = env && (!strcmp(env, "box") || !strcmp(env, "strips"));
        if (s_on) s_method = !strcmp(env, "strips");
    }
    pe_card_tick();
    pe_model_tick();
}

void pe_viewer_test_register(const char *mod_id) {
    const char *page = PE_PAGE_DEBUG_VIEWER;
    (void)mod_id;
    pe_menu_register_toggle(page, "Test card", "Show a 128 x 128 test card through the image presenter. Off by default.", &s_on, NULL);
    pe_menu_register_choice(page, "Method", "Box: one opaque paste. Strips: only the card's pixels, row by row.", &s_method, kMethods, 2);
    pe_menu_register_int(page, "X", "Card's left edge on the 320 x 224 game screen.", &s_x, 0, 320 - PE_CARD, 4);
    pe_menu_register_int(page, "Y", "Card's top edge on the 320 x 224 game screen.", &s_y, 0, 224 - PE_CARD, 4);
    pe_menu_register_info(page, "Frame", "The buffer the card was pasted into last.", pe_frame_info);
    pe_menu_register_info(page, "Video", "Internal resolution and filters (change them in Escape > Graphics).", pe_video_info);
    pe_menu_register_info(page, "Paste cost", "Average time of a frame's pastes, and how many.", pe_cost_info);
    pe_menu_register_action(page, "Dump VRAM", "Writes pe_vram_dump_mNNN_K.bmp beside the preferences.", pe_dump_vram);
    pe_menu_register_toggle(page, "Creature", "Show a Bestiary creature's model, turning, at the card's place. Off by default.", &s_model_on, NULL);
    pe_menu_register_dynamic_choice(page, "Which creature", "Left / right picks the creature.", &s_model_pick, 512, pe_creature_count, pe_creature_name);
    pe_menu_register_int(page, "Form", "Its body (0), then parts or phases.", &s_model_form, 0, PE_BESTIARY_MAX_FORMS - 1, 1);
    pe_menu_register_toggle(page, "Silhouette", "Draw the shape only, as an unmet creature.", &s_model_silhouette, NULL);
    pe_menu_register_info(page, "Model", "The model shown: its room, slot and size.", pe_model_info);
    pe_menu_register_tick(pe_tick);
    pe_hook_add(PE_HOOK_FRAME_DRAWN, pe_on_scene, NULL, PE_PRIORITY_LATE, mod_id);   /* runs on the title screens too */
    if (getenv("PE_VIEWER_TEST")) {            /* timing runs: show it from the first frame */
        PeImage card = {PE_CARD, PE_CARD, s_card_rgba};
        pe_card_build();
        pe_present_show(PE_SLOT_TEST, s_x, s_y, &card,
                        !strcmp(getenv("PE_VIEWER_TEST"), "strips") ? PE_PRESENT_STRIPS : PE_PRESENT_BOX, PE_BOX_RGB);
    }
}
