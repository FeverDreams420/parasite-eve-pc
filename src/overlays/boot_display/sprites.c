#include "common.h"
#include "pe1/boot_display.h"
#include "pe1/cdrom_buffers.h"

#include "sprites_externs.h"

void BootDisplay_InitDisplayResources(void) {
    s16 rect[4];
    char *resource;
    s32 offset;

    ResetGraph(1);
    SetGraphDebug(0);
    SetDispMask(0);

    g_BootDisplayTextScrollX = 0xE000;
    g_BootDisplayTextScrollStepX = -0x100;
    g_BootDisplayTransition = 1;
    rect[2] = 0x3FF;
    g_BootDisplayStateTimer = 0;
    rect[0] = 0;
    rect[1] = 0;
    rect[3] = 0x1FF;
    g_BootDisplayBufferIndex ^= 1;
    ClearImage(rect, 0, 0, 0);
    DrawSync(0);

    resource = g_BootDisplayFontTim;
    Asset_LoadTimImage(resource);
    g_BootDisplayFontImagePixels = Asset_LoadTimImage(resource);
    g_BootDisplayFontClutPixels = Asset_GetTimClutPixels(resource);
    g_BootDisplayFontImageRect = Asset_GetTimImageRect(resource);
    g_BootDisplayFontClutRect = Asset_GetTimClutRect(resource);
    g_BootDisplayTPage = GetTPage(0, 0, g_BootDisplayFontImageRect[0], g_BootDisplayFontImageRect[1]);
    g_BootDisplayClut = GetClut(g_BootDisplayFontClutRect[0], g_BootDisplayFontClutRect[1]);
    DecDCTReset(0);
    offset = 0x43000;
    MdecDiag_ExpandDecoderTable(g_CdLoadAreaBase + (((g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) << 11) + offset));
    SetDispMask(1);
}

void BootDisplay_SelectFrameBuffers(void) {
    s32 orderingOffset;
    s32 primitiveOffset;
    s32 sectorOffset;
    if (g_ActiveDrawSlot != 0) {
        orderingOffset = 0x581E0;
        primitiveOffset = 0x560D0;
        sectorOffset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) << 11;
    } else {
        orderingOffset = 0x581A0;
        primitiveOffset = 0x54000;
        sectorOffset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) << 11;
    }
    orderingOffset = sectorOffset + orderingOffset;

    {
        u8 *base = g_CdLoadAreaBase;
        u8 *orderingBuffer;
        sectorOffset += primitiveOffset;
        orderingBuffer = base + orderingOffset;
        base = base + sectorOffset;
        g_BootDisplayOrderingTable = orderingBuffer;
        g_BootDisplayPrimitiveCursor = base;
    }
    ClearOTagR(g_BootDisplayOrderingTable, 16);
}

void BootDisplay_DisplayNoOp(void) {
}

void BootDisplay_StepImageSprite(void) {
    BootDisplaySpritePacket *packet = (BootDisplaySpritePacket *)g_BootDisplayPrimitiveCursor;
    u16 tpage;
    s16 rect[4];
    tpage = GetTPage(2, 0, g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].x, g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].y);
    SetDrawTPage(packet, 0, 0, tpage);
    /* PsyQ packet length occupies the high byte of its ordering-table tag. */
    ((u8 *)&packet->sprite.tag)[3] = 4;
    packet->sprite.code = 0x64;
    MargePrim(packet, &packet->sprite);
    switch ((u16)g_BootDisplayStateTimer & 3) {
    case 0: packet->sprite.x0 = 160; packet->sprite.y0 = 0; break;
    case 1: packet->sprite.x0 = 0; packet->sprite.y0 = 0; break;
    case 2: packet->sprite.x0 = 160; packet->sprite.y0 = 105; break;
    case 3: packet->sprite.x0 = 0; packet->sprite.y0 = 105; break;
    }
    packet->sprite.u0 = 0;
    packet->sprite.v0 = 0;
    packet->sprite.r0 = g_BootDisplayImageBrightness;
    packet->sprite.g0 = g_BootDisplayImageBrightness;
    packet->sprite.b0 = g_BootDisplayImageBrightness;
    packet->sprite.w = 160;
    packet->sprite.h = 120;
    if (g_BootDisplayTransition == 0) {
        s32 brightness = g_BootDisplayImageBrightness + 4;
        g_BootDisplayImageBrightness = brightness;
        if (brightness >= 128) {
            g_BootDisplayImageBrightness = 128;
            g_BootDisplayTransition = 1;
        }
    } else if (g_BootDisplayTransition == 2) {
        s32 brightness = g_BootDisplayImageBrightness - 4;
        g_BootDisplayImageBrightness = brightness;
        if (brightness <= 0) {
            g_BootDisplayImageBrightness = 0;
            g_BootDisplayTransition = 1;
        }
    }
    AddPrim(g_BootDisplayOrderingTable + 56, packet);
    g_BootDisplayPrimitiveCursor += sizeof(*packet);
    switch (g_BootDisplayState) {
    case 0: {
        s32 offset;
        s32 startOffset;
        s32 endOffset;
        char *base;

        DecDCTvlcSize2(func_8010BF94(g_BootDisplayCreditImages[(u16)g_BootDisplayStateTimer]) >> 2);
        startOffset = 0x38800;
        endOffset = 0x43000;
        offset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048;
        base = g_CdLoadAreaBase;
        startOffset += offset;
        DecDCTvlc2(g_BootDisplayCreditImages[(u16)g_BootDisplayStateTimer], base + startOffset, base + (offset + endOffset));
        g_BootDisplayState = 1;
        break;
    }
    case 1: {
        s32 offset;
        offset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048;
        if (DecDCTvlc2(0, 0, g_CdLoadAreaBase + (offset + 0x43000)) == 0) {
            g_BootDisplayState = 2;
            g_BootDisplayTransferProgress = 0;
        }
        break;
    }
    case 2: {
        s32 offset;
        offset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048;
        g_BootDisplayTransferCursor = g_CdLoadAreaBase + offset;
        DecDCTin(g_CdLoadAreaBase + (offset + 0x38800), 0);
        DecDCTout(g_BootDisplayTransferCursor, 0x2800);
        g_BootDisplayState = 3;
        break;
    }
    case 3:
        if (DecDCToutSync(1) == 0) {
            g_BootDisplayState = 4;
            g_BootDisplayTransferProgress = g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].transferX;
        }
        break;
    case 4:
        if ((u16)g_BootDisplayTransferProgress < g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].transferX + 160) {
            rect[0] = g_BootDisplayTransferProgress;
            rect[1] = g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].transferY;
            rect[2] = 16;
            rect[3] = 128;
            LoadImage(rect, g_BootDisplayTransferCursor);
            g_BootDisplayTransferCursor += 0x1000;
            g_BootDisplayTransferProgress += 16;
        } else g_BootDisplayState = 5;
        break;
    case 5: {
        u32 frame = g_BootDisplayFrameCounter;
        u32 divisor = 450;
        u32 phase;
        /* Preserve retail hardware division rather than reciprocal multiplication. */
        phase = frame % divisor;
        frame = divisor - 93;
        if (phase == frame) g_BootDisplayTransition = 2;
        if (phase == 0) {
            g_BootDisplayState = 6;
            g_BootDisplayTransition = 0;
        }
        break;
    }
    case 6: {
        u16 timer = (u16)g_BootDisplayStateTimer + 1;
        g_BootDisplayStateTimer = timer;
        if (timer < 30) {
            g_BootDisplayState = 0;
        } else {
            g_BootDisplayState = 7;
        }
        g_BootDisplayTransition = 0;
        g_BootDisplayBufferIndex ^= 1;
        break;
    }
    case 7: {
        u32 frame = g_BootDisplayFrameCounter;
        u32 divisor = 450;
        u32 phase;
        /* Preserve retail hardware division rather than reciprocal multiplication. */
        phase = frame % divisor;
        frame = divisor - 93;
        if (phase == frame) g_BootDisplayTransition = 2;
        break;
    }
    }
}

