/* ASSEMBLER: GNU */
/* PSY-Q LIBSPU S_STM: SpuSetTransferMode. */

#include "pe1/psyq_spu_internal.h"

int SpuSetTransferMode(int mode) {
    int enabled;
    switch (mode) {
    case 0:
        enabled = 0;
        break;
    case 1:
        enabled = 1;
        break;
    default:
        enabled = 0;
        break;
    }
    _spu_trans_mode = mode;
    _spu_transMode = enabled;
    return enabled;
}
