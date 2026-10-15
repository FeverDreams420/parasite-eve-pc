#include "common.h"
#include <string.h>
#include "pe1/boot_display.h"
#include "pe1/cdrom_buffers.h"

#include "sprites_externs.h"

void BootDisplay_InitDisplayResources(void) {
    RECT rect;
    TimFile *resource;
    s32 offset;

    s64 workspace=((s64)g_MainAssetLbaTbl[0]-g_BootDisplayDataLbaTbl)*2048;
    if(!g_CdLoadAreaBase || workspace<0 ||
       workspace+0x54000+2*BOOT_DISPLAY_SPRITE_CAPACITY*sizeof(BootDisplaySpritePacket)+32*sizeof(RenderNativeTag)>BOOT_DISPLAY_WORK_SIZE){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }
    ResetGraph(1);
    SetGraphDebug(0);
    SetDispMask(0);

    g_BootDisplayTextScrollX = 0xE000;
    g_BootDisplayTextScrollStepX = -0x100;
    g_BootDisplayTransition = 1;
    g_BootDisplayState=g_BootDisplayDone=g_BootDisplayTransferProgress=0;
    g_BootDisplayImageBrightness=g_BootDisplayFontStyle=0;
    rect.w = 0x3FF;
    g_BootDisplayStateTimer = 0;
    rect.x = 0;
    rect.y = 0;
    rect.h = 0x1FF;
    g_BootDisplayBufferIndex = 1;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);

    resource = (TimFile *)g_BootDisplayFontTim;
    g_BootDisplayFontImagePixels = (u8 *)Asset_LoadTimImage(resource);
    g_BootDisplayFontClutPixels = (u16 *)Asset_GetTimClutPixels(resource);
    g_BootDisplayFontImageRect = Asset_GetTimImageRect(resource);
    g_BootDisplayFontClutRect = Asset_GetTimClutRect(resource);
    g_BootDisplayTPage = GetTPage(0, 0, g_BootDisplayFontImageRect->x, g_BootDisplayFontImageRect->y);
    g_BootDisplayClut = GetClut(g_BootDisplayFontClutRect->x, g_BootDisplayFontClutRect->y);
    DecDCTReset(0);
    offset = 0x43000;
    MdecDiag_ExpandDecoderTable(g_CdLoadAreaBase + (((g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048) + offset));
    SetDispMask(1);
}

void BootDisplay_SelectFrameBuffers(void) {
    s64 offset=((s64)g_MainAssetLbaTbl[0]-g_BootDisplayDataLbaTbl)*2048;
    size_t bytes=BOOT_DISPLAY_SPRITE_CAPACITY*sizeof(BootDisplaySpritePacket);
    if(!g_CdLoadAreaBase || (unsigned)g_ActiveDrawSlot>=2 || offset<0 ||
       offset+0x54000+2*bytes+32*sizeof(RenderNativeTag)>BOOT_DISPLAY_WORK_SIZE){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }
    char *base=(char *)g_CdLoadAreaBase+offset+0x54000;
    g_BootDisplayPrimitiveCursor=base+g_ActiveDrawSlot*bytes;
    g_BootDisplayPrimitiveEnd=g_BootDisplayPrimitiveCursor+bytes;
    g_BootDisplayOrderingTable=(RenderNativeTag *)(base+2*bytes)+g_ActiveDrawSlot*16;
    ClearOTagR((u_int *)g_BootDisplayOrderingTable,16);
}

void BootDisplay_DisplayNoOp(void) {
}

