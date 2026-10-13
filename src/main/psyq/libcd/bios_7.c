/* ASSEMBLER: GNU */
/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -mno-split-addresses */
/* Psy-Q LIBCD BIOS.OBJ, part 7 of 7: CD_getsector, CD_getsector2,
 * CD_set_test_parmnum and callback. */

#include "pe1/psyq_cd.h"

int CD_getsector(void *destination, int words) {
    volatile u8 *status;
    u32 ready;

    *g_CdRegIndexBase = 0;
    *g_CdRegResponse = 0x80;
    *g_CdRegDelaySize = 0x20943;
    *g_CdRegRequest = 0x1323;
    *g_CdRegDmaPcr |= 0x8000;
    *g_CdRegDmaMadr = destination;
    *g_CdRegDmaBcr = words | 0x10000;

    status = g_CdRegIndexBase;
    do {
        ready = *status & 0x40;
    } while (!ready);

    *g_CdRegDmaControl = 0x11000000;
    {
        volatile u32 *control = g_CdRegDmaControl;
        if (*control & 0x1000000) {
            volatile u32 *pending = control;
            u32 mask = 0x1000000;
            do {
            } while (*pending & mask);
        }
    }

    {
        volatile u32 *request = g_CdRegRequest;

        *request = 0x1325;
    }
    return 0;
}

int CD_getsector2(void *destination, int words) {
    volatile u32 readback;
    volatile u8 *index;

    *g_CdRegIndexBase = 0;
    *g_CdRegResponse = 0x80;
    *g_CdRegDelaySize = 0x21020843;
    *g_CdRegRequest = 0x1325;
    *g_CdRegDmaPcr |= 0x8000;
    *g_CdRegDmaMadr = destination;
    *g_CdRegDmaBcr = words | 0x10000;
    index = g_CdRegIndexBase;
    if (!(*index & 0x40)) {
        do {
        } while (!(*index & 0x40));
    }
    *g_CdRegDmaControl = 0x11400100;
    readback = *g_CdRegDmaControl;
    return 0;
}

void CD_set_test_parmnum(int status) {
    g_CdCommandTables.parameter_counts[0x19] = status; /* CdlTest */
}

void callback(void) {
    int pending;
    volatile u8 *ready = &g_CdInterruptEvents.ready;
    int bank = *g_CdRegIndexBase & 3;
    volatile u8 *complete = ready - 1;

    while ((pending = getintr()) != 0) {
        if ((pending & 4) && g_CdReadyCallback) {
            g_CdReadyCallback(*ready, g_CdReadyResult);
        }
        if ((pending & 2) && g_CdSyncCallback) {
            g_CdSyncCallback(*complete, g_CdSyncResult);
        }
    }

    *g_CdRegIndexBase = bank;
}
