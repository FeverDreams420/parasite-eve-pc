/* Geometry sprite entries and mesh lists, the screen fade and tint, the
 * background scroll/parallax step and the room shadow and primitive colour
 * draws (G0833/G0835). */

#include "common.h"
#include "pe1/field_tile.h"
#include "pe1/fieldfx_task.h"
#include "pe1/game_state.h"
#include "pe1/geom_state.h"
#include "pe1/gte.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_nop.h"
#include "pe1/render_buffers.h"
#include "pe1/render_camera.h"
#include "pe1/render_prim.h"
#include "pe1/render_shadow.h"

/* Matching debt: fixed-pins and scheduling constraints preserve the stock PSYQ instruction schedule. */

/* A packet or ordering-table slot address as the 24-bit link word. */
typedef union RenderLinkAddress {
    void *pointer;
    u8 *bytes;
    u32 *words;
    GeomEntry *entry;
    u32 word;
} RenderLinkAddress;

#define LINK_PACKET_PRELOADED(packet, ordering, packet_tag, mask24, maskTop) \
    do {                                                                    \
        u32 ot_tag;                                      \
        RenderLinkAddress link;                                             \
        ot_tag = *(u32 *)(ordering);                                        \
        *(u32 *)(packet) = ((packet_tag) & (maskTop)) | (ot_tag & (mask24)); \
        ot_tag = *(u32 *)(ordering);                                        \
        link.pointer = (packet);                                            \
        *(u32 *)(ordering) = (ot_tag & (maskTop)) | (link.word & (mask24)); \
    } while (0)

#define LINK_PAGE(packet, ordering, mask24, maskTop)                         \
    do {                                                                     \
        u32 packet_tag = *(u32 *)(packet);               \
        u32 ot_tag;                                      \
        RenderLinkAddress link;                                              \
        ot_tag = *(u32 *)(ordering);                                         \
        ot_tag &= (mask24);                                                   \
        packet_tag &= (maskTop);                                              \
        packet_tag |= ot_tag;                                                 \
        *(u32 *)(packet) = packet_tag;                                        \
        ot_tag = *(u32 *)(ordering);                                         \
        link.pointer = (packet);                                             \
        *(u32 *)(ordering) = (ot_tag & (maskTop)) | (link.word & (mask24)); \
    } while (0)

