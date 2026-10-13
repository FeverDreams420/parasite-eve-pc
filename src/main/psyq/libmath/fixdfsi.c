/* GCC_VERSION: 2.8.1 */
/* PSY-Q LIBMATH FIXDFSI: __fixdfsi. */

#include "common.h"
#include "pe1/math64.h"

s32 __fixdfsi(register double value)
{
    register MathDoubleBits input;
    volatile MathU64 parts;
    register s32 exponent;
    register s32 shift;
    s32 mantissa;
    s32 result;

    input.value = value;
    if (((input.bits.hi & 0x7FFFFFFF) == 0) && (input.bits.lo == 0)) {
        result = input.bits.hi & 0x80000000;
        return result;
    }

    exponent = 20;
    exponent = (input.signedBits.hi >> exponent) & 0x7FF;
    shift = exponent - 0x41D;
    if (shift > 0) {
        _err_math(0x22, 0x11);
        result = 0x7FFFFFFF;
        if (input.signedBits.hi < 0) {
            result = 0x80000000;
        }
        return result;
    }

    parts.hi = (input.bits.hi & 0xFFFFF) | 0x100000;
    parts.lo = input.bits.lo;
    _dbl_shift(&parts, 0, parts, 10);
    mantissa = parts.hi;
    if (exponent >= 0x3FE && exponent < 0x41E) {
    if (mantissa != 0) {
    mantissa >>= -shift;
    goto applySign;

    }
    }
zero:
    result = 0;
    return result;
applySign:
    result = mantissa;
    if (input.signedBits.hi < 0) {
        result = -result;
    }
done:
    return result;
}
