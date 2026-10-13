/* Psy-Q LIBDS DSREAD.OBJ, part 2 of 2: DS_read_cbready, DS_read_cbdata, DsReadSync, DsReadCallback, DsReadBreak, DsReadMode. */

#include "pe1/cdrom.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"
#include "pe1/psyq_ds_queue.h"
#include "pe1/psyq_gpu.h"

#include "dsread_2_externs.h"
#include "dsread_2_externs_1.h"

#define CD_READ_FIELD(anchor, field)                                      \
    ((anchor)[(PE1_OFFSETOF(CdReadProgressState, field) -                  \
               PE1_OFFSETOF(CdReadProgressState, currentVsync)) /         \
              sizeof(int)])

void DS_read_cbready(int status, void *data, void *detail) {
    int savedStatus = status;
    int *state = &g_CdReadCurrentVsync;
    CD_READ_FIELD(state, currentVsync) = VSync(-1);
    if (CD_READ_FIELD(state, flags) & 1) {
        if (CD_READ_FIELD(state, remainingSectors) > 0) {
            DsGetSector2(CD_READ_FIELD(state, destination),
                          CD_READ_FIELD(state, sectorSize));
            CD_READ_FIELD(state, eventData) = (int)data;
        } else {
            DsReadBreak();
            if (g_CdReadCompleteCallback) {
                if (CD_READ_FIELD(state, remainingSectors) < 0) savedStatus = 5;
                g_CdReadCompleteCallback((u8)savedStatus, data);
            }
        }
    } else {
        if (CD_READ_FIELD(state, remainingSectors) > 0) {
            DsGetSector(CD_READ_FIELD(state, destination),
                         CD_READ_FIELD(state, sectorSize));
            CD_READ_FIELD(state, destination) +=
                CD_READ_FIELD(state, sectorSize) * 4;
            CD_READ_FIELD(state, remainingSectors)--;
        }
        if (VSync(-1) > CD_READ_FIELD(state, startVsync) + 1200)
            CD_READ_FIELD(state, remainingSectors) = -1;
        if (CD_READ_FIELD(state, remainingSectors) == 0 ||
            VSync(-1) > CD_READ_FIELD(state, startVsync) + 1200) {
            DsReadBreak();
            if (g_CdReadCompleteCallback) {
                savedStatus =
                    CD_READ_FIELD(state, remainingSectors) < 0 ? 5 : 2;
                g_CdReadCompleteCallback((u8)savedStatus, data);
            }
        }
    }
}

#undef CD_READ_FIELD

void DS_read_cbdata(void) {
    int *state = D_8009B6B0;
    int query;
    query = -1;
    /* Keep the clock query independent of the sector decrement. */
    state[0] += state[-1] * 4;
    state[1]--;
    if (VSync(query) > state[5] + 1200) state[1] = -1;
    if (!state[1] || VSync(-1) > state[5] + 1200) {
        DsReadBreak();
        if (g_CdReadCompleteCallback)
            g_CdReadCompleteCallback(state[1] < 0 ? 5 : 2, (void *)state[3]);
    }
}

int DsReadSync(void *argument) {
    int v0;
    int s0;
    int *state;

    v0 = VSync(-1);
    state = &g_CdReadStartVsync;

    if ((state[0] + 0x4B0) < v0) {
        DsReadBreak();
        s0 = -1;
    } else {
        s0 = state[-4];
    }

    DsReady(argument);
    return s0;
}

CdReadCompleteCallback DsReadCallback(CdReadCompleteCallback callback) {
    CdReadCompleteCallback old;

    old = g_CdReadCompleteCallback;
    g_CdReadCompleteCallback = callback;
    return old;
}

void DsFlush(void);
int DS_cw_system(int mode, void *parameter);

void DsReadBreak(void) {
    int *readInProgress;
    int particleType;
    void *zeroArg1;
    DslCB callback;

    readInProgress = &g_CdReadInProgress;

    if (*readInProgress == 1) {
        DsFlush();
        ER_clear();
        if (((CdReadProgressState *)((char *)readInProgress -
            PE1_OFFSETOF(CdReadProgressState, inProgress)))->flags & 1) {
            DsDataCallback(((CdReadProgressState *)((char *)readInProgress -
            PE1_OFFSETOF(CdReadProgressState, inProgress)))->dataCallback);
        }
        DS_cw_system(1, 0);
        particleType = 9;
        zeroArg1 = 0;
        callback = 0;
        DsCommand(particleType, zeroArg1, callback, -1);
    }

    g_CdReadInProgress = 0;
}

void DsReadMode(unsigned int mode) {
    if (mode < 2) {
        g_DsReadMode = mode;
    }
}
