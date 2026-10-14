#include "pe1/script_expression.h"
#include "pe1/fixed_point.h"

/* Keep original 32-bit word arithmetic without host signed-overflow UB. */
static s32 signed_word(u32 value)
{
    return value <= INT32_MAX ? (s32)value : -1 - (s32)(UINT32_MAX - value);
}
static s32 shift_right(s32 value, unsigned shift)
{
    s64 divisor = (s64)1 << shift;
    return value >= 0 ? (s32)(value / divisor) : (s32)(-1 - (-1 - (s64)value) / divisor);
}

int Script_EvalExpr(int **args) {
    switch (*args[0]) {
    case 0:
        *args[1] = signed_word((u32)*args[2] + (u32)*args[3]);
        break;
    case 1:
        *args[1] = signed_word((u32)*args[2] - (u32)*args[3]);
        break;
    case 2:
        *args[1] = *args[2] | *args[3];
        break;
    case 3:
        *args[1] = *args[2] & *args[3];
        break;
    case 4:
        *args[1] = *args[2] ^ *args[3];
        break;
    case 5:
        *args[1] = (*args[2] || *args[3]);
        break;
    case 6:
        *args[1] = (*args[2] && *args[3]);
        break;
    case 7:
        *args[1] = !*args[2];
        break;
    case 8:
        *args[1] = ~*args[2];
        break;
    case 9:
        *args[1] = *args[2] > *args[3];
        break;
    case 10:
        *args[1] = *args[2] < *args[3];
        break;
    case 11:
        *args[1] = *args[2] == *args[3];
        break;
    case 12:
        *args[1] = *args[2] >= *args[3];
        break;
    case 13:
        *args[1] = *args[2] <= *args[3];
        break;
    case 14:
        *args[1] = *args[2] != *args[3];
        break;
    case 15:
        *args[1] = signed_word((u32)*args[2] * (u32)*args[3]);
        break;
    case 16:
        *args[1] = (*args[2] == INT32_MIN && *args[3] == -1) ? INT32_MIN : *args[2] / *args[3];
        break;
    case 17:
        *args[1] = signed_word((u32)*args[2] << ((u32)*args[3] & 31));
        break;
    case 18:
        *args[1] = shift_right(*args[2], (u32)*args[3] & 31);
        break;
    case 19:
        *args[1] = *args[2];
        break;
    case 20:
        *args[1] = Math_FixedMul(*args[2], *args[3]);
        break;
    case 21:
        *args[1] = Math_FixedDivide(*args[2], *args[3]);
        break;
    case 22:
        *args[1] = (*args[2] == INT32_MIN && *args[3] == -1) ? 0 : *args[2] % *args[3];
        break;
    case 23:
        *args[1] = signed_word(0u - (u32)*args[2]);
        break;
    }
    return 1;
}
