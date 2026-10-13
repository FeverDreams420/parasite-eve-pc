/* PSY-Q LIBGTE FGO_05: RotMatrixY. */

#include "pe1/gte_sine_table.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_nop.h"

/* Q12 rotation with the original wrap and rounding behavior. */
GteMatrix *RotMatrixY(s32 angle, GteMatrix *m) {
    unsigned long long product;
    unsigned low;
    s32 a = angle;
    GteMatrix *ret = m;
    s32 packed;
    s32 temp;
    s32 scratch;
    s32 cosine;
    s32 negative_sine;
    s32 x0;
    s32 x1;
    s32 x2;
    s32 z0;
    /* The second factor is -sin(angle), simplifying the row updates. */
    packed = a & 0xfff;
    if (a < 0) {
        a = 0U - a;
        /* INT_MIN wraps to table offset zero after the unsigned shift. */
        if (a >= 0)
            a &= 0xfff;
        temp = a << 2;
        packed = GTE_SINE_WORD_AT_OFFSET(temp);
        scratch = packed << 16;
        negative_sine = scratch >> 16;
        cosine = packed >> 16;
    } else {
        temp = packed << 2;
        packed = GTE_SINE_WORD_AT_OFFSET(temp);
        temp = packed << 16;
        a = temp >> 16;
        negative_sine = 0U - a;
        cosine = packed >> 16;
    }
    x0 = m->m[0][0];
    z0 = m->m[2][0];
    product = GTE_MULTU(cosine, x0);
    x1 = m->m[0][1];
    scratch = m->m[2][1];
    temp = low;
    x2 = m->m[0][2];
    a = m->m[2][2];
    product = GTE_MULTU(negative_sine, z0);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    packed = low;
    packed = temp - packed;
    temp = packed >> 12;
    m->m[0][0] = temp;
    product = GTE_MULTU(cosine, x1);
    PE1_NOP_DEP("x", product);
    temp = low;
    product = GTE_MULTU(negative_sine, scratch);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    packed = low;
    packed = temp - packed;
    temp = packed >> 12;
    m->m[0][1] = temp;
    product = GTE_MULTU(cosine, x2);
    PE1_NOP_DEP("x", product);
    temp = low;
    product = GTE_MULTU(negative_sine, a);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    packed = low;
    packed = temp - packed;
    temp = packed >> 12;
    m->m[0][2] = temp;
    product = GTE_MULTU(negative_sine, x0);
    PE1_NOP_DEP("x", product);
    temp = low;
    product = GTE_MULTU(cosine, z0);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    packed = low;
    packed = temp + packed;
    temp = packed >> 12;
    m->m[2][0] = temp;
    product = GTE_MULTU(negative_sine, x1);
    PE1_NOP_DEP("x", product);
    temp = low;
    product = GTE_MULTU(cosine, scratch);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    packed = low;
    packed = temp + packed;
    temp = packed >> 12;
    m->m[2][1] = temp;
    product = GTE_MULTU(negative_sine, x2);
    PE1_NOP_DEP("x", product);
    temp = low;
    product = GTE_MULTU(cosine, a);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    packed = low;
    packed = temp + packed;
    temp = packed >> 12;
    m->m[2][2] = temp;
    PE1_NOP();
    return ret;
}
