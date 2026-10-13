/* Psy-Q LIBCD BIOS.OBJ, part 1 of 7: getintr. BIOS stays in seven units:
 * neighbouring parts alternate between GCC 2.7.2 (bios_2.c, bios_4.c,
 * bios_5.c) and GCC 2.8.1 (bios.c, bios_3.c, bios_6.c, bios_7.c), and
 * bios_2.c, bios_3.c, bios_4.c and bios_6.c each need their own
 * -fno-expensive-optimizations or scheduling options. */

#include "pe1/psyq_bios.h"
#include "pe1/psyq_cd.h"

#include "bios_externs.h"
#include "bios_internal.h"

int getintr(void) {
    volatile u8 interrupt;
    volatile u8 result[8];
    int count;
    int errors = 0;
    int i;
    CdInterruptEvents *events;
    *g_CdRegIndexBase = 1;
    interrupt = *g_CdRegResponse & 7;
    if (interrupt == 0) return 0;
    while (interrupt != (*g_CdRegResponse & 7)) interrupt = *g_CdRegResponse & 7;
    for (count = 0; count < 8; count++) {
        if (!(*g_CdRegIndexBase & 0x20)) break;
        result[count] = *g_CdRegPort1;
    }
    for (i = count; i < 8; i++) result[i] = 0;
    *g_CdRegIndexBase = 1;
    *g_CdRegResponse = 7;
    *g_CdRegDataWrite = 7;
    if (interrupt != 3 || g_CdCommandTables.update_status_on_ack[g_CdLastCom]) {
        if (!(g_CdStatus & 0x10) && (result[0] & 0x10)) g_CdLidOpenCount++;
        g_CdStatus = result[0];
        errors = g_CdStatus & 0x1D;
        g_CdStatus1 = result[1];
    }
    if (interrupt == 5 && CD_debug > 0) {
        printf(g_CdDiskErrorText);
        if (CD_debug > 0)
            printf(g_CdDiskErrorDetailFormat, g_CdCommandNames[g_CdLastCom], g_CdStatus, g_CdStatus1);
    }
    switch (interrupt) {
    case 3:
        if (errors) {
            g_CdInterruptEvents.sync = 5;
            copy_result(g_CdSyncResult, result);
            return 2;
        }
        if (g_CdCommandAckOnly[g_CdLastCom]) {
            g_CdInterruptEvents.sync = 3;
            copy_result(g_CdSyncResult, result);
            return 1;
        }
        g_CdInterruptEvents.sync = 2;
        copy_result(g_CdSyncResult, result);
        return 2;
    case 2:
        g_CdInterruptEvents.sync = errors ? 5 : 2;
        copy_result(g_CdSyncResult, result);
        return 2;
    case 1:
        if (errors && count == 1) errors = 0;
        events = &g_CdInterruptEvents;
        events->ready = errors ? 5 : 1;
        copy_result(g_CdReadyResult, result);
        *g_CdRegIndexBase = 0;
        *g_CdRegResponse = 0;
        return 4;
    case 4: {
        CdInterruptEvents *endEvents = &g_CdInterruptEvents;
        endEvents->end = 4;
        endEvents->ready = endEvents->end;
        copy_result(g_CdEndResult, result);
        copy_result(g_CdReadyResult, result);
        return 4;
    }
    case 5: {
        CdInterruptEvents *errorEvents = &g_CdInterruptEvents;
        errorEvents->ready = 5;
        errorEvents->sync = errorEvents->ready;
        copy_result(g_CdSyncResult, result);
        copy_result(g_CdReadyResult, result);
        return 6;
    }
    default:
        puts(g_CdUnknownIntrText);
        printf(g_CdUnknownIntrFormat, interrupt);
        return 0;
    }
}
