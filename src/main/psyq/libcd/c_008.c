/* Psy-Q LIBCD C_008.OBJ: init_ring_status. */

#include "pe1/psyq_cd.h"

void init_ring_status(int start, u32 count) {
    u32 i;

    i = 0;
    if (count != 0) {
        do {
            {
                StHeaderAddress at;
                at.header = StRingAddr;
                at.word += (i + start) << 5;
                *at.statusWord = 0;
            }
            i++;
        } while (i < count);
    }
}
