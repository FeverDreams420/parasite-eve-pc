#include "common.h"
#include "pe1/field_vector_math.h"

int FieldFx_TurnToward(int current, int target, int step) {
    int target_copy;
    int requestedStep;
    unsigned angleWord;
    int diff;
    int abs_diff;
    int step_s16;
    int delta;
    int result;
    current &= 0xFFF;
    result = target & 0xFFF;
    diff = result;
    target_copy = diff;
    diff = (s16)(diff - current);

    result = diff;
    if (diff < 0) {
        result = -result;
    }
    requestedStep = step;
    abs_diff = (s16)result;

    angleWord = requestedStep;
    step_s16 = (s16)requestedStep;
    delta = angleWord;
    if (step_s16 >= abs_diff) {
        result = target_copy;
    } else {
        if (diff < 0) {
            delta = -requestedStep;
        }
        if (abs_diff >= 0x801) {
            delta = -delta;
        }
        angleWord = delta + current;
        result = angleWord;
    }

    return result & 0xFFF;
}


s32 FieldFx_AngleDistance(s32 lhs, s32 rhs) {
    s32 delta;
    s32 abs_delta;
    int wrapThreshold = 0x801;

    delta = lhs - rhs;
    abs_delta = delta;
    if ((delta << 16) < 0) {
        delta = -delta;
        abs_delta = delta;
    }

    if ((short)abs_delta >= wrapThreshold) {
        abs_delta = 0x1000 - abs_delta;
    }

    delta = (short)abs_delta;
    return delta;
}
