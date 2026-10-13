/* Psy-Q LIBCD C_005.OBJ: StSetStream. */

#include "pe1/psyq_cd.h"

void StSetStream(u32 mode, u32 startFrame, u32 endFrame,
                 void (*callback1)(void), void (*callback2)(void)) {
    StSetMask(1, startFrame, endFrame);
    g_CdStreamMemorySource = 0;
    g_StrDataReadyCallback = callback1;
    g_StRgb24Mode = mode & 1;
    g_CdStreamExpectedTypeId = 0;
    D_800B6914 = 0;
    g_StFrameSectorIndex = 0;
    g_StCurrentFrameNumber = 0;
    g_StrEndCallback = callback2;
}
