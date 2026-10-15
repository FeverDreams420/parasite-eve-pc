#include "common.h"
#include "pe1/bounds_check.h"
#include "pe1/draw_state.h"
#include "pe1/draw_wipe_rect.h"
#include "pe1/menu_inventory.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_prim.h"

unsigned char *g_DrawPacketCursor;
unsigned char *g_DrawPacketArenaBase;
int g_DrawTextDimmed;
unsigned int g_DrawPrimColor;
unsigned int g_DrawColorShaded;
unsigned int *g_DrawOrderingTableEntry;
int g_TextCursorX;
int g_TextCursorY;
u16 *g_DrawVertexWritePtr;

u32 g_DrawBlendColorPair[2];
u8 g_WipeRectEdgeList[10] = {0,1,0,2,0xFF,1,3,2,3,0xFF};

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
    u8 *next;
    DrawVertexPair a, b, farA, farB;
    int magnitude;
    int color;
    RenderNativeTag *ot;
    ColorTriPrim *prim;
    int bx, fax, by, fay;

    if ((unsigned)(firstVertex & 0x7f) >= 12 || (unsigned)(secondVertex & 0x7f) >= 12 || (unsigned)mode >= 2) return;
    magnitude = width < 0 ? (int)(0u - (u32)width) : width;
    if ((size_t)(end + 0x4000 - old) > sizeof(ColorTriPair)) {
        next = old + sizeof(ColorTriPair);
        g_DrawPacketCursor = next;
        pair = (ColorTriPair *)old;
    } else Inv_BoundsCheckAssertStub(1);
    if (pair != 0 && pair < pair + 1) {
        prim = &pair->first;
        do {
            if (g_DrawTextDimmed) prim->color.word = g_DrawColorShaded;
            else prim->color.word = g_DrawPrimColor;
            prim->tag.length = 4;
            prim->color.bytes.code = 0x20;
            prim++;
        } while (prim < (ColorTriPrim *)(pair + 1));
    }
    if (!pair) return;
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
    ot = (RenderNativeTag *)g_DrawOrderingTableEntry;
    p->first.tag.address = ot->address;
    ot->address = (uintptr_t)&p->first;
    p->second.tag.address = ot->address;
    ot->address = (uintptr_t)&p->second;
}

static inline RenderDrawModePacket *AllocateDrawMode(int mode) {
    RenderDrawModePacket *packet = 0;
    u8 *old = g_DrawPacketCursor;
    u8 *next;
    if ((size_t)(g_DrawPacketArenaBase + 0x4000 - old) > sizeof(*packet)) {
        next = old + sizeof(*packet);
        g_DrawPacketCursor = next;
        packet = (RenderDrawModePacket *)old;
    } else Inv_BoundsCheckAssertStub(1);
    if (packet) SetDrawTPage(packet, 0, 0, (mode & 3) << 5);
    return packet;
}

void Draw_EmitWipeBar(u8 *edges, int mode) {
    u8 *start = g_DrawPacketCursor;
    RenderDrawModePacket *first = AllocateDrawMode(mode + 1);
    RenderDrawModePacket *second = AllocateDrawMode(2 - mode);
    if (!first || !second) { g_DrawPacketCursor = start; return; }
    while ((s8)edges[0] >= 0) {
        Draw_AllocColorRect((s8)edges[0], (s8)edges[1], 2, mode);
        edges += 2;
    }
    ++edges;
    RenderNativeTag *ot = (RenderNativeTag *)g_DrawOrderingTableEntry;
    first->tag.address = ot->address;
    ot->address = (uintptr_t)first;
    while ((s8)edges[0] >= 0) {
        Draw_AllocColorRect((s8)edges[0], (s8)edges[1], -2, !mode);
        edges += 2;
    }
    second->tag.address = ot->address;
    ot->address = (uintptr_t)second;
}

#define PUSH_WIPE_BAR_VERTEX(xValue, yValue)                     \
    {                                                           \
        int x = (xValue);                                       \
        int y = (yValue);                                       \
        u16 *out = g_DrawVertexWritePtr;                         \
                                                                \
        if (out < ((u16 *)g_TextCursorStackTop + 0x18)) { \
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

            if (out < end) {
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
            sprite.sprite->tag.length = 4;
            sprite.sprite->color.bytes.code = 0x64;
            sprite.sprite->x = g_TextCursorX;
            sprite.sprite->y = g_TextCursorY;
            sprite.sprite->u = 0;
            sprite.sprite->v = 0;
            sprite.sprite->clut = 0x391C;
            sprite.sprite->width = width;
            sprite.sprite->height = height;
            DRAW_LINK_PACKET(sprite);
        }

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
    if ((size_t)(g_DrawPacketArenaBase + 0x4000 - old) > sizeof(TilePrim)) {
        next = old + sizeof(TilePrim);
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
        tile->tag.length = 3;
        tile->color.bytes.code = 0x60;
    }

    if (!tile) return;
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

    tile->w = width - 4;
    tile->h = height - 4;
    tile->color.bytes.code |= 2;
    tile->y0 = g_TextCursorY + 2;
    tile->x0 = g_TextCursorX + 2;
    RenderNativeTag *ordering = (RenderNativeTag *)g_DrawOrderingTableEntry;
    tile->tag.address = ordering->address;
    ordering->address = (uintptr_t)tile;
    old = g_DrawPacketCursor;

    drawMode = 0;
    if ((size_t)(g_DrawPacketArenaBase + 0x4000 - old) > sizeof(DrawModePrim)) {
        next = old + sizeof(DrawModePrim);
        g_DrawPacketCursor = next;
        drawMode = (DrawModePrim *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (drawMode != 0) {
        SetDrawTPage(drawMode, 0, 0, 0x20);
    }
    if (drawMode) {
        drawMode->tag.address = ordering->address;
        ordering->address = (uintptr_t)drawMode;
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
