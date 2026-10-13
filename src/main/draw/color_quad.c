#include "common.h"
#include "pe1/bounds_check.h"
#include "pe1/draw_state.h"
#include "pe1/menu_sound.h"
#include "pe1/render_prim.h"

unsigned char *D_8009D100;
unsigned char *D_8009D104;
int g_DrawTextDimmed;
unsigned int g_DrawPrimColor;
unsigned int g_DrawColorShaded;
unsigned int *D_8009D11C;

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
    u32 mask24;
    u32 maskTop;
    u32 *ot;
    u32 firstTag;
    int firstY;
    int firstRight;
    int firstBottom;
    int firstLeft;
    int secondRight;
    int secondTop;
    int secondBottom;
    int secondBound;
    int tileY;
    u32 tileMask24;
    u32 tileMaskTop;
    int tileWidth;
    int tileHeight;
    int tileCode;
    u32 modeMask24;
    u32 modeMaskTop;
    u32 modeColor1;
    u32 modeColor2;
    u32 *modeOt;
    u32 modeTag;
    u32 secondTag;
    u32 linkHigh;
    int darkColor;
    int darkerColor;
    DrawPrimLink link;

    first = 0;
    shadeOffset = Battle_GetStateFlag1() == 0 ? 7 : 0;
    old = D_8009D100;
    next = old + sizeof(RenderGouraudQuad);
    if (next < D_8009D104 + 0x4000) {
        D_8009D100 = next;
        first = (RenderGouraudQuad *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (first != 0) {
        *(u32 *)&first->r0 = 0x808080;
        ((RenderPacketTag *)&first->tag)->bytes.length = 8;
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
    mask24 = 0xFFFFFF;
    firstTag = packet->tag;

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

    maskTop = 0xFF000000;
    ot = D_8009D11C;
    packet->tag = (firstTag & maskTop) | (*ot & mask24);
    linkHigh = *ot & maskTop;
    old = D_8009D100;
    link.packet = packet;
    *ot = linkHigh | (link.word & mask24);

    second = 0;
    next = old + sizeof(RenderGouraudQuad);
    if (next < D_8009D104 + 0x4000) {
        D_8009D100 = next;
        second = (RenderGouraudQuad *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (second != 0) {
        *(u32 *)&second->r0 = 0x808080;
        ((RenderPacketTag *)&second->tag)->bytes.length = 8;
        second->code = 0x38;
    }

    packet = second;
    secondRight = ratio + 0xE5;
    secondTop = shadeOffset + 0xAA;
    secondBottom = shadeOffset + 0xAD;
    mask24 = 0xFFFFFF;
    darkColor = 0x303030;
    darkerColor = 0x181818;
    secondTag = packet->tag;
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
    tileMask24 = 0xFF000000;
    ot = D_8009D11C;
    packet->tag = (secondTag & tileMask24) | (*ot & mask24);
    linkHigh = *ot & tileMask24;
    old = D_8009D100;
    link.packet = packet;
    *ot = linkHigh | (link.word & mask24);

    tile = 0;
    next = old + sizeof(RenderColorTilePacket);
    if (next < D_8009D104 + 0x4000) {
        D_8009D100 = next;
        tile = (RenderColorTilePacket *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (tile != 0) {
        if (g_DrawTextDimmed != 0) {
            *(u32 *)&tile->r0 = g_DrawColorShaded;
        } else {
            *(u32 *)&tile->r0 = g_DrawPrimColor;
        }
        ((RenderPacketTag *)&tile->tag)->bytes.length = 3;
        tile->code = 0x60;
    }
    tileMask24 = 0xFFFFFF;
    tile->x0 = 0xDD;
    tileY = shadeOffset + 0xA6;
    tile->y0 = tileY;
    tileWidth = 0x54;
    tileHeight = 0x0B;
    tileMaskTop = 0xFF000000;
    tile->w = tileWidth;
    tileCode = tile->code;
    tile->h = tileHeight;
    tile->code = tileCode | 2;
    ot = D_8009D11C;
    tile->tag = (tile->tag & tileMaskTop) | (*ot & tileMask24);
    linkHigh = *ot & tileMaskTop;
    old = D_8009D100;
    link.packet = tile;
    *ot = linkHigh | (link.word & tileMask24);

    drawMode = 0;
    next = old + sizeof(RenderDrawModePacket);
    if (next < D_8009D104 + 0x4000) {
        D_8009D100 = next;
        drawMode = (RenderDrawModePacket *)old;
    } else {
        Inv_BoundsCheckAssertStub(1);
    }
    if (drawMode != 0) {
        SetDrawTPage(drawMode, 0, 0, 0);
    }
    modeMask24 = 0xFFFFFF;
    modeColor1 = 0x808080;
    modeColor2 = 0x400000;
    modeMaskTop = 0xFF000000;
    modeTag = drawMode->tag;
    modeOt = D_8009D11C;
    modeColor2 |= 0x4040;
    g_DrawPrimColor = modeColor1;
    g_DrawColorShaded = modeColor2;
    drawMode->tag = (modeTag & modeMaskTop) | (*modeOt & modeMask24);
    link.packet = drawMode;
    *modeOt = (*modeOt & modeMaskTop) | (link.word & modeMask24);
}
