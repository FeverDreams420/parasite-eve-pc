/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -mno-split-addresses */
/* PSY-Q LIBSPU S_SRMT: SpuSetReverbModeType. */

#include "common.h"
#include "pe1/psyq_spu_internal.h"

int SpuSetReverbModeType(u32 mode) {
    SpuReverbRegisterAttrs attr;
    int clear = 0;
    u32 enabled;
    u32 *area;
    unsigned char *src;
    unsigned char *dst;
    int i;
    int result;
    if (mode & 0x100) {
        mode &= ~0x100;
        clear = 1;
    }
    if (mode >= 10) return -1;
    result = mode << 2;
    area = (u32 *)((unsigned char *)g_SpuReverbWorkAreaTable + result);
    result = _SpuIsInAllocateArea_(*area);
    dst = (unsigned char *)&attr;
    if (result) return -1;
    g_SpuReverbState.mode = mode;
    _spu_rev_offsetaddr = *area;
    src = (unsigned char *)&g_SpuReverbPresetRegisters[mode];
    for (i = sizeof(attr) - 1; i != -1; i--) {
        unsigned char byte = *src++;
        *dst++ = byte;
    }
    attr.mask = 0;
    switch (mode) {
    case 7:
        g_SpuReverbState.feedback = 0x7F;
        g_SpuReverbState.delay = 0x7F;
        break;
    case 8:
        g_SpuReverbState.feedback = 0;
        g_SpuReverbState.delay = 0x7F;
        break;
    default:
        g_SpuReverbState.feedback = 0;
        g_SpuReverbState.delay = 0;
        break;
    }
    {
        u32 current = (_spu_RXX->spucnt >> 7) & 1;
        enabled = current;
        if (current) _spu_RXX->spucnt &= ~0x80;
    }
    _spu_RXX->reverb_volume_left = 0;
    _spu_RXX->reverb_volume_right = 0;
    g_SpuReverbState.depth.left = 0;
    g_SpuReverbState.depth.right = 0;
    _spu_setReverbAttr(&attr);
    if (clear) SpuClearReverbWorkArea(mode);
    _spu_FsetRXX(0xD1, _spu_rev_offsetaddr, 0);
    if (enabled) _spu_RXX->spucnt |= 0x80;
    return 0;
}
