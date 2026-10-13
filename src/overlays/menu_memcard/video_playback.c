#include "common.h"
#include "pe1/cdrom.h"
#include "pe1/menu_memcard_display.h"
#include "pe1/psyq_press.h"

#include "menu_memcard_video.h"
#include "video_playback_externs.h"

#define MENUCARD_SET_VIDEO_DISPLAY_NAME MenuCard_SetVideoDisplay
#define MENUCARD_VIDEO_DISPLAY_MODE D_801D0DBE
#include "video_display.inc"

#undef MENUCARD_SET_VIDEO_DISPLAY_NAME
#undef MENUCARD_VIDEO_DISPLAY_MODE

#define MENUCARD_UPDATE_VIDEO_FRAME_NAME MenuCard_UpdateVideoFrame
#define MENUCARD_VIDEO_ENTRY g_MenuCardScreenVideoEntry
#define MENUCARD_VIDEO_LAST_TIME g_MenuCardScreenVideoLastTime
#define MENUCARD_VIDEO_FINISHED D_801D0DBD
#define MENUCARD_VIDEO_SIZE D_801D0DE0
#include "video_frame.inc"

#undef MENUCARD_UPDATE_VIDEO_FRAME_NAME
#undef MENUCARD_VIDEO_ENTRY
#undef MENUCARD_VIDEO_LAST_TIME
#undef MENUCARD_VIDEO_FINISHED
#undef MENUCARD_VIDEO_SIZE

#include "video_upload.inc"

#undef MENUCARD_UPLOAD_VIDEO_SLICE_NAME
#undef MENUCARD_VIDEO_DISPLAY
#undef MENUCARD_VIDEO_FLIP
#undef MENUCARD_VIDEO_SET_DISPLAY

#define MENUCARD_INIT_VIDEO_BUFFERS_NAME MenuCard_InitVideoBuffers
#define MENUCARD_VIDEO_PTR0 D_801D0DE8
#define MENUCARD_VIDEO_PTR1 D_801D0DEC
#define MENUCARD_VIDEO_PTR2 D_801D0DF0
#define MENUCARD_VIDEO_PTR3 D_801D0DF4
#define MENUCARD_VIDEO_PTR4 D_801D0DF8
#define MENUCARD_VIDEO_PTR5 D_801D0DFC
#define MENUCARD_VIDEO_LAST_TIME g_MenuCardScreenVideoLastTime
#define MENUCARD_VIDEO_SAVED_DISPLAY g_MenuCardScreenSavedDispEnv
#define MENUCARD_VIDEO_SAVED_DRAW D_801D13AC
#include "video_buffers.inc"

#undef MENUCARD_INIT_VIDEO_BUFFERS_NAME
#undef MENUCARD_VIDEO_PTR0
#undef MENUCARD_VIDEO_PTR1
#undef MENUCARD_VIDEO_PTR2
#undef MENUCARD_VIDEO_PTR3
#undef MENUCARD_VIDEO_PTR4
#undef MENUCARD_VIDEO_PTR5
#undef MENUCARD_VIDEO_LAST_TIME
#undef MENUCARD_VIDEO_SAVED_DISPLAY
#undef MENUCARD_VIDEO_SAVED_DRAW

#include "display_reset.inc"

#undef MENUCARD_RESET_DISPLAY_STATE_NAME
#undef MENUCARD_DISPLAY_PTR0
#undef MENUCARD_DISPLAY_PTR1
#undef MENUCARD_DISPLAY_PTR2
#undef MENUCARD_DISPLAY_PTR3
#undef MENUCARD_DISPLAY_PTR4
#undef MENUCARD_DISPLAY_PTR5
#undef MENUCARD_SLOT20_SOURCE
#undef MENUCARD_SLOT92_SOURCE
#undef PE1_EXT_6E76AD431068

#include "video_open.inc"

