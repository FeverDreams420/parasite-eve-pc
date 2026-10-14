#include "pe1/gte.h"
#include <stdio.h>

GteMatrix *MulMatrix0(const GteMatrix *, const GteMatrix *, GteMatrix *);

int Pe1_RotAverageNclip3(const GteShortVector *, const GteShortVector *,
    const GteShortVector *, u32 *, u32 *, u32 *, s32 *, s32 *, u32 *);

static int triangle_checks(void)
{
    GteMatrix identity = {{{4096,0,0},{0,4096,0},{0,0,4096}}, {0,0,0}};
    GteShortVector v[3] = {{0,0,512,0},{100,0,512,0},{0,100,512,0}};
    u32 xy[3] = {0}, flags;
    s32 depth = -1, interpolation = -1;
    int area;
    gte_ldrotmatrix(&identity);
    gte_ldtransmatrix(&identity);
    gte_SetGeomOffset(160,112);
    gte_ctc2_26(256);
    Psyz_GteCtrlWrite(29, 0x555);
    gte_lddqa(0);
    gte_lddqb(0);
    area = Pe1_RotAverageNclip3(v, v+1, v+2, xy, xy+1, xy+2,
                           &interpolation, &depth, &flags);
    if (area != 2500 || depth != 511 || interpolation != 0 || flags ||
        xy[0] != (112u<<16|160u) || xy[1] != (112u<<16|210u) ||
        xy[2] != (162u<<16|160u)) return 10;
    xy[0] = xy[1] = xy[2] = 0xabcdef01;
    depth = interpolation = -123;
    area = Pe1_RotAverageNclip3(v, v+2, v+1, xy, xy+1, xy+2,
                           &interpolation, &depth, &flags);
    if (area != -2500 || xy[0] != 0xabcdef01 || xy[1] != 0xabcdef01 ||
        xy[2] != 0xabcdef01 || depth != -123 || interpolation != -123)
        return 11;
    return 0;
}

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
    { int error = triangle_checks(); if (error) return error; }
    puts("Original GTE macros run through PsyZ without a window");
    return 0;
}
