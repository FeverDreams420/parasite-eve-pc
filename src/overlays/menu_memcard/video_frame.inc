#ifndef MENUCARD_UPDATE_VIDEO_FRAME_NAME
#define MENUCARD_UPDATE_VIDEO_FRAME_NAME MenuCard_UpdateVideoFrame
#define MENUCARD_VIDEO_ENTRY g_MenuCardScreenVideoEntry
#define MENUCARD_VIDEO_LAST_TIME g_MenuCardScreenVideoLastTime
#define MENUCARD_VIDEO_FINISHED g_MenuCardScreenVideoFinished
#define MENUCARD_VIDEO_SIZE g_MenuCardScreenVideoSize
#endif

void *MENUCARD_UPDATE_VIDEO_FRAME_NAME(VideoDisplay *display) {
    VideoFrame header;
    VideoFrame *frame = (VideoFrame *)MENUCARD_VIDEO_STREAM_BUFFER;
    u8 raw[2352];
    u8 *data;
    u32 part = 0;
    s32 end, remaining;
    VideoRect clear;
    if (!g_CdImage || !display || !MENUCARD_VIDEO_ENTRY || !MENUCARD_VIDEO_STREAM_BUFFER) return 0;
    data = MENUCARD_VIDEO_STREAM_BUFFER + sizeof(VideoFrame);
    while (MENUCARD_VIDEO_SECTOR < ((u64)MENUCARD_VIDEO_FILE.size + 2047) / 2048) {
        if (!pe_disc_read_raw(g_CdImage, &MENUCARD_VIDEO_FILE, MENUCARD_VIDEO_SECTOR++, raw)) return 0;
        memcpy(&header, raw + 24, sizeof(header));
        if (header.id != 0x160 || header.type != 0x8001) continue; /* Interleaved XA. */
        if (!part) {
            if (header.secCount) return 0;
            if (header.frameCount > 32767 || !header.nSectors || header.nSectors > (0x20000 - sizeof(*frame)) / 2016 ||
                header.frameSize < 12 || header.frameSize > (u32)header.nSectors * 2016 ||
                !header.width || header.width > 320 || !header.height || header.height > 240)
                return 0;
            *frame = header;
        }
        if (header.secCount != part || header.frameCount != frame->frameCount ||
            header.nSectors != frame->nSectors || header.frameSize != frame->frameSize ||
            header.width != frame->width || header.height != frame->height) return 0;
        memcpy(data + part * 2016, raw + 56, 2016);
        if (++part == frame->nSectors) break;
    }
    if (!part || part != frame->nSectors) return 0;
    u16 words;
    memcpy(&words, data, sizeof(words));
    if (!words || 4u + (u32)words * 4 > 0xfa00) return 0;
    end = MENUCARD_VIDEO_ENTRY->end;
    if (frame->frameCount >= (end - 16)) {
        s32 position = frame->frameCount + 16;

        remaining = position - end;
        remaining = 14 - remaining;
        if (remaining < 0) {
            remaining = 0;
        }
        Spu_SetVoiceVolume(g_GameState.scene_audio.tracks.movie_volume * remaining / 14);
    }
    if (frame->frameCount < MENUCARD_VIDEO_LAST_TIME || frame->frameCount >= MENUCARD_VIDEO_ENTRY->end) {
        MENUCARD_VIDEO_FINISHED = 1;
    }
    MENUCARD_VIDEO_LAST_TIME = frame->frameCount;
    if (MENUCARD_VIDEO_SIZE[0] != frame->width || MENUCARD_VIDEO_SIZE[1] != frame->height) {
        if (g_MovieWide) {
            clear.x = 0;
            clear.y = 0;
            clear.w = 480;
            clear.h = 480;
        } else {
            clear.x = 0;
            clear.y = 0;
            clear.w = 320;
            clear.h = 480;
        }
        ClearImage(&clear, 0, 0, 0);
        MENUCARD_VIDEO_SIZE[0] = frame->width;
        MENUCARD_VIDEO_SIZE[1] = frame->height;
    }
    if (g_MovieWide) {
        display->regions[0].w = display->regions[1].w = MENUCARD_VIDEO_SIZE[0] * 3 / 2;
    } else {
        display->regions[0].w = display->regions[1].w = MENUCARD_VIDEO_SIZE[0];
    }
    display->regions[0].h = display->regions[1].h = MENUCARD_VIDEO_SIZE[1];
    display->rect.h = MENUCARD_VIDEO_SIZE[1];
    return data;
}

#undef MENUCARD_VIDEO_SIZE
#undef MENUCARD_VIDEO_FINISHED
#undef MENUCARD_VIDEO_LAST_TIME
#undef MENUCARD_VIDEO_ENTRY
#undef MENUCARD_UPDATE_VIDEO_FRAME_NAME

#undef MENUCARD_VIDEO_FILE
#undef MENUCARD_VIDEO_SECTOR
#undef MENUCARD_VIDEO_STREAM_BUFFER