#undef MENUCARD_OPEN_VIDEO_NAME
#undef MENUCARD_VIDEO_TABLE
#undef MENUCARD_VIDEO_ENTRY
#undef MENUCARD_VIDEO_SET_DISPLAY
#undef MENUCARD_VIDEO_DIRECTORY
#undef MENUCARD_VIDEO_DIRECTORY_LATE
#undef MENUCARD_VIDEO_FILE
#undef MENUCARD_VIDEO_RESTART
#undef MENUCARD_VIDEO_DISPLAY
#undef MENUCARD_VIDEO_PTR0
#undef MENUCARD_VIDEO_PTR1
#undef MENUCARD_VIDEO_PTR2
#undef MENUCARD_VIDEO_PTR3
#undef MENUCARD_VIDEO_PTR4
#undef MENUCARD_VIDEO_PTR5
#undef MENUCARD_VIDEO_UPLOAD
#undef MENUCARD_VIDEO_UPDATE_FRAME
#undef MENUCARD_VIDEO_FINISHED

#include "video_step.inc"

#undef MENUCARD_STEP_VIDEO_NAME
#undef MENUCARD_VIDEO_SET_DISPLAY
#undef MENUCARD_VIDEO_UPDATE_FRAME
#undef MENUCARD_VIDEO_MODE
#undef MENUCARD_VIDEO_FLIP
#undef MENUCARD_VIDEO_FILE
#undef MENUCARD_VIDEO_RESTART
#undef MENUCARD_VIDEO_DISPLAY
#undef MENUCARD_VIDEO_PTR4
#undef MENUCARD_VIDEO_FINISHED

void Spu_SetVoiceVolume(int arg0);

void MenuCard_StopVideo(void) {
    unsigned char *counter;
    int value;

    counter = &D_800B0DBA;
    value = *counter - 1;
    *counter = value;
    Spu_SetVoiceVolume(0);
    DecDCToutCallback(0);
    StUnSetRing();
    DsControlB(9, 0, 0);
}

#define MENUCARD_EVENT_FLAG_NAME func_8012B1F0
#define MENUCARD_EVENT_FLAG D_801D0DC0
#include "event_flag.inc"

#undef MENUCARD_EVENT_FLAG_NAME
#undef MENUCARD_EVENT_FLAG

s32 MenuCard_PlayVideo(s32 index) {
    u8 *buffer;
    s32 entry = index;
    s32 result = 0;
    u32 *flags;
    s32 ready;
    flags=&D_800B0CD8;
    *flags |= 0x200;
    D_801D0E00[entry].wide=1;
    SetDispMask(0); DrawSync(0); ResetGraph(1);
retry:
    {
        VideoDiscRange *range = (VideoDiscRange *)g_SceneInitLbaTbl0;
        s32 readError = -1;
        s32 pollError;
        do {} while(CdRom_ReadSectorsFromLba(D_800B0DD8.baseLba+range->start,D_8001160C,range->end-range->start)==readError);
        pollError=-1;
        for (;;) {
            ready=CdRom_PollReady();
            if (ready==0) break;
            if (ready==pollError) goto retry;
        }
    }
    EnterCriticalSection(); FlushCache(); ExitCriticalSection();
    buffer=g_CdLoadAreaBase+((g_SceneAssetLbaTbl[0]-g_SceneInitLbaTbl1[0])<<11);
    MenuCard_InitVideoBuffers(1,&buffer);
    MenuCard_OpenVideo((s16)entry);
    {
        u8 *active=&D_800B0DBA;
        if (*active) {
            s16 *remainingPtr=(s16 *)(active+2);
            do {
                s32 left = *remainingPtr;
                if (left<=0) break;
                Field_HandleStateTransition();
                if (!(s8)MenuCard_StepVideo()) {
                    D_801D0DE8=0;D_801D0DEC=0;D_801D0DFC=0;D_801D0DF8=0;D_801D0DF0=0;D_801D0DF4=0;
                    D_800B0DBA=0;
                }
                else if (g_FieldPadBits & 0x20000004) {
                    s32 remaining;
                    D_800B0DBA--;
                    Spu_SetVoiceVolume(0);DecDCToutCallback(0);StUnSetRing();DsControlB(9,0,0);
                    remaining=*remainingPtr;
                    D_801D0DE8=0;D_801D0DEC=0;D_801D0DFC=0;D_801D0DF8=0;D_801D0DF0=0;D_801D0DF4=0;
                    D_800B0DBA=0;
                    if(remaining<1400) {VSync(0);SetDispMask(0);result=1;}
                }
                Draw_RenderFrame();
            } while(D_800B0DBA);
        }
    }
    { s32 returnValue = result;
        flags=&D_800B0CD8;
        *flags &= ~0x200;
        return returnValue; }
}
