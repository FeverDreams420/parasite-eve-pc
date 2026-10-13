#include "pe1/geom_state.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_prim.h"

/* A mesh entry seen as the address its self-relative array offsets add to. */
typedef union GeomEntryAddress {
    GeomEntry *entry;
    u32 address;
} GeomEntryAddress;

/* Build both frame buffers of room-tile sprites and texture-page packets. */
int Render_LoadMeshEntry(GeomEntry *entry, RenderTilePacket *buffer, void **end)
{
    unsigned int paletteStart = D_800BD024;
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
                sprite->length = 3;
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
    D_800BD024 = paletteMax + 33;
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
    int stackFrame[8]; /* Preserves the original 0x20-byte frame. */

    current = entry;
    sum = current->base_x;
    screenY = current->base_y;
    depthWord = current->head.packed;
    sum += x;
    screenX = sum;
    /* Keeps the unclamped x copy apart from the sum the clamp truncates;
     * the pins reproduce retail moving the entry to $t0 and reusing $a0
     * for y, which local allocation otherwise swaps. */
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
