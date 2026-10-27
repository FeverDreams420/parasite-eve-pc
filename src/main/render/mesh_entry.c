#include "pe1/geom_state.h"
#include "common.h"
u16 GetClut(int x, int y);
u16 GetTPage(int tp, int abr, int x, int y);
#include "pe1/render_prim.h"
#include <psyz/gpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

static uint64_t hd_geometry_key;
static const char *hd_background_path;

static uint64_t Render_HdHash(uint64_t hash, const void *data, size_t size) {
    const u8 *bytes=data;
    for (size_t i=0;i<size;++i) hash=(hash^bytes[i])*UINT64_C(1099511628211);
    return hash;
}

static int Render_HdCommand(const u_long *words, int available, void *unused) {
    (void)unused;
    if (available<3) return -1;
    Psyz_GpuSelectSpriteAtlas((uint64_t)(u32)words[1] | ((uint64_t)(u32)words[2]<<32),
                             (u32)words[0]&0xFFFFFF);
    return 3;
}

/* Hash immutable descriptors before scripts animate the native entries. The
 * exporter hashes the same little-endian fields, independent of host pointers. */
void Render_PrepareHdBackgrounds(void) {
    const uint64_t previous_key=hd_geometry_key;
    hd_background_path=getenv("PE_HD_BACKGROUNDS");
    hd_geometry_key=0;
    if (!hd_background_path || !*hd_background_path) {
        if (previous_key) Psyz_GpuResetSpriteAtlases();
        return;
    }
    Psyz_GpuRegisterCommandHandler(0xF8,Render_HdCommand,NULL);
    const GeomEntry *entries=(const void *)((const u8 *)g_GeomState+g_GeomState->entry_offset);
    uint64_t hash=UINT64_C(14695981039346656037);
    for (unsigned i=0;i<g_GeomState->entry_count06;++i) {
        const GeomEntry *entry=&entries[i];
        hash=Render_HdHash(hash,&entry->group,1);
        hash=Render_HdHash(hash,&entry->prim_count,2);
        hash=Render_HdHash(hash,&entry->base_x,4);
        hash=Render_HdHash(hash,(const u8 *)entry+entry->u28.pos_ptr,(size_t)entry->prim_count*4);
        hash=Render_HdHash(hash,(const u8 *)entry+entry->u2C.uv_ptr,(size_t)entry->prim_count*8);
    }
    if (previous_key!=hash) Psyz_GpuResetSpriteAtlases();
    hd_geometry_key=hash;
    fprintf(stderr,"HD background geometry %016" PRIx64 " (%u entries)\n",hash,(unsigned)g_GeomState->entry_count06);
}

/* A mesh entry seen as the address its self-relative array offsets add to. */
typedef union GeomEntryAddress {
    GeomEntry *entry;
    uintptr_t address;
} GeomEntryAddress;

