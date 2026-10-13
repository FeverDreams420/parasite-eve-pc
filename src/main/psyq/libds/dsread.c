/* Psy-Q LIBDS DSREAD.OBJ, part 1 of 2: DsRead, DS_read_cbsync. */
/* DSREAD is split in two: these functions match with GCC 2.7.2, the
 * callbacks and controls in dsread_2.c only with GCC 2.8.1 and
 * -mno-split-addresses (2.7.2 cannot keep the shared address base in a
 * register). */

#include "pe1/cdrom.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"
#include "pe1/psyq_ds_queue.h"

#define READ_STATE(anchor, field) ((CdReadProgressState *)((char *)(anchor) - PE1_OFFSETOF(CdReadProgressState, field)))

int DsRead(CdlLOC *position, int sectors, void *destination, int incomingMode) {
    int incoming = incomingMode;
    int mode;
    CdlLOC location;
    int *state;
    int result;
    /* Empty constraints preserve the retail saves and incoming a3 lifetime. */
    state = &g_CdReadInProgress;
    mode = incoming;
    if (*state == 1 || ER_active()) {
        result = 0;
        return result;
    }
    READ_STATE(state, inProgress)->sectorSize = 512;
    READ_STATE(state, inProgress)->destination = (int)destination;
    READ_STATE(state, inProgress)->remainingSectors = sectors;
    /* GCC merges these calls after scheduling the two position copies. */
    if (!position) {
        location = *DsLastPos(0);
        mode |= 0x20;
        mode = DsPacket((u8)mode, &location, 6,
                                         (DslCB)DS_read_cbsync, -1);
    } else {
        location = *position;
        mode |= 0x20;
        mode = DsPacket((u8)mode, &location, 6,
                                         (DslCB)DS_read_cbsync, -1);
    }
    result = 0;
    if (!mode) return result;
    result = VSync(-1);
    state = &g_CdReadStartVsync;
    *state = result;
    if (READ_STATE(state, startVsync)->flags & 1)
        READ_STATE(state, startVsync)->dataCallback = DsDataCallback(DS_read_cbdata);
    READ_STATE(state, startVsync)->inProgress = 1;
    result = mode;
done:
    return result;
}

#undef READ_STATE

void DS_read_cbsync(unsigned char arg0) {
    if (arg0 == 2) {
        DsStartReadySystem(DS_read_cbready, -1);
    }
}
