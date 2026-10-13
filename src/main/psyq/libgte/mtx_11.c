/* PSY-Q LIBGTE MTX_11: SetColorMatrix. */

#include "pe1/gte.h"
#include "pe1/gte_types.h"

void SetColorMatrix(const GteMatrix *matrix) {
    const GteMatrixWords *m = (const GteMatrixWords *)matrix;
    int r0 = m->r11_r12;
    int r1 = m->r13_r21;
    int r2 = m->r22_r23;
    int r3 = m->r31_r32;
    int r4 = m->r33_pad;

    gte_ctc2_16(r0);
    gte_ctc2_17(r1);
    gte_ctc2_18(r2);
    gte_ctc2_19(r3);
    gte_ctc2_20(r4);
}
