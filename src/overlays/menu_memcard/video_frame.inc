#ifndef MENUCARD_UPDATE_VIDEO_FRAME_NAME
#define MENUCARD_UPDATE_VIDEO_FRAME_NAME MenuCard_UpdateVideoFrame
#define MENUCARD_VIDEO_ENTRY g_MenuCardScreenVideoEntry
#define MENUCARD_VIDEO_LAST_TIME g_MenuCardScreenVideoLastTime
#define MENUCARD_VIDEO_FINISHED g_MenuCardScreenVideoFinished
#define MENUCARD_VIDEO_SIZE g_MenuCardScreenVideoSize
#endif

void *MENUCARD_UPDATE_VIDEO_FRAME_NAME(VideoDisplay *display) {
    struct {
        u32 *data;
        u32 *header;
        VideoFrame *frame;
        VideoRect clear;
    } local;
    s32 retries;
    s32 end;
    s32 remaining;
    void *data;
    u32 **dataPtr;

    retries = 2000;
    dataPtr = &local.data;
    while (StGetNext(dataPtr, &local.header) != 0) {
        dataPtr = &local.data;
        if (--retries == 0) {
            return 0;
        }
    }
    local.frame = (VideoFrame *)local.header;
    end = MENUCARD_VIDEO_ENTRY->end;
    if (local.frame->frameCount >= (end - 16)) {
        s32 position = local.frame->frameCount + 16;

        remaining = position - end;
        remaining = 14 - remaining;
        if (remaining < 0) {
            remaining = 0;
        }
        Spu_SetVoiceVolume(g_GameState.scene_audio.tracks.movie_volume * remaining / 14);
    }
    if (local.frame->frameCount < MENUCARD_VIDEO_LAST_TIME || local.frame->frameCount >= MENUCARD_VIDEO_ENTRY->end) {
        MENUCARD_VIDEO_FINISHED = 1;
    }
    MENUCARD_VIDEO_LAST_TIME = local.frame->frameCount;
    if (MENUCARD_VIDEO_SIZE[0] != local.frame->width || MENUCARD_VIDEO_SIZE[1] != local.frame->height) {
        if (g_MovieWide) {
            local.clear.x = 0;
            local.clear.y = 0;
            local.clear.w = 480;
            local.clear.h = 480;
        } else {
            local.clear.x = 0;
            local.clear.y = 0;
            local.clear.w = 320;
            local.clear.h = 480;
        }
        ClearImage(&local.clear, 0, 0, 0);
        MENUCARD_VIDEO_SIZE[0] = local.frame->width;
        MENUCARD_VIDEO_SIZE[1] = local.frame->height;
    }
    if (g_MovieWide) {
        display->regions[0].w = display->regions[1].w = MENUCARD_VIDEO_SIZE[0] * 3 / 2;
    } else {
        display->regions[0].w = display->regions[1].w = MENUCARD_VIDEO_SIZE[0];
    }
    data = local.data;
    display->regions[0].h = display->regions[1].h = MENUCARD_VIDEO_SIZE[1];
    display->rect.h = MENUCARD_VIDEO_SIZE[1];
    return data;
}

#undef MENUCARD_VIDEO_SIZE
#undef MENUCARD_VIDEO_FINISHED
#undef MENUCARD_VIDEO_LAST_TIME
#undef MENUCARD_VIDEO_ENTRY
#undef MENUCARD_UPDATE_VIDEO_FRAME_NAME
