#include "common.h"
#include "pe1/bounds_check.h"
#include "pe1/draw_state.h"
#include "pe1/draw_wipe_rect.h"
#include "pe1/menu_inventory.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_prim.h"

/* COMMON supplies small-data metadata to MASPSX: these words are reached
 * through $gp, every other extern stays absolute. */
unsigned char *g_DrawPacketCursor;
unsigned char *g_DrawPacketArenaBase;
int g_DrawTextDimmed;
unsigned int g_DrawPrimColor;
unsigned int g_DrawColorShaded;
unsigned int *g_DrawOrderingTableEntry;
int g_TextCursorX;
int g_TextCursorY;
u16 *g_DrawVertexWritePtr;
int g_TextCursorX;
int g_TextCursorY;

int g_DrawBlendColor;

int Draw_GetBlendColor(void) {
    return g_DrawBlendColor;
}

void Draw_BlendColor(int arg0) {
    int *dst;
    int value;

    g_DrawBlendColor = arg0 & 0xFFFFFF;

    value = (((arg0 >> 16) & 0xFF) + ((arg0 >> 8) & 0xFF)) >> 1;
    dst = g_DrawScratchBuffer;
    if (value >= 0x100) {
        value = 0xFF;
    }

    {
        int half = (((arg0 >> 16) & 0xFF) + (arg0 & 0xFF)) >> 1;
        if (half < 0x100) {
            value |= half << 8;
        } else {
            value |= 0xFF00;
        }
    }
    {
        int half = (((arg0 >> 8) & 0xFF) + (arg0 & 0xFF)) >> 1;
        *dst = value | ((half < 0x100) ? (half << 16) : 0xFF0000);
    }
}

typedef RenderColorTrianglePacket ColorTriPrim;

typedef DrawWipeVertex DrawVertexCoord;

#define COORD(v) (*(DrawVertexCoord *)&(v))

typedef struct ColorTriPair {
    ColorTriPrim first, second;
} ColorTriPair;

void Draw_AllocColorRect(int firstVertex, int secondVertex, int width, int mode) {
    ColorTriPair *pair = 0;
    ColorTriPair *p;
    u8 *end = g_DrawPacketArenaBase;
    u8 *old = g_DrawPacketCursor;
    u8 *next = old + sizeof(ColorTriPair);
    DrawVertexPair a, b, farA, farB;
    int magnitude;
    int color;
    RenderPacketTag *ot;
    ColorTriPrim *prim;
    int bx, fax, by, fay;
    DrawPrimLink link;

    magnitude = __builtin_abs(width);
    if (next < end + 0x4000) {
        g_DrawPacketCursor = next;
        pair = (ColorTriPair *)old;
    } else Inv_BoundsCheckAssertStub(1);
    if (pair != 0 && pair < pair + 1) {
        prim = &pair->first;
        do {
            if (g_DrawTextDimmed) prim->color.word = g_DrawColorShaded;
            else prim->color.word = g_DrawPrimColor;
            prim->tag.bytes.length = 4;
            prim->color.bytes.code = 0x20;
            prim++;
        } while (prim < (ColorTriPrim *)(pair + 1));
    }
    farA = ((DrawVertexPair *)g_TextCursorStackTop)[firstVertex & 0x7f];
    a = farA;
    farB = ((DrawVertexPair *)g_TextCursorStackTop)[secondVertex & 0x7f];
    b = farB;
    p = pair;
    p->first.color.bytes.code |= 2;
    p->second.color.bytes.code |= 2;
    color = g_DrawBlendColorPair[mode];
    if (COORD(a).y == COORD(b).y) {
        COORD(farA).x += magnitude;
        COORD(farB).x += (secondVertex & 0x80) ? magnitude : -magnitude;
        COORD(farA).y += width;
        COORD(farB).y += width;
        p->first.color.word = (p->first.color.word & 0xff000000) | color;
        p->second.color.word = (p->second.color.word & 0xff000000) | color;
    } else {
        COORD(farA).x += width;
        COORD(farB).x += width;
        COORD(farA).y += magnitude;
        COORD(farB).y += (secondVertex & 0x80) ? magnitude : -magnitude;
        p->first.color.word = (p->first.color.word & 0xff000000) | ((color >> 1) & 0x7f7f7f);
        p->second.color.word = (p->second.color.word & 0xff000000) | ((color >> 1) & 0x7f7f7f);
    }
    p->first.x0 = COORD(a).x;
    bx = (u16)COORD(b).x;
    p->second.x2 = bx;
    p->first.x1 = bx;
    fax = (u16)COORD(farA).x;
    p->second.x1 = fax;
    p->first.x2 = fax;
    p->first.y0 = COORD(a).y;
    by = (u16)COORD(b).y;
    p->second.y2 = by;
    p->first.y1 = by;
    fay = (u16)COORD(farA).y;
    p->second.y1 = fay;
    p->first.y2 = fay;
    p->second.x0 = COORD(farB).x;
    p->second.y0 = COORD(farB).y;
    ot = (RenderPacketTag *)g_DrawOrderingTableEntry;
    p->first.tag.link.address = ot->link.address;
    link.packet = &p->first;
    ot->link.address = link.word;
    p->second.tag.link.address = ot->link.address;
    link.packet = &p->second;
    ot->link.address = link.word;
}

