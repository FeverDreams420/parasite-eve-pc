#include "pe1/boot.h"
#include "pe1/boot_display.h"
#include "pe1/psyq_gpu.h"
#include <string.h>

#include "image_display_externs.h"

void BootDisplay_InitDisplayBuffers(void) {
    RECT rect;
    u16 line;
    /* Both RGB24 screens and the decoder workspace share the original area. */
    s32 delta;
    char *base;
    char *start;
    char *end;
    s32 offset;
    u16 *screenY;
    void *resource;

    s64 workspace=((s64)g_MainAssetLbaTbl[0]-g_BootDisplayDataLbaTbl)*2048;
    if(!g_CdLoadAreaBase || workspace<0 || workspace+0x54000+320*224*3>BOOT_DISPLAY_WORK_SIZE){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }
    ResetGraph(1);
    SetGraphDebug(0);
    SetDispMask(0);

    screenY = g_MainAssetLbaTbl;
    g_BootDisplayTextScrollX = 0xE000;
    g_BootDisplayTextScrollStepX = -0x100;
    g_BootDisplayFontStyle=0;
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x3FF;
    rect.h = 0x1FF;
    g_BootDisplayBufferIndex = 0;
    g_BootDisplayState = 0;
    g_BootDisplayDone=0;
    g_BootDisplayTransferProgress = 0;
    g_BootDisplayStateTimer = 0;

    line = *screenY;
    base = (char *)g_CdLoadAreaBase;
    delta = (line - g_BootDisplayDataLbaTbl) * 2048;
    start = base + delta;
    delta += 0x54000;
    end = (char *)g_CdLoadAreaBase + delta;
    g_BootDisplayImageBuffers[0].pixels = start;
    g_BootDisplayImageBuffers[0].decodeTarget = end;
    g_BootDisplayImageBuffers[1].pixels = end;
    g_BootDisplayImageBuffers[1].decodeTarget = start;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    DecDCTReset(0);
    offset = 0x43000;
    MdecDiag_ExpandDecoderTable(((*screenY - g_BootDisplayDataLbaTbl) * 2048) + offset + g_CdLoadAreaBase);

    resource = g_BootDisplayFontTim;
    g_BootDisplayFontImagePixels = (u8 *)Asset_GetTimImagePixels(resource);
    g_BootDisplayFontClutPixels = (u16 *)Asset_GetTimClutPixels(resource);
    g_BootDisplayFontImageRect = Asset_GetTimImageRect(resource);
    g_BootDisplayFontClutRect = Asset_GetTimClutRect(resource);
    g_BootDisplayTPage = GetTPage(0, 0, g_BootDisplayFontImageRect->x, g_BootDisplayFontImageRect->y);
    g_BootDisplayClut = GetClut(g_BootDisplayFontClutRect->x, g_BootDisplayFontClutRect->y);
    BootDisplay_SetImageDisplay(0);
    BootDisplay_SetImageDisplay(1);
    PutDispEnv(&g_RenderDispEnvArray[g_ActiveDrawSlot]);
    SetDispMask(1);
}

void BootDisplay_PresentFrame(void) {
    RECT rect;
    u16 timer;
    if((unsigned)g_BootDisplayBufferIndex>=2 || (unsigned)g_ActiveDrawSlot>=2 || g_BootDisplayState<0){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }
    if(!g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].pixels){g_BootDisplayState=g_BootDisplayDone=-1;return;}
    if (g_ActiveDrawSlot != 0) {
        rect.w = 480;
        rect.x = 0;
        rect.y = 0;
        rect.h = 224;
    } else {
        rect.x = 0;
        rect.y = 224;
        rect.w = 480;
        rect.h = 224;
    }
    LoadImage(&rect, g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].pixels);
    if (g_BootDisplayState == 6) {
        timer = g_BootDisplayStateTimer + 1;
        g_BootDisplayStateTimer = timer;
        g_BootDisplayBufferIndex ^= 1;
        if (timer < 31)
            g_BootDisplayState = 0;
        else
            g_BootDisplayState = 7;
    }
    DrawSync(0);
    VSync(2);
    PutDispEnv(&g_RenderDispEnvArray[g_ActiveDrawSlot]);
    PutDrawEnv(&g_RenderDrawEnvArray[g_ActiveDrawSlot]);
    g_ActiveDrawSlot ^= 1;
}

