/* ASSEMBLER: GNU */
/* Psy-Q LIBCD BIOS.OBJ, part 5 of 7: CD_init. */

#include "pe1/psyq_bios.h"
#include "pe1/psyq_cd.h"

#include "bios_5_externs.h"

int CD_init(void) {
    const int zero = 0;
    int result;
    register int cmd asm("$4");
    register int arg asm("$5");

    puts(g_CdInitName);
        printf(g_CdInitAddrFormat, &g_CdBiosInitDescriptor);
    g_CdLastCom = 0;
    g_CdMode = 0;
    g_CdReadyCallback = 0;
    g_CdSyncCallback = 0;
    g_CdStatus1 = 0;
    g_CdStatus = 0;
    ResetCallback();
    InterruptCallback(2, callback);
    *g_CdRegIndexBase = 1;
    while (*g_CdRegResponse & 7) {
        *g_CdRegIndexBase = 1;
        *g_CdRegResponse = 7;
        *g_CdRegDataWrite = 7;
    }
    {
        volatile u8 *index;
        CdInterruptEvents *state;
        cmd = 1;
        arg = zero;
        state = &g_CdInterruptEvents;
        /* Debt: retain one shared event-state base. */
        asm volatile("" : "=r"(state) : "0"(state));

        state->end = 0;
        state->ready = state->end;
        index = g_CdRegIndexBase;
        state->sync = 2;
        *index = 0;
    }
    *g_CdRegResponse = 0;
    *g_CdRegRequest = 0x1325;
    CD_cw(cmd, arg, zero, zero);
    if (g_CdStatus & 0x10) CD_cw(1, zero, zero, zero);
    if (CD_cw(10, zero, zero, zero)) return -1;
    if (!CD_cw(12, zero, zero, zero)) {
        register int status asm("$4") = CD_sync(zero, zero);
        register int two asm("$3") = 2;
        asm volatile("" : "=r"(status), "=r"(two) : "0"(status), "1"(two));
        if (status != two) return -1;
        result = 0;
    } else {
        asm volatile("");
        result = -1;
    }
    return result;
}
