#include "pe1/fixed_point.h"

int Math_FixedRoundToInt(int value)
{
    s64 rounded = (s64)value + 0x8000;
    return rounded >= 0 ? (int)(rounded / 0x10000)
                        : (int)(-1 - (-1 - rounded) / 0x10000);
}

int Math_FixedRoundToByte(int value)
{
    s64 rounded = (s64)value + 0x8000;
    return rounded >= 0 ? (int)(rounded / 0x100)
                        : (int)(-1 - (-1 - rounded) / 0x100);
}
