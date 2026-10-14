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

int Script_EvalExpr(ExprOpArgs *args) {
    switch (*args->op) {
    case 0:
        *args->dst = signed_word((u32)*args->lhs + (u32)*args->rhs);
        break;
    case 1:
        *args->dst = signed_word((u32)*args->lhs - (u32)*args->rhs);
        break;
    case 2:
        *args->dst = *args->lhs | *args->rhs;
        break;
    case 3:
        *args->dst = *args->lhs & *args->rhs;
        break;
    case 4:
        *args->dst = *args->lhs ^ *args->rhs;
        break;
    case 5:
        *args->dst = (*args->lhs || *args->rhs);
        break;
    case 6:
        *args->dst = (*args->lhs && *args->rhs);
        break;
    case 7:
        *args->dst = !*args->lhs;
        break;
    case 8:
        *args->dst = ~*args->lhs;
        break;
    case 9:
        *args->dst = *args->lhs > *args->rhs;
        break;
    case 10:
        *args->dst = *args->lhs < *args->rhs;
        break;
    case 11:
        *args->dst = *args->lhs == *args->rhs;
        break;
    case 12:
        *args->dst = *args->lhs >= *args->rhs;
        break;
    case 13:
        *args->dst = *args->lhs <= *args->rhs;
        break;
    case 14:
        *args->dst = *args->lhs != *args->rhs;
        break;
    case 15:
        *args->dst = signed_word((u32)*args->lhs * (u32)*args->rhs);
        break;
    case 16:
        *args->dst = (*args->lhs == INT32_MIN && *args->rhs == -1) ? INT32_MIN : *args->lhs / *args->rhs;
        break;
    case 17:
        *args->dst = signed_word((u32)*args->lhs << ((u32)*args->rhs & 31));
        break;
    case 18:
        *args->dst = shift_right(*args->lhs, (u32)*args->rhs & 31);
        break;
    case 19:
        *args->dst = *args->lhs;
        break;
    case 20:
        *args->dst = Math_FixedMul(*args->lhs, *args->rhs);
        break;
    case 21:
        *args->dst = Math_FixedDivide(*args->lhs, *args->rhs);
        break;
    case 22:
        *args->dst = (*args->lhs == INT32_MIN && *args->rhs == -1) ? 0 : *args->lhs % *args->rhs;
        break;
    case 23:
        *args->dst = signed_word(0u - (u32)*args->lhs);
        break;
    }
    return 1;
}