int Render_DrawSpriteEntry(GeomEntry *input)
{
    u32 *position_cursor;
    s32 workValue;
    GeomEntry *entry;
    s32 active;
    RenderTilePacket *sprite;
    RenderTexturePagePacket *page;
    RenderLinkAddress ordering;
    GeomState *state;
    u32 flags;
    u32 count;
    u32 i;
    s32 scroll_x;
    s32 scroll_y;
    s32 texture_base;
    u32 *pos;
    s32 raw_x;
    s32 raw_y;
    s32 screen_src;
    char frame_pad[9];

    active = g_ActiveDrawSlot;
    entry = input;
    sprite = entry->u30.prim;
    count = entry->prim_count;
    ordering.pointer = D_800B0E38.ordering[active];
    if (active != 0) {
        sprite += count;
    }
    page = entry->u34.pagePackets;
    if (active != 0)
        page += count;

    state = g_GeomState;
    {
        s32 screen_add;
        screen_src = entry->scr_x;
        screen_add = (u16)state->out_disp_x;
        raw_x = screen_src + screen_add;
        screen_src = entry->scr_y;
        screen_add = (u16)state->out_disp_y;
        scroll_x = raw_x;
        raw_y = screen_src + screen_add;
        scroll_y = raw_y;
    }
    {
        workValue = *(u32 *)entry >> 8;
        workValue &= 0xFFF;
        screen_src = state->field26;
        texture_base = screen_src + workValue;
    }
    screen_src = entry->u28.pos_ptr;
    flags = entry->head.b.flags;
    {
        /* The position words lie pos_ptr bytes past the entry. */
        RenderLinkAddress positions;

        positions.entry = entry;
        positions.word += screen_src;
        pos = positions.words;
    }

    if (flags & 4) {
        s32 mod_x;
        s32 mod_y;
        s32 divisor_x;
        s32 remainder_y;
        s32 signed_x;
        s32 base_y;
        signed_x = (s16)raw_x;
        mod_x = entry->anim_mod_x;
        signed_x -= 320;
        divisor_x = mod_x & 0xFFFF;
        signed_x += divisor_x;
        scroll_x = signed_x % divisor_x;
        mod_y = entry->anim_mod_y;
        workValue = (s16)raw_y;
        workValue -= 224;
        workValue += mod_y & 0xFFFF;
        remainder_y = workValue % (mod_y & 0xFFFF);
        i = 0;
        raw_x = mod_x - 320;
        scroll_x -= raw_x;
        base_y = mod_y;
        base_y -= 224;
        scroll_y = remainder_y - base_y;
        if (count != 0) {
            u32 mask24 = 0x00FFFFFF;
            u32 maskTop = 0xFF000000;
            RenderTexturePagePacket *page_cursor = page;
            RenderTilePacket *sprite_cursor = sprite;
            position_cursor = pos;
                do {
                s32 draw_x;
                {
                    u32 word_x = *position_cursor;
                    s32 x = scroll_x + (word_x >> 22);
                    draw_x = x;
                    if ((s16)x >= 320) {
                        s32 modulus_x = entry->anim_mod_x;
                        draw_x = x - modulus_x;
                    } else if ((s16)x < -15) {
                        s32 modulus_x = entry->anim_mod_x;
                        draw_x = modulus_x + x;
                    }
                }
                if ((u16)(draw_x + 15) < 0x14F) {
                        s32 draw_y;
                        {
                            u32 word_y = *position_cursor;
                            s32 y = scroll_y + ((word_y >> 12) & 0x3FF);
                            draw_y = y;
                            if ((s16)y >= 224) {
                                workValue = entry->anim_mod_y;
                                draw_y = y - workValue;
                            } else if ((s16)y < -15) {
                                workValue = entry->anim_mod_y;
                                draw_y = workValue + y;
                            }
                        }
                        if ((u16)(draw_y + 15) < 0xEF) {
                            s32 tile_u = texture_base + (*(u16 *)position_cursor & 0xFFF);
                            if (((tile_u - 8) & 0xFFFFU) < 0xFF1) {
                                u32 sprite_tag;
                                s32 ot_index = tile_u;
                                RenderLinkAddress ot;
                                ot_index <<= 16;
                                ot_index >>= 14;
                                ot.word = ot_index + ordering.word;
                                sprite_tag = *(u32 *)sprite_cursor;
                                (sprite_cursor)->x = draw_x;
                                (sprite_cursor)->y = draw_y;
                                LINK_PACKET_PRELOADED(sprite_cursor, ot.bytes, sprite_tag, mask24, maskTop);
                                LINK_PAGE(page_cursor, ot.bytes, mask24, maskTop);
                            }
                        }
                }
                page_cursor += 1;
                sprite_cursor += 1;
                ++i;
                ++position_cursor;
            } while (i < count);
        }
    } else {
        u32 mask24;
        u32 maskTop;
        RenderTilePacket *sprite_cursor;
        RenderTexturePagePacket *page_cursor;
        i = 0;
        if (count != 0) {
            mask24 = 0x00FFFFFF;
            maskTop = 0xFF000000;
            sprite_cursor = sprite;
            page_cursor = page;
            position_cursor = pos;
            do {
            u32 word = *position_cursor;
            s32 draw_x;
            workValue = scroll_x + (word >> 22);
            draw_x = workValue;
            if ((u16)(workValue + 15) < 0x14F) {
                    {
                    s32 y;
                    s32 draw_y;
                    s32 tile_u;
                    y = scroll_y + ((word >> 12) & 0x3FF);
                    draw_y = y;
                    if ((u16)(y + 15) < 0xEF) {
                        tile_u = texture_base + (*(u16 *)position_cursor & 0xFFF);
                        if (((tile_u - 8) & 0xFFFFU) < 0xFF1) {
                            u32 sprite_tag;
                            s32 ot_index = tile_u;
                            RenderLinkAddress ot;
                            ot_index <<= 16;
                            ot_index >>= 14;
                            ot.word = ot_index + ordering.word;
                            sprite_tag = *(u32 *)sprite_cursor;
                            (sprite_cursor)->x = draw_x;
                            (sprite_cursor)->y = draw_y;
                            LINK_PACKET_PRELOADED(sprite_cursor, ot.bytes, sprite_tag, mask24, maskTop);
                            LINK_PAGE(page_cursor, ot.bytes, mask24, maskTop);
                        }
                    }
                    }
                }
            page_cursor += 1;
            sprite_cursor += 1;
            ++i;
            ++position_cursor;
            } while (i < count);
        }
    }

    entry->disp_x = scroll_x;
    entry->disp_y = scroll_y;
    return 0;
}


