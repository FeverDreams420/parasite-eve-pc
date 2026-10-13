/* ASSEMBLER: GNU */
/* PSY-Q LIBSPU S_SI: SpuSetIRQ. */

#include "pe1/psyq_bios.h"
#include "pe1/psyq_spu_internal.h"

#include "s_si_externs.h"

long SpuSetIRQ(long mode) {
    u32 timer;

    if (mode == 0 || mode == 3) {
        _spu_RXX->spucnt &= ~0x40;
        timer = 0;
        while (_spu_RXX->spucnt & 0x40) {
            if (++timer > 0xF00u) {
                printf(g_SpuTimeoutFormatIrq, g_SpuWaitIrqOnText);
                return -1;
            }
        }
    }

    if (mode == 1 || mode == 3) {
        _spu_RXX->spucnt |= 0x40;
        timer = 0;
        while (!(_spu_RXX->spucnt & 0x40)) {
            if (++timer > 0xF00u) {
                printf(g_SpuTimeoutFormatIrq, g_SpuWaitIrqOffText);
                return -1;
            }
        }
    }

    return mode;
}
