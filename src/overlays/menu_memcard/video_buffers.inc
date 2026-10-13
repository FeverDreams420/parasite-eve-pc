#ifndef MENUCARD_INIT_VIDEO_BUFFERS_NAME
#define MENUCARD_INIT_VIDEO_BUFFERS_NAME MenuCard_InitVideoBuffers
#define MENUCARD_VIDEO_PTR0 g_MenuCardScreenVideoDecodeBuffer0
#define MENUCARD_VIDEO_PTR1 g_MenuCardScreenVideoDecodeBuffer1
#define MENUCARD_VIDEO_PTR2 g_MenuCardScreenVideoSliceBuffer0
#define MENUCARD_VIDEO_PTR3 g_MenuCardScreenVideoSliceBuffer1
#define MENUCARD_VIDEO_PTR4 g_MenuCardScreenVideoVlcTable
#define MENUCARD_VIDEO_PTR5 g_MenuCardScreenVideoStreamRing
#define MENUCARD_VIDEO_LAST_TIME g_MenuCardScreenVideoLastTime
#define MENUCARD_VIDEO_SAVED_DISPLAY g_MenuCardScreenSavedDispEnv
#define MENUCARD_VIDEO_SAVED_DRAW g_MenuCardScreenSavedDrawEnv
#endif

s32 MENUCARD_INIT_VIDEO_BUFFERS_NAME(s8 count, u8 **buffers) {
    s8 i;
    VideoRect rectangle;
    u8 *base;
    s32 offset;

    if ((u8)(count - 1) >= 2) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (!buffers[i]) {
            return 0;
        }
    }
    if (g_DrawEnabled.bits) {
        return 0;
    }
    switch (count) {
        case 1:
            base = buffers[0];
            MENUCARD_VIDEO_PTR0 = base;
            offset = 0xFA00;
            MENUCARD_VIDEO_PTR1 = base + offset;
            offset = 0x1F400;
            MENUCARD_VIDEO_PTR5 = base + offset;
            offset = 0x3F400;
            MENUCARD_VIDEO_PTR4 = base + offset;
            offset = 0x50400;
            MENUCARD_VIDEO_PTR2 = base + offset;
            offset = 0x53100;
            MENUCARD_VIDEO_PTR3 = base + offset;
            break;
        case 2:
            base = buffers[0];
            MENUCARD_VIDEO_PTR0 = base;
            offset = 0xFA00;
            MENUCARD_VIDEO_PTR1 = base + offset;
            offset = 0x1F400;
            MENUCARD_VIDEO_PTR5 = base + offset;
            buffers++;
            base = *buffers;
            offset = 0x11000;
            MENUCARD_VIDEO_PTR2 = base + offset;
            MENUCARD_VIDEO_PTR4 = base;
            offset = 0x13D00;
            MENUCARD_VIDEO_PTR3 = base + offset;
            break;
    }
    g_DrawEnabled.bits = 1;
    g_GameState.scene_audio.tracks.movie_volume = 152;
    g_SeqElapsedRecord.value = 0;
    MENUCARD_VIDEO_LAST_TIME = -1;
    for (i = 0; i < 2; i++) {
        MENUCARD_VIDEO_SAVED_DISPLAY[i] = g_RenderDispEnvArray[i];
        MENUCARD_VIDEO_SAVED_DRAW[i] = g_RenderDrawEnvArray[i];
    }
    rectangle.x = 320;
    rectangle.y = 0;
    rectangle.w = 192;
    rectangle.h = 256;
    MoveImage(&rectangle, 512, 0);
    if (!(g_GameStateFlagsWord & 0x08000000)) {
        rectangle.x = 0;
        rectangle.y = 448;
        rectangle.w = 320;
        rectangle.h = 64;
        MoveImage(&rectangle, 512, 256);
    }
    return 1;
}

#undef MENUCARD_VIDEO_SAVED_DRAW
#undef MENUCARD_VIDEO_SAVED_DISPLAY
#undef MENUCARD_VIDEO_LAST_TIME
#undef MENUCARD_VIDEO_PTR5
#undef MENUCARD_VIDEO_PTR4
#undef MENUCARD_VIDEO_PTR3
#undef MENUCARD_VIDEO_PTR2
#undef MENUCARD_VIDEO_PTR1
#undef MENUCARD_VIDEO_PTR0
#undef MENUCARD_INIT_VIDEO_BUFFERS_NAME