int Render_SetEntryVisible(int index, int enabled) {
    GeomStateAddress table, base;
    GeomEntry *entry = (GEOM_STATE_OFFSET(table, base, entry_offset, 0), table.entry) + index;

    if (enabled != 0) {
        entry->head.b.flags |= 2;
    } else {
        entry->head.b.flags &= 0xFD;
    }
    return 0;
}

int Render_SetEntryScrolled(int index, int enabled, unsigned int arg2, unsigned int arg3) {
    GeomStateAddress table, base;
    GeomEntry *entry = (GEOM_STATE_OFFSET(table, base, entry_offset, 0), table.entry) + index;

    if (enabled != 0) {
        entry->head.b.flags |= 4;
    } else {
        entry->head.b.flags &= 0xFB;
    }
    entry->speedX = arg2 >> 8;
    entry->speedY = arg3 >> 8;
    return 0;
}

int Render_SetEntryMirrored(int index, int enabled, unsigned int arg2, unsigned int arg3) {
    GeomStateAddress table, base;
    GeomEntry *entry = (GEOM_STATE_OFFSET(table, base, entry_offset, 0), table.entry) + index;

    if (enabled != 0) {
        entry->head.b.flags |= 8;
    } else {
        entry->head.b.flags &= 0xF7;
    }
    entry->speedX = (0x10000 - arg2) >> 8;
    entry->speedY = (0x10000 - arg3) >> 8;
    return 0;
}

int Render_SetEntryPosition(int index, int x, int y) {
    GeomStateAddress table, base;
    GeomEntry *entry = (GEOM_STATE_OFFSET(table, base, entry_offset, 0), table.entry) + index;
    int *flags = &g_GeomScroll.state.flags;

    entry->scr_x = x;
    entry->base_x = x;
    entry->scr_y = y;
    entry->base_y = y;
    *flags |= 0x80;
    return 0;
}

int Render_LoadGeomState(int);
/* Loads the selected group's viewport, centres the camera on it and builds
 * every mesh entry's packets into `buffer`; *end returns the free space. */
int Render_DrawMeshList(void *buffer, void **end)
{
    GeomState *state = g_GeomState;
    u8 *group = &g_GeomScroll.state.position.groupSelect;
    GeomStateAddress address;
    CameraViewport *view;
    GeomEntry *entry;
    unsigned int count;
    unsigned int i;
    int x;

    Render_LoadGeomState(*group);
    address.state = state;
    address.word += state->entry_offset_1C;
    view = &address.viewport[*group];
    x = (view->minX + view->maxX) / 2;
    g_GeomScroll.state.position.x = g_GeomScroll.state.position.originX = x;
    g_GeomScroll.state.position.y = g_GeomScroll.state.position.originY =
        (view->minY + view->maxY) / 2;
    g_GeomScroll.state.position.boundsMinX = view->minX;
    g_GeomScroll.state.position.boundsMaxX = view->maxX;
    i = 0;
    g_GeomScroll.state.position.boundsMinY = view->minY;
    g_GeomScroll.state.position.boundsMaxY = view->maxY;
    x -= 160;
    count = state->entry_count06;
    state->out_disp_x = state->disp_src_x - x;
    state->out_disp_y = state->disp_src_y - ((u16)g_GeomScroll.state.position.y - 112);
    address.state = state;
    address.word += state->entry_offset;
    *end = buffer;
    if (i < count) {
        int min = -32768, max = 32767;

        entry = address.entry;
        do {
            if (Render_LoadMeshEntry(entry, *end, end))
                return -18;
            entry->bounds.minX = min;
            entry->bounds.maxX = max;
            entry->bounds.minY = min;
            entry->bounds.maxY = max;
            entry++;
        } while (++i < count);
    }
    {
        int *flags = &g_GeomScroll.state.flags;

        g_RenderFadeFrameCount = g_RenderFadeFrameCounter = 0;
        *flags &= ~0xC00;
    }
    return 0;
}

