/* Psy-Q LIBETC INTR_DMA.OBJ: startIntrDMA, trapIntrDMA, setIntrDMA, memclrIntrDMA. */

#include "pe1/psyq_api_internal.h"
#include "pe1/psyq_bios.h"

DmaCallbackSetter startIntrDMA(void) {
    memclrIntrDMA((int *)g_IntrDmaHandlerTable, 8);
    *g_IntrDmaDispatchPtr = 0;
    InterruptCallback(3, trapIntrDMA);
    return setIntrDMA;
}

#define DMA_STATUS (*(volatile u32 *)g_IntrDmaDispatchPtr)

void trapIntrDMA(void) {
    u32 mask;
    int i;
    mask = (DMA_STATUS >> 24) & 0x7F;
    if (mask != 0) {
        u32 bit = 1;
        u32 preserve = 0xFFFFFF;
        DmaInterruptCallback *base;
        base = g_IntrDmaHandlerTable;
        do {
            for (i = 0; mask != 0 && i < 7; i++, mask >>= 1) {
                if (mask & 1) {
                    DMA_STATUS &= (bit << (i + 24)) | preserve;
                    if (base[i] != 0) {
                        base[i]();
                    }
                }
            }
            mask = (DMA_STATUS >> 24) & 0x7F;
        } while (mask != 0);
    }
    if ((DMA_STATUS & 0xFF000000) == 0x80000000 || (DMA_STATUS & 0x8000)) {
        printf(g_DmaBusErrorFormat, DMA_STATUS);
        for (i = 0; i < 7; i++) {
            printf(g_DmaMadrFormat, i, g_DmaChannelRegsPtr[i].address);
        }
    }
}

DmaInterruptCallback setIntrDMA(int channel, DmaInterruptCallback callback) {
    DmaInterruptCallback callbackReg;
    DmaInterruptCallback *base;
    DmaInterruptCallback *slot;
    DmaInterruptCallback old;
    DmaInterruptCallback ret;
    volatile u32 *dmaReg;
    u32 tmp;
    callbackReg = callback;
    base = g_IntrDmaHandlerTable;
    slot = &base[channel];
    old = *slot;
    ret = old;

    if (callbackReg != old) {
        if (callbackReg != 0) {
            int bitSet;
            u32 maskSet;

            dmaReg = g_IntrDmaDispatchPtr;
            tmp = 0xFFFFFF;
            *slot = callbackReg;
            maskSet = *dmaReg;
            bitSet = channel + 0x10;
            maskSet &= tmp;
            tmp = 1;
            tmp <<= bitSet;
            bitSet = 0x800000;
            tmp |= bitSet;
            maskSet |= tmp;
            *dmaReg = maskSet;
            ret = old;
            return ret;
        } else {
            u32 maskClear;

            dmaReg = g_IntrDmaDispatchPtr;
            tmp = 0xFFFFFF;
            *slot = 0;
            maskClear = *dmaReg;
            maskClear &= tmp;
            tmp = 0x800000;
            maskClear |= tmp;
            tmp = ~(1U << (channel + 0x10));
            maskClear &= tmp;
            *dmaReg = maskClear;
            ret = old;
            return ret;
        }
    }

    return ret;
}

void memclrIntrDMA(int *ptr, int count) {
    int i = count - 1;

    if (count != 0) {
        do {
            *ptr = 0;
            i--;
            ptr++;
        } while (i != -1);
    }
}
