/* PSY-Q LIBGTE MTX_004: ApplyMatrixLV. */

#include "pe1/gte.h"
#include "pe1/gte_types.h"

/* Split signed inputs at bit 15 before the two GTE rotation passes.
 * Empty outputs on architectural $0 retain retail's relative branches;
 * the final sign-dependent shifts deliberately preserve its wrapping path. */
GteVector *ApplyMatrixLV(const GteMatrix *matrix, const GteVector *v, GteVector *out) {
    s32 x;
    s32 y;
    s32 z;
    s32 hx;
    s32 hy;
    s32 hz;
    int zero = 0;
    GteVector *dst = out;
    const GteMatrixWords *m = (const GteMatrixWords *)matrix;
    x = m->r11_r12;
    y = m->r13_r21;
    z = m->r22_r23;
    hx = m->r31_r32;
    hy = m->r33_pad;
    gte_ctc2_0(x);
    gte_ctc2_1(y);
    gte_ctc2_2(z);
    gte_ctc2_3(hx);
    gte_ctc2_4(hy);
    x = v->x;
    y = v->y;
    z = v->z;
    if (x < 0) {
    x = 0U - x;
    hx = x >> 15;
    hx = 0U - hx;
    x &= 0x7fff;
        x = 0U - x;
    if (zero == 0) {
        goto split_x;
    }
    }
positive_x:
    hx = x >> 15;
    x &= 0x7fff;
split_x:
    if (y < 0) {
    y = 0U - y;
    hy = y >> 15;
    hy = 0U - hy;
    y &= 0x7fff;
        y = 0U - y;
    if (zero == 0) {
        goto split_y;
    }
    }
positive_y:
    hy = y >> 15;
    y &= 0x7fff;
split_y:
    if (z < 0) {
    z = 0U - z;
    hz = z >> 15;
    hz = 0U - hz;
    z &= 0x7fff;
        z = 0U - z;
    if (zero == 0) {
        goto split_z;
    }
    }
positive_z:
    hz = z >> 15;
    z &= 0x7fff;
split_z:
    gte_mtc2_9(hx);
    gte_mtc2_10(hy);
    gte_mtc2_11(hz);
    gte_cop2_hazard_slot();
    gte_mvmva_rotation_ir_sf0();
    gte_mfc2_25(hx);
    gte_mfc2_26(hy);
    gte_mfc2_27(hz);
    gte_mtc2_9(x);
    gte_mtc2_10(y);
    gte_mtc2_11(z);
    gte_cop2_hazard_slot();
    gte_mvmva_rotation_ir_sf12();
    if (hx < 0) {
    hx = 0U - hx;
    hx = hx << 3;
    hx = 0U - hx;
    if (zero == 0) {
        goto shifted_x;
    }
    }
shift_positive_x:
    hx = hx << 3;
shifted_x:
    if (hy < 0) {
    hy = 0U - hy;
    hy = hy << 3;
    hy = 0U - hy;
    if (zero == 0) {
        goto shifted_y;
    }
    }
shift_positive_y:
    hy = hy << 3;
shifted_y:
    if (hz < 0) {
    hz = 0U - hz;
    hz = hz << 3;
    hz = 0U - hz;
    if (zero == 0) {
        goto shifted_z;
    }
    }
shift_positive_z:
    hz = hz << 3;
shifted_z:
    gte_mfc2_25(x);
    gte_mfc2_26(y);
    gte_mfc2_27(z);
    x = x + hx;
    y = y + hy;
    z = z + hz;
    dst->x = x;
    dst->y = y;
    dst->z = z;
    return dst;
}
