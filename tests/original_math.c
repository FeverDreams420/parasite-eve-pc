#include "pe1/fixed_point.h"
#include <stdio.h>

static u32 seed = 521;
static s32 next_value(void)
{
    seed = seed * 1664525u + 1013904223u;
    return seed <= INT32_MAX ? (s32)seed : -1 - (s32)(UINT32_MAX - seed);
}

int main(void)
{
    unsigned i;
    for (i = 0; i < 100000; ++i) {
        s32 a = next_value(), b = next_value(), c = next_value();
        s64 product = (s64)a * b;
        s64 quotient = product / 65536;
        u32 bits;
        /* Signed division truncates; the original SRA rounds downward. */
        if (product < 0 && product % 65536) --quotient;
        bits = (u32)quotient;
        if ((u32)Math_FixedMul(a, b) != bits) return 1;
        if (Math_SqrtApprox3(a, b, c) !=
            (u32)(((u64)((s64)a * a) + (u64)((s64)b * b) +
                   (u64)((s64)c * c)) >> 20)) return 2;
        if (a <= INT32_MAX - 0x8000) {
            s64 rounded = (s64)a + 0x8000;
            s64 integer = rounded / 65536;
            s64 byte = rounded / 256;
            if (rounded < 0 && rounded % 65536) --integer;
            if (rounded < 0 && rounded % 256) --byte;
            if (Math_FixedRoundToInt(a) != integer ||
                Math_FixedRoundToByte(a) != byte) return 3;
        }
        {
            s64 divisor = b / 256;
            if (b < 0 && b % 256) --divisor;
            if (divisor && (u32)Math_FixedDivide(a, b) !=
                (u32)((s64)a / divisor * 256)) return 4;
        }
    }
    puts("Original game math: 100000 deterministic inputs passed");
    return 0;
}
