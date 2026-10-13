/* ASSEMBLER: GNU */
/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -mno-split-addresses */
/* Psy-Q LIBGPU SYS.OBJ, part 6 of 11: the private draw-offset and
 * texture-window command builders, _status, the OTC ordering-table clear, the
 * VRAM fill, upload and readback transfers, the GP1 control mirror and the
 * DMA start helpers. */

#include "common.h"
#include "pe1/gpu_command_builders.h"
#include "pe1/gpu_dma_register_window.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_types.h"

#include "sys_6_externs.h"

unsigned int Gpu_BuildDrawOffsetCmd(unsigned int arg0, unsigned int arg1) {
    arg1 &= 0x7FF;
    arg1 <<= 11;
    arg0 &= 0x7FF;
    arg0 |= 0xE5000000;
    return arg1 | arg0;
}

u32 Gpu_BuildTexWindowCmd(GpuTextureWindowRectBytes *tw) {
    volatile int slots[4];
    int x;
    int y;
    int w;
    int h_raw;
    int h;
    u32 command;
    u32 result;

    if (tw == 0) {
        result = 0;
    } else {
        x = tw->xLow >> 3;
        slots[0] = x;
        w = ((-tw->w) & 0xFF) >> 3;
        slots[2] = w;
        y = tw->yLow >> 3;
        slots[1] = y;
        y <<= 15;
        h_raw = tw->h;
        h = ((-h_raw) & 0xFF) >> 3;
        slots[3] = h;

        command = (x << 10) | 0xE2000000;
        result = y | command | (h << 5) | w;
    }

    return result;
}

u_long _status(void) {
    return *g_GpuGp1Ptr;
}

void Gpu_ResetDmaWaitTimer(void);
int Gpu_DmaTimeoutCheck(void);

/* Initialize a reverse ordering table through the OTC DMA channel. */
int Gpu_SendOtChain(unsigned int *table, int count)
{
    unsigned int status;
    unsigned int firstMask;
    int busyBit;
    *g_GpuDmaControlRegPtr |= 0x08000000;
    *g_GpuOtDmaChcrPtr = 0;
    *g_GpuOtDmaMadrPtr = PSX_ADDRESS(&table[count - 1]);
    *g_GpuOtDmaBcrPtr = count;
    /* This store occupies the following call's delay slot in retail. */
    *(unsigned int *)g_GpuOtDmaChcrPtr = 0x11000002;
    Gpu_ResetDmaWaitTimer();
    status = *g_GpuOtDmaChcrPtr;
    /* Keep the first status test separate from the loop's invariant mask. */
    asm volatile("" : "+r"(status), "+r"(count));
    busyBit = 0x01000000;
    firstMask = busyBit;
    asm volatile("" : "+r"(firstMask) : "r"(status));
    if (status & firstMask) {
        asm volatile("" : : "r"(count));
        do {
            if (Gpu_DmaTimeoutCheck()) return -1;
        } while (*g_GpuOtDmaChcrPtr & 0x01000000);
    }
    return count;
}

unsigned int _param(int parameter);
void Gpu_StartDmaTransfer(unsigned int packet);

