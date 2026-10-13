/* PSY-Q LIBGTE MTX_07: TransMatrix. */

#include "pe1/gte_types.h"

GteMatrix *TransMatrix(GteMatrix *m, const GteVector *v) {
    int x = v->x;
    int y = v->y;
    int z = v->z;
    GteMatrix *ret;
    m->t[0] = x;
    m->t[1] = y;
    m->t[2] = z;

    /* Finish the translation stores before copying the matrix pointer. */
    ret = m;
    /* Keep the pointer copy ahead of the return branch. */
    return ret;
}
