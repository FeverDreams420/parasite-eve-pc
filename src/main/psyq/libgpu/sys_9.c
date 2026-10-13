/* Psy-Q LIBGPU SYS.OBJ, part 9 of 11: the private DMA wait timer, timeout
 * check and status query. */

#include "pe1/gpu_dma_register_window.h"
#include "pe1/gpu_dma_timeout_state.h"
#include "pe1/psyq_bios.h"

#include "sys_9_externs.h"

int SetIntrMask(int mask);

/* The status query reads and writes the GP0 port as device memory. */
#define GPU_GP0_PORT (*(volatile unsigned int *)g_GpuGp0Ptr)
/* Queue indices can change asynchronously; preserve ordered status reads. */

void Gpu_ResetDmaWaitTimer(void) {
    g_GpuDmaTimeoutDeadline = VSync(-1) + 0xF0;
    g_GpuDmaWaitLoopCounter = 0;
}

int Gpu_DmaTimeoutCheck(void) {
    int now;
    int count;
    int old_count;
    int *counter;
    int mask;
    volatile u32 *ptr;
    volatile u32 *gp1;
    u32 head;

    now = VSync(-1);
    if (g_GpuDmaTimeoutDeadline >= now) {
        counter = &g_GpuDmaWaitLoopCounter;
        count = *counter;
        old_count = count;
        count = count + 1;
        *counter = count;
        if (old_count <= 0xF0000) {
            goto return_zero;
        }
    }

    gp1 = g_GpuGp1Ptr;
    (void)*gp1;
    head = g_GpuDmaQueueHead;

    printf(g_GpuTimeoutFormat,
           (head - g_GpuDmaQueueTail) & 0x3F,
           *gp1,
           *g_GpuDmaChcrPtr,
           *g_GpuDmaMadrPtr);

    mask = SetIntrMask(0);
    g_GpuDmaQueueTail = 0;
    g_GpuSavedIntrMaskDma = mask;
    g_GpuDmaQueueHead = g_GpuDmaQueueTail;

    *g_GpuDmaChcrPtr = 0x401;
    ptr = g_GpuDmaControlRegPtr;
    *ptr = *ptr | 0x800;
    *g_GpuGp1Ptr = 0x02000000;
    *g_GpuGp1Ptr = 0x01000000;
    SetIntrMask(g_GpuSavedIntrMaskDma);
    return -1;

return_zero:
    return 0;
}

int Gpu_QueryStatus(int arg0) {
    u32 status;

    *g_GpuGp1Ptr = 0x10000007;
    status = GPU_GP0_PORT & 0xFFFFFF;
    if (status != 2) {
        GPU_GP0_PORT = (*g_GpuGp1Ptr & 0x3FFF) | 0xE1001000;
        (void)GPU_GP0_PORT;
        return 0;
    }
    if ((arg0 & 8) == 0) {
        return 1;
    }
    *g_GpuGp1Ptr = 0x09000001;
    return 2;
}
