#include "common.h"
#include <string.h>
#include "pe1/bounds_check.h"
#include "pe1/draw_state.h"
#include "pe1/menu_sound.h"
#include "pe1/render_prim.h"
#include "pe1/psyq_gpu.h"

/* Renderer state shared with the original draw modules. */
unsigned char *g_DrawPacketCursor;
unsigned char *g_DrawPacketArenaBase;
int g_DrawTextDimmed;
unsigned int g_DrawPrimColor;
unsigned int g_DrawColorShaded;
unsigned int *g_DrawOrderingTableEntry;

void Draw_AllocColorQuad(int width, int height) {
    int w = width;
    int h = height;
    RenderGouraudQuad *first;
    RenderGouraudQuad *packet;
    RenderGouraudQuad *second;
    RenderColorTilePacket *tile;
    RenderDrawModePacket *drawMode;
    u8 *old, *next;
    int shadeOffset;
    int ratio;
    int minimum;
    int widthSmaller;
    RenderNativeTag *ot;
    int firstY;
    int firstRight;
    int firstBottom;
    int firstLeft;
    int secondRight;
    int secondTop;
    int secondBottom;
    int secondBound;
    int tileY;
    int tileWidth;
    int tileHeight;
    int tileCode;
    u32 modeColor1;
    u32 modeColor2;
    RenderNativeTag *modeOt;
    int darkColor;
    int darkerColor;

    /* Four original commands must fit before any state or list is changed. */
    size_t required = 2 * sizeof(RenderGouraudQuad) + sizeof(RenderColorTilePacket) + sizeof(RenderDrawModePacket);
    uintptr_t begin = (uintptr_t)g_DrawPacketArenaBase, cursor = (uintptr_t)g_DrawPacketCursor;
    if (!begin || !g_DrawOrderingTableEntry || cursor < begin || cursor % _Alignof(RenderGouraudQuad) || cursor - begin >= 0x4000 ||
        required >= 0x4000 - (cursor - begin)) {
        Inv_BoundsCheckAssertStub(1);
        return;
    }
    first = 0;
    shadeOffset = Battle_GetStateFlag1() == 0 ? 7 : 0;
    old = g_DrawPacketCursor;
    next = old + sizeof(RenderGouraudQuad);
    if (next < g_DrawPacketArenaBase + 0x4000) {
        g_DrawPacketCursor = next;
        first = (RenderGouraudQuad *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (first != 0) {
        u32 color = 0x808080;memcpy(&first->r0, &color, sizeof(color));
        first->tag.length = 8;
        first->code = 0x38;
    }

    packet = first;
    if (h > 0) {
    widthSmaller = w < h;
    minimum = h;
    if (widthSmaller) minimum = w;
    if (minimum >= 0) {
    {
        int secondMinimum = h;
        int scaled;
        if (widthSmaller) secondMinimum = w;
        scaled = secondMinimum * 56;
        ratio = scaled / h;
    }
    } else {
        ratio = 0;
    }
    } else {
        ratio = 0;
    }
    firstY = shadeOffset + 0xAA;
    firstRight = ratio + 0xE5;
    firstBottom = shadeOffset + 0xAD;

    packet->g2 = 0x82;
    packet->g0 = 0x82;
    packet->b2 = 0x36;
    packet->b0 = 0x36;
    packet->r3 = 0x4A;
    packet->r1 = 0x4A;
    packet->g3 = 0xFF;
    packet->g1 = 0xFF;
    packet->b3 = 0x3B;
    packet->b1 = 0x3B;
    firstLeft = 0xE5;
    packet->x3 = firstRight;
    packet->x1 = packet->x3;
    packet->r2 = 0;
    packet->r0 = 0;
    packet->y1 = firstY;
    packet->y0 = packet->y1;
    packet->x2 = firstLeft;
    packet->x0 = firstLeft;
    packet->y3 = firstBottom;
    packet->y2 = packet->y3;

    ot = (RenderNativeTag *)g_DrawOrderingTableEntry;
    packet->tag.address = ot->address;
    old = g_DrawPacketCursor;
    ot->address = (uintptr_t)packet;

    second = 0;
    next = old + sizeof(RenderGouraudQuad);
    if (next < g_DrawPacketArenaBase + 0x4000) {
        g_DrawPacketCursor = next;
        second = (RenderGouraudQuad *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (second != 0) {
        u32 color = 0x808080;memcpy(&second->r0, &color, sizeof(color));
        second->tag.length = 8;
        second->code = 0x38;
    }

    packet = second;
    secondRight = ratio + 0xE5;
    secondTop = shadeOffset + 0xAA;
    secondBottom = shadeOffset + 0xAD;
    darkColor = 0x303030;
    darkerColor = 0x181818;
    packet->r2 = 0xFF;
    packet->r0 = 0xFF;
    packet->g2 = 0x3D;
    packet->g0 = 0x3D;
    packet->b2 = 0x81;
    packet->b0 = 0x81;
    packet->r3 = 0x83;
    packet->r1 = 0x83;
    packet->g3 = 0x13;
    packet->g1 = 0x13;
    packet->b3 = 1;
    packet->b1 = 1;
    secondBound = 0x11D;
    packet->y1 = secondTop;
    packet->y0 = packet->y1;
    packet->x2 = secondRight;
    packet->x0 = packet->x2;
    packet->x3 = secondBound;
    packet->x1 = secondBound;
    packet->y3 = secondBottom;
    packet->y2 = packet->y3;

    g_DrawPrimColor = darkColor;
    g_DrawColorShaded = darkerColor;
    g_DrawTextDimmed = 0;
    ot = (RenderNativeTag *)g_DrawOrderingTableEntry;
    packet->tag.address = ot->address;
    old = g_DrawPacketCursor;
    ot->address = (uintptr_t)packet;

    tile = 0;
    next = old + sizeof(RenderColorTilePacket);
    if (next < g_DrawPacketArenaBase + 0x4000) {
        g_DrawPacketCursor = next;
        tile = (RenderColorTilePacket *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (tile != 0) {
        if (g_DrawTextDimmed != 0) {
            memcpy(&tile->r0, &g_DrawColorShaded, sizeof(u32));
        } else {
            memcpy(&tile->r0, &g_DrawPrimColor, sizeof(u32));
        }
        tile->tag.length = 3;
        tile->code = 0x60;
    }
    tile->x0 = 0xDD;
    tileY = shadeOffset + 0xA6;
    tile->y0 = tileY;
    tileWidth = 0x54;
    tileHeight = 0x0B;
    tile->w = tileWidth;
    tileCode = tile->code;
    tile->h = tileHeight;
    tile->code = tileCode | 2;
    ot = (RenderNativeTag *)g_DrawOrderingTableEntry;
    tile->tag.address = ot->address;
    old = g_DrawPacketCursor;
    ot->address = (uintptr_t)tile;

    drawMode = 0;
    next = old + sizeof(RenderDrawModePacket);
    if (next < g_DrawPacketArenaBase + 0x4000) {
        g_DrawPacketCursor = next;
        drawMode = (RenderDrawModePacket *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (drawMode != 0) {
        SetDrawTPage(drawMode, 0, 0, 0);
    }
    modeColor1 = 0x808080;
    modeColor2 = 0x400000;
    modeOt = (RenderNativeTag *)g_DrawOrderingTableEntry;
    modeColor2 |= 0x4040;
    g_DrawPrimColor = modeColor1;
    g_DrawColorShaded = modeColor2;
    drawMode->tag.address = modeOt->address;
    modeOt->address = (uintptr_t)drawMode;
}
