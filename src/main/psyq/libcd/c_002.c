/* Psy-Q LIBCD C_002.OBJ: StClearRing. */

#include "pe1/psyq_cd.h"

void StClearRing(void) {
    u32 size = StRingSize;

    g_CdStreamRingIndex = 0;
    g_CdStreamRingReadSlot = 0;
    g_CdStreamRingWriteSlot = 0;
    g_CdStreamDataReadyFlag = 0;
    init_ring_status(0, size);
    g_CdStreamReady.word = 0;
    g_StFrameSectorIndex = 0;
    g_StCurrentFrameNumber = 0;
}
