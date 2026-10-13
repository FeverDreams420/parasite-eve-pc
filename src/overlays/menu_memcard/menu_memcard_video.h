#ifndef MENU_MEMCARD_VIDEO_H
#define MENU_MEMCARD_VIDEO_H

#include "common.h"
#include "pe1/cdrom.h"
#include "pe1/cdrom_buffers.h"
#include "pe1/disc_load_state.h"
#include "pe1/display_env.h"
#include "pe1/game_state.h"
#include "pe1/gpu_image.h"
#include "pe1/psyq_bios.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_press.h"
#include "pe1/script_state.h"

typedef struct VideoFrame { u32 reserved[2], time, reserved0c; u16 width, height; } VideoFrame;
/* One entry of a video table: file name, colour depth and playback layout. */
typedef struct VideoEntry {
    char *name;            /* 0x00 file name appended to the directory */
    u8 wide;               /* 0x04 24-bit frames */
    s16 speed;             /* 0x06 */
    s16 end;               /* 0x08 last frame */
    u16 x, y;              /* 0x0A display origin */
    u8 reserved0e[6];
} VideoEntry;
typedef struct VideoRect { s16 x,y,w,h; } VideoRect;
/* Playback state of one video player copy. */
typedef struct VideoDisplay {
    u8 *decode[2];        /* 0x00 MDEC decode buffers */
    u8 decodeIndex;       /* 0x08 */
    u8 *buffers[2];       /* 0x0C slice upload buffers */
    u8 selector;          /* 0x14 */
    VideoRect regions[2]; /* 0x16 display areas of the two frame buffers */
    u8 region;            /* 0x26 */
    VideoRect rect;       /* 0x28 next slice */
    u8 done;              /* 0x30 frame fully uploaded */
} VideoDisplay;
extern s32 DsSystemStatus(void),DsShellOpen(void);
extern void exit(s32),Spu_SetVoiceVolume(s32);
extern VideoEntry *g_MenuCardScreenVideoEntry;
extern u8 D_800B0DBE,D_801D0DBD;
extern s8 D_800B0DBB;
extern s16 g_MenuCardScreenVideoLastTime,D_801D0DE0[2];

void *MenuCard_UpdateVideoFrame(VideoDisplay *display);
void *func_80121270(VideoDisplay *display);

/* State of the first video player copy (linked at 0x80120D00). */
extern VideoEntry *g_MenuCardVideoEntry;
extern s16 g_MenuCardVideoLastTime, D_80122418[2];
extern u8 D_801223F5;

extern u8 D_801D0DC0;
extern s32 g_ActiveDrawSlot;
extern void StCdInterrupt(void),DecDCTout(void *,s32),func_801918F8(s8,s8);

void MenuCard_UploadVideoSlice(void);

extern DISPENV g_MenuCardScreenSavedDispEnv[2];
extern DRAWENV D_801D13AC[2];
extern u8 *D_801D0DE8,*D_801D0DEC,*D_801D0DF0,*D_801D0DF4,*D_801D0DF8,*D_801D0DFC;
extern u8 D_800B0DBA;
typedef struct VideoFrameCount { s16 value; } VideoFrameCount;
extern VideoFrameCount D_800B0DBC;

s32 MenuCard_InitVideoBuffers(s8 count, u8 **buffers);
s32 func_801216C4(s8 count, u8 **buffers);
/* Buffers and saved environments of the first video player copy. */
extern u8 *D_80122420, *D_80122424, *D_80122428, *D_8012242C, *D_80122430, *D_80122434;
extern DISPENV g_MenuCardVideoSavedDispEnv[2];
extern DRAWENV D_80122814[2];

typedef struct VideoDiscRange { u16 start,end; } VideoDiscRange;
extern VideoEntry D_801D0E00[];
extern u32 g_FieldPadBits;
s32 MenuCard_StepVideo(void);
extern void SetDispMask(s32), ResetGraph(s32), Field_HandleStateTransition(void), StUnSetRing(void), Draw_RenderFrame(void);

s32 MenuCard_PlayVideo(s32 index);

void MenuCard_SetVideoDisplay(s8 index, s8 wide);
void func_80121004(s8 index, s8 wide);

/* Video open: file lookup, stream start and first decoded frame. */
extern u8 g_MovieIndex;
extern VideoEntry D_80122438[];
extern DslFILE D_801223FC, D_801D0DC4;
extern CdlLOC D_80122414, D_801D0DDC;
extern VideoDisplay D_801228CC, D_801D1464;
extern char D_80120FF4[], D_80120FFC[], D_8018F2E4[], D_8018F2EC[];
extern void strcat(char *, char *), StSetRing(void *, s32);
extern void DecDCTvlc2(void *, void *, void *), func_801214D4(void);
extern s32 DsRead2(CdlLOC *, s32);
s32 func_80121C04(s32 number);
s32 MenuCard_OpenVideo(s32 number);

/* Per-frame video step: decode, restart the stream on a stall, page flip. */
extern u8 D_801223F6, D_801223F8, D_801D0DBE;
extern void DecDCTin(void *, s32);
s32 func_80122040(void);

#endif