int Gpu_DmaVramTransfer(RECT *rect, unsigned int color) {
    unsigned short width, height;
    if (rect->w >= 0) {
        if (g_GpuMaxWidth - 1 < rect->w) width = g_GpuMaxWidth - 1;
        else width = rect->w;
    } else width = 0;
    rect->w = width;
    if (rect->h >= 0) {
        if (g_GpuMaxHeight - 1 < rect->h) height = g_GpuMaxHeight - 1;
        else height = rect->h;
    } else height = 0;
    rect->h = height;
    if ((rect->x & 63) || (rect->w & 63)) {
        g_GpuVramFillPacket[0] = ((unsigned int)g_GpuVramFillTailPacket & 0xFFFFFF) | 0x08000000;
        g_GpuVramFillPacket[1] = 0xE3000000;
        g_GpuVramFillPacket[2] = 0xE4FFFFFF;
        g_GpuVramFillPacket[3] = 0xE5000000;
        g_GpuVramFillPacket[4] = 0xE6000000;
        g_GpuVramFillPacket[5] = 0xE1000000 | (*g_GpuGp1Ptr & 0x7FF) | ((color >> 31) << 10);
        g_GpuVramFillPacket[6] = (color & 0xFFFFFF) | 0x60000000;
        g_GpuVramFillPacket[7] = ((GpuRectWords *)rect)->position;
        g_GpuVramFillPacket[8] = ((GpuRectWords *)rect)->size;
        g_GpuVramFillTailPacket[0] = 0x03FFFFFF;
        g_GpuVramFillTailPacket[1] = _param(3) | 0xE3000000;
        g_GpuVramFillTailPacket[2] = _param(4) | 0xE4000000;
        g_GpuVramFillTailPacket[3] = _param(5) | 0xE5000000;
    } else {
        g_GpuVramFillPacket[0] = 0x05FFFFFF;
        g_GpuVramFillPacket[1] = 0xE6000000;
        g_GpuVramFillPacket[2] = 0xE1000000 | (*g_GpuGp1Ptr & 0x7FF) | ((color >> 31) << 10);
        g_GpuVramFillPacket[3] = (color & 0xFFFFFF) | 0x02000000;
        g_GpuVramFillPacket[4] = ((GpuRectWords *)rect)->position;
        g_GpuVramFillPacket[5] = ((GpuRectWords *)rect)->size;
    }
    {
        Gpu_StartDmaTransfer((unsigned int)g_GpuVramFillPacket);
    }
    {
        int result = 0;
        asm volatile("" : "+r"(result));
        return result;
    }
}

/* Upload pixels using a CPU prefix followed by 16-word DMA blocks. */
int Gpu_WaitDmaComplete(RECT *rect, unsigned int *data)
{
    int words, remainder, blocks;
    register int variant asm("$21");
    unsigned int status;
    unsigned int firstMask;
    int readyBit;
    int firstEnd;
    unsigned int command;
    unsigned int *port;
    unsigned short width, height;
    Gpu_ResetDmaWaitTimer();
    variant = 0;
    if (rect->w >= 0) {
        if (g_GpuMaxWidth < rect->w) width = g_GpuMaxWidth;
        else width = rect->w;
    } else width = 0;
    rect->w = width;
    if (rect->h >= 0) {
        if (g_GpuMaxHeight < rect->h) height = g_GpuMaxHeight;
        else height = rect->h;
    } else height = 0;
    rect->h = height;
    words = (rect->w * rect->h + 1) / 2;
    if (words <= 0) return -1;
    remainder = words % 16;
    /* Retail retains the variant branch and this rectangle's register lifetime. */
    asm volatile("" : "+r"(variant), "+r"(rect));
    blocks = words / 16;
    status = *g_GpuDmaRegisterWindow.gp1;
    /* Keep the first readiness mask separate from the loop's invariant. */
    asm volatile("" : "+r"(status));
    readyBit = 0x04000000;
    firstMask = readyBit;
    asm volatile("" : "+r"(firstMask) : "r"(status));
    if (!(status & firstMask)) {
        do {
            if (Gpu_DmaTimeoutCheck()) return -1;
        } while (!(*g_GpuDmaRegisterWindow.gp1 & 0x04000000));
    }
    command = 0xA0000000;
    asm volatile("" : "+r"(command));
    *g_GpuDmaRegisterWindow.gp1 = 0x04000000;
    *g_GpuGp0Ptr = 0x01000000;
    port = g_GpuGp0Ptr;
    if (variant) command = 0xB0000000;
    *port = command;
    *g_GpuGp0Ptr = ((GpuRectWords *)rect)->position;
    *g_GpuGp0Ptr = ((GpuRectWords *)rect)->size;
    --remainder;
    firstEnd = -1;
    /* The loop materializes its own sentinel after this first test. */
    asm volatile("" : "+r"(firstEnd) : "r"(remainder));
    if (remainder != firstEnd) {
        do {
            *g_GpuGp0Ptr = *data++;
        } while (--remainder != -1);
    }
    if (blocks) {
        *g_GpuDmaRegisterWindow.gp1 = 0x04000002;
        *g_GpuDmaRegisterWindow.madr = (unsigned int)data;
        *g_GpuDmaRegisterWindow.bcr = (blocks << 16) | 16;
        *g_GpuDmaRegisterWindow.chcr = 0x01000201;
    }
    return 0;
}

