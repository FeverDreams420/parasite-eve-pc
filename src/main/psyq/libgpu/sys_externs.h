#ifndef PSYQ_LIBGPU_SYS_EXTERNS_H
#define PSYQ_LIBGPU_SYS_EXTERNS_H

#include "common.h"
#include "pe1/gpu_callbacks.h"

extern unsigned char g_GpuDefaultCallbacks[];
extern char g_ResetGraphJtbFormat[], g_ResetGraphModeFormat[];
extern unsigned short g_GpuTypeWidthTable[][2], g_GpuTypeHeightTable[][2];
extern GpuCallbacks *g_GpuCallbacks;
extern GpuDebugPrintf g_GpuDebugPrintf;
extern char g_SetGraphDebugFormat[];
extern char g_DrawSyncCallbackFormat[];
extern char g_SetDispMaskFormat[];
extern u8 g_GraphDebug[];
extern char g_DrawSyncFormat[];
extern char g_RectNameFormat[], g_RectCoordsFormat[], g_BadRectFormat[];
extern char g_ClearImageName[];
extern char g_ClearImage2Name[];
extern char g_LoadImageName[];
extern char g_StoreImageName[];
extern char g_MoveImageName[];
extern u32 g_GpuMoveImagePacket[];
extern char g_ClearOTagFormat[];
extern u32 g_GpuOtTerminatorTarget;
extern u32 g_GpuOtTerminatorWord;
extern char g_ClearOTagRFormat[];
extern char g_DrawOTagFormat[];
extern char g_PutDrawEnvFormat[];
extern char g_DrawOTagEnvFormat[];
extern void *memcpy(void *dest, const void *src, unsigned int n);

#endif /* PSYQ_LIBGPU_SYS_EXTERNS_H */
