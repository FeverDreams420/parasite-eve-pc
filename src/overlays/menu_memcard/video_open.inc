#ifndef MENUCARD_OPEN_VIDEO_NAME
#define MENUCARD_OPEN_VIDEO_NAME MenuCard_OpenVideo
#define MENUCARD_VIDEO_TABLE D_801D0E00
#define MENUCARD_VIDEO_ENTRY g_MenuCardScreenVideoEntry
#define MENUCARD_VIDEO_SET_DISPLAY MenuCard_SetVideoDisplay
#define MENUCARD_VIDEO_DIRECTORY D_8018F2E4
#define MENUCARD_VIDEO_DIRECTORY_LATE D_8018F2EC
#define MENUCARD_VIDEO_FILE D_801D0DC4
#define MENUCARD_VIDEO_RESTART D_801D0DDC
#define MENUCARD_VIDEO_DISPLAY D_801D1464
#define MENUCARD_VIDEO_PTR0 D_801D0DE8
#define MENUCARD_VIDEO_PTR1 D_801D0DEC
#define MENUCARD_VIDEO_PTR2 D_801D0DF0
#define MENUCARD_VIDEO_PTR3 D_801D0DF4
#define MENUCARD_VIDEO_PTR4 D_801D0DF8
#define MENUCARD_VIDEO_PTR5 D_801D0DFC
#define MENUCARD_VIDEO_UPLOAD MenuCard_UploadVideoSlice
#define MENUCARD_VIDEO_UPDATE_FRAME MenuCard_UpdateVideoFrame
#define MENUCARD_VIDEO_FINISHED D_801D0DBD
#endif

/* Wait for the next decoded frame and start its decode; -1 on a stall. */
static inline s16 MenuCard_OpenNextVideoFrame(VideoDisplay *dec) {
    s16 retries = 2000;
    void *frame;

    while ((frame = MENUCARD_VIDEO_UPDATE_FRAME(dec)) == 0) {
        if (--retries == 0) {
            return -1;
        }
    }
    D_800B0DBC.value++;
    dec->decodeIndex ^= 1;
    DecDCTvlc2(frame, dec->decode[dec->decodeIndex], MENUCARD_VIDEO_PTR4);
    StFreeRing(frame);
    return 0;
}

/* Restart reading at a stream position until the drive accepts it. */
static inline void MenuCard_OpenKickVideoStream(CdlLOC *loc, u8 *result) {
    do {
        do {
            while (DsSystemStatus() != 1) {
            }
        } while (DsQueueLen() != 0);
        DsControl(2, loc, result);
    } while (!DsRead2(loc, 0x1E0));
}

/* Start reading at the file position, retrying from the drive wait. */
static inline void MenuCard_OpenStartVideoStream(CdlLOC *loc, u8 *result) {
    /* Recorded crutch debt: goto restart of the stream start, not
     * reproducible with a structured loop in stock GCC 2.7.2 (loop.c would
     * hoist the per-try constant 1 out of the restart loop). */
start:
    do {
        while (DsSystemStatus() != 1) {
        }
    } while (DsQueueLen() != 0);
    DsControl(2, loc, result);
    if (!DsRead2(loc, 0x1E0)) goto start;
}

/* Reset the decode environment: buffers, both display areas, first slice. */
static inline void MenuCard_OpenSetDecodeEnv(VideoDisplay *dec, s16 x0, s16 y0, s16 x1, s16 y1) {
    dec->decode[0] = MENUCARD_VIDEO_PTR0;
    dec->decode[1] = MENUCARD_VIDEO_PTR1;
    dec->decodeIndex = 0;
    dec->buffers[0] = MENUCARD_VIDEO_PTR2;
    dec->buffers[1] = MENUCARD_VIDEO_PTR3;
    dec->selector = 0;
    dec->regions[0].x = x0;
    dec->regions[0].y = y0;
    dec->regions[1].x = x1;
    dec->regions[1].y = y1;
    dec->region = g_ActiveDrawSlot;
    dec->rect.x = dec->regions[dec->region].x;
    dec->rect.y = dec->regions[dec->region].y;
    if (D_800B0DBB) {
        dec->rect.w = 24;
    } else {
        dec->rect.w = 16;
    }
    dec->done = 0;
}

/* Look up the video file, set up the decoder state, start the stream and
 * decode the first frame. */
s32 MENUCARD_OPEN_VIDEO_NAME(s32 number) {
    s16 index = number;
    VideoRect size;
    char name[32];
    u8 result[8];
    VideoEntry *entry;
    DslFILE *found;

    if ((u16)number >= 47) {
        return 0;
    }
    g_MovieIndex = index;
    MENUCARD_VIDEO_ENTRY = &MENUCARD_VIDEO_TABLE[index];
    D_800B0DBB = MENUCARD_VIDEO_ENTRY->wide;
    MENUCARD_VIDEO_SET_DISPLAY(0, D_800B0DBB);
    MENUCARD_VIDEO_SET_DISPLAY(1, D_800B0DBB);
    name[0] = 0;
    if ((u16)number < 21) {
        strcat(name, MENUCARD_VIDEO_DIRECTORY);
    } else {
        strcat(name, MENUCARD_VIDEO_DIRECTORY_LATE);
    }
    strcat(name, MENUCARD_VIDEO_ENTRY->name);
    /* Recorded crutch debt: retail's CD retry loop is a goto restart, not
     * reproducible with a structured loop in stock GCC 2.7.2 (loop.c
     * hoists the per-try constants out of any loop with loop notes). */
search:
    found = 0;
    if (DsSystemStatus() == 1 && DsQueueLen() == 0) {
        found = DsSearchFile(&MENUCARD_VIDEO_FILE, name);
    }
    /* DsSearchFile: the file record, 0 when absent, all ones on a read error. */
    if (found == 0 || found == (DslFILE *)-1) goto search;
    MENUCARD_VIDEO_RESTART = MENUCARD_VIDEO_FILE.pos;
    entry = MENUCARD_VIDEO_ENTRY;
    size.x = entry->x;
    size.y = entry->y;
    MenuCard_OpenSetDecodeEnv(&MENUCARD_VIDEO_DISPLAY, size.x, size.y + 240, size.x, size.y);
    DecDCTReset(0);
    DecDCToutCallback(MENUCARD_VIDEO_UPLOAD);
    StSetRing(MENUCARD_VIDEO_PTR5, 0x40);
    StSetStream(1, MENUCARD_VIDEO_ENTRY->speed, -1, 0, 0);
    MenuCard_OpenStartVideoStream(&MENUCARD_VIDEO_FILE.pos, result);
    Spu_SetVoiceVolume(D_800B0DBE);
    MdecDiag_ExpandDecoderTable(MENUCARD_VIDEO_PTR4);
    while (MenuCard_OpenNextVideoFrame(&MENUCARD_VIDEO_DISPLAY) == -1) {
        MENUCARD_VIDEO_RESTART = MENUCARD_VIDEO_FILE.pos;
        MenuCard_OpenKickVideoStream(&MENUCARD_VIDEO_RESTART, result);
    }
    MENUCARD_VIDEO_FINISHED = 0;
    D_800B0DBC.value = 1;
    {
        u8 *active = &D_800B0DBA;

        (*active)++;
    }
    return 1;
}