/* Read VRAM with a CPU prefix followed by 16-word DMA blocks. */
int Gpu_FlushDisplayBuffer(RECT *rect, unsigned int *data)
{
    int words, remainder, blocks;
    unsigned int commandStatus;
    unsigned int readStatus;
    unsigned int firstMask;
    int firstEnd;
    unsigned short width, height;
    Gpu_ResetDmaWaitTimer();
    if (rect->w >= 0) {
        if (g_GpuMaxWidth < rect->w) width = g_GpuMaxWidth;
        else width = rect->w;
    } else width = 0;
    rect->w = width;
    if (rect->h >= 0) {
        if (g_GpuMaxHeight < rect->h) height = g_GpuMaxHeight;
        else height = rect->h;
    } else height = 0;
    rect->h = height;
    words = (rect->w * rect->h + 1) / 2;
    if (words <= 0) return -1;
    remainder = words % 16;
    asm volatile("" : "+r"(rect));
    blocks = words / 16;
    commandStatus = *g_GpuDmaRegisterWindow.gp1;
    /* The initial test and retry loop materialize their masks separately. */
    asm volatile("" : "+r"(commandStatus));
    firstMask = 0x04000000;
    asm volatile("" : "+r"(firstMask) : "r"(commandStatus));
    commandStatus &= firstMask;
    if (!commandStatus) {
        do {
            if (Gpu_DmaTimeoutCheck()) return -1;
        } while (!(*g_GpuDmaRegisterWindow.gp1 & 0x04000000));
    }
    *g_GpuDmaRegisterWindow.gp1 = 0x04000000;
    *g_GpuGp0Ptr = 0x01000000;
    *g_GpuGp0Ptr = 0xC0000000;
    *g_GpuGp0Ptr = ((GpuRectWords *)rect)->position;
    *g_GpuGp0Ptr = ((GpuRectWords *)rect)->size;
    readStatus = *g_GpuDmaRegisterWindow.gp1;
    asm volatile("" : "+r"(readStatus));
    firstMask = 0x08000000;
    asm volatile("" : "+r"(firstMask) : "r"(readStatus));
    readStatus &= firstMask;
    if (!readStatus) {
        do {
            if (Gpu_DmaTimeoutCheck()) return -1;
        } while (!(*g_GpuDmaRegisterWindow.gp1 & 0x08000000));
    }
    --remainder;
    firstEnd = -1;
    /* Keep the retry loop's sentinel independent of the initial test. */
    asm volatile("" : "+r"(firstEnd) : "r"(remainder));
    if (remainder != firstEnd) {
        do {
            *data++ = *g_GpuGp0Ptr;
        } while (--remainder != -1);
    }
    if (blocks) {
        *g_GpuDmaRegisterWindow.gp1 = 0x04000003;
        *g_GpuDmaRegisterWindow.madr = (unsigned int)data;
        *g_GpuDmaRegisterWindow.bcr = (blocks << 16) | 16;
        *g_GpuDmaRegisterWindow.chcr = 0x01000200;
    }
    return 0;
}

void Gpu_WriteControlAndMirror(unsigned int value) {
    unsigned int index;

    *g_GpuGp1Ptr = value;
    index = value >> 24;
    g_GpuControlRegMirror[index] = value;
}

int Gpu_GetControlMirrorByte(int index) {
    return g_GpuControlRegMirror[index];
}

int Gpu_WriteDataWords(unsigned int *src, int count) {
    int remaining;
    remaining = count - 1;
    *g_GpuGp1Ptr = 0x4000000;
    if (count != 0) {
        do {
            *g_GpuGp0Ptr = *src++;
            remaining--;
        } while (remaining != -1);
    }
    return 0;
}

void Gpu_StartDmaTransfer(unsigned int value) {
    *g_GpuGp1Ptr = 0x4000002;
    *g_GpuDmaMadrPtr = value;
    *g_GpuDmaBcrPtr = 0;
    *g_GpuDmaChcrPtr = 0x1000401;
}
