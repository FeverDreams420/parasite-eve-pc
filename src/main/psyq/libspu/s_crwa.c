/* PSY-Q LIBSPU S_CRWA: SpuClearReverbWorkArea. */
/* SPDX-License-Identifier: MIT */
/* Adapted from sozud/psy-q-decomp src/spu/s_crwa.c; see THIRD_PARTY_NOTICES.md. */
#include "common.h"
#include "pe1/psyq_spu_internal.h"

#include "s_crwa_externs.h"

int _spu_t(int, ...);
int WaitEvent(int);

int SpuClearReverbWorkArea(int mode)
{
    SpuCallback volatile callback;
    s32 oldTransmode;
    s32 address;
    s32 more;
    s32 transmodeCleared;
    u32 size;
    u32 length;

    callback = 0;
    transmodeCleared = 0;
    if (mode >= 0xAU) {
        return -1;
    }
    {
        u32 offset;
        SpuReverbEntryAddress base;
        base.entry = g_SpuReverbWorkAreaTable;
        offset = mode << 2;
        /* Reuse the address temporary for the remaining byte count below. */
        length = offset + base.word;
    }
    if (_SpuIsInAllocateArea_(*(u32 *)length) != 0) {
        return -1;
    }
    if (mode == 0) {
        length = 0x10 << _spu_mem_mode.shift;
        address = 0xFFF0 << _spu_mem_mode.shift;
    } else {
        u32 start = *(u32 *)length;
        length = (0x10000 - start) << _spu_mem_mode.shift;
        address = start << _spu_mem_mode.shift;
    }
    oldTransmode = _spu_transMode;
    if (_spu_transMode == 1) {
        _spu_transMode = 0;
        /* Materialize the restore flag independently of the comparison value. */
        transmodeCleared = 1;
    }
    more = 1;
    if (_spu_transferCallback != 0) {
        callback = _spu_transferCallback;
        _spu_transferCallback = 0;
    }
    do {
        size = 0x400;
        if (length <= 0x400U) {
            size = length;
            more = 0;
        }
        _spu_t(2, address);
        _spu_t(1);
        _spu_t(3, &g_SpuZeroBuffer, size);
        WaitEvent(_spu_EVdma);
        length -= 0x400;
        address += 0x400;
    } while (more != 0);
    if (transmodeCleared != 0) {
        _spu_transMode = oldTransmode;
    }
    if (callback != 0) {
        _spu_transferCallback = callback;
    }
    return 0;
}
