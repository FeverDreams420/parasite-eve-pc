/* ASSEMBLER: GNU */
/* PSY-Q LIBMATH MULDF3: __muldf3 and its helper _mul_mant_d. */

#include "pe1/math64.h"

double __muldf3(double x, double y)
{
    struct {
        MathU64 product;
        MathU64 rounding;
        MathU64 a;
        MathU64 b;
        MathU64 temporary;
        MathU64 cross;
    } work;
    MathDoubleBits result;
    unsigned int infinityBits;
    MathDoubleBits left;
    MathDoubleBits right;
    int leftExponent, rightExponent;
    int leftHighIsZero;
    unsigned sign;
    unsigned aHigh;
    unsigned bHigh;
    unsigned aLow;
    unsigned bLow;
    int exponent;
    int scratch;

    left.value = x;
    right.value = y;
    sign = (left.bits.hi & 0x80000000u) ^ (right.bits.hi & 0x80000000u);
    leftHighIsZero = (left.bits.hi & 0x7fffffffu) == 0;
    scratch = leftHighIsZero;
    if ((scratch && left.bits.lo == 0) ||
        ((right.bits.hi & 0x7fffffffu) == 0 && right.bits.lo == 0)) {
        result.bits.hi = sign;
        result.bits.lo = 0;
    } else {
        leftExponent = ((int)left.bits.hi >> 20) & 0x7ff;
        rightExponent = ((int)right.bits.hi >> 20) & 0x7ff;
        exponent = leftExponent + rightExponent - 1022;
        /* Keep the adjusted exponent independent of the shared sum. */
        asm volatile("" : "=r"(exponent) : "0"(exponent));

        work.a.hi = (left.bits.hi & 0xfffff) | 0x100000;
        work.a.lo = left.bits.lo;
        work.b.hi = (right.bits.hi & 0xfffff) | 0x100000;
        work.b.lo = right.bits.lo;
        _dbl_shift_us(&work.temporary, 1, work.a, 21);
        aHigh = work.temporary.lo;
        _dbl_shift_us(&work.temporary, 1, work.b, 21);
        bHigh = work.temporary.lo;
        aLow = work.a.lo & 0x1fffff;
        bLow = work.b.lo & 0x1fffff;

        /* Form the retained product from 32-bit high and 21-bit low parts. */
        _mul_mant_d(&work.product, aHigh, bHigh);
        _mul_mant_d(&work.cross, aLow, bHigh);
        _dbl_shift_us(&work.temporary, 1, work.cross, 21);
        scratch = aHigh;
        _add_mant_d(&work.product, work.product, work.temporary);
        _mul_mant_d(&work.cross, bLow, scratch);
        _dbl_shift_us(&work.temporary, 1, work.cross, 21);
        _add_mant_d(&work.product, work.product, work.temporary);
        if ((scratch = work.product.hi & 0x80000000u)) {
            work.rounding.hi = 0;
            work.rounding.lo = 0x400;
            _add_mant_d(&work.product, work.product, work.rounding);
            _dbl_shift_us(&work.product, 1, work.product, 11);
        } else {
            work.rounding.hi = 0;
            work.rounding.lo = 0x200;
            _add_mant_d(&work.product, work.product, work.rounding);
            if (work.product.hi & 0x80000000u) {
                _dbl_shift_us(&work.product, 1, work.product, 11);
            } else {
                _dbl_shift_us(&work.product, 1, work.product, 10);
                exponent = leftExponent + rightExponent - 1023;
            }
        }

        work.product.hi = work.product.hi & 0xffefffff;
        if (exponent >= 2047) {
            _err_math(34, 13);
            infinityBits = sign ? 0xfff00000u : 0x7ff00000u;
            result.bits.hi = infinityBits;
            result.bits.lo = 0;
        } else {
            result.bits.hi = (sign | ((unsigned)exponent << 20)) | work.product.hi;
            result.bits.lo = work.product.lo;
        }
    }
    return result.value;
}


/* Four 16-bit partial products, with the two cross terms accumulated by
 * _add_mant_d. The output pointer is passed in a0 and returned in v0.
 * Register pins and empty scheduling barriers are included in debt. */
MathU64 *_mul_mant_d(MathU64 *out, unsigned int left, unsigned int right) {
    volatile MathU64 result;
    MathU64 part;
    register unsigned int low asm("$18") = left & 0xFFFF;
    register unsigned int firstProduct;
    register unsigned int product asm("$2");
    register MathU64 *resultPtr;

    asm("" : "=r"(low), "=r"(right) : "0"(low), "1"(right));
    product = right & 0xFFFF;
    firstProduct = low * product;
    left >>= 16;
    product = left * product;
    resultPtr = (MathU64 *)&result;
    asm("" : "=r"(resultPtr) : "0"(resultPtr));
    result.hi = 0;
    result.lo = firstProduct;
    part.hi = product >> 16;
    part.lo = product << 16;
    right >>= 16;
    _add_mant_d(resultPtr, result, part);

    product = low * right;
    part.hi = product >> 16;
    part.lo = product << 16;
    _add_mant_d(&result, result, part);
    {
        unsigned int last = left * right;
        register unsigned int hi = result.hi;
        hi += last;
        result.hi = hi;
    }
    {
        register unsigned int lo;
        register unsigned int hi;
        lo = result.lo;
        hi = result.hi;
        out->lo = lo;
        out->hi = hi;
    }
    asm volatile("" : : "m"(*out));
    return out;
}