/* Field geometry: clamp the display-source point into the clip window and
 * draw the selected group's sprite entries relative to the camera.
 * Contiguous default-profile pair on g_GeomState (main_tu_evidence G0836
 * joins them with the mesh renderer before and the fades after). */

/* Offsets a point by the display origin and clamps it into the clip
 * window; the result becomes the display source point and depth. */
int Render_ClipPoint(int x, int y, int z) {
    GeomState *state = g_GeomState;
    u16 clippedX = state->clip_offset_x + x;
    u16 clippedY = state->clip_offset_y + y;
    int depth = state->depth_offset + z;

    if ((s16)clippedX < state->clip_min_x)
        clippedX = state->clip_min_x;
    else if (state->clip_max_x < (s16)clippedX)
        clippedX = state->clip_max_x;
    if ((s16)clippedY < state->clip_min_y)
        clippedY = state->clip_min_y;
    else if (state->clip_max_y < (s16)clippedY)
        clippedY = state->clip_max_y;
    state->disp_src_x = clippedX;
    state->disp_src_y = clippedY;
    state->field26 = depth;
    return 0;
}

/* Re-derives the display offsets from the camera and draws the sprite
 * entries of the selected group. */
s32 Render_BuildMeshList(void) {
    GeomStateAddress base;
    GeomEntry *entry;
    s32 count;
    s32 i;

    base.state = g_GeomState;
    base.state->out_disp_x = base.state->disp_src_x - ((u16)g_GeomScroll.state.position.x - 160);
    count = base.state->entry_count06;
    base.state->out_disp_y = base.state->disp_src_y - ((u16)g_GeomScroll.state.position.y - 112);
    base.word += base.state->entry_offset;
    i = 0;
    if (i < count) {
        entry = base.entry;
        do {
            if ((entry->head.b.flags & 2) && entry->group == g_GeomScroll.state.position.groupSelect)
                Render_DrawSpriteEntry(entry);
            i++;
            entry++;
        } while (i < count);
    }
    return 0;
}

int Render_StartFadeIn(int arg0)
{
  int new_var;
  int *flags;
  int value;
  flags = &g_GeomScroll.state.flags;
  g_RenderFadeFrameCount = arg0;
  g_RenderFadeFrameCounter = 0;
  new_var = *flags;
  value = new_var;
  value &= ~0xC00;
  value |= 0x400;
  *flags = value;
  return 0;
}

/* Steps the screen fade: every sprite primitive of every entry gets the
 * current grey level; when the fade has run its frames the fade-in mode is
 * switched off and the fade-out flag set. */
int Render_StepFade(void) {
    GeomState *state;
    GeomStateAddress entries;
    GeomEntry *entry;
    u8 *counter;
    int level;
    int count;
    int i;

    if (!(g_GeomScroll.state.flags & 0x400))
        return 0;
    level = (g_RenderFadeFrameCounter << 7) / (g_RenderFadeFrameCount - 1);
    state = g_GeomState;
    entries.state = g_GeomState;
    entries.word += state->entry_offset;
    count = state->entry_count06;
    level = 0x80 - level;
    entry = entries.entry;
    for (i = 0; i < count; i++, entry++) {
        PrimEntry *prim = entry->u30.prim;
        u32 primCount = entry->prim_count;
        u8 grey = level;
        u32 j;

        if (g_ActiveDrawSlot != 0)
            prim += primCount;
        for (j = 0; j < primCount; j++) {
            prim->b = grey;
            prim->g = grey;
            prim->r = grey;
            prim++;
        }
    }
    counter = &g_RenderFadeFrameCounter;
    if (++*counter >= g_RenderFadeFrameCount) {
        g_GeomScroll.state.flags = (g_GeomScroll.state.flags & ~0xC00) | 0x800;
        g_GeomState->field26 = 0x1FF0;
    }
    return 0;
}

/* Starts the scene-load fade: sets the load flag (toggling the 0x2000
 * phase bit) and selects tint mode 1. */
