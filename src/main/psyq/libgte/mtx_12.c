/* PSY-Q LIBGTE MTX_12: SetTransMatrix. */

#include "pe1/gte.h"
#include "pe1/gte_types.h"

void SetTransMatrix(const GteMatrix *m) {
    int tx = m->t[0];
    int ty = m->t[1];
    int tz = m->t[2];

    gte_ctc2_5(tx);
    gte_ctc2_6(ty);
    gte_ctc2_7(tz);
}
