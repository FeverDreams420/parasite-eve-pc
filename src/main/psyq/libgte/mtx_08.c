/* PSY-Q LIBGTE MTX_08: ScaleMatrix. */

#include "pe1/gte_types.h"

/* Packed-word scaling preserves the original full-word padding overwrite.
 * The empty x -> l constraint binds the low word of the C unsigned product
 * to LO. It keeps MULTU instead of GCC's low-only MULT and emits no opcode.
 * Register pins and sequencing barriers are counted in crutch debt. */
GteMatrix *ScaleMatrix(GteMatrix *m, const GteVector *scale) {
    s32 sx;
    s32 sy;
    s32 sz;
    s32 packed;
    s32 lo;
    s32 hi;
    unsigned long long product;
    u32 low;
    u32 *words = (u32 *)m;
    sx = scale->x;
    sy = scale->y;
    sz = scale->z;
    packed = words[0];
    lo = packed & 0xffff;
    lo = lo << 16;
    lo >>= 16;
    product = GTE_MULTU(lo, sx);
    lo = low;
    lo >>= 12;
    lo &= 0xffff;
    hi = packed >> 16;
    product = GTE_MULTU(hi, sy);
    hi = low;
    hi >>= 12;
    hi = hi << 16;
    lo |= hi;
    words[0] = lo;
    packed = words[1];
    lo = packed & 0xffff;
    lo = lo << 16;
    lo >>= 16;
    product = GTE_MULTU(lo, sz);
    lo = low;
    lo >>= 12;
    lo &= 0xffff;
    hi = packed >> 16;
    product = GTE_MULTU(hi, sx);
    hi = low;
    hi >>= 12;
    hi = hi << 16;
    lo |= hi;
    words[1] = lo;
    packed = words[2];
    lo = packed & 0xffff;
    lo = lo << 16;
    lo >>= 16;
    product = GTE_MULTU(lo, sy);
    lo = low;
    lo >>= 12;
    lo &= 0xffff;
    hi = packed >> 16;
    product = GTE_MULTU(hi, sz);
    hi = low;
    hi >>= 12;
    hi = hi << 16;
    lo |= hi;
    words[2] = lo;
    packed = words[3];
    lo = packed & 0xffff;
    lo = lo << 16;
    lo >>= 16;
    product = GTE_MULTU(lo, sx);
    lo = low;
    lo >>= 12;
    lo &= 0xffff;
    hi = packed >> 16;
    product = GTE_MULTU(hi, sy);
    hi = low;
    hi >>= 12;
    hi = hi << 16;
    lo |= hi;
    words[3] = lo;
    packed = words[4];
    lo = packed & 0xffff;
    lo = lo << 16;
    lo >>= 16;
    product = GTE_MULTU(lo, sz);
    lo = low;
    lo >>= 12;
    words[4] = lo;
    return (GteMatrix *)words;
}
