/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -mno-split-addresses */
/* PSY-Q LIBSPU S_R: SpuRead. */

#include "common.h"
#include "pe1/psyq_spu_internal.h"

u32 SpuRead(void *address, u32 size) {
    if (size > 0x7EFF0) {
        size = 0x7EFF0;
    }

    _spu_Fr(address, size);
    if (_spu_transferCallback == 0) {
        _spu_inTransfer = 0;
    }
    return size;
}
