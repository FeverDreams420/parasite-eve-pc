#ifndef MENUCARD_UPLOAD_VIDEO_SLICE_NAME
#define MENUCARD_UPLOAD_VIDEO_SLICE_NAME MenuCard_UploadVideoSlice
#define MENUCARD_VIDEO_DISPLAY g_MenuCardScreenVideoDisplay
#define MENUCARD_VIDEO_FLIP g_MenuCardScreenVideoDepthFlip
#define MENUCARD_VIDEO_SET_DISPLAY MenuCard_SetVideoDisplay
#endif

/* Slice callback of the PSY-Q movie player: upload the decoded slice to
 * VRAM, flip the slice buffers and start the next slice. When the frame is
 * complete, switch to the other display region and apply a pending colour
 * depth change (24-bit slices are 24 pixels wide, 16-bit ones 16). */
void MENUCARD_UPLOAD_VIDEO_SLICE_NAME(void) {
    VideoRect rectangle;
    s8 old;

    if (g_MovieWide && g_CdStreamReadyHalfword) {
        StCdInterrupt();
        g_CdStreamReadyHalfword = 0;
    }
    old = MENUCARD_VIDEO_DISPLAY.selector;
    rectangle = MENUCARD_VIDEO_DISPLAY.rect;
    MENUCARD_VIDEO_DISPLAY.selector ^= 1;
    MENUCARD_VIDEO_DISPLAY.rect.x += MENUCARD_VIDEO_DISPLAY.rect.w;
    if (MENUCARD_VIDEO_DISPLAY.rect.x
        < MENUCARD_VIDEO_DISPLAY.regions[MENUCARD_VIDEO_DISPLAY.region].x
              + MENUCARD_VIDEO_DISPLAY.regions[MENUCARD_VIDEO_DISPLAY.region].w) {
        DecDCTout(MENUCARD_VIDEO_DISPLAY.buffers[MENUCARD_VIDEO_DISPLAY.selector],
                      MENUCARD_VIDEO_DISPLAY.rect.w * MENUCARD_VIDEO_DISPLAY.rect.h / 2);
    } else {
        MENUCARD_VIDEO_DISPLAY.done = 1;
        MENUCARD_VIDEO_DISPLAY.region ^= 1;
        MENUCARD_VIDEO_DISPLAY.rect.x = MENUCARD_VIDEO_DISPLAY.regions[MENUCARD_VIDEO_DISPLAY.region].x;
        MENUCARD_VIDEO_DISPLAY.rect.y = MENUCARD_VIDEO_DISPLAY.regions[MENUCARD_VIDEO_DISPLAY.region].y;
        if (MENUCARD_VIDEO_FLIP == 1) {
            s8 *wide = &g_MovieWide;

            *wide ^= 1;
            if (*wide) {
                MENUCARD_VIDEO_DISPLAY.rect.w = 24;
            } else {
                MENUCARD_VIDEO_DISPLAY.rect.w = 16;
            }
            MENUCARD_VIDEO_SET_DISPLAY(g_ActiveDrawSlot ^ 1, g_MovieWide);
            MENUCARD_VIDEO_FLIP = 2;
        }
    }
    LoadImage(&rectangle, MENUCARD_VIDEO_DISPLAY.buffers[old]);
}