static inline u32 *AllocateDrawMode(int mode) {
    u32 *packet = 0;
    u8 *old = g_DrawPacketCursor;
    u8 *next = old + 8;
    if (next < g_DrawPacketArenaBase + 0x4000) {
        g_DrawPacketCursor = next;
        packet = (u32 *)old;
    } else Inv_BoundsCheckAssertStub(1);
    if (packet) SetDrawTPage(packet, 0, 0, (mode & 3) << 5);
    return packet;
}

void Draw_EmitWipeBar(u8 *edges, int mode) {
    u32 *first = AllocateDrawMode(mode + 1);
    u32 *second = AllocateDrawMode(2 - mode);
    while ((s8)edges[0] >= 0) {
        Draw_AllocColorRect((s8)edges[0], (s8)edges[1], 2, mode);
        edges += 2;
    }
    ++edges;
    {
        u32 mask24 = 0xffffff, maskTop = 0xff000000;
        u32 *ot = g_DrawOrderingTableEntry;
        DrawPrimLink link;

        link.packet = first;
        *first = (*first & maskTop) | (*ot & mask24);
        *ot = (*ot & maskTop) | (link.word & mask24);
    }
    while ((s8)edges[0] >= 0) {
        Draw_AllocColorRect((s8)edges[0], (s8)edges[1], -2, !mode);
        edges += 2;
    }
    {
        u32 mask24 = 0xffffff, maskTop = 0xff000000;
        u32 *ot = g_DrawOrderingTableEntry;
        DrawPrimLink link;

        link.packet = second;
        *second = (*second & maskTop) | (*ot & mask24);
        *ot = (*ot & maskTop) | (link.word & mask24);
    }
}

#define PUSH_WIPE_BAR_VERTEX(xValue, yValue)                     \
    {                                                           \
        int x = (xValue);                                       \
        int y = (yValue);                                       \
        u16 *out = g_DrawVertexWritePtr;                         \
                                                                \
        if ((unsigned int)out <                                  \
            (unsigned int)((u16 *)g_TextCursorStackTop + 0x18)) { \
            out[1] = x;                                          \
            g_DrawVertexWritePtr = out + 2;                      \
            out[0] = y;                                          \
        } else {                                                 \
            Inv_BoundsCheckAssertStub(4);                           \
        }                                                       \
    }

void Draw_EmitWipeBarRect(int width, int height, int mode)
{
    g_DrawVertexWritePtr = (u16 *)g_TextCursorStackTop;

    PUSH_WIPE_BAR_VERTEX(g_TextCursorX, g_TextCursorY);
    PUSH_WIPE_BAR_VERTEX(g_TextCursorX + width, g_TextCursorY);
    PUSH_WIPE_BAR_VERTEX(g_TextCursorX, g_TextCursorY + height);
    PUSH_WIPE_BAR_VERTEX(g_TextCursorX + width, g_TextCursorY + height);

    Draw_EmitWipeBar(g_WipeRectEdgeList, mode);
}

void Draw_EmitWipeBarPoly(int arg0, int arg1, u8 *arg2) {
    u8 *cursor = arg2;
    u32 value;
    u16 *end;

    g_DrawVertexWritePtr = (u16 *)g_TextCursorStackTop;
    value = cursor[0];
    if (value < 0xFF) {
        end = (u16 *)g_TextCursorStackTop + 0x18;
        do {
            u16 *out = g_DrawVertexWritePtr;
            int x = value + g_TextCursorX;
            int y = g_TextCursorY + cursor[1];

            if ((unsigned int)out < (unsigned int)end) {
                out[1] = x;
                g_DrawVertexWritePtr = out + 2;
                out[0] = y;
            } else {
                Inv_BoundsCheckAssertStub(4);
            }

            cursor += 2;
            value = cursor[0];
        } while (value < 0xFF);
    }

    Draw_EmitWipeBar(cursor + 1, 0);
}

