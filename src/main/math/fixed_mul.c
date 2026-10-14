#include "pe1/fixed_point.h"

/* Preserve the middle 32 bits of the original MIPS signed product. */
int Math_FixedMul(int a, int b)
{
    u32 bits = (u32)((u64)((s64)a * b) >> 16);
    return bits <= INT32_MAX ? (s32)bits : -1 - (s32)(UINT32_MAX - bits);
}
