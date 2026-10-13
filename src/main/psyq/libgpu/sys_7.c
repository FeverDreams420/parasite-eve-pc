/* Psy-Q LIBGPU SYS.OBJ, part 7 of 11: _param and the private packet submit
 * Gpu_SubmitPacket. */

#include "pe1/gpu_dma_state.h"

#include "sys_7_externs.h"

typedef unsigned long u_long;

int _param(int x) {
    *g_GpuGp1Ptr = x | 0x10000000;
    return *g_GpuGp0Ptr & 0xFFFFFF;
}

void Gpu_SubmitPacket(GpuQueueFunction function, u32 *source, u32 argument) {
    Gpu_SwapDisplayBuffers(function, source, 0, argument);
}
