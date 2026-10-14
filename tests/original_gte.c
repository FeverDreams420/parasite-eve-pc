#include "pe1/gte.h"
#include <stdio.h>

GteMatrix *MulMatrix0(const GteMatrix *, const GteMatrix *, GteMatrix *);

static int matrix_checks(void)
{
    GteMatrix left = {{{0, 0, 4096}, {0, 4096, 0}, {-4096, 0, 0}},
                      {40000, -40000, 123}};
    GteMatrix right = {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}},
                       {-100, 200, 300}};
    GteMatrix ir = {0}, mac = {0}, original = {0};
    u8 unaligned[33] = {0};
    GteShortVector vector = {-123, 456, -789, 0}, result = {0};
    if (MulMatrix0(&left, &right, &original) != &original ||
        memcmp(original.m, left.m, sizeof(left.m))) return 8;
    original = right;
    if (MulMatrix0(&left, &original, &original) != &original ||
        memcmp(original.m, left.m, sizeof(left.m))) return 9;
    gte_CompMatrix(&left, &right, &ir);
    gte_CompMatrixMac(&left, &right, &mac);
    if (memcmp(ir.m, left.m, sizeof(left.m)) ||
        memcmp(mac.m, left.m, sizeof(left.m))) return 4;
    /* IR saturation and the full MAC translation are distinct results. */
    if (ir.t[0] != 32767 || ir.t[1] != -32768 || ir.t[2] != 223 ||
        mac.t[0] != 40300 || mac.t[1] != -39800 || mac.t[2] != 223) return 5;
    gte_ldrotmatrix(&left);
    gte_ldtransmatrix(&left);
    gte_ReadMatrix(unaligned + 1);
    if (memcmp(unaligned + 1, &left, 18) ||
        memcmp(unaligned + 21, left.t, 12)) return 6;
    gte_ldv0(&vector);
    gte_mvmva();
    gte_stsv(&result);
    if (result.x != -789 || result.y != 456 || result.z != 123) return 7;
    return 0;
}

int main(void)
{
    /* Call the game's macros directly, with an intentionally unaligned
     * buffer, to check host loads and stores as well as projection. */
    u8 bytes[17] = {0};
    u32 word = 100, output, mac;
    gte_ctc2_0(4096);
    gte_ctc2_1(0);
    gte_ctc2_2(4096);
    gte_ctc2_3(0);
    gte_ctc2_4(4096);
    gte_ctc2_5(0);
    gte_ctc2_6(0);
    gte_ctc2_7(0);
    gte_ctc2_24(160u << 16);
    gte_ctc2_25(112u << 16);
    gte_ctc2_26(256);
    memcpy(bytes + 1, &word, 4);
    word = 512;
    memcpy(bytes + 5, &word, 4);
    gte_lwc2_0_0(bytes + 1);
    gte_lwc2_1_4(bytes + 1);
    gte_rtps_command();
    gte_swc2_14_8(bytes + 1);
    memcpy(&output, bytes + 9, 4);
    if (output != (112u << 16 | 210u)) return 1;
    gte_mfc2_25(mac);
    if (mac != 100) return 2;
    gte_ctc2_5(-123);
    gte_cfc2_5(output);
    if (output != (u32)-123) return 3;
    { int error = matrix_checks(); if (error) return error; }
    puts("Original GTE macros run through PsyZ without a window");
    return 0;
}
