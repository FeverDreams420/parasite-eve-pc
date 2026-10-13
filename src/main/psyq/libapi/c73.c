/* Psy-Q LIBAPI C73.OBJ: the GPU_cw BIOS veneer (A0 function 0x49) and its
 * alignment word, linked between LIBGPU SYS and P00. */

#include "pe1/psyq_bios.h"

PSYQ_BIOS_TRAMPOLINE(GPU_cw, 0xA0, 0x49);
