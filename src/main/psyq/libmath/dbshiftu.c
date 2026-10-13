/* PSY-Q LIBMATH DBSHIFTU: _dbl_shift_us. */

#include "pe1/math64.h"

MathU64 *_dbl_shift_us(MathU64 *result, int right, MathU64 value, int amount)
{
    MathU64 *out = result;
    int shift = amount;
    int i;
    unsigned int lo;
    unsigned int hi;
    if (!right) {
        for (i = 0; i < shift; i++) {
            value.hi <<= 1;
            if (value.lo & 0x80000000) {
                value.hi |= 1;
            }
            value.lo <<= 1;
        }
    } else {
        for (i = 0; i < shift; i++) {
            value.lo >>= 1;
            if (value.hi & 1) {
                value.lo |= 0x80000000;
            }
            value.hi = (unsigned int)value.hi >> 1;
        }
    }

    lo = value.lo;
    hi = value.hi;
    out->lo = lo;
    out->hi = hi;
    return out;
}