void BootDisplay_QueueTintedSprite(u16 x, u16 y, u8 color) {
    char *primitive = g_BootDisplayPrimitiveCursor;
    s32 packed = BootDisplay_GetGlyphIndex(color);

    func_8012462C(primitive, x, y, (packed & 15) << 4, packed & 0xF0);
    AddPrim(g_BootDisplayOrderingTable + 0x2C, primitive);
    g_BootDisplayPrimitiveCursor += 0x1C;
}

void BootDisplay_InitSprite16(BootDisplaySpritePacket *primitive, s16 x, s16 y,
                       u16 u, u16 v) {
    volatile BootDisplaySpritePacket *packet = primitive;

    SetDrawTPage(&primitive->draw_mode, 0, 0, g_BootDisplayTPage);
    ((u8 *)&primitive->sprite.tag)[3] = 4;
    primitive->sprite.code = 100;
    SetSemiTrans(&primitive->sprite, 1);
    MargePrim(&primitive->draw_mode, &primitive->sprite);
    packet->sprite.w = 16;
    packet->sprite.h = 16;
    packet->sprite.x0 = x;
    packet->sprite.y0 = y;
    packet->sprite.u0 = u;
    packet->sprite.v0 = v;
    packet->sprite.r0 = 255;
    packet->sprite.g0 = 255;
    packet->sprite.b0 = 255;
    packet->sprite.clut = g_BootDisplayClut;
}

void BootDisplay_QueueFadeSprite(s32 color) {
    BootDisplaySpritePacket *primitive = (BootDisplaySpritePacket *)g_BootDisplayPrimitiveCursor;

    SetDrawTPage(&primitive->draw_mode, 0, 0, g_BootDisplayTPage);
    ((u8 *)&primitive->sprite.tag)[3] = 4;
    primitive->sprite.code = 100;
    SetSemiTrans(&primitive->sprite, 1);
    MargePrim(&primitive->draw_mode, &primitive->sprite);

    primitive->sprite.x0 = 120;
    primitive->sprite.y0 = 110;
    primitive->sprite.u0 = 0;
    primitive->sprite.v0 = 0x80;
    primitive->sprite.r0 = (u8)color;
    primitive->sprite.g0 = (u8)color;
    primitive->sprite.b0 = (u8)color;
    primitive->sprite.w = 0x60;
    primitive->sprite.h = 0x18;
    primitive->sprite.clut = g_BootDisplayClut;
    AddPrim(g_BootDisplayOrderingTable + 0x2C, primitive);
    g_BootDisplayPrimitiveCursor += 0x1C;
}

void BootDisplay_PresentSpriteFrame(void) {
    DrawSync(0);
    VSync(2);
    ResetGraph(1);
    PutDispEnv(&g_RenderDispEnvArray[g_ActiveDrawSlot]);
    DrawOTagEnv(g_BootDisplayOrderingTable + 0x3C,
                  &g_RenderDrawEnvArray[g_ActiveDrawSlot]);
    g_ActiveDrawSlot ^= 1;
}