void BootDisplay_StepImageSprite(void) {
    if(g_BootDisplayState<0 || !g_BootDisplayOrderingTable || !g_BootDisplayPrimitiveCursor || !g_BootDisplayPrimitiveEnd ||
       (uintptr_t)g_BootDisplayPrimitiveCursor%_Alignof(BootDisplaySpritePacket) ||
       (uintptr_t)g_BootDisplayPrimitiveCursor>(uintptr_t)g_BootDisplayPrimitiveEnd ||
       (uintptr_t)g_BootDisplayPrimitiveEnd-(uintptr_t)g_BootDisplayPrimitiveCursor<sizeof(BootDisplaySpritePacket)){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }

    if((unsigned)g_BootDisplayBufferIndex>=2){g_BootDisplayState=g_BootDisplayDone=-1;return;}
    BootDisplaySpritePacket *packet = (BootDisplaySpritePacket *)g_BootDisplayPrimitiveCursor;
    u16 tpage;
    RECT rect;
    tpage = GetTPage(2, 0, g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].x, g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].y);
    SetDrawTPage(packet, 0, 0, tpage);
    SetSprt(&packet->sprite);
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
        s32 brightness = (s32)((u32)g_BootDisplayImageBrightness + 4u);
        g_BootDisplayImageBrightness = brightness;
        if (brightness >= 128) {
            g_BootDisplayImageBrightness = 128;
            g_BootDisplayTransition = 1;
        }
    } else if (g_BootDisplayTransition == 2) {
        s32 brightness = (s32)((u32)g_BootDisplayImageBrightness - 4u);
        g_BootDisplayImageBrightness = brightness;
        if (brightness <= 0) {
            g_BootDisplayImageBrightness = 0;
            g_BootDisplayTransition = 1;
        }
    }
    AddPrim(g_BootDisplayOrderingTable + 14, packet);
    g_BootDisplayPrimitiveCursor += sizeof(*packet);
    switch (g_BootDisplayState) {
    case 0: {
        s32 offset;
        s32 startOffset;
        s32 endOffset;
        char *base;

        unsigned imageIndex=(u16)g_BootDisplayStateTimer;u16 words;
        if(imageIndex>=31 || !g_BootDisplayCreditImages[imageIndex]){g_BootDisplayState=g_BootDisplayDone=-1;break;}
        memcpy(&words,g_BootDisplayCreditImages[imageIndex],2);
        if(!words || 4u+words*4u>0xa800){g_BootDisplayState=g_BootDisplayDone=-1;break;}
        DecDCTvlcSize2(words >> 2);
        startOffset = 0x38800;
        endOffset = 0x43000;
        offset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048;
        base = (char *)g_CdLoadAreaBase;
        startOffset += offset;
        int result=DecDCTvlc2(g_BootDisplayCreditImages[imageIndex], base+startOffset, base+offset+endOffset);
        if(result<0)g_BootDisplayState=g_BootDisplayDone=-1;
        else {g_BootDisplayState=result?1:2;g_BootDisplayTransferProgress=0;}
        break;
    }
    case 1: {
        s32 offset;
        offset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048;
        int result=DecDCTvlc2(0,0,g_CdLoadAreaBase+offset+0x43000);
        if(result<0){g_BootDisplayState=g_BootDisplayDone=-1;break;}
        if (result == 0) {
            g_BootDisplayState = 2;
            g_BootDisplayTransferProgress = 0;
        }
        break;
    }
    case 2: {
        s32 offset;
        offset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048;
        g_BootDisplayTransferCursor = (char *)g_CdLoadAreaBase + offset;
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
            rect.x = g_BootDisplayTransferProgress;
            rect.y = g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].transferY;
            rect.w = 16;
            rect.h = 128;
            LoadImage(&rect, g_BootDisplayTransferCursor);
            g_BootDisplayTransferCursor += 0x1000;
            g_BootDisplayTransferProgress += 16;
        } else g_BootDisplayState = 5;
        break;
    case 5: {
        u32 frame = g_BootDisplayFrameCounter;
        u32 divisor = 450;
        u32 phase;
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
        phase = frame % divisor;
        frame = divisor - 93;
        if (phase == frame) g_BootDisplayTransition = 2;
        break;
    }
    }
}

