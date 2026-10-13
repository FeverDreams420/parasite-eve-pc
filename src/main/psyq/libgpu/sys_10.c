/* Psy-Q LIBGPU SYS.OBJ, part 10 of 11: LoadImage2, StoreImage2, MoveImage2
 * and DrawOTag2. */

#include "pe1/gpu_callbacks.h"
#include "pe1/gpu_dma_register_window.h"
#include "pe1/gpu_dma_timeout_state.h"
#include "pe1/gpu_state.h"
#include "pe1/psyq_callbacks.h"
#include "pe1/psyq_gpu.h"

#include "sys_10_externs.h"
#include "sys_10_externs_1.h"
#include "sys_10_externs_2.h"
#include "sys_10_externs_3.h"

void checkRECT(char *, RECT *);
/* GPU status read through the GP1 port; it must be re-read on every poll. */
#define GPU_STATUS_READY() (*(volatile unsigned int *)g_GpuGp1Ptr & 0x04000000)
int Gpu_DmaTimeoutCheck(void);
void Gpu_RestoreDmaCallback(void);

int LoadImage2(RECT *rect, unsigned int *data)
{
    checkRECT(g_LoadImage2Name, rect);
    g_GpuDmaTimeoutDeadline = VSync(-1) + 240;
    g_GpuDmaWaitLoopCounter = 0;
    /* Read GPU readiness only after the volatile DMA status reports idle. */
    while ((*g_GpuDmaChcrPtr & 0x01000000) || !GPU_STATUS_READY()) {
        if (Gpu_DmaTimeoutCheck()) return -1;
    }
    DMACallback(2, Gpu_RestoreDmaCallback);
    g_GpuCallbacks->u20.load(rect, data);
    return 0;
}

int StoreImage2(RECT *rect, unsigned int *data)
{
    checkRECT(g_StoreImageName, rect);
    g_GpuDmaTimeoutDeadline = VSync(-1) + 240;
    g_GpuDmaWaitLoopCounter = 0;
    /* Read GPU readiness only after the volatile DMA status reports idle. */
    while ((*g_GpuDmaChcrPtr & 0x01000000) || !GPU_STATUS_READY()) {
        if (Gpu_DmaTimeoutCheck()) return -1;
    }
    DMACallback(2, Gpu_RestoreDmaCallback);
    g_GpuCallbacks->u1c.store(rect, data);
    return 0;
}

int MoveImage2(RECT *rect, int x, int y)
{
    checkRECT(g_MoveImageName, rect);
    g_GpuDmaTimeoutDeadline = VSync(-1) + 240;
    g_GpuDmaWaitLoopCounter = 0;
    /* Read GPU readiness only after the volatile DMA status reports idle. */
    while ((*g_GpuDmaChcrPtr & 0x01000000) || !GPU_STATUS_READY()) {
        if (Gpu_DmaTimeoutCheck()) return -1;
    }
    DMACallback(2, Gpu_RestoreDmaCallback);
    if (!rect->w || !rect->h) return -1;
    g_GpuMoveImagePacket[0] = *(unsigned int *)&rect->x;
    g_GpuMoveImagePacket[1] = ((unsigned int)y << 16) | (x & 0xFFFF);
    g_GpuMoveImagePacket[2] = *(unsigned int *)&rect->w;
    g_GpuCallbacks->u18.moveImage((char *)g_GpuMoveImagePacket - 8);
    return 0;
}

int DrawOTag2(void *packet)
{
    if (g_GpuSystemState.queueState.debugLevel >= 2) g_GpuDebugPrintf(g_DrawOTagFormat, packet);
    g_GpuDmaTimeoutDeadline = VSync(-1) + 240;
    g_GpuDmaWaitLoopCounter = 0;
    /* Read GPU readiness only after the volatile DMA status reports idle. */
    while ((*g_GpuDmaChcrPtr & 0x01000000) || !GPU_STATUS_READY()) {
        if (Gpu_DmaTimeoutCheck()) return -1;
    }
    DMACallback(2, Gpu_RestoreDmaCallback);
    g_GpuCallbacks->u18.moveImage(packet);
    return 0;
}
