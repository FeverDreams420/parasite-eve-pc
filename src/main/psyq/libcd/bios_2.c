/* Psy-Q LIBCD BIOS.OBJ, part 2 of 7: CD_sync, CD_ready. */

#include "bios_internal.h"

/* Adjacent command/ready polling routines from LIBCD BIOS_1.OBJ. */

int CD_sync(int mode, u8 *result) {
    u8 *sync;
    u8 *ready;
    const char **commands;
    const char **events;
    g_CdTimeoutDeadlineVsync = VSync(-1) + 0x3C0;
    commands = g_CdCommandNames;
    events = g_CdInterruptNames;
    sync = (u8 *)&g_CdInterruptEvents;
    ready = (u8 *)&((CdInterruptEvents *)sync)->ready;
    g_CdTimeoutPollCount = 0;
    g_CdTimeoutFuncName = g_CdSyncName;
    do {
        int status;
        if (timed_out(commands, events, (CdInterruptEvents *)sync)) {
            return -1;
        }
        if (CheckCallback()) {
            dispatch_interrupts(sync, ready);
        }
        status = ((CdInterruptEvents *)sync)->sync;
        if (status == 2 || status == 5) {
            ((CdInterruptEvents *)sync)->sync = 2;
            copy_result(result, g_CdSyncResult);
            return status;
        }
    } while (!mode);
    return 0;
}

int CD_ready(int mode, u8 *result) {
    u8 *sync;
    u8 *ready;
    volatile u8 *end;
    const char **commands;
    const char **events;
    g_CdTimeoutDeadlineVsync = VSync(-1) + 0x3C0;
    commands = g_CdCommandNames;
    events = g_CdInterruptNames;
    sync = (u8 *)&g_CdInterruptEvents;
    ready = (u8 *)&((CdInterruptEvents *)sync)->ready;
    end = (volatile u8 *)&((CdInterruptEvents *)sync)->end;
    g_CdTimeoutPollCount = 0;
    g_CdTimeoutFuncName = g_CdReadyName;
    do {
        int status;
        if (timed_out(commands, events, (CdInterruptEvents *)sync)) {
            return -1;
        }
        if (CheckCallback()) {
            dispatch_interrupts(sync, ready);
        }
        status = *end;
        if (status) {
            ((CdInterruptEvents *)sync)->end = 0;
            copy_result(result, g_CdEndResult);
            return status;
        }
        status = end[-1];
        if (status) {
            ((CdInterruptEvents *)sync)->ready = 0;
            copy_result(result, g_CdReadyResult);
            return status;
        }
    } while (!mode);
    return 0;
}
