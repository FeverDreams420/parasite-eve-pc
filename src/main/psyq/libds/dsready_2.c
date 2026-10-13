/* Psy-Q LIBDS DSREADY.OBJ, part 2 of 2: ER_cbready, LIBDS_DSREADY_text_3D8,
 * ER_retry, ER_cbsync, ER_active, ER_clear. */

#include "pe1/cdrom.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"
#include "pe1/psyq_ds_queue.h"

#include "dsready_2_externs.h"

#define ASYNC_FROM_RETRY(p)                                                            \
    ((DsAsyncReadState *)((u8 *)(p) - PE1_OFFSETOF(DsAsyncReadState, retryPending)))
void ER_cbready(int inEvent, u8 *inResult) {
    u8 detail[12];
    int event;
    u8 *result = inResult;
    DsAsyncReadState *state;
    int one;
    /* Order prologue saves. This temporary output is used only by the next
     * empty constraint, then replaced with the incoming event. */
    state = &g_DsAsyncReadState;
    event = inEvent;
    if (state->nextSector == -1)
        state->nextSector = DsPosToInt(DS_lastpos());
    event = (u8)event;
    one = 1;
    if (event == one) {
        if (DS_lastmode() & 32) {
            {
                DsCallback cb = DsDataCallback(0);
                void *dest = detail;
                int count = 3;
                event = (int)cb;
                DsGetSector(dest, count);
            }
            DsDataCallback((DsCallback)event);
            event = DsPosToInt((CdlLOC *)detail);
            if (event != state->nextSector)
                goto retry;
            if (state->callback && state->lastDeliveredSector < event) {
                state->callback(1, result, detail);
                state->lastDeliveredSector = event;
            }
        } else {
            if (state->callback) {
                int sector = state->nextSector;
                if (state->lastDeliveredSector < sector) {
                    state->callback(1, result, detail);
                    state->lastDeliveredSector = state->nextSector;
                }
            }
        }
        {
            DsAsyncReadState *current = &g_DsAsyncReadState;
            current->nextSector++;
        }
    } else if (event == 4) {
        if (state->callback)
            state->callback(4, result, detail);
    } else {
        if (*result & 16) {
            if (state->reserved1C == one) {
                DsReadyCallback(0);
                return;
            }
            DsReadyCallback(state->savedReadyCallback);
            DsStartCallback(state->savedStartCallback);
            state->active = 0;
            if (state->callback)
                state->callback(event, result, detail);
            return;
        } else if (!DsQueueLen() && !(*result & 160)) {
        retry:
            state->retryPending = one;
        }
    }
    {
        int *pending = &g_DsAsyncReadRetryPending;
        if (*pending == 1) {
            if (ASYNC_FROM_RETRY(pending)->retriesRemaining > 0 ||
                ASYNC_FROM_RETRY(pending)->retriesRemaining == -1) {
                ER_retry();
                if (ASYNC_FROM_RETRY(pending)->retriesRemaining > 0)
                    --ASYNC_FROM_RETRY(pending)->retriesRemaining;
            } else {
                if (g_DsReadBusy == *pending) {
                    DsReadyCallback(g_DsAsyncReadSavedReadyCallback);
                    DsStartCallback(g_DsAsyncReadSavedStartCallback);
                    DsCommand(9, 0, 0, -1);
                }
                g_DsReadBusy = 0;
                if (ASYNC_FROM_RETRY(pending)->callback)
                    ASYNC_FROM_RETRY(pending)->callback(5, result, detail);
            }
            g_DsAsyncReadRetryPending = 0;
        }
    }
done:;
}

void LIBDS_DSREADY_text_3D8(unsigned char arg0, unsigned char *arg1) {
    DsCallback *state;
    int status;
    DsCallback *active;
    void *data;
    DsAsyncReadCallback callback;

    state = &g_DsReadySystemCallback;
    active = state + 1;
    data = arg1;
    if (state[0] == 0) {
        return;
    }
    /* The loop ends cse's jump-following path here, so GCC 2.7.2 keeps
     * the state address in $s0 for the field reads below. */
    do {
        if (state[1] == 0) {
            return;
        }
    } while (0);

    status = arg0;
    if (status == 2) {
        if (DsQueueLen() == 0) {
            ER_retry();
        }
        return;
    }

    DsReadyCallback(DS_ASYNC_READ_FIELD(active, savedReadyCallback));
    DsStartCallback(DS_ASYNC_READ_FIELD(active, savedStartCallback));
    callback = DS_ASYNC_READ_FIELD(active, callback);
    state[1] = 0;
    if (callback != 0) {
        callback(status, data, 0);
    }
}

int ER_retry(void) {
    int mode;
    CdlLOC *position;
    int command;
    int argumentMode;
    int limit;

    DsReadyCallback(0);
    g_DsAsyncReadState.nextSector = DsPosToInt(DS_lastpos());
    mode = DS_lastmode() & 0xFF;
    position = DS_lastpos();
    command = DS_lastread();
    argumentMode = mode;
    limit = -1;
    return DsPacket(argumentMode, position,
        (unsigned char)command, (DslCB)ER_cbsync, limit);
}

void ER_cbsync(u_char event) {
    if (event == 2) {
        DsReadyCallback((DsEventCallback)ER_cbready);
    }
}

int ER_active(void) {
    return g_DsReadBusy;
}

void ER_clear(void) {
    int *state;

    state = &g_DsReadBusy;
    if (DS_ASYNC_READ_FIELD(state, active) == 1) {
        DsReadyCallback(DS_ASYNC_READ_FIELD(state, savedReadyCallback));
        DsStartCallback(DS_ASYNC_READ_FIELD(state, savedStartCallback));
    }
    DS_ASYNC_READ_FIELD(state, active) = 0;
}
