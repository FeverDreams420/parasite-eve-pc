/* PSY-Q LIBMATH ADDDF3: __adddf3. */

#include "pe1/math64.h"

double __adddf3(double x, double y) {
    struct {
        MathU64 a, b, round;
        MathDoubleBits zero;
    } w;
    MathDoubleBits left, right;
    int exponent, rexp;
    unsigned sign;
    unsigned signMask;
    unsigned shift;
    MathU64 *bp;
    unsigned topMask;
    int one;
    int amount;
    w.zero.bits.lo = 0;
    sign = 0;
    left.value = x;
    right.value = y;
    w.zero.bits.hi = 0;
    if (!(left.bits.hi & 0x7fffffff) && !left.bits.lo)
        return y;
    if (!(right.bits.hi & 0x7fffffff)) {
        {
            int raw = (int)left.bits.hi >> 20;
        }
        if (!right.bits.lo)
            return left.value;
    }
    {
        int raw = (int)left.bits.hi >> 20;
        exponent = raw & 0x7ff;
    }
    rexp = ((int)right.bits.hi >> 20) & 0x7ff;
    if (exponent > rexp + 54)
        return left.value;
    if (rexp > exponent + 54)
        return right.value;
    w.a.hi = (left.bits.hi & 0xfffff) | 0x100000;
    w.a.lo = left.bits.lo;
    w.b.hi = (right.bits.hi & 0xfffff) | 0x100000;
    w.b.lo = right.bits.lo;
    signMask = 0x80000000;
    if ((int)left.bits.hi < 0)
        _mainasu(&w.a, w.a);
    {
        unsigned negative = right.bits.hi & signMask;
        if (negative)
            _mainasu(&w.b, w.b);
    }
    shift = 9;
    _dbl_shift(&w.a, 0, w.a, shift);
    bp = &w.b;
    _dbl_shift(bp, 0, w.b, shift);
    if (exponent > rexp)
        _dbl_shift(bp, 1, w.b, exponent - rexp);
    else {
        MathU64 *ptr = &w.a;
        int direction = 1;
        amount = rexp - exponent;
        _dbl_shift(ptr, direction, w.a, amount);
        exponent = rexp;
    }
    _add_mant_d(&w.a, w.a, w.b);
    if ((int)w.a.hi >= 0) {
        if (!w.a.hi && !w.a.lo)
            return w.zero.value;
    } else {
        _mainasu(&w.a, w.a);
        sign = 0x80000000;
    }
    if (!(w.a.hi & 0xe0000000)) {
        topMask = 0xe0000000;
        one = 1;
        do {
            _dbl_shift(&w.a, 0, w.a, one);
            exponent--;
        } while (!(w.a.hi & topMask));
    }
    topMask = 0x40000000;
    if (w.a.hi & topMask) {
        _dbl_shift(&w.a, 1, w.a, 1);
        exponent++;
    }
    w.round.hi = 0;
    w.round.lo = (w.a.lo & 0x200) ? 0x100 : 0xff;
    _add_mant_d(&w.a, w.a, w.round);
    if (w.a.hi & topMask) {
        _dbl_shift(&w.a, 1, w.a, 1);
        exponent++;
    }
    _dbl_shift(&w.a, 1, w.a, 9);
    w.a.hi &= 0xffefffff;
    if (exponent >= 2047) {
        _err_math(34, 11);
        left.bits.hi = sign ? 0xfff00000 : 0x7ff00000;
        left.bits.lo = 0;
    } else {
        {
            unsigned packed = sign | ((unsigned)exponent << 20);
            left.bits.hi = packed | w.a.hi;
        }
        left.bits.lo = w.a.lo;
    }
    return left.value;
}
