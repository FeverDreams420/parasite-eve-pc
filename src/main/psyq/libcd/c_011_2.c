/* Psy-Q LIBCD C_011.OBJ, part 2 of 2: dma_execute. */

#include "pe1/psyq_bios.h"
#include "pe1/psyq_cd.h"

void dma_execute(int channel, void *address, int blockCount, int blockSize,
                 volatile u32 control, u8 interrupt, int reserved) {
    int i = 0;
    CdDmaInterruptRegister *interruptRegister;
    u32 bits;
    u32 blockControl;
    int shift;
    volatile u32 readback;
    int priorityBit;
    volatile u32 *dma;

    while (*(volatile u32 *)(0x1F801088 + (channel << 4)) & 0x01000000) {
        if (i == 0x10000) {
            printf(g_CdDmaStatusErrorFormat,
                   *(volatile u32 *)(0x1F801088 + (channel << 4)));
            break;
        }
        i++;
    }

    bits = 1;
    if (interrupt == bits) {
        interruptRegister = g_CdDmaInterruptRegPtr;
        bits = interruptRegister->fields.enableControl | (bits << channel);
    } else {
        interruptRegister = g_CdDmaInterruptRegPtr;
        bits = interruptRegister->fields.enableControl & ~(bits << channel);
    }
    interruptRegister->fields.enableControl = bits;
    if (channel) {
        readback = g_CdDmaInterruptRegPtr->word;
    } else {
        readback = g_CdDmaInterruptRegPtr->word;
    }

    shift = channel << 2;
    shift += 3;
    priorityBit = 1 << shift;
    shift = *g_CdDmaControlRegPtr;
    dma = (volatile u32 *)0x1F801080;
    dma += channel << 2;
    blockControl = (blockCount << 16) | blockSize;
    *g_CdDmaControlRegPtr = shift | priorityBit;
    *dma++ = PSX_ADDRESS(address);
    *dma++ = blockControl;

    while (!(*g_CdStIntrIndexRegPtr & 0x40)) {
    }

    if (address) {
        *dma = control;
        readback = *dma;
    } else {
        *dma = control;
        readback = *dma;
    }
}