/* Build both frame buffers of room-tile sprites and texture-page packets. */
int Render_LoadMeshEntry(GeomEntry *entry, RenderTilePacket *buffer, void **end)
{
    unsigned int paletteStart = g_GeomScroll.state.position.otZ;
    GeomEntryAddress base;
    unsigned int count;
    u32 *positions;
    RenderTileTexture *textures;
    unsigned int pass;
    RenderTexturePagePacket *pages;
    unsigned int shade;
    int paletteMax;
    RenderTexturePagePacket **pageSlot;
    u16 *dispX;

    base.entry = entry;
    count = entry->prim_count;
    positions = (u32 *)(base.address + entry->u28.pos_ptr);
    textures = (RenderTileTexture *)(base.address + entry->u2C.uv_ptr);
    pass = 0;
    pages = (RenderTexturePagePacket *)(buffer + count * 2);
    paletteStart += 479;
    shade = 128;
    paletteMax = paletteStart;

    uint64_t atlas_key=0;
    if (hd_geometry_key && hd_background_path) {
        const GeomEntry *entries=(const void *)((const u8 *)g_GeomState+g_GeomState->entry_offset);
        u32 index=(u32)(entry-entries);
        atlas_key=Render_HdHash(hd_geometry_key,&index,4);
        char filename[4096];
        int size=snprintf(filename,sizeof(filename),"%s/%016" PRIx64 ".bmp",hd_background_path,atlas_key);
        if (size<0 || (size_t)size>=sizeof(filename) || Psyz_GpuLoadSpriteAtlas(atlas_key,filename)!=1)
            atlas_key=0;
    }
    entry->u30.prim = buffer;
    /* The page pointer and disp_x are stored through pointers to the fields.
     * GCC 2.7 lets a load of a fixed scalar pass a structure field store, but
     * not an indirect store, so the g_GeomState reload stays after them as in
     * retail. */
    pageSlot = &entry->u34.pagePackets;
    *pageSlot = pages;
    dispX = &entry->disp_x;
    *dispX = entry->scr_x + g_GeomState->out_disp_x;
    entry->disp_y = entry->scr_y + g_GeomState->out_disp_y;

    do {
        unsigned int i = 0;
        /* Keep the relational guard: GCC's eliminated comparison leaves a
         * reload slot that is part of the retail 104-byte stack frame. */
        if (i < count) {
            RenderTileTexture *texture = textures;
            u32 *position = positions;
            RenderTexturePagePacket *page = pages;
            RenderTilePacket *sprite = buffer;
            do {
                sprite->length = 7;
                sprite->hd_command = atlas_key ? 0xF8000000u | i : 0;
                sprite->hd_key_low = atlas_key ? (u32)atlas_key : 0;
                sprite->hd_key_high = atlas_key ? (u32)(atlas_key>>32) : 0;
                sprite->packed_marker = 0x005A4D50u;
                sprite->code = 0x7c;
                sprite->r = shade;
                sprite->g = shade;
                sprite->b = shade;
                sprite->x = entry->disp_x + (*position >> 22);
                sprite->y = entry->disp_y + ((*position >> 12) & 1023);
                sprite->u = texture->bytes[4];
                sprite->v = texture->bytes[3];
                if (texture->words[1] & 0x10000000)
                    sprite->code |= 2;
                else
                    sprite->code &= ~2;
                sprite->clut = GetClut((texture->words[0] >> 5) & 0x3f0,
                                      texture->words[0] & 511);
                if (paletteMax < (int)(texture->words[0] & 511))
                    paletteMax = texture->words[0] & 511;
                page->length = 1;
                {
                    u32 word = texture->words[0];
                    page->command = (GetTPage(1, (word >> 22) & 3,
                                             (word >> 10) & 0x3c0,
                                             (word >> 7) & 256) & 0x9ff) | 0xe1000600;
                }
                texture++;
                position++;
                sprite++;
                page++;
            } while (++i < count);
        }
        buffer += count;
        pages += count;
    } while (++pass < 2);
    *end = pages;
    g_GeomScroll.state.position.otZ = paletteMax + 33;
    return 0;
}

/* The adjacent point command updates the same mesh-entry bounds and depth. */

int Render_TransformPoint(GeomEntry *entry, int x, int y, int depth) {
    GeomEntry *current;
    int screenX;
    int screenY;
    int minimum;
    int truncated;
    int sum;
    int bound;
    int comparison;
    unsigned int depthWord;
    unsigned int packed;
    unsigned int mask;

    current = entry;
    sum = current->base_x;
    screenY = current->base_y;
    depthWord = current->head.packed;
    sum += x;
    screenX = sum;
    /* Compare the wrapped signed coordinate while retaining the original
     * sum when it falls within the bounds. */
    depthWord >>= 20;
    depth += depthWord;

    bound = current->bounds.minX;
    minimum = bound;
    screenY += y;
    truncated = (short)sum;
    comparison = truncated < bound;
    if (!comparison) {
        bound = current->bounds.maxX;
        minimum = bound;
        comparison = bound < truncated;
    }
    screenX = comparison ? minimum : screenX;
    sum = (unsigned int)screenY << 16;
    bound = current->bounds.minY;
    truncated = sum >> 16;
    minimum = bound;
    comparison = truncated < bound;
    if (!comparison) {
        bound = current->bounds.maxY;
        minimum = bound;
        comparison = bound < truncated;
    }
    screenY = comparison ? minimum : screenY;
    current->scr_x = screenX;
    current->scr_y = screenY;
    mask = 0xFFF000FF;
    packed = current->head.packed & mask;
    /* Clamping is complete; reuse the scratch for the packed depth. */
    bound = depth;
    packed |= (bound & 0xFFF) << 8;
    current->head.packed = packed;
    return 0;
}
