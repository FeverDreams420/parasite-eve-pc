/* Psy-Q LIBGPU SYS.OBJ, part 11 of 11: the private DMA callback restore and
 * GPU_memset. */

#include "pe1/psyq_callbacks.h"
#include "pe1/psyq_gpu.h"

void Gpu_RestoreDmaCallback(void) {
    DMACallback(2, (PsyqInterruptHandler)Gpu_SetDisplayBuffer);
}

void GPU_memset(unsigned char *dst, int value, int count)
{
    int remaining;

    remaining = count - 1;
    if (count != 0) {
        do {
            *dst++ = value;
            remaining--;
        } while (remaining != -1);
    }
}

static unsigned int GPU_memset_alignment[] __attribute__((section(".text"))) = { 0x00000000, 0x00000000 };
