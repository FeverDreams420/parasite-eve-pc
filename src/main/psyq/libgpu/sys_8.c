/* ASSEMBLER: GNU */
/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -mno-split-addresses */
/* Psy-Q LIBGPU SYS.OBJ, part 8 of 11: the private display-buffer swap and DMA
 * queue. */

#include "pe1/gpu_dma_register_window.h"
#include "pe1/gpu_dma_state.h"
#include "pe1/gpu_state.h"
#include "pe1/psyq_gpu.h"

#include "sys_8_externs.h"
#include "sys_8_externs_1.h"
#include "sys_8_externs_2.h"

void Gpu_ResetDmaWaitTimer(void);
int Gpu_DmaTimeoutCheck(void);
int SetIntrMask(int);

/* LIBGPU command submission: execute immediately when possible, otherwise
 * copy an optional packet into the ring and publish its callback/arguments.
 * The existing project symbol name is retained. Empty ordering barriers
 * are tracked in debt. */
int Gpu_SwapDisplayBuffers(void (*function)(u32, u32), u32 *source, int size, u32 argument) {
    int i;
    GpuDebugState *state;

    Gpu_ResetDmaWaitTimer();
    while (((g_GpuDmaQueueHead + 1) & 63) == g_GpuDmaQueueTail) {
        if (Gpu_DmaTimeoutCheck()) return -1;
        Gpu_SetDisplayBuffer();
    }
    g_GpuSavedIntrMask = SetIntrMask(0);
    state = &g_GpuSystemState;
    state->syncCallbackPending = 1;
    if (state->queueState.queue) {
        register u32 status;
        register u32 mask;
        if (g_GpuDmaQueueHead != g_GpuDmaQueueTail) goto queued;
        status = *(volatile u32 *)g_GpuDmaChcrPtr;
        mask = 0x01000000;
        if (status & mask) goto queued;
        if (state->drawSyncCallback) goto queued;
    }
    {
        register volatile u32 *gp1 = (volatile u32 *)g_GpuDmaRegisterWindow.gp1;
        register u32 ready = 0x04000000;
        do {} while (!(*gp1 & ready));
        function(PSX_ADDRESS(source), argument);
        SetIntrMask(g_GpuSavedIntrMask);
        return 0;
    }

queued:
    DMACallback(2, (PsyqInterruptHandler)Gpu_SetDisplayBuffer);
    i = 0;
    if (size) {
        register u32 *base = g_GpuDmaQueue[0].packet;
        register u32 *cursor = source;
        for (; i < size / 4;) {
            register u32 offset = i * 4;
            register u32 value = *cursor++;
            register u32 head;
            u32 entry;
            head = g_GpuDmaQueueHead;
            i++;
            entry = head * sizeof(GpuQueueEntry);
            entry += PSX_ADDRESS(base);
            offset += entry;
            *(u32 *)offset = value;
            asm volatile("" ::: "memory");
        }
        g_GpuDmaQueue[g_GpuDmaQueueHead].argument0 = PSX_ADDRESS(g_GpuDmaQueue[g_GpuDmaQueueHead].packet);
    } else {
        g_GpuDmaQueue[g_GpuDmaQueueHead].argument0 = PSX_ADDRESS(source);
    }
    g_GpuDmaQueue[g_GpuDmaQueueHead].argument1 = argument;
    asm volatile("" ::: "memory");
    g_GpuDmaQueue[g_GpuDmaQueueHead].function = function;
    asm volatile("" ::: "memory");
    g_GpuDmaQueueHead = (g_GpuDmaQueueHead + 1) & 63;
    SetIntrMask(g_GpuSavedIntrMask);
    Gpu_SetDisplayBuffer();
    return (g_GpuDmaQueueHead - g_GpuDmaQueueTail) & 63;
}

int SetIntrMask(int);

/* LIBGPU queue drain (_exeque); the existing project name is retained.
 * Entries have a callback, two arguments, and an 84-byte packet payload.
 * Empty barriers and register pins preserve the retail scheduling and
 * interrupt-visible reads; they are included in the matching debt report. */
