#include "pe1/fixed_point.h"

/* 24.8 quotient: discard the divisor fraction, then restore scaling. */
int Math_FixedDivide(int a, int b)
{
    int divisor = b >= 0 ? b / 256 : -1 - (-1 - b) / 256;
    u32 bits = (u32)((s64)a / divisor) << 8;
    return bits <= INT32_MAX ? (s32)bits : -1 - (s32)(UINT32_MAX - bits);
}
