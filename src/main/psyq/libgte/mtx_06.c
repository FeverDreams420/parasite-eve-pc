/* PSY-Q LIBGTE MTX_06: ApplyMatrixSV. */

#include "pe1/gte.h"
#include "pe1/gte_types.h"

GteShortVector *ApplyMatrixSV(const GteMatrix *matrix, const GteShortVector *v,
                       GteShortVector *out) {
    const GteMatrixWords *m = (const GteMatrixWords *)matrix;
    register int r0 asm("$8") = m->r11_r12;
    register int r1 asm("$9") = m->r13_r21;
    register int r2 asm("$10") = m->r22_r23;
    register int r3 asm("$11") = m->r31_r32;
    register int r4 asm("$12") = m->r33_pad;
    GteShortVector *ret;
    gte_ctc2_0(r0);
    gte_ctc2_1(r1);
    gte_ctc2_2(r2);
    gte_ctc2_3(r3);
    gte_ctc2_4(r4);
    gte_lwc2_0_0(v);
    gte_lwc2_1_4(v);
    gte_cop2_hazard_slot();
    gte_mvmva_rotation_v0_sf12();
    /* Matrix words are loaded; reuse their scratch registers for the result. */
    gte_mfc2_9(r0);
    gte_mfc2_10(r1);
    gte_mfc2_11(r2);
    out->x = r0;
    out->y = r1;
    out->z = r2;
    asm volatile("" ::: "memory");
    asm volatile("" : "=r"(ret) : "0"(out));
    return ret;
}
