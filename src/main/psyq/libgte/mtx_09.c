/* PSY-Q LIBGTE MTX_09: SetRotMatrix. */

#include "pe1/gte.h"
#include "pe1/gte_types.h"

void SetRotMatrix(const GteMatrix *matrix) {
    const GteMatrixWords *m = (const GteMatrixWords *)matrix;
    int r0 = m->r11_r12;
    int r1 = m->r13_r21;
    int r2 = m->r22_r23;
    int r3 = m->r31_r32;
    int r4 = m->r33_pad;

    gte_ctc2_0(r0);
    gte_ctc2_1(r1);
    gte_ctc2_2(r2);
    gte_ctc2_3(r3);
    gte_ctc2_4(r4);
}
