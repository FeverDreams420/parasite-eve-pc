#include "common.h"

/* Original routine returns bits 20..51 of the squared length, not a root. */
unsigned int Math_SqrtApprox3(int x, int y, int z)
{
    u64 sum = (u64)((s64)x * x) + (u64)((s64)y * y)
            + (u64)((s64)z * z);
    return (u32)(sum >> 20);
}