/* Wipe-bar panel over the polygon `points` ((x, y) byte pairs ending in
 * 0xFF, followed by its edge list) or, without points, over the
 * width x height rectangle at the text cursor. A textured panel also
 * gets a 4-bit sprite between two texture-window resets. */
/* Texture-window packet for `area`, linked into the ordering table. */
#define DRAW_ADD_TEXTURE_WINDOW(packet, area)               \
    {                                                       \
        RECT *rect = (area);                                \
                                                            \
        DRAW_ALLOC_PACKET(packet, sizeof(GpuCmdPacket));    \
        if ((packet).word)                                  \
            SetTexWindow((packet).window, rect);            \
        DRAW_LINK_PACKET(packet);                           \
    }

void Draw_AllocColorGradient(int width, int height, u8 *points, int textured)
{
    DrawPacketAddress windowPacket, sprite, mode;
    RECT window;

    if (points) {
        u8 *cursor = points;
        u32 value;

        g_DrawVertexWritePtr = (u16 *)g_TextCursorStackTop;
        value = cursor[0];
        if (value < 0xFF) {
            u16 *end = (u16 *)g_TextCursorStackTop + 0x18;

            do {
                DRAW_PUSH_WIPE_VERTEX(value + g_TextCursorX,
                                      g_TextCursorY + cursor[1], end);
                cursor += 2;
                value = cursor[0];
            } while (value < 0xFF);
        }
        Draw_EmitWipeBar(cursor + 1, 0);
    } else {
        g_DrawVertexWritePtr = (u16 *)g_TextCursorStackTop;
        DRAW_PUSH_WIPE_VERTEX(g_TextCursorX, g_TextCursorY,
                              (u16 *)g_TextCursorStackTop + 0x18);
        DRAW_PUSH_WIPE_VERTEX(g_TextCursorX + width, g_TextCursorY,
                              (u16 *)g_TextCursorStackTop + 0x18);
        DRAW_PUSH_WIPE_VERTEX(g_TextCursorX, g_TextCursorY + height,
                              (u16 *)g_TextCursorStackTop + 0x18);
        DRAW_PUSH_WIPE_VERTEX(g_TextCursorX + width, g_TextCursorY + height,
                              (u16 *)g_TextCursorStackTop + 0x18);
        Draw_EmitWipeBar(g_WipeRectEdgeList, 0);
    }

    if (textured) {
        window.x = 0;
        window.y = 0;
        window.w = 0;
        window.h = 0;
        DRAW_ADD_TEXTURE_WINDOW(windowPacket, &window);

        DRAW_ALLOC_PACKET(sprite, sizeof(RenderSpritePacket));
        if (sprite.word) {
            if (g_DrawTextDimmed)
                sprite.sprite->color.word = g_DrawColorShaded;
            else
                sprite.sprite->color.word = g_DrawPrimColor;
            sprite.sprite->tag.bytes.length = 4;
            sprite.sprite->color.bytes.code = 0x64;
        }
        sprite.sprite->x = g_TextCursorX;
        sprite.sprite->y = g_TextCursorY;
        sprite.sprite->u = 0;
        sprite.sprite->v = 0;
        sprite.sprite->clut = 0x391C;
        sprite.sprite->width = width;
        sprite.sprite->height = height;
        DRAW_LINK_PACKET(sprite);

        window.x = 0;
        window.y = 0;
        window.w = 0x20;
        window.h = 0x20;
        DRAW_ADD_TEXTURE_WINDOW(windowPacket, &window);
    }

    DRAW_ALLOC_PACKET(mode, sizeof(RenderDrawModePacket));
    if (mode.word)
        SetDrawTPage(mode.bytes, 0, 0, 7);
    DRAW_LINK_PACKET(mode);
}

typedef RenderFlatTilePacket TilePrim;
typedef RenderDrawModePacket DrawModePrim;

