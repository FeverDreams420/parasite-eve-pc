/* Psy-Q LIBCD CDROM.OBJ: StSetRing. */

#include "pe1/psyq_cd.h"

void StSetRing(u32 *ring_addr, u32 ring_size) {
    StRingAddr = (StHEADER *)ring_addr;
    StRingSize = ring_size;
    StClearRing();
}
