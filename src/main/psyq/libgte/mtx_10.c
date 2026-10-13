/* PSY-Q LIBGTE MTX_10: SetLightMatrix. */

#include "pe1/gte.h"
#include "pe1/gte_types.h"

void SetLightMatrix(const GteMatrix *matrix) {
    const GteMatrixWords *m = (const GteMatrixWords *)matrix;
    int r0 = m->r11_r12;
    int r1 = m->r13_r21;
    int r2 = m->r22_r23;
    int r3 = m->r31_r32;
    int r4 = m->r33_pad;

    gte_ctc2_8(r0);
    gte_ctc2_9(r1);
    gte_ctc2_10(r2);
    gte_ctc2_11(r3);
    gte_ctc2_12(r4);
}
