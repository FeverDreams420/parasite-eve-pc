/* Psy-Q LIBCD C_007.OBJ: StFreeRing. */

#include "pe1/psyq_cd.h"

u32 StFreeRing(u32 *ptr) {
    StHEADER *base;
    int index;
    int i;
    int count;
    int limit;
    StHEADER *entry;
    int status;
    int raw_count;
    int expected_status;
    int next_index;
    base = StRingAddr;
    index = (ptr - (u32 *)&base[StRingSize]) / 504;
    entry = &base[index];
    expected_status = 4;
    status = (s16)entry->id;
    raw_count = entry->nSectors;
    if (status != expected_status) {
        return 1;
    }

    count = (s16)raw_count;
    i = 0;
    if (count > 0) {
        limit = count;
        do {
            int slot = i + index;
            i++;
            {
                StHeaderAddress at;
                at.header = StRingAddr;
                at.word += slot << 5;
                *at.status = 0;
            }
        } while (i < limit);
    }

    next_index = i + index;
    g_CdStreamRingIndex = next_index;
    return 0;
}