void Draw_AllocColorTri(int width, int height, int pulse) {
    TilePrim *tile;
    DrawModePrim *drawMode;
    int shade;
    int sync;
    u8 *old, *next;

    tile = 0;
    old = g_DrawPacketCursor;
    next = old + sizeof(TilePrim);
    if (next < g_DrawPacketArenaBase + 0x4000) {
        g_DrawPacketCursor = next;
        tile = (TilePrim *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (tile != 0) {
        if (g_DrawTextDimmed != 0) {
            tile->color.word = g_DrawColorShaded;
        } else {
            tile->color.word = g_DrawPrimColor;
        }
        tile->tag.bytes.length = 3;
        tile->color.bytes.code = 0x60;
    }

    if (pulse != 0) {
        sync = VSync(-1);
        if (sync & 0x20) {
            tile->color.bytes.b = (sync & 0x1F) << 1;
        } else {
            sync &= 0x1F;
            tile->color.bytes.b = 0x40 - (sync << 1);
        }
        tile->color.bytes.r = tile->color.bytes.g = tile->color.bytes.b =
            tile->color.bytes.b;
    } else {
        u32 *color = &tile->color.word;

        *color &= 0xFF000000;
        *color |= g_DrawColorShaded & 0xFFFFFF;
    }

    {
        u32 tag;
        u32 mask24 = 0xFFFFFF;
        u32 maskTop;
        int w = width - 4;
        int h = height - 4;
        int y;
        int x;
        u8 code;
        u32 linkedTag, lowTile, otValue;
        u32 *ot;
        DrawPrimLink link;
        tag = tile->tag.word;
        maskTop = 0xFF000000;
        tile->w = w;
        code = tile->color.bytes.code;
        tile->h = h;
        code |= 2;
        tile->color.bytes.code = code;
        y = g_TextCursorY;
        x = g_TextCursorX;
        tile->y0 = y + 2;
        ot = g_DrawOrderingTableEntry;
        tile->x0 = x + 2;
        otValue = *ot;
        linkedTag = (tag & maskTop) | (otValue & mask24);
        link.packet = tile;
        lowTile = link.word & mask24;
        tile->tag.word = linkedTag;
        old = g_DrawPacketCursor;
        *ot = (*ot & maskTop) | lowTile;
    }

    drawMode = 0;
    next = old + sizeof(DrawModePrim);
    if (next < g_DrawPacketArenaBase + 0x4000) {
        g_DrawPacketCursor = next;
        drawMode = (DrawModePrim *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (drawMode != 0) {
        SetDrawTPage(drawMode, 0, 0, 0x20);
    }
    {
        u32 mask24 = 0xFFFFFF;
        u32 maskTop = 0xFF000000;
        u32 *ot = g_DrawOrderingTableEntry;
        DrawPrimLink link;

        link.packet = drawMode;
        drawMode->tag = (drawMode->tag & maskTop) | (*ot & mask24);
        *ot = (*ot & maskTop) | (link.word & mask24);
    }
}

int g_DrawGradientBlendColor;

void Menu_SetSaveSlotBlendBase(int arg0) {
    g_DrawGradientBlendColor = arg0;
}

void Draw_AllocColorTriGradient(int width, int height, int mode, int pulse)
{
    if (g_DrawGradientBlendColor != 0) {
        Draw_BlendColorInline(g_DrawGradientBlendColor);
    }

    g_DrawVertexWritePtr = (u16 *)g_TextCursorStackTop;
    DRAW_PUSH_WIPE_VERTEX(g_TextCursorX, g_TextCursorY,
                          (u16 *)g_TextCursorStackTop + 0x18);
    DRAW_PUSH_WIPE_VERTEX(g_TextCursorX + width, g_TextCursorY,
                          (u16 *)g_TextCursorStackTop + 0x18);
    DRAW_PUSH_WIPE_VERTEX(g_TextCursorX, g_TextCursorY + height,
                          (u16 *)g_TextCursorStackTop + 0x18);
    DRAW_PUSH_WIPE_VERTEX(g_TextCursorX + width, g_TextCursorY + height,
                          (u16 *)g_TextCursorStackTop + 0x18);

    Draw_EmitWipeBar(g_WipeRectEdgeList, mode);
    g_TextCursorX -= 2;
    g_TextCursorY -= 2;
    Draw_AllocColorTri(width + 4, height + 4, pulse);

    if (g_DrawGradientBlendColor != 0) {
        Draw_BlendColorInline(g_SavedDrawBlendColor[0]);
    }
}