int Render_BeginSceneLoad(void) {
    {
        int *flags = &g_GeomScroll.state.flags;
        int old = *flags;
        int value = old | 0x1000;

        *flags = value;
        if (value & 0x2000)
            *flags = value & ~0x2000;
        else
            *flags = old | 0x3000;
    }
    {
        int *flags = &g_GeomScroll.state.flags;

        *flags = (*flags & ~0xC000) | 0x4000;
    }
    return 0;
}

/* Tints every sprite primitive with the tint level (or neutral grey while
 * the tint is off) and advances the two-step tint mode. */
int Render_ApplyScreenTint(void) {
    GeomState *state;
    GeomStateAddress entries;
    GeomEntry *entry;
    int *flags;
    int mode;
    int masked;
    int level;
    int count;
    int i;

    if (!(g_GeomScroll.state.flags & 0x1000))
        return 0;
    level = 0x80;
    if (g_GeomScroll.state.flags & 0x2000)
        level = g_GeomScroll.state.position.meshTintLevel;
    state = g_GeomState;
    entries.state = g_GeomState;
    entries.word += state->entry_offset;
    count = state->entry_count06;
    entry = entries.entry;
    for (i = 0; i < count; i++, entry++) {
        PrimEntry *prim = entry->u30.prim;
        u32 primCount = entry->prim_count;
        u8 grey = level;
        u32 j;

        if (g_ActiveDrawSlot != 0)
            prim += primCount;
        for (j = 0; j < primCount; j++) {
            prim->b = grey;
            prim->g = grey;
            prim->r = grey;
            prim++;
        }
    }
    flags = &g_GeomScroll.state.flags;
    mode = *flags;
    masked = mode & ~0xC000;
    *flags = masked;
    switch (mode & 0xC000) {
    case 0x4000:
        *flags = masked | 0x8000;
        break;
    case 0x8000:
        *flags = masked & ~0x1000;
        break;
    default:
        return 0;
    }
    return 0;
}

/* Applies texture scrolling and camera-relative parallax to the entries. */
int Render_ScrollEntries(void)
{
    if (!(g_GameStateFlags & 0x104)) {
        int i;
        GeomState *state = g_GeomState;
        GeomStateAddress table;
        GeomEntryView *entries;
        int count;
        GeomScrollEntry *entry;

        table.state = g_GeomState;
        table.word += state->entry_offset;
        entries = table.views;
        count = state->entry_count06;

        i = 0;
        if (i < count) {
            GeomScrollCoordinates *camera = &g_GeomScroll.state.position;

            entry = &entries->scroll;
            do {
                int position;

                if (entry->flags & 4) {
                    int x, y, fracX;

                    position = (entry->x * 256) | entry->fractionX.byte;
                    position += entry->speedX;
                    x = (position >> 8) % entry->modulusX;
                    fracX = position & 255;
                    position = (entry->y * 256) | entry->fractionY.byte;
                    position += entry->speedY;
                    y = (position >> 8) % entry->modulusY;
                    entry->fractionX.word = fracX;
                    entry->fractionY.word = position & 255;
                    entry->x = x;
                    entry->y = y;
                }
                if (entry->flags & 8) {
                    position = entry->baseX * 256 +
                        (camera->x - camera->originX) * entry->speedX;
                    entry->x = position >> 8;
                    entry->fractionX.word = position & 255;
                    position = entry->baseY * 256 +
                        (camera->y - camera->originY) * entry->speedY;
                    entry->y = position >> 8;
                    entry->fractionY.word = position & 255;
                }
                entry++;
            } while (++i < count);
        }
        if (g_GeomScroll.state.flags & 0x80) {
            unsigned short x = g_GeomScroll.state.position.x, y = g_GeomScroll.state.position.y;
            g_GeomScroll.state.flags &= ~0x80;
            g_GeomScroll.state.position.savedX = x;
            g_GeomScroll.state.position.savedY = y;
        }
    }
    return 0;
}

/* Sets the colour of every primitive in both halves of the double-buffered
 * primitive list. */
void Render_SetPrimColour(PrimObj *obj, unsigned char r, unsigned char g, unsigned char b) {
    int i;

    for (i = 0; i < obj->count; i++) {
        PrimEntry *entries = obj->entries;

        entries[i].r = r;
        entries[i].g = g;
        entries[i].b = b;
        entries = obj->entries + obj->count;
        entries[i].r = r;
        entries[i].g = g;
        entries[i].b = b;
    }
}