/* RGB24 pixels receive saturated brightness from four packed palette indices. */
#define BRIGHTEN_NIBBLE(mask, shift) { \
    u32 index = bits & (mask); \
    if (index && pixelOffset >= 0 && pixelOffset + 2 < 320*224*3) { \
        BootRgb24Pixel *pixel = (BootRgb24Pixel *)(screen + pixelOffset); \
        u8 *greenPixel = &pixel->green; \
        u32 brightness = (((u16 *)g_BootDisplayFontClutPixels)[index >> (shift)] & 31) << 3; \
        u32 blue = greenPixel[1]; \
        u32 green = greenPixel[0]; \
        u32 red = pixel->red; \
        blue += brightness; if (blue >= 256) blue = 255; \
        green += brightness; if (green >= 256) green = 255; \
        red += brightness; if (red >= 256) red = 255; \
        greenPixel[1] = blue; greenPixel[0] = green; pixel->red = red; \
    } \
    pixelOffset += 3; \
}
void BootDisplay_BrightenGlyph(s16 x, s16 y, u8 character) {
    if((unsigned)g_BootDisplayBufferIndex>=2 || !g_BootDisplayFontImagePixels || !g_BootDisplayFontClutPixels){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }
    if(!g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].pixels){g_BootDisplayState=g_BootDisplayDone=-1;return;}
    s32 savedX = x;
    s32 row = y;
    char *screen = g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].pixels;
    s32 glyph = BootDisplay_GetGlyphIndex(character);
    s32 low = glyph & 15;
    s32 high = glyph & 240;
    s32 imageOffset;
    u8 *image;
    s32 endY;
    s32 computedEnd;
    s32 screenOffset;
    s32 nextRow;
    imageOffset = (low * 4 + (high << 6)) * 2;
    savedX = (s16)savedX; y = (s16)y;
    computedEnd = (s16)y * 15;
    image = (u8 *)g_BootDisplayFontImagePixels;
    computedEnd *= 64;
    image += imageOffset;
    screenOffset = (s16)savedX * 3 + computedEnd;
    computedEnd = (s16)y + 16;
    if ((s16)y < computedEnd) {
        endY = computedEnd;
        do {
            u16 *source = (u16 *)image;
            s32 pixelOffset = screenOffset;
            s16 chunk;
            if ((s16)row >= 0) {
                if ((s16)row >= 224) break;
                chunk = 0;
                for (; chunk < 4; ++chunk) {
                    u32 bits = *source;
                    if (bits) {
                        BRIGHTEN_NIBBLE(0xF, 0);
                        BRIGHTEN_NIBBLE(0xF0, 4);
                        BRIGHTEN_NIBBLE(0xF00, 8);
                        BRIGHTEN_NIBBLE(0xF000, 12);
                    } else {
                        pixelOffset += 12;
                    }
                    ++source;
                }
            }
            image += 128;
            nextRow = row + 1;
            row = nextRow;
            screenOffset += 960;
        } while ((s16)nextRow < endY);
    }
}

#undef BRIGHTEN_NIBBLE

void BootDisplay_SetImageDisplay(u16 index) {
    s32 displayY, drawY;
    DRAWENV *drawArg;
    if(index>=2){g_BootDisplayState=g_BootDisplayDone=-1;return;}
    if (index != 0) {
        displayY = 0;
        drawY = 224;
    } else {
        displayY = 224;
        drawY = 0;
    }
    SetDefDispEnv(&g_RenderDispEnvArray[index], 0, displayY, 320, 224);
    drawArg = &g_RenderDrawEnvArray[index];
    g_RenderDispEnvArray[index].isrgb24 = 1;
    g_RenderDispEnvArray[index].screen.y = 8;
    g_RenderDispEnvArray[index].screen.h = 224;
    SetDefDrawEnv(drawArg, 0, drawY, 480, 224);
    g_RenderDrawEnvArray[index].dtd = 0;
    g_RenderDrawEnvArray[index].isbg = 0;
    g_RenderDrawEnvArray[index].clip.w = g_RenderDrawEnvArray[index].clip.w * 2 / 3;
}

