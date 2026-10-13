/* Psy-Q LIBDS DSSYS_2.OBJ, part 2 of 3: DsInit, DsReset. */

#include "pe1/cdrom.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"
#include "pe1/psyq_ds_queue.h"

#include "dssys_2_2_externs.h"

/* These adjacent retail functions share DS queue reset state:
 * DsInit at 0x8007EC14 and DsReset at
 * 0x8007ED58. Their combined GNU 2.8.1 object preserves both code ranges. */
int DsInit(void) {
    int i, j, k, offset;
    CdQueuedCmdSlot *state;
    DsCallbackRegistry *callbacks;
    if (DS_system_active()) return 0;
    i = 0;
    callbacks = &g_DsReadCallbackState;
    callbacks->sync = 0;
    callbacks->ready = 0;
    callbacks->start = 0;
    state = g_CdQueuedCmdSlots;
    state[2].state = 0;
    state[1].state = 0;
    state[0].state = 0;
    state[2].result = 0;
    state[1].result = 0;
    state[0].result = 0;
    for (; i < 8; ++i) {
        g_CdQueuedCmdSlots[0].payload[i] = 0;
        g_CdQueuedCmdSlots[1].payload[i] = 0;
        g_CdQueuedCmdSlots[2].payload[i] = 0;
    }
    for (j = 0; j < 8; ++j) CQ_clear_queue(&g_CdDsReadQueue[j]);
    g_CdDsReadIndex = 0;
    g_CdDsReadQueueState = 0;
    g_CdPendingReadCount = 0;
    for (k = 7, offset = 112; k >= 0; --k, offset -= 16)
        ((DsReadCallbackSlot *)((unsigned char *)g_DsReadCallbackSlots + offset))->value = 0;
    g_DsReadCallbackCursor = 0;
    DS_init();
    DS_sync_callback((DsEventCallback)CQ_sync_system);
    DS_ready_callback((DsEventCallback)CQ_ready_system);
    DS_start_callback((DsEventCallback)LIBDS_DSSYS_2_text_13CC);
    DS_vsync_callback(CQ_vsync_system);
    ER_clear();
    DsReadySystemMode(0);
    return 1;
}

int DsReset(void) {
    int i, j, k, offset;
    CdQueuedCmdSlot *state;
    DsCallbackRegistry *callbacks;
    DS_stop();
    i = 0;
    callbacks = &g_DsReadCallbackState;
    callbacks->sync = 0;
    callbacks->ready = 0;
    callbacks->start = 0;
    state = g_CdQueuedCmdSlots;
    state[2].state = 0;
    state[1].state = 0;
    state[0].state = 0;
    state[2].result = 0;
    state[1].result = 0;
    state[0].result = 0;
    for (; i < 8; ++i) {
        g_CdQueuedCmdSlots[0].payload[i] = 0;
        g_CdQueuedCmdSlots[1].payload[i] = 0;
        g_CdQueuedCmdSlots[2].payload[i] = 0;
    }
    for (j = 0; j < 8; ++j) CQ_clear_queue(&g_CdDsReadQueue[j]);
    g_CdDsReadIndex = 0;
    g_CdDsReadQueueState = 0;
    g_CdPendingReadCount = 0;
    for (k = 7, offset = 112; k >= 0; --k, offset -= 16)
        *(int *)((unsigned char *)g_DsReadCallbackSlots + offset) = 0;
    g_DsReadCallbackCursor = 0;
    ER_clear();
    DsReadySystemMode(0);
    DS_restart();
    return 1;
}
