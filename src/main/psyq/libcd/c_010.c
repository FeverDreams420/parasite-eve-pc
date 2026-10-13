/* Psy-Q LIBCD C_010.OBJ: StSetMask. */

#include "pe1/psyq_cd.h"

void StSetMask(u32 mask, u32 start, u32 end) {
    g_CdStreamMask = mask;
    g_CdStreamStartFrame = start;
    g_CdStreamEndState.endSector = end;
}