void BootDisplay_StepImageDisplay(void) {
    RECT rect;
    if((unsigned)g_BootDisplayBufferIndex>=2 || g_BootDisplayState<0){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }
    if(!g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].pixels || !g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].decodeTarget){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }
    rect.x = g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].x;
    rect.y = g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].y;
    rect.w = 480;
    rect.h = 224;
    StoreImage(&rect, g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].pixels);
    DrawSync(0);
    if ((int)g_FieldPadBits < 0) {
        if (g_FieldPadPressed & 0x1000000) {
            u16 timer = (u16)g_BootDisplayStateTimer;
            if (timer != 0) {
                g_BootDisplayStateTimer = timer - 1;
                g_BootDisplayState = 0;
            }
        } else if (g_FieldPadPressed & 0x2000000) {
            u16 timer = (u16)g_BootDisplayStateTimer;
            if (timer < 30) {
                g_BootDisplayStateTimer = timer + 1;
                g_BootDisplayState = 0;
            }
        }
    }
    switch (g_BootDisplayState) {
    case 0: {
        s32 offset;
        s32 startOffset;
        s32 endOffset;
        char *base;

        unsigned imageIndex=(u16)g_BootDisplayStateTimer;
        u16 words;
        if(imageIndex>=31 || !g_BootDisplayCreditImages[imageIndex]){g_BootDisplayState=g_BootDisplayDone=-1;break;}
        memcpy(&words,g_BootDisplayCreditImages[imageIndex],2);
        if(!words || 4u+words*4u>0xa800){g_BootDisplayState=g_BootDisplayDone=-1;break;}
        DecDCTvlcSize2(words >> 4);
        startOffset = 0x38800;
        endOffset = 0x43000;
        offset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048;
        base = (char *)g_CdLoadAreaBase;
        startOffset += offset;
        int result=DecDCTvlc2(g_BootDisplayCreditImages[imageIndex], base + startOffset, base + (offset + endOffset));
        if(result<0)g_BootDisplayState=g_BootDisplayDone=-1;
        else {g_BootDisplayState=result?1:2;g_BootDisplayTransferProgress=0;}
        break;
    }
    case 1: {
        s32 offset;
        offset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048;
        int result=DecDCTvlc2(0, 0, g_CdLoadAreaBase + (offset + 0x43000));
        if(result<0){g_BootDisplayState=g_BootDisplayDone=-1;break;}
        if (result == 0) {
            g_BootDisplayState = 2;
            g_BootDisplayTransferProgress = 0;
        }
        break;
    }
    case 2: {
        s32 offset;
        g_BootDisplayTransferCursor = g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].decodeTarget;
        DecDCTReset(0);
        offset = (g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) * 2048;
        DecDCTin(g_CdLoadAreaBase + (offset + 0x38800), 1);
        DecDCTout(g_BootDisplayTransferCursor, 0x3C00);
        g_BootDisplayState = 3;
        break;
    }
    case 3:
        if (DecDCToutSync(1) == 0) {
            g_BootDisplayState = 4;
            g_BootDisplayTransferProgress = g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].transferX + 120;
        }
        break;
    case 4:
        if ((u16)g_BootDisplayTransferProgress < g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].transferX + 360) {
            rect.x = g_BootDisplayTransferProgress;
            rect.y = g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].transferY + 52;
            rect.w = 24;
            rect.h = 120;
            LoadImage(&rect, g_BootDisplayTransferCursor);
            g_BootDisplayTransferCursor += 0x1800;
            g_BootDisplayTransferProgress += 24;
        } else g_BootDisplayState = 5;
        break;
    case 5:
        if (g_BootDisplayFrameCounter % 450 == 0) g_BootDisplayState = 6;
        break;
    case 6:
    case 7:
        break;
    }
}

void BootDisplay_CallInitSystem(void) {
    Boot_InitSystem();
}

void BootDisplay_BrightenImageRegion(u8 intensity)
{
    (void)intensity; /* The image callback always uses the font palette. */
    if((unsigned)g_BootDisplayBufferIndex>=2 || !g_BootDisplayFontImagePixels || !g_BootDisplayFontClutPixels){
        g_BootDisplayState=g_BootDisplayDone=-1;return;
    }
    if(!g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].pixels){g_BootDisplayState=g_BootDisplayDone=-1;return;}
    s16 line;
    u8 *image = (u8 *)g_BootDisplayFontImagePixels + 0x4000;
    u8 *screen = (u8 *)g_BootDisplayImageBuffers[g_BootDisplayBufferIndex].pixels + 0x19DAC;

    for (line = 110; line < 134; line++) {
        s16 row;
        u16 *source = (u16 *)image;
        BootRgb24Pixel *pixel = (BootRgb24Pixel *)screen;

        for (row = 0; row < 24; row++) {
            u32 bits = *source;
            if (bits) {
                s16 column;
                column = 0;
                for (; column < 4; column++) {
                    int index = bits & 15;
                    if (index) {
                        int brightness = (((u16 *)g_BootDisplayFontClutPixels)[index] & 31) << 3;
                        u32 blue = pixel->blue;
                        u32 green = pixel->green;
                        u32 red = pixel->red;
                        blue += brightness;
                        if (blue >= 256)
                            blue = 255;
                        green += brightness;
                        if (green >= 256)
                            green = 255;
                        red += brightness;
                        if (red >= 256)
                            red = 255;
                        pixel->blue = blue;
                        pixel->green = green;
                        pixel->red = red;
                    }
                    bits >>= 4;
                    ++pixel;
                }
            } else {
                pixel += 4;
            }
            source++;
        }
        image += 0x80;
        screen += 0x3C0;
    }
}

/* Original image state and both native pointer-bearing screen records. */
BootDisplayImageBuffer g_BootDisplayImageBuffers[2]={
    {.x=512,.y=0,.transferX=512,.transferY=256},
    {.x=512,.y=256,.transferX=512,.transferY=0}
};
s32 g_BootDisplayBufferIndex,g_BootDisplayState;
s16 g_BootDisplayStateTimer,g_BootDisplayTransferProgress;
char *g_BootDisplayTransferCursor;
RECT *g_BootDisplayFontImageRect,*g_BootDisplayFontClutRect;
u8 *g_BootDisplayFontImagePixels;
u16 *g_BootDisplayFontClutPixels;
u16 g_BootDisplayTPage,g_BootDisplayClut;
