/* ASSEMBLER: GNU */
/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -fno-expensive-optimizations -mno-split-addresses */
/* Psy-Q LIBCD BIOS.OBJ, part 6 of 7: CD_datasync. */

#include "pe1/psyq_bios.h"
#include "pe1/psyq_cd.h"

#include "bios_6_externs.h"

static inline int data_timed_out(const char **commands, const char **events,
                                 int limit, CdInterruptEvents *state) {
    if (VSync(-1) > g_CdTimeoutDeadlineVsync || g_CdTimeoutPollCount++ > limit) {
        puts(g_CdTimeoutText);
        printf(g_CdTimeoutDetailFormat, g_CdTimeoutFuncName, commands[g_CdLastCom],
               events[state->sync], events[state->ready]);
        CD_flush();
        return -1;
    }
    return 0;
}

int CD_datasync(int mode) {
    int saved_mode = mode;
    int limit;
    const char **commands;
    const char **events;
    CdInterruptEvents *state;

    g_CdTimeoutDeadlineVsync = VSync(-1) + 0x3C0;
    g_CdTimeoutPollCount = 0;
    g_CdTimeoutFuncName = g_CdDataSyncName;
    limit = 0x3C0000;
    commands = g_CdCommandNames;
    state = &g_CdInterruptEvents;
    events = g_CdInterruptNames;

    do {
        if (data_timed_out(commands, events, limit, state)) {
            return -1;
        }
        if (!(*g_CdRegDmaControl & 0x1000000)) {
            return 0;
        }
    } while (!saved_mode);

    return 1;
}
