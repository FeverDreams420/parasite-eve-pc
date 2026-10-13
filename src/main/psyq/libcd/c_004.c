/* Psy-Q LIBCD C_004.OBJ: data_ready_callback, StGetBackloc. */
/* Complete Psy-Q LIBCD C_004.OBJ code; trailing object alignment is in the manifest. */

#include "pe1/psyq_cd.h"

void data_ready_callback(void) {
    StHEADER *base =
        (StHEADER *)(g_CdRingBufPtr + (g_CdStreamRingReadSlot * sizeof(StHEADER)));
    int sector_data;
    int next_slot;
    DsCallback callback;

    base->id = 2;
    __builtin_memcpy(&g_StLastFrameLoc, &base->loc, 4);
    sector_data = base->frameCount;
    /* Retain the frame load before fetching the next ring slot. */
    next_slot = g_CdStreamRingWriteSlot;
    callback = g_StrDataReadyCallback;
    g_StLastFrameNumber = sector_data;
    g_CdStreamRingReadSlot = next_slot;
    if (callback != 0) {
        callback();
    }
    g_CdStreamDataReadyFlag = 0;
}

int StGetBackloc(CdlLOC *arg0) {
    CdlLOC *saved_arg = arg0;

    if (g_DsStreamNoLocFlag != 0) {
        return -1;
    }

    CdIntToPos(CdPosToInt(&g_StLastFrameLoc) + 1, saved_arg);
    return g_StLastFrameNumber;
}