int Gpu_SetDisplayBuffer(void) {
    register u32 readyMask;
    register u32 busyMask;

    if (*g_GpuDmaChcrPtr & 0x01000000) return 1;
    g_GpuSavedIntrMaskDrain = SetIntrMask(0);
    if (g_GpuDmaQueueHead != g_GpuDmaQueueTail && !(*g_GpuDmaChcrPtr & 0x01000000)) {
        readyMask = 0x04000000;
        busyMask = 0x01000000;
        do {
            if (((g_GpuDmaQueueTail + 1) & 63) == g_GpuDmaQueueHead &&
                g_GpuSystemState.drawSyncCallback == 0) {
                DMACallback(2, 0);
            }
            {
                register volatile u32 *gp1 = (volatile u32 *)g_GpuDmaRegisterWindow.gp1;
                if (!(*gp1 & readyMask)) {
                    register u32 ready = 0x04000000;
                    do {} while (!(*gp1 & ready));
                }
            }
            {
                register u32 fnIndex asm("$5");
                register u32 argIndex;
                register u32 argOffset asm("$2");
                register u32 fnOffset;
                register u32 arg0;
                register u32 arg1Index;
                u32 arg1Offset;
                register u32 arg1;
                void (*function)(u32, u32);
                /* Keep the three separate reads of the interrupt-visible tail. */
                fnIndex = g_GpuDmaQueueTail;
                argIndex = g_GpuDmaQueueTail;
                argOffset = argIndex * sizeof(GpuQueueEntry);
                fnOffset = fnIndex * 3;
                {
                    GpuQueueEntryAddress at;
                    at.entry = g_GpuDmaQueue;
                    at.word += argOffset;
                    arg0 = at.entry->argument0;
                }
                asm volatile("" : "=r"(arg0), "=r"(fnOffset)
                             : "0"(arg0), "1"(fnOffset) : "memory");
                arg1Index = g_GpuDmaQueueTail;
                fnOffset <<= 5;
                arg1Offset = arg1Index * sizeof(GpuQueueEntry);
                {
                    GpuQueueEntryAddress at;
                    at.entry = g_GpuDmaQueue;
                    at.word += arg1Offset;
                    arg1 = at.entry->argument1;
                }
                {
                    GpuQueueEntryAddress at;
                    at.entry = g_GpuDmaQueue;
                    at.word += fnOffset;
                    function = at.entry->function;
                }
                function(arg0, arg1);
            }
            g_GpuDmaQueueTail = (g_GpuDmaQueueTail + 1) & 63;
        } while (g_GpuDmaQueueHead != g_GpuDmaQueueTail && !(*g_GpuDmaChcrPtr & busyMask));
    }
    SetIntrMask(g_GpuSavedIntrMaskDrain);
    if (g_GpuDmaQueueHead == g_GpuDmaQueueTail) {
        register u32 status = *(volatile u32 *)g_GpuDmaChcrPtr;
        u32 mask;
        mask = 0x01000000;
        if (!(status & mask)) {
            register u32 *pending = &g_GpuSystemState.syncCallbackPending;
                        if (*pending) {
                void (*callback)(void) = *(void (**)(void))(pending + 1);
                if (callback) {
                    register GpuDebugState *base = (GpuDebugState *)((char *)pending - 8);
                    asm volatile("" : "=r"(base) : "0"(base));
                    base->syncCallbackPending = 0;
                    asm volatile("" ::: "memory");
                    callback();
                }
            }
        }
    }
    return (g_GpuDmaQueueHead - g_GpuDmaQueueTail) & 63;
}

int SetIntrMask(int mask);
int Gpu_QueryStatus(int arg0);

int Gpu_InitDmaQueue(int mode) {
    int saved_mask;
    int channel;

    saved_mask = SetIntrMask(0);
    g_GpuDmaQueueTail = 0;
    g_GpuSavedIntrMaskDma = saved_mask;
    g_GpuDmaQueueHead = g_GpuDmaQueueTail;

    channel = mode & 7;
    switch (channel) {
    case 0:
    case 5:
        *g_GpuDmaChcrPtr = 0x401;
        *g_GpuDmaControlRegPtr |= 0x800;
        *g_GpuGp1Ptr = 0;
        GPU_memset(g_GpuControlRegMirror, 0, 0x100);
        GPU_memset((unsigned char *)g_GpuDmaQueue, 0, 0x1800);
        break;

    case 1:
    case 3:
        *g_GpuDmaChcrPtr = 0x401;
        *g_GpuDmaControlRegPtr |= 0x800;
        *g_GpuGp1Ptr = 0x02000000;
        *g_GpuGp1Ptr = 0x01000000;
        break;
    }

    SetIntrMask(g_GpuSavedIntrMaskDma);

    if ((mode & 7) == 0) {
        return Gpu_QueryStatus(mode);
    }

    return 0;
}

static __inline__ unsigned int readGpuStatus(void) {
    return *(volatile u32 *)g_GpuDmaRegisterWindow.gp1;
}

int Gpu_DrainDmaQueue(int mode) {
    int queued;
    if (mode == 0) {
        Gpu_ResetDmaWaitTimer();
        goto checkQueue;
    retryQueue:
        Gpu_SetDisplayBuffer();
        if (Gpu_DmaTimeoutCheck()) return -1;
    checkQueue:
        if (g_GpuDmaQueueHead != g_GpuDmaQueueTail) goto retryQueue;
        goto checkHardware;
    retryHardware:
        if (Gpu_DmaTimeoutCheck()) return -1;
    checkHardware:
        if (*g_GpuDmaChcrPtr & 0x01000000) goto retryHardware;
        if (!(readGpuStatus() & 0x04000000)) goto retryHardware;
        return 0;
    }
    queued = (g_GpuDmaQueueHead - g_GpuDmaQueueTail) & 63;
    if (queued) Gpu_SetDisplayBuffer();
    if ((*g_GpuDmaChcrPtr & 0x01000000) || !(readGpuStatus() & 0x04000000)) {
        return queued ? queued : 1;
    }
    return queued;
}
