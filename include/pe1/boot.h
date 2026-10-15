#ifndef PE1_BOOT_H
#define PE1_BOOT_H

#include "common.h"
#include "pe1/pe_image.h"

void Boot_InitSystem(void);
void Boot_VsyncCallback(void);

/* Frame counter boot/frame_loop.c advances once per frame (an array view, so
 * -G8 units still address it absolutely). */
extern unsigned int g_GlobalFrameCounter[];

/* Boot sequence (boot/main.c) and the services it starts. */
void Boot_InitSubsystems(void);
/* 0 on success; -1..-8 identify read/poll failure in four startup stages,
 * -9 sample upload failure, -10 invalid workspace or sector extents. */
int Scene_LoadSceneData(void);
/* 0 success; -1 audio; -2/-3 main read/poll; -4/-5 texture read/poll;
 * -6/-7 directory read/poll; -8 invalid workspace or sector range. */
int Boot_LoadOverlayTables(void);
int CdRom_LoadBootAudio(void);
int CdRom_LoadInitialImage(void);
int Draw_InitDisplay(int mode);
void Boot_RunFrame(void);
/* Returns the room loader error before entering an incomplete scene. */
int Draw_InitPipeline(void);
/* Entry points of the boot overlays: the title display, the movie player
 * and the display-mode selection that returns the mode for Draw_InitDisplay. */
void func_8019234C(void);
void func_801235DC(void);
int func_801909B4(void);

/* Disc table load (pm/disc_tables.c): PE.IMG sector ranges of the
 * startup assets (entries of the g_PeImageSectorTable sector table). */

/* Original per-frame additive RNG: 14 retail instruction constants,
 * two byte-offset cursors and 17 seeded words in one contiguous object. */
extern u32 g_DrawPrimGeneratorWords[33];
void Draw_InitGpuHwRegs(void);
u32 Draw_FlushPrimQueue(void);

/* System start-up (boot/init.c). */
/* Frame counter stepped by Boot_RunFrame; battle colour cycles, the status
 * pointer rotation and the battle srand seed read it. */
extern u32 g_FrameRngCounter;

/* Pad sampling (boot/init.c): render flag table, the
 * button mask per game bit, frames each bit has been held, the actuator
 * buffers and the cheat sequence masks. */
extern s32 g_FieldRenderFlagTable[];
extern u32 g_FieldPadHeldFrameCounts[];
extern u8 g_PadSetActBuffer[];
extern u8 g_PadActuatorAlign[];
extern u32 g_PadComboSequence[];

/* Render pipeline set-up (draw/pipeline.c): the projection distances
 * of the field and battle views. */
extern u32 g_EntityRenderScratch[9];
#define g_RenderProjectionDistance (g_EntityRenderScratch + 8)

void Draw_RenderFrame(void);
int Draw_CheckDrawStatus(void);

#endif
