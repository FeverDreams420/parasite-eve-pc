/* ASSEMBLER: GNU */
/* PSY-Q LIBGTE FGO_06: RotMatrixZ. */

#include "pe1/gte_sine_table.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_nop.h"

/* Q12 rotation with retail wrap/rounding order. Pins and empty HI/LO
 * scheduling constraints are tracked matching debt. The NOP slots reproduce
 * the handwritten LIBGTE schedule and are not debt.
 * Provenance: configs/USA/psyq_provenance.json (LIBGTE FGO objects).
 */
GteMatrix *RotMatrixZ(s32 angle, GteMatrix *m) {
    unsigned long long product;
    unsigned low;
    register s32 a asm("$15") = angle;
    register GteMatrix *ret = m;
    register s32 packed asm("$25");
    register s32 temp asm("$24");
    register s32 scratch asm("$14");
    register s32 cosine asm("$8");
    register s32 negative_sine asm("$9");
    register s32 x0 asm("$10");
    register s32 x1 asm("$11");
    register s32 x2 asm("$12");
    register s32 y0 asm("$13");
    asm volatile("" : "=r"(a), "=r"(ret) : "0"(a), "1"(ret));
    /* The second factor is -sin(angle), simplifying the row updates. */
    packed = a & 0xfff;
    if (a < 0) {
        a = 0U - a;
        /* INT_MIN wraps to table offset zero after the unsigned shift. */
        if (a < 0)
            asm volatile("" : : : "15");
        else
            a &= 0xfff;
        asm volatile("" : : : "24");
        temp = a << 2;
        packed = GTE_SINE_WORD_AT_OFFSET(temp);
        scratch = packed << 16;
        asm volatile("" : "=r"(scratch) : "0"(scratch));
        scratch = scratch >> 16;
        negative_sine = 0U - scratch;
        cosine = packed >> 16;
    } else {
        temp = packed << 2;
        packed = GTE_SINE_WORD_AT_OFFSET(temp);
        temp = packed << 16;
        asm volatile("" : "=r"(temp) : "0"(temp));
        negative_sine = temp >> 16;
        cosine = packed >> 16;
        asm volatile("" : "=r"(cosine) : "0"(cosine));
    }
    x0 = m->m[0][0];
    y0 = m->m[1][0];
    asm volatile("" : : "r"(x0), "r"(y0));
    product = GTE_MULTU(cosine, x0);
    x1 = m->m[0][1];
    asm volatile("" : : "r"(x1) : "memory");
    scratch = m->m[1][1];
    asm volatile("" : "=l"(low) : "x"(product));
    temp = low;
    asm volatile("" : "=r"(temp) : "0"(temp));
    x2 = m->m[0][2];
    asm volatile("" : : "r"(x2) : "memory");
    a = m->m[1][2];
    asm volatile("" : : "r"(temp), "r"(x2), "r"(a));
    product = GTE_MULTU(negative_sine, y0);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    packed = low;
    asm volatile("" : "=r"(temp), "=r"(packed) : "0"(temp), "1"(packed));
    packed = temp - packed;
    temp = packed >> 12;
    asm volatile("" : : "r"(temp));
    m->m[0][0] = temp;
    product = GTE_MULTU(cosine, x1);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    temp = low;
    asm volatile("" : "=r"(temp) : "0"(temp));
    product = GTE_MULTU(negative_sine, scratch);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    packed = low;
    asm volatile("" : "=r"(temp), "=r"(packed) : "0"(temp), "1"(packed));
    packed = temp - packed;
    temp = packed >> 12;
    asm volatile("" : : "r"(temp));
    m->m[0][1] = temp;
    product = GTE_MULTU(cosine, x2);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    temp = low;
    asm volatile("" : "=r"(temp) : "0"(temp));
    product = GTE_MULTU(negative_sine, a);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    packed = low;
    asm volatile("" : "=r"(temp), "=r"(packed) : "0"(temp), "1"(packed));
    packed = temp - packed;
    temp = packed >> 12;
    asm volatile("" : : "r"(temp));
    m->m[0][2] = temp;
    product = GTE_MULTU(negative_sine, x0);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    temp = low;
    asm volatile("" : "=r"(temp) : "0"(temp));
    product = GTE_MULTU(cosine, y0);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    packed = low;
    asm volatile("" : "=r"(temp), "=r"(packed) : "0"(temp), "1"(packed));
    packed = temp + packed;
    temp = packed >> 12;
    asm volatile("" : : "r"(temp));
    m->m[1][0] = temp;
    product = GTE_MULTU(negative_sine, x1);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    temp = low;
    asm volatile("" : "=r"(temp) : "0"(temp));
    product = GTE_MULTU(cosine, scratch);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    packed = low;
    asm volatile("" : "=r"(temp), "=r"(packed) : "0"(temp), "1"(packed));
    packed = temp + packed;
    temp = packed >> 12;
    asm volatile("" : : "r"(temp));
    m->m[1][1] = temp;
    product = GTE_MULTU(negative_sine, x2);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    temp = low;
    product = GTE_MULTU(cosine, a);
    PE1_NOP_DEP("x", product);
    PE1_NOP_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    packed = low;
    asm volatile("" : "=r"(temp), "=r"(packed) : "0"(temp), "1"(packed));
    packed = temp + packed;
    temp = packed >> 12;
    m->m[1][2] = temp;
    PE1_NOP();
    return ret;
}
