#ifndef MENUCARD_STEP_VIDEO_NAME
#define MENUCARD_STEP_VIDEO_NAME MenuCard_StepVideo
#define MENUCARD_VIDEO_SET_DISPLAY MenuCard_SetVideoDisplay
#define MENUCARD_VIDEO_UPDATE_FRAME MenuCard_UpdateVideoFrame
#define MENUCARD_VIDEO_MODE g_MenuCardScreenVideoDecodeMode
#define MENUCARD_VIDEO_FLIP g_MenuCardScreenVideoDepthFlip
#define MENUCARD_VIDEO_FILE g_MenuCardScreenVideoFile
#define MENUCARD_VIDEO_RESTART g_MenuCardScreenVideoRestartLoc
#define MENUCARD_VIDEO_DISPLAY g_MenuCardScreenVideoDisplay
#define MENUCARD_VIDEO_PTR4 g_MenuCardScreenVideoVlcTable
#define MENUCARD_VIDEO_FINISHED g_MenuCardScreenVideoFinished
#endif

/* Wait for the next decoded frame and start its decode; -1 on a stall. */
static inline s16 MenuCard_NextVideoFrame(VideoDisplay *dec) {
    s16 retries = 2000;
    void *frame;

    while ((frame = MENUCARD_VIDEO_UPDATE_FRAME(dec)) == 0) {
        if (--retries == 0) {
            return -1;
        }
    }
    g_SeqElapsed++;
    dec->decodeIndex ^= 1;
    DecDCTvlc2(frame, dec->decode[dec->decodeIndex], MENUCARD_VIDEO_PTR4);
    StFreeRing(frame);
    return 0;
}

/* Restart the stream at the saved position. */
static inline void MenuCard_KickVideoStream(CdlLOC *loc) {
    u8 result[8];

    StGetBackloc(loc);
    do {
        do {
            while (DsSystemStatus() != 1) {
            }
        } while (DsQueueLen() != 0);
        DsControl(2, loc, result);
    } while (!DsRead2(loc, 0x1E0));
}

/* Wait for the slice upload; on a timeout skip to the other frame buffer. */
static inline void MenuCard_SyncVideoUpload(VideoDisplay *dec) {
    volatile s32 wait = 0x800000;

    while (dec->done == 0) {
        if (--wait == 0) {
            dec->done = 1;
            dec->region ^= 1;
            dec->rect.x = dec->regions[dec->region].x;
            dec->rect.y = dec->regions[dec->region].y;
        }
    }
    dec->done = 0;
}

/* Per-frame step: start the decode and the slice upload, wait for the next
 * frame (restarting the stream on a stall), wait for the upload, then stop
 * the player once the last frame has been shown. */
s32 MENUCARD_STEP_VIDEO_NAME(void) {
    s8 finished;
    u8 *flip;

    if (g_DrawEnabled.bits >= 2) {
        flip = &MENUCARD_VIDEO_FLIP;
        finished = 0;
        if (*flip == 2) {
            MENUCARD_VIDEO_SET_DISPLAY(g_ActiveDrawSlot ^ 1, g_MovieWide);
            *flip = 0;
        }
        MENUCARD_VIDEO_RESTART = MENUCARD_VIDEO_FILE.pos;
        DecDCTin(MENUCARD_VIDEO_DISPLAY.decode[MENUCARD_VIDEO_DISPLAY.decodeIndex], MENUCARD_VIDEO_MODE);
        DecDCTout(MENUCARD_VIDEO_DISPLAY.buffers[MENUCARD_VIDEO_DISPLAY.selector],
                      MENUCARD_VIDEO_DISPLAY.rect.w * MENUCARD_VIDEO_DISPLAY.rect.h / 2);
        while (MenuCard_NextVideoFrame(&MENUCARD_VIDEO_DISPLAY) == -1) {
            MenuCard_KickVideoStream(&MENUCARD_VIDEO_RESTART);
        }
        MenuCard_SyncVideoUpload(&MENUCARD_VIDEO_DISPLAY);
        if (MENUCARD_VIDEO_FINISHED == 1) {
            finished = 1;
        }
        if (!finished) {
            return 1;
        }
        {
            u8 *active = &g_DrawEnabled.bits;

            (*active)--;
        }
        DecDCToutCallback(0);
        StUnSetRing();
        DsControlB(9, 0, 0);
    }
    return 0;
}