void BootDisplay_QueueTintedSprite(s16 x, s16 y, u8 color) {
    if(g_BootDisplayState<0 || !g_BootDisplayOrderingTable || !g_BootDisplayPrimitiveCursor || !g_BootDisplayPrimitiveEnd ||
       (uintptr_t)g_BootDisplayPrimitiveCursor%_Alignof(BootDisplaySpritePacket) ||
       (uintptr_t)g_BootDisplayPrimitiveCursor>(uintptr_t)g_BootDisplayPrimitiveEnd ||
       (uintptr_t)g_BootDisplayPrimitiveEnd-(uintptr_t)g_BootDisplayPrimitiveCursor<sizeof(BootDisplaySpritePacket)){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }

    BootDisplaySpritePacket *primitive = (BootDisplaySpritePacket *)g_BootDisplayPrimitiveCursor;
    s32 packed = BootDisplay_GetGlyphIndex(color);

    BootDisplay_InitSprite16(primitive, x, y, (packed & 15) << 4, packed & 0xF0);
    AddPrim(g_BootDisplayOrderingTable + 11, primitive);
    g_BootDisplayPrimitiveCursor += sizeof(BootDisplaySpritePacket);
}

void BootDisplay_InitSprite16(BootDisplaySpritePacket *primitive, s16 x, s16 y,
                       u16 u, u16 v) {
    if(!primitive){g_BootDisplayState=g_BootDisplayDone=-1;return;}
    BootDisplaySpritePacket *packet = primitive;

    SetDrawTPage(&primitive->draw_mode, 0, 0, g_BootDisplayTPage);
    SetSprt(&primitive->sprite);
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

void BootDisplay_QueueFadeSprite(u8 color) {
    if(g_BootDisplayState<0 || !g_BootDisplayOrderingTable || !g_BootDisplayPrimitiveCursor || !g_BootDisplayPrimitiveEnd ||
       (uintptr_t)g_BootDisplayPrimitiveCursor%_Alignof(BootDisplaySpritePacket) ||
       (uintptr_t)g_BootDisplayPrimitiveCursor>(uintptr_t)g_BootDisplayPrimitiveEnd ||
       (uintptr_t)g_BootDisplayPrimitiveEnd-(uintptr_t)g_BootDisplayPrimitiveCursor<sizeof(BootDisplaySpritePacket)){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }

    BootDisplaySpritePacket *primitive = (BootDisplaySpritePacket *)g_BootDisplayPrimitiveCursor;

    SetDrawTPage(&primitive->draw_mode, 0, 0, g_BootDisplayTPage);
    SetSprt(&primitive->sprite);
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
    AddPrim(g_BootDisplayOrderingTable + 11, primitive);
    g_BootDisplayPrimitiveCursor += sizeof(BootDisplaySpritePacket);
}

void BootDisplay_PresentSpriteFrame(void) {
    if(g_BootDisplayState<0 || (unsigned)g_ActiveDrawSlot>=2 || !g_BootDisplayOrderingTable){g_BootDisplayState=g_BootDisplayDone=-1;return;}
    DrawSync(0);
    VSync(2);
    ResetGraph(1);
    PutDispEnv(&g_RenderDispEnvArray[g_ActiveDrawSlot]);
    DrawOTagEnv(g_BootDisplayOrderingTable + 15,
                  &g_RenderDrawEnvArray[g_ActiveDrawSlot]);
    g_ActiveDrawSlot ^= 1;
}

/* Both arenas and ordering tables live in the original shared workspace. */
RenderNativeTag *g_BootDisplayOrderingTable;
char *g_BootDisplayPrimitiveCursor,*g_BootDisplayPrimitiveEnd;
s32 g_BootDisplayTransition,g_BootDisplayImageBrightness;
BootDisplayCallbacks g_BootDisplaySpriteCallbacks={
    BootDisplay_InitDisplayResources,BootDisplay_SelectFrameBuffers,BootDisplay_StepImageSprite,
    BootDisplay_QueueTintedSprite,BootDisplay_QueueFadeSprite,BootDisplay_PresentSpriteFrame,BootDisplay_DisplayNoOp
};
