/* GAS_VERSION: 2.8.1 */
/* PSY-Q LIBSPU SPU, part 3 of 3: _spu_FsetRXX, _spu_FsetRXXa, _spu_FgetRXXa,
 * _spu_FsetPCR, _spu_FsetDelayW, _spu_FsetDelayR, _spu_Fw1ts. */

#include "pe1/psyq_spu_internal.h"

void _spu_FsetRXX(u32 offset, u32 value, u32 mode) {
    if (mode == 0) {
        ((u16 *)_spu_RXX)[offset] = value;
    } else {
        ((u16 *)_spu_RXX)[offset] =
            value >> _spu_mem_mode.shift;
    }
}

u32 _spu_FsetRXXa(s32 arg0, u32 value) {
    s32 offset;
    register u32 shifted asm("$7");
    u32 rem;
    u32 unit;
    u32 alignment;
    u32 ret;

    offset = arg0;

    if (_spu_mem_mode.mode != 0) {
        unit = _spu_mem_mode.bytes;
        rem = value % unit;
        if (rem != 0) {
            value += unit;
            alignment = ~_spu_mem_mode.mask;
            value &= alignment;
        }
    }

    alignment = _spu_mem_mode.shift;
    shifted = value >> alignment;
    asm volatile("" : "=r"(shifted) : "0"(shifted));
    ret = shifted;

    switch (offset) {
    case -1:
        return ret & 0xFFFF;
    case -2:
        return value;
    default:
    {
        ((u16 *)_spu_RXX)[offset] = shifted;
        return value;
    }
    }
}

u32 _spu_FgetRXXa(u32 offset, s32 mode)
{
    u16 value = ((volatile u16 *)_spu_RXX)[offset];
    if (mode == -1) {
        return value;
    }
    return value << _spu_mem_mode.shift;
}

void _spu_FsetPCR(s32 high_priority) {
    *_spu_sys_pcr &= 0xFFF8FFFF;

    if (high_priority) {
        *_spu_sys_pcr |= 0x30000;
    } else {
        *_spu_sys_pcr |= 0x50000;
    }
}

void _spu_FsetDelayW(void) {
    *_spu_delay = (*_spu_delay & 0xF0FFFFFF) | 0x20000000;
}

void _spu_FsetDelayR(void) {
    *_spu_delay = (*_spu_delay & 0xF0FFFFFF) | 0x22000000;
}


void _spu_Fw1ts(void) {
    volatile int i;
    volatile u32 value;

    value = 13;
    for (i = 0; i < 60; i = i + 1) {
        value = value * 13;
    }
}
