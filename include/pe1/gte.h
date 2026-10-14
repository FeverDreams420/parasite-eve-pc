#ifndef PE1_GTE_H
#define PE1_GTE_H

#include "pe1/gte_types.h"
#include <string.h>

/* Existing PsyZ endpoints, without importing conflicting SDK types. */
unsigned int Psyz_GteDataRead(unsigned reg);
void Psyz_GteDataWrite(unsigned reg, unsigned int value);
unsigned int Psyz_GteCtrlRead(unsigned reg);
void Psyz_GteCtrlWrite(unsigned reg, unsigned int value);
void Psyz_GteCommand(unsigned int command);

/*
 * PSY-Q-style GTE operations.  Keep these macros limited to COP2 state and
 * commands; the calling function owns all normal CPU-side algorithmic work.
 */

/* Single-instruction transfers used by ApplyMatrix and ApplyMatrixSV.
 * Hazard slots are explicit at the call site, separate from the command. */
#define GTE_INSN_LWC2_0_0 "lwc2 $0,0(%0)"
#define GTE_INSN_LWC2_1_4 "lwc2 $1,4(%0)"
#define gte_lwc2_0_0(ptr) \
    do { u32 word; memcpy(&word, (ptr), 4); Psyz_GteDataWrite(0, word); } while (0)
#define gte_lwc2_1_4(ptr) \
    do { u32 word; memcpy(&word, (const u8 *)(ptr) + 4, 4); Psyz_GteDataWrite(1, word); } while (0)
#define gte_lwc2_2_0(p) do { u32 word; memcpy(&word, (const u8 *)(p) + 0, 4); Psyz_GteDataWrite(2, word); } while (0)
#define gte_lwc2_3_4(p) do { u32 word; memcpy(&word, (const u8 *)(p) + 4, 4); Psyz_GteDataWrite(3, word); } while (0)
#define gte_lwc2_4_0(p) do { u32 word; memcpy(&word, (const u8 *)(p) + 0, 4); Psyz_GteDataWrite(4, word); } while (0)
#define gte_lwc2_5_4(p) do { u32 word; memcpy(&word, (const u8 *)(p) + 4, 4); Psyz_GteDataWrite(5, word); } while (0)
#define gte_rtpt_command() Psyz_GteCommand(0x4A280030u & 0x01ffffffu)
#define gte_avsz3_command() Psyz_GteCommand(0x4B58002Du & 0x01ffffffu)
#define gte_avsz4_command() Psyz_GteCommand(0x4B68002Eu & 0x01ffffffu)

#define gte_swc2_11_16(ptr) do { u32 word = Psyz_GteDataRead(11); memcpy((u8 *)(ptr) + 16, &word, 4); } while (0)

#define gte_swc2_25_0(ptr) do { u32 word = Psyz_GteDataRead(25); memcpy((u8 *)(ptr) + 0, &word, 4); } while (0)
#define gte_swc2_26_4(ptr) do { u32 word = Psyz_GteDataRead(26); memcpy((u8 *)(ptr) + 4, &word, 4); } while (0)
#define gte_swc2_27_8(ptr) do { u32 word = Psyz_GteDataRead(27); memcpy((u8 *)(ptr) + 8, &word, 4); } while (0)

/* MVMVA: SF=1, rotation matrix, V0, no translation, LM=0. */
#define gte_mvmva_rotation_v0_sf12() Psyz_GteCommand(0x4A486012u & 0x01ffffffu)

/* Single commands; callers place their required transfer hazard slots. */
#define gte_rtps_command() Psyz_GteCommand(0x4A180001u & 0x01ffffffu)
#define gte_mvmva_rotation_v0_translation_sf12() Psyz_GteCommand(0x4A480012u & 0x01ffffffu)

/* Individual MAC transfers and rotation commands for long-vector inputs. */
#define gte_mfc2_25(value) ((value) = Psyz_GteDataRead(25))
#define gte_mfc2_26(value) ((value) = Psyz_GteDataRead(26))
#define gte_mfc2_27(value) ((value) = Psyz_GteDataRead(27))
#define gte_mvmva_rotation_ir_sf0() Psyz_GteCommand(0x4A41E012u & 0x01ffffffu)
#define gte_mvmva_rotation_ir_sf12() Psyz_GteCommand(0x4A49E012u & 0x01ffffffu)

/* Far-colour controls and three-vertex colour output for NCCT.
 * Each macro contains exactly one GTE instruction. */
#define gte_ctc2_13(v) Psyz_GteCtrlWrite(13, (u32)(v))
#define gte_ctc2_14(v) Psyz_GteCtrlWrite(14, (u32)(v))
#define gte_ctc2_15(v) Psyz_GteCtrlWrite(15, (u32)(v))
#define gte_lwc2_6_0(p) do { u32 word; memcpy(&word, (const u8 *)(p) + 0, 4); Psyz_GteDataWrite(6, word); } while (0)
#define gte_ncct_command() Psyz_GteCommand(0x4B18043Fu & 0x01ffffffu)
#define gte_swc2_20_0(p) do { u32 word = Psyz_GteDataRead(20); memcpy((u8 *)(p) + 0, &word, 4); } while (0)
#define gte_swc2_21_0(p) do { u32 word = Psyz_GteDataRead(21); memcpy((u8 *)(p) + 0, &word, 4); } while (0)
#define gte_swc2_22_0(p) do { u32 word = Psyz_GteDataRead(22); memcpy((u8 *)(p) + 0, &word, 4); } while (0)

/* Read the current packed rotation/translation control words. */
#define gte_cfc2_0(v) ((v) = Psyz_GteCtrlRead(0))
#define gte_cfc2_1(v) ((v) = Psyz_GteCtrlRead(1))
#define gte_cfc2_2(v) ((v) = Psyz_GteCtrlRead(2))
#define gte_cfc2_3(v) ((v) = Psyz_GteCtrlRead(3))
#define gte_cfc2_4(v) ((v) = Psyz_GteCtrlRead(4))
#define gte_cfc2_5(v) ((v) = Psyz_GteCtrlRead(5))
#define gte_cfc2_6(v) ((v) = Psyz_GteCtrlRead(6))
#define gte_cfc2_7(v) ((v) = Psyz_GteCtrlRead(7))

#define gte_cop2_hazard_slot() ((void)0)

/* Perspective center in signed 16.16 screen coordinates. */
#define gte_ctc2_24(value) Psyz_GteCtrlWrite(24, (u32)(value))
#define gte_ctc2_25(value) Psyz_GteCtrlWrite(25, (u32)(value))

/* Projection distance (H). */
#define gte_ctc2_26(value) Psyz_GteCtrlWrite(26, (u32)(value))

#define gte_ctc2_0(value) Psyz_GteCtrlWrite(0, (u32)(value))
#define gte_ctc2_1(value) Psyz_GteCtrlWrite(1, (u32)(value))
#define gte_ctc2_2(value) Psyz_GteCtrlWrite(2, (u32)(value))
#define gte_ctc2_3(value) Psyz_GteCtrlWrite(3, (u32)(value))
#define gte_ctc2_4(value) Psyz_GteCtrlWrite(4, (u32)(value))
#define gte_ctc2_5(value) Psyz_GteCtrlWrite(5, (u32)(value))
#define gte_ctc2_6(value) Psyz_GteCtrlWrite(6, (u32)(value))
#define gte_ctc2_7(value) Psyz_GteCtrlWrite(7, (u32)(value))

/* Packed light-matrix words L11..L33 (control registers 8..12). */
#define gte_ctc2_8(value) Psyz_GteCtrlWrite(8, (u32)(value))
#define gte_ctc2_9(value) Psyz_GteCtrlWrite(9, (u32)(value))
#define gte_ctc2_10(value) Psyz_GteCtrlWrite(10, (u32)(value))
#define gte_ctc2_11(value) Psyz_GteCtrlWrite(11, (u32)(value))
#define gte_ctc2_12(value) Psyz_GteCtrlWrite(12, (u32)(value))

/* Packed light-color matrix words (control registers 16..20). */
#define gte_ctc2_16(value) Psyz_GteCtrlWrite(16, (u32)(value))
#define gte_ctc2_17(value) Psyz_GteCtrlWrite(17, (u32)(value))
#define gte_ctc2_18(value) Psyz_GteCtrlWrite(18, (u32)(value))
#define gte_ctc2_19(value) Psyz_GteCtrlWrite(19, (u32)(value))
#define gte_ctc2_20(value) Psyz_GteCtrlWrite(20, (u32)(value))

#define gte_mtc2_1(value) Psyz_GteDataWrite(1, (u32)(value))

#define gte_mtc2_0(value) Psyz_GteDataWrite(0, (u32)(value))
#define gte_mtc2_9(value) Psyz_GteDataWrite(9, (u32)(value))
#define gte_mtc2_10(value) Psyz_GteDataWrite(10, (u32)(value))
#define gte_mtc2_11(value) Psyz_GteDataWrite(11, (u32)(value))

#define gte_mfc2_9(out) ((out) = Psyz_GteDataRead(9))
#define gte_mfc2_10(out) ((out) = Psyz_GteDataRead(10))
#define gte_mfc2_11(out) ((out) = Psyz_GteDataRead(11))

#define gte_lwc2_9_0(ptr) do { u32 word; memcpy(&word, (const u8 *)(ptr) + 0, 4); Psyz_GteDataWrite(9, word); } while (0)
#define gte_lwc2_10_4(ptr) do { u32 word; memcpy(&word, (const u8 *)(ptr) + 4, 4); Psyz_GteDataWrite(10, word); } while (0)
#define gte_lwc2_11_8(ptr) do { u32 word; memcpy(&word, (const u8 *)(ptr) + 8, 4); Psyz_GteDataWrite(11, word); } while (0)
/* IR vector loads with precise read operands, preserving unrelated locals. */
#define gte_ldir1_precise(ptr) do { u32 word; memcpy(&word, (const u8 *)(ptr) + 0, 4); Psyz_GteDataWrite(9, word); } while (0)
#define gte_ldir2_precise(ptr) do { u32 word; memcpy(&word, (const u8 *)(ptr) + 4, 4); Psyz_GteDataWrite(10, word); } while (0)
#define gte_ldir3_precise(ptr) do { u32 word; memcpy(&word, (const u8 *)(ptr) + 8, 4); Psyz_GteDataWrite(11, word); } while (0)

#define gte_op_sf12_command() Psyz_GteCommand(0x4B78000Cu & 0x01ffffffu)

#define gte_lwc2_1_8(ptr) do { u32 word; memcpy(&word, (const u8 *)(ptr) + 8, 4); Psyz_GteDataWrite(1, word); } while (0)

/* Load V0 from an s32[3] translation (a matrix's t): the low halfwords of
 * t[0] and t[1] packed into VXY0 (narrowed to lhu), t[2] by lwc2 into VZ0.
 * source is an s32 * lvalue so callers can reuse the constrained address. */
#define gte_ldv0_word3(source) do { const u8 *pe_gte_src = (const u8 *)(source); u16 pe_gte_x, pe_gte_y; u32 pe_gte_z; memcpy(&pe_gte_x, pe_gte_src, 2); memcpy(&pe_gte_y, pe_gte_src + 4, 2); memcpy(&pe_gte_z, pe_gte_src + 8, 4); Psyz_GteDataWrite(0, pe_gte_x | ((u32)pe_gte_y << 16)); Psyz_GteDataWrite(1, pe_gte_z); } while (0)

#define gte_ldv0_word3_at(vec) \
    { \
        s32 *source = (vec); \
        gte_ldv0_word3(source); \
    }

#define gte_swc2_9_0(ptr) do { u32 word = Psyz_GteDataRead(9); memcpy((u8 *)(ptr) + 0, &word, 4); } while (0)
#define gte_swc2_10_4(ptr) do { u32 word = Psyz_GteDataRead(10); memcpy((u8 *)(ptr) + 4, &word, 4); } while (0)
#define gte_swc2_11_8(ptr) do { u32 word = Psyz_GteDataRead(11); memcpy((u8 *)(ptr) + 8, &word, 4); } while (0)

#define gte_mvmva_light_ir_sf0() Psyz_GteCommand(0x4A49E012u & 0x01ffffffu)

/* Load a strided matrix column into IR1..3 and apply RTIR12. */
#define gte_ldrtir12_matrix_column(column) do { gte_ldclmv(column); gte_rtir(); } while (0)

/* Store IR1..3 back to a strided matrix column. */
#define gte_stir123_column(out) do { u8 *pe_gte_dst = (u8 *)(out); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u16 pe_gte_half = (u16)Psyz_GteDataRead(9 + pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 6, &pe_gte_half, 2); } } while (0)

/* PSY-Q gte_stsv: store IR1..3 as a short vector (mfc2 into $12..$14, then
 * three halfword stores inside the same statement, as the SDK macro does). */
#define gte_stsv(out) do { u8 *pe_gte_dst = (u8 *)(out); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u16 pe_gte_half = (u16)Psyz_GteDataRead(9 + pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 2, &pe_gte_half, 2); } } while (0)

#define gte_stir123_column_at(column) \
    { \
        volatile short *out = (volatile short *)(column); \
        gte_stir123_column(out); \
    }

#define gte_rtv0tr_sf0() Psyz_GteCommand(0x4A480012u & 0x01ffffffu)

/* Load the five packed words which hold R11..R33. */
#define gte_ldrotmatrix(matrix) do { const u8 *pe_gte_src = (const u8 *)(matrix); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 5; ++pe_gte_i) { u32 pe_gte_word; memcpy(&pe_gte_word, pe_gte_src + 0 + pe_gte_i * 4, 4); Psyz_GteCtrlWrite(0 + pe_gte_i, pe_gte_word); } } while (0)

/* Load the three translation-vector words into TRX, TRY, and TRZ. */
#define gte_ldtransmatrix(matrix) do { const u8 *pe_gte_src = (const u8 *)(matrix); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u32 pe_gte_word; memcpy(&pe_gte_word, pe_gte_src + 20 + pe_gte_i * 4, 4); Psyz_GteCtrlWrite(5 + pe_gte_i, pe_gte_word); } } while (0)

/* Preserve and replace the first column of the rotation matrix. */
#define gte_getrotcol0(x, y, z) \
    asm volatile("cfc2 %0,$0\n\t" \
                 "cfc2 %1,$2\n\t" \
                 "cfc2 %2,$4" \
                 : "=r"(x), "=r"(y), "=r"(z))

/* OP temporarily replaces R11, R22, and R33, so preserve their COP2 values. */
#define gte_pushrotcol0() \
    asm volatile("cfc2 $13,$0\n\t" \
                 "cfc2 $14,$2\n\t" \
                 "cfc2 $15,$4" \
                 : : : "$13", "$14", "$15")

#define gte_poprotcol0() \
    asm volatile("ctc2 $13,$0\n\t" \
                 "ctc2 $14,$2\n\t" \
                 "ctc2 $15,$4" \
                 : : : "$13", "$14", "$15")

#define gte_setrotcol0(x, y, z) \
    asm volatile("ctc2 %0,$0\n\t" \
                 "ctc2 %1,$2\n\t" \
                 "ctc2 %2,$4" \
                 : : "r"(x), "r"(y), "r"(z))

#define gte_ldv0(vec) do { const u8 *pe_gte_src = (const u8 *)(vec); u32 pe_gte_word; memcpy(&pe_gte_word, pe_gte_src, 4); Psyz_GteDataWrite(0, pe_gte_word); memcpy(&pe_gte_word, pe_gte_src + 4, 4); Psyz_GteDataWrite(1, pe_gte_word); } while (0)

#define gte_ldv1(vec) do { const u8 *pe_gte_src = (const u8 *)(vec); u32 pe_gte_word; memcpy(&pe_gte_word, pe_gte_src, 4); Psyz_GteDataWrite(2, pe_gte_word); memcpy(&pe_gte_word, pe_gte_src + 4, 4); Psyz_GteDataWrite(3, pe_gte_word); } while (0)

#define gte_ldv2(vec) do { const u8 *pe_gte_src = (const u8 *)(vec); u32 pe_gte_word; memcpy(&pe_gte_word, pe_gte_src, 4); Psyz_GteDataWrite(4, pe_gte_word); memcpy(&pe_gte_word, pe_gte_src + 4, 4); Psyz_GteDataWrite(5, pe_gte_word); } while (0)

/* MVMVA: rotation matrix, V0, no translation, SF=1 (12-bit fraction), LM=0. */
#define gte_mvmva() do { Psyz_GteCommand(0x4A486012u & 0x01ffffffu); } while (0)

/* MVMVA: light matrix, IR vector, no translation, SF=1, LM=1. */
#define gte_lightcolor() do { Psyz_GteCommand(0x4A4DA412u & 0x01ffffffu); } while (0)

#define gte_stmac(out) do { u8 *pe_gte_dst = (u8 *)(out); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u32 pe_gte_word = Psyz_GteDataRead(25 + pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 4, &pe_gte_word, 4); } } while (0)

/* MAC stores followed by a pointer return leave v0 available to the caller. */
#define gte_stmac_return(out) do { u8 *pe_gte_dst = (u8 *)(out); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u32 pe_gte_word = Psyz_GteDataRead(25 + pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 4, &pe_gte_word, 4); } } while (0)

/* LZCS/LZCR: count leading zeroes in a 32-bit value. */
#define gte_ldlzcs(value) Psyz_GteDataWrite(30, (u32)(value))

#define gte_stlzcr(out) do { u32 word = Psyz_GteDataRead(31); memcpy((u8 *)(out) + 0, &word, 4); } while (0)

/* The LZCR-to-GPR transfer needs two COP2 hazard slots. */
#define gte_getlzcr(out) \
    asm volatile("nop\n\t" \
                 "nop\n\t" \
                 "mfc2 %0,$31" : "=r"(out))

/* NCLIP: signed screen-space triangle area from SXY0, SXY1, and SXY2. */
#define gte_ldsxy0(value) Psyz_GteDataWrite(12, (u32)(value))

#define gte_ldsxy1(value) Psyz_GteDataWrite(13, (u32)(value))

#define gte_ldsxy2(value) Psyz_GteDataWrite(14, (u32)(value))

/* PSY-Q gte_ldsxy3: load SXY0, SXY2 and SXY1 in the SDK's order. */
#define gte_ldsxy3(r0, r1, r2) \
    asm volatile("mtc2 %0,$12\n\t" \
                 "mtc2 %2,$14\n\t" \
                 "mtc2 %1,$13" \
                 : : "r"(r0), "r"(r1), "r"(r2))

#define gte_nclip() do { Psyz_GteCommand(0x4B400006u & 0x01ffffffu); } while (0)

#define gte_nclip_now() Psyz_GteCommand(0x4B400006u & 0x01ffffffu)

#define gte_getmac0(out) ((out) = Psyz_GteDataRead(24))

#define gte_stmac0(out) do { u32 word = Psyz_GteDataRead(24); memcpy((u8 *)(out) + 0, &word, 4); } while (0)

/* PSY-Q gte_stotz: store the averaged OTZ. */
#define gte_stotz(r0) do { u32 word = Psyz_GteDataRead(7); memcpy((u8 *)(r0) + 0, &word, 4); } while (0)

/* RTV0TR: rotate V0 and add the translation vector. */
#define gte_rtv0tr() \
    asm volatile("nop\n\t" \
                 ".word 0x4A480012")

#define gte_getir1(out) ((out) = Psyz_GteDataRead(9))

#define gte_getir2(out) ((out) = Psyz_GteDataRead(10))

#define gte_getir3(out) ((out) = Psyz_GteDataRead(11))

#define gte_rtv0tr_mac() \
    asm volatile("nop\n\t" \
                 "nop\n\t" \
                 ".word 0x4A480012")

#define gte_stir123(out) \
    asm volatile("swc2 $9,0(%0)\n\t" \
                 "swc2 $10,4(%0)\n\t" \
                 "swc2 $11,8(%0)" \
                 : : "r"(out) : "memory")

/* The GPL wrappers consume v1 before transferring the final IR vector. */
#define gte_stir123_gpl(out) \
    asm volatile("swc2 $9,0(%0)\n\t" \
                 "swc2 $10,4(%0)\n\t" \
                 "swc2 $11,8(%0)" \
                 : : "r"(out) : "$3", "memory")

/* RTIR12: rotate the current IR vector with no translation. */
#define gte_ldir123(x, y, z) \
    asm volatile("mtc2 %0,$9\n\t" \
                 "mtc2 %1,$10\n\t" \
                 "mtc2 %2,$11" \
                 : : "r"(x), "r"(y), "r"(z))

/* Load the three scalar IR inputs from consecutive words. */
#define gte_ldir123v(vec) \
    asm volatile("lwc2 $9,0(%0)\n\t" \
                 "lwc2 $10,4(%0)\n\t" \
                 "lwc2 $11,8(%0)" \
                 : : "r"(vec) : "memory")

/* OP consumes IR3 before IR1 and IR2 in the original PSY-Q wrapper. */
#define gte_ldopv(vec) \
    asm volatile("lwc2 $11,8(%0)\n\t" \
                 "lwc2 $9,0(%0)\n\t" \
                 "lwc2 $10,4(%0)" \
                 : : "r"(vec) : "memory")

/* OP uses R11, R22, and R33 as its temporary diagonal matrix entries. */
#define gte_ldopv1(vec) \
    asm volatile("lw $8,0(%0)\n\t" \
                 "lw $9,4(%0)\n\t" \
                 "lw $10,8(%0)\n\t" \
                 "ctc2 $8,$0\n\t" \
                 "ctc2 $9,$2\n\t" \
                 "ctc2 $10,$4" \
                 : : "r"(vec) : "$8", "$9", "$10", "memory")

/* RGB is the colour input used by the colour interpolation commands. */
#define gte_ldrgb(rgb) do { u32 word; memcpy(&word, (const u8 *)(rgb) + 0, 4); Psyz_GteDataWrite(6, word); } while (0)

#define gte_strgb(out) do { u32 word = Psyz_GteDataRead(22); memcpy((u8 *)(out) + 0, &word, 4); } while (0)

#define gte_ldrgb0(rgb) do { u32 word; memcpy(&word, (const u8 *)(rgb) + 0, 4); Psyz_GteDataWrite(20, word); } while (0)

#define gte_ldrgb1(rgb) do { u32 word; memcpy(&word, (const u8 *)(rgb) + 0, 4); Psyz_GteDataWrite(21, word); } while (0)

#define gte_ldrgb2(rgb) do { u32 word; memcpy(&word, (const u8 *)(rgb) + 0, 4); Psyz_GteDataWrite(22, word); } while (0)

#define gte_strgb0(out) do { u32 word = Psyz_GteDataRead(20); memcpy((u8 *)(out) + 0, &word, 4); } while (0)

#define gte_strgb1(out) do { u32 word = Psyz_GteDataRead(21); memcpy((u8 *)(out) + 0, &word, 4); } while (0)

#define gte_strgb2(out) do { u32 word = Psyz_GteDataRead(22); memcpy((u8 *)(out) + 0, &word, 4); } while (0)

#define gte_declare_three_outputs()                                         \
    void *gte_out0;                                      \
    void *gte_out1;                                      \
    void *gte_out2

#define gte_bind_three_outputs(out0, out1, out2)                            \
    {                                                                     \
        gte_out0 = (out0);                                                   \
        gte_out1 = (out1);                                                   \
        gte_out2 = (out2);                                                   \
    }

#define gte_dpct_separate()                                                 \
    asm volatile("nop\n\t"                                                \
                 ".word 0x4AF8002A"                                         \
                 : : : "$8", "$9", "$10")

#define gte_rtpt_three_outputs()                                            \
    asm volatile("nop\n\t"                                                \
                 ".word 0x4A280030"                                         \
                 : : : "$8", "$9", "$10")

#define gte_rtps_three_outputs()                                            \
    asm volatile("nop\n\t"                                                \
                 ".word 0x4A180001"                                         \
                 : : : "$8", "$9", "$10")

#define gte_strgb0_bound()                                                  \
    asm volatile("swc2 $20,0($8)" : : "r"(gte_out0) : "memory")
#define gte_strgb1_bound()                                                  \
    asm volatile("swc2 $21,0($9)" : : "r"(gte_out1) : "memory")
#define gte_strgb2_bound()                                                  \
    asm volatile("swc2 $22,0($10)" : : "r"(gte_out2) : "memory")
#define gte_stsxy_three_0()                                                 \
    asm volatile("swc2 $12,0($8)" : : "r"(gte_out0) : "memory")
#define gte_stsxy_three_1()                                                 \
    asm volatile("swc2 $13,0($9)" : : "r"(gte_out1) : "memory")
#define gte_stsxy_three_2()                                                 \
    asm volatile("swc2 $14,0($10)" : : "r"(gte_out2) : "memory")
#define gte_stsxy2_three_0()                                                \
    asm volatile("swc2 $14,0($8)" : : "r"(gte_out0) : "memory")
#define gte_stir0_three_1()                                                 \
    asm volatile("swc2 $8,0($9)" : : "r"(gte_out1) : "memory")

#define gte_getflag(out) ((out) = Psyz_GteCtrlRead(31))

#define gte_declare_rt4_results(first_flags, flags, depth)                   \
    int first_flags;                                     \
    int flags;                                           \
    int depth

/* IR0 is the scalar input used by GPF. */
#define gte_ldir0(value) Psyz_GteDataWrite(8, (u32)(value))

#define gte_ldmac123(x, y, z) \
    asm volatile("mtc2 %0,$25\n\t" \
                 "mtc2 %1,$26\n\t" \
                 "mtc2 %2,$27" \
                 : : "r"(x), "r"(y), "r"(z))

#define gte_lddqa(value) Psyz_GteCtrlWrite(27, (u32)(value))

#define gte_lddqb(value) Psyz_GteCtrlWrite(28, (u32)(value))

/* SQR squares IR1..IR3 into MAC1..MAC3. */
#define gte_sqr() \
    asm volatile("nop\n\t" \
                 ".word 0x4AA00428")

/* SQR with the PSY-Q SF flag set. */
#define gte_sqr12() \
    asm volatile("nop\n\t" \
                 ".word 0x4AA80428")

/* AVSZ uses the four depth FIFO registers and writes OTZ. */
#define gte_ldsz3(z1, z2, z3) \
    asm volatile("mtc2 %0,$17\n\t" \
                 "mtc2 %1,$18\n\t" \
                 "mtc2 %2,$19" \
                 : : "r"(z1), "r"(z2), "r"(z3))

#define gte_ldsz4(z0, z1, z2, z3) \
    asm volatile("mtc2 %0,$16\n\t" \
                 "mtc2 %1,$17\n\t" \
                 "mtc2 %2,$18\n\t" \
                 "mtc2 %3,$19" \
                 : : "r"(z0), "r"(z1), "r"(z2), "r"(z3))

#define gte_avsz3() \
    asm volatile("nop\n\t" \
                 ".word 0x4B58002D")

#define gte_avsz4() \
    asm volatile("nop\n\t" \
                 ".word 0x4B68002E")

#define gte_getotz(out) ((out) = Psyz_GteDataRead(7))

#define gte_getflag(out) ((out) = Psyz_GteCtrlRead(31))

/* Colour interpolation commands with the current IR vector and IR0. */
#define gte_dpcl() \
    asm volatile("nop\n\t" \
                 ".word 0x4A680029")

#define gte_intpl() \
    asm volatile("nop\n\t" \
                 ".word 0x4A980011")

/* GPF multiplies IR1..IR3 by IR0 into MAC1..MAC3. */
#define gte_gpf() \
    asm volatile("nop\n\t" \
                 "nop\n\t" \
                 ".word 0x4B90003D")

#define gte_gpf0() \
    asm volatile("nop\n\t" \
                 ".word 0x4B90003D")

#define gte_gpf12() \
    asm volatile("nop\n\t" \
                 ".word 0x4B98003D")

#define gte_gpl0() \
    asm volatile("nop\n\t" \
                 ".word 0x4BA0003E")

#define gte_gpl12() \
    asm volatile("nop\n\t" \
                 ".word 0x4BA8003E")

/* Fixed-staging used by the handwritten PSY-Q transfer wrappers. */
#define gte_declare_xy_staging()                                             \
    int gte_x;                                            \
    int gte_y

#define gte_declare_xyz_staging()                                            \
    gte_declare_xy_staging();                                                \
    int gte_z

#define gte_declare_shift12() int gte_shift
#define gte_set_shift12() (gte_shift = 12)

#define gte_load_byte2(input)                                                \
    {                                                                     \
        gte_x = ((unsigned char *)(input))[0];                               \
        gte_y = ((unsigned char *)(input))[1];                               \
    }

#define gte_load_byte3(input)                                                \
    {                                                                     \
        gte_load_byte2(input);                                               \
        gte_z = ((unsigned char *)(input))[2];                               \
    }

#define gte_ldir0_ir12(scale)                                                \
    asm volatile("mtc2 %0,$8\n\t"                                          \
                 "mtc2 $8,$9\n\t"                                          \
                 "mtc2 $9,$10"                                               \
                 : : "r"(scale), "r"(gte_x), "r"(gte_y))

#define gte_ldir0_ir123(scale)                                               \
    asm volatile("mtc2 %0,$8\n\t"                                          \
                 "mtc2 $8,$9\n\t"                                          \
                 "mtc2 $9,$10\n\t"                                         \
                 "mtc2 $10,$11"                                              \
                 : : "r"(scale), "r"(gte_x), "r"(gte_y), "r"(gte_z))

/* GPL without an inserted hazard slot, for wrappers which already provide it. */
#define gte_gpl0_now() Psyz_GteCommand(0x4BA0003Eu & 0x01ffffffu)
#define gte_gpl12_now() Psyz_GteCommand(0x4BA8003Eu & 0x01ffffffu)

/* The extra inputs keep the caller's output/shift setup before MAC reads. */
#define gte_getmac12_staged(out)                                             \
    asm volatile("mfc2 $8,$25\n\t"                                         \
                 "mfc2 $9,$26"                                              \
                 : "=r"(gte_x), "=r"(gte_y)                                 \
                 : "r"(out), "r"(gte_shift) : "memory")

#define gte_getmac123_staged(out)                                            \
    asm volatile("mfc2 $8,$25\n\t"                                         \
                 "mfc2 $9,$26\n\t"                                         \
                 "mfc2 $10,$27"                                             \
                 : "=r"(gte_x), "=r"(gte_y), "=r"(gte_z)                    \
                 : "r"(out), "r"(gte_shift) : "memory")

#define gte_op0() \
    asm volatile("nop\n\t" \
                 ".word 0x4B70000C")

#define gte_op12() \
    asm volatile("nop\n\t" \
                 ".word 0x4B78000C")

#define gte_getlzcr_now(out) ((out) = Psyz_GteDataRead(31))

#define gte_getmac123(x, y, z) \
    asm volatile("mfc2 %0,$25\n\t" \
                 "mfc2 %1,$26\n\t" \
                 "mfc2 %2,$27" \
                 : "=r"(x), "=r"(y), "=r"(z))

#define gte_rtir12() \
    asm volatile("nop\n\t" \
                 ".word 0x4A49E012")

/* Individual triple-vertex transfers for the morph projection path. */
#define gte_lwc2_2_8(p) do { u32 word; memcpy(&word, (const u8 *)(p) + 8, 4); Psyz_GteDataWrite(2, word); } while (0)
#define gte_lwc2_3_12(p) do { u32 word; memcpy(&word, (const u8 *)(p) + 12, 4); Psyz_GteDataWrite(3, word); } while (0)
#define gte_lwc2_4_16(p) do { u32 word; memcpy(&word, (const u8 *)(p) + 16, 4); Psyz_GteDataWrite(4, word); } while (0)
#define gte_lwc2_5_20(p) do { u32 word; memcpy(&word, (const u8 *)(p) + 20, 4); Psyz_GteDataWrite(5, word); } while (0)
#define gte_swc2_12_0(p) do { u32 word = Psyz_GteDataRead(12); memcpy((u8 *)(p) + 0, &word, 4); } while (0)
#define gte_swc2_13_4(p) do { u32 word = Psyz_GteDataRead(13); memcpy((u8 *)(p) + 4, &word, 4); } while (0)
#define gte_swc2_14_8(p) do { u32 word = Psyz_GteDataRead(14); memcpy((u8 *)(p) + 8, &word, 4); } while (0)
#define gte_swc2_17_0(p) do { u32 word = Psyz_GteDataRead(17); memcpy((u8 *)(p) + 0, &word, 4); } while (0)
#define gte_swc2_18_4(p) do { u32 word = Psyz_GteDataRead(18); memcpy((u8 *)(p) + 4, &word, 4); } while (0)
#define gte_swc2_19_8(p) do { u32 word = Psyz_GteDataRead(19); memcpy((u8 *)(p) + 8, &word, 4); } while (0)

/* RTPS: project V0 and update SXY2/SZ3. */
#define gte_rtps() \
    asm volatile("nop\n\t" \
                 "nop\n\t" \
                 ".word 0x4A180001")

#define gte_stsxy2(out) do { u32 word = Psyz_GteDataRead(14); memcpy((u8 *)(out) + 0, &word, 4); } while (0)

#define gte_getsz3(out) ((out) = Psyz_GteDataRead(19))

#define gte_getsz3_for_store(value, out) \
    asm volatile("mfc2 %0,$19\n\t" \
                 "nop" : "=r"(value) : "r"(out))

/* RTPT: project V0, V1, and V2. */
#define gte_ldv012(vec) \
    asm volatile("lwc2 $0,0(%0)\n\t" \
                 "lwc2 $1,4(%0)\n\t" \
                 "lwc2 $2,8(%0)\n\t" \
                 "lwc2 $3,12(%0)\n\t" \
                 "lwc2 $4,16(%0)\n\t" \
                 "lwc2 $5,20(%0)" \
                 : : "r"(vec) : "memory")

#define gte_rtpt() \
    asm volatile("nop\n\t" \
                 ".word 0x4A280030")

#define gte_declare_separate_outputs()                                      \
    void *gte_out1;                                      \
    void *gte_out2;                                      \
    void *gte_depth_out;                                \
    void *gte_flags_out;                                \
    int gte_flags;                                       \
    int gte_depth_value

/* Keep the ABI-bound output registers free until RTPT has completed. */
#define gte_rtpt_separate()                                                 \
    asm volatile("nop\n\t"                                                \
                 ".word 0x4A280030"                                         \
                 : : : "$8", "$9", "$10", "$11")

#define gte_bind_separate_outputs(out1, out2, depth, flags)                  \
    {                                                                     \
        gte_out1 = (out1);                                                   \
        gte_out2 = (out2);                                                   \
        gte_depth_out = (depth);                                             \
        gte_flags_out = (flags);                                             \
    }

#define gte_stsxy0(out) do { u32 word = Psyz_GteDataRead(12); memcpy((u8 *)(out) + 0, &word, 4); } while (0)
#define gte_stsxy1(out) do { u32 word = Psyz_GteDataRead(13); memcpy((u8 *)(out) + 0, &word, 4); } while (0)
#define gte_stsxy1_bound()                                                  \
    asm volatile("swc2 $13,0($8)" : : "r"(gte_out1) : "memory")
#define gte_stsxy2_bound()                                                  \
    asm volatile("swc2 $14,0($9)" : : "r"(gte_out2) : "memory")
#define gte_stir0(out) do { u32 word = Psyz_GteDataRead(8); memcpy((u8 *)(out) + 0, &word, 4); } while (0)
#define gte_stir0_bound()                                                   \
    asm volatile("swc2 $8,0($10)" : : "r"(gte_depth_out) : "memory")
#define gte_getflag_bound()                                                 \
    asm volatile("cfc2 $3,$31" : "=r"(gte_flags))
#define gte_getsz3_bound()                                                  \
    asm volatile("mfc2 $2,$19" : "=r"(gte_depth_value))
#define gte_stsxy012(out) \
    asm volatile("swc2 $12,0(%0)\n\t" \
                 "swc2 $13,4(%0)\n\t" \
                 "swc2 $14,8(%0)" \
                 : : "r"(out) : "memory")

#define gte_stsz123(out) \
    asm volatile("swc2 $17,0(%0)\n\t" \
                 "swc2 $18,4(%0)\n\t" \
                 "swc2 $19,8(%0)" \
                 : : "r"(out) : "memory")

/* Single COP2 transfers used by the subdividing polygon renderer.
 * Precise memory operands describe the touched word instead of all memory.
 * The keep variants retain allocator references (one VXY0 input, one extra
 * SXY1 address input, three extra SXY2 address inputs); these are matching
 * debt, not additional hardware operations. Padded commands retain the two
 * retail transfer-hazard slots. */
#define gte_ldvxy0_precise_keep(address, keep) do { u32 word; memcpy(&word, (const u8 *)(address) + 0, 4); Psyz_GteDataWrite(0, word); } while (0)
#define gte_stsxy1_precise_keep_address(address) do { u32 word = Psyz_GteDataRead(13); memcpy((u8 *)(address) + 0, &word, 4); } while (0)
#define gte_stsxy2_precise_keep_address(address) do { u32 word = Psyz_GteDataRead(14); memcpy((u8 *)(address) + 0, &word, 4); } while (0)
#define gte_ldvxy0_mem(address) asm volatile("lwc2 $0,%0" : : "m"(*(const u32 *)(address)))
#define gte_ldvz0_mem(address) asm volatile("lwc2 $1,%0" : : "m"(*(const u32 *)((const u8 *)(address) + 4)))
#define gte_ldvxy0_precise(address) do { u32 word; memcpy(&word, (const u8 *)(address) + 0, 4); Psyz_GteDataWrite(0, word); } while (0)
#define gte_ldvz0_precise(address) do { u32 word; memcpy(&word, (const u8 *)(address) + 4, 4); Psyz_GteDataWrite(1, word); } while (0)
#define gte_ldvxy1_precise(address) do { u32 word; memcpy(&word, (const u8 *)(address) + 0, 4); Psyz_GteDataWrite(2, word); } while (0)
#define gte_ldvz1_precise(address) do { u32 word; memcpy(&word, (const u8 *)(address) + 4, 4); Psyz_GteDataWrite(3, word); } while (0)
#define gte_ldvxy2_precise(address) do { u32 word; memcpy(&word, (const u8 *)(address) + 0, 4); Psyz_GteDataWrite(4, word); } while (0)
#define gte_ldvz2_precise(address) do { u32 word; memcpy(&word, (const u8 *)(address) + 4, 4); Psyz_GteDataWrite(5, word); } while (0)
#define gte_stsxy2_precise(address) do { u32 word = Psyz_GteDataRead(14); memcpy((u8 *)(address) + 0, &word, 4); } while (0)
#define gte_ldvxy1_mem(address) asm volatile("lwc2 $2,%0" : : "m"(*(const u32 *)((const u8 *)(address) + 0)))
#define gte_stsxy2_mem(address) asm volatile("swc2 $14,%0" : "=m"(*(u32 *)(address)) :)
#define gte_stmac0_precise(address) do { u32 word = Psyz_GteDataRead(24); memcpy((u8 *)(address) + 0, &word, 4); } while (0)
#define gte_stotz_precise(address) do { u32 word = Psyz_GteDataRead(7); memcpy((u8 *)(address) + 0, &word, 4); } while (0)
#define gte_stsxy0_precise(address) do { u32 word = Psyz_GteDataRead(12); memcpy((u8 *)(address) + 0, &word, 4); } while (0)
#define gte_stsxy1_precise(address) do { u32 word = Psyz_GteDataRead(13); memcpy((u8 *)(address) + 0, &word, 4); } while (0)

/* PSY-Q style three-operand transfers: load V0..V2 from three vectors and
 * store SXY0..SXY2 to three addresses in one statement. */
#define gte_ldv3(r1, r2, r3) do { gte_ldv0(r1); gte_ldv1(r2); gte_ldv2(r3); } while (0)

#define gte_stsxy3(r1, r2, r3) \
    asm volatile("swc2 $12,0(%0)\n\t" \
                 "swc2 $13,0(%1)\n\t" \
                 "swc2 $14,0(%2)" \
                 : : "r"(r1), "r"(r2), "r"(r3) : "memory")

/* Store SZ3 / 4 through a pointer: the ordering-table depth of a projected
 * primitive (PSY-Q gte_stszotz). */
#define gte_stszotz(out) \
    asm volatile("mfc2 $12,$19\n\t" \
                 "nop\n\t" \
                 "sra $12,$12,2\n\t" \
                 "sw $12,0(%0)" \
                 : : "r"(out) : "$12", "memory")

#define gte_rtpt_padded() Psyz_GteCommand(0x4A280030u & 0x01ffffffu)
#define gte_avsz3_padded() Psyz_GteCommand(0x4B58002Du & 0x01ffffffu)

/* PSY-Q gte_ldclmv / gte_rtir / gte_stclmv: one strided matrix column through
 * RTIR (sf=1), the three steps of the SDK's inline gte_MulMatrix0. */
#define gte_ldclmv(column) do { const u8 *pe_gte_src = (const u8 *)(column); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u16 pe_gte_half; memcpy(&pe_gte_half, pe_gte_src + pe_gte_i * 6, 2); Psyz_GteDataWrite(9 + pe_gte_i, pe_gte_half); } } while (0)

#define gte_rtir() Psyz_GteCommand(0x4A49E012u & 0x01ffffffu)

#define gte_stclmv(column) do { u8 *pe_gte_dst = (u8 *)(column); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u16 pe_gte_half = (u16)Psyz_GteDataRead(9 + pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 6, &pe_gte_half, 2); } } while (0)

/* PSY-Q gte_ldlv0 / gte_rt / gte_stlvl: a long vector through RT with
 * translation (sf=1), as used by the SDK's inline gte_CompMatrix. */
#define gte_ldlv0(vector) do { const u8 *pe_gte_src = (const u8 *)(vector); u16 pe_gte_x, pe_gte_y; u32 pe_gte_z; memcpy(&pe_gte_x, pe_gte_src, 2); memcpy(&pe_gte_y, pe_gte_src + 4, 2); memcpy(&pe_gte_z, pe_gte_src + 8, 4); Psyz_GteDataWrite(0, pe_gte_x | ((u32)pe_gte_y << 16)); Psyz_GteDataWrite(1, pe_gte_z); } while (0)

#define gte_rt() Psyz_GteCommand(0x4A480012u & 0x01ffffffu)

/* PSY-Q inline_c.h gte_stlvl: store IR1..3 as a long vector. */
#define gte_stlvl(vector) do { u8 *pe_gte_dst = (u8 *)(vector); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u32 pe_gte_word = Psyz_GteDataRead(9 + pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 4, &pe_gte_word, 4); } } while (0)

/* PSY-Q inline_c.h gte_MulMatrix0 / gte_CompMatrix (r3 = r1 * r2, with
 * the composed translation for CompMatrix). */
#define gte_MulMatrix0(r1, r2, r3) \
    { \
        gte_ldrotmatrix(r1); \
        gte_ldclmv(r2); \
        gte_rtir(); \
        gte_stclmv(r3); \
        gte_ldclmv((char *)(r2) + 2); \
        gte_rtir(); \
        gte_stclmv((char *)(r3) + 2); \
        gte_ldclmv((char *)(r2) + 4); \
        gte_rtir(); \
        gte_stclmv((char *)(r3) + 4); \
    }

#define gte_CompMatrix(r1, r2, r3) \
    { \
        gte_MulMatrix0(r1, r2, r3); \
        gte_ldtransmatrix(r1); \
        gte_ldlv0((char *)(r2) + 20); \
        gte_rt(); \
        gte_stlvl((char *)(r3) + 20); \
    }

/* gte_CompMatrix storing the composed translation from MAC1..3
 * (gte_stlvnl) instead of IR1..3; the skinned and bone transforms use it. */
#define gte_CompMatrixMac(r1, r2, r3) \
    { \
        gte_MulMatrix0(r1, r2, r3); \
        gte_ldtransmatrix(r1); \
        gte_ldlv0((char *)(r2) + 20); \
        gte_rt(); \
        gte_stlvnl((char *)(r3) + 20); \
    }

/* Stock PSY-Q inline_c.h (DMPSX version 3) forms, copied for
 * and instruction for instruction.  The SDK writes the COP2 commands
 * as DMPSX placeholder words; the real command words DMPSX substitutes are
 * used here.  Kept separate from the older gte_ldopv1 / gte_op12 shapes above,
 * which matched code depends on. */

/* PSY-Q gte_ldopv1: R11, R22, R33 from a long vector through $12..$14. */
#define gte_ldopv1_psyq(r0) \
    asm volatile("lw $12,0(%0)\n\t" \
                 "lw $13,4(%0)\n\t" \
                 "ctc2 $12,$0\n\t" \
                 "lw $14,8(%0)\n\t" \
                 "ctc2 $13,$2\n\t" \
                 "ctc2 $14,$4" \
                 : : "r"(r0) : "$12", "$13", "$14")

/* PSY-Q gte_SetGeomOffset: OFX/OFY from two integers shifted to 16.16. */
#define gte_SetGeomOffset(r0, r1) \
    asm volatile("sll $12,%0,16\n\t" \
                 "sll $13,%1,16\n\t" \
                 "ctc2 $12,$24\n\t" \
                 "ctc2 $13,$25" \
                 : : "r"(r0), "r"(r1) : "$12", "$13")

/* PSY-Q gte_ldopv2: IR3, IR1, IR2 from a long vector. */
#define gte_ldopv2(r0) \
    asm volatile("lwc2 $11,8(%0)\n\t" \
                 "lwc2 $9,0(%0)\n\t" \
                 "lwc2 $10,4(%0)" \
                 : : "r"(r0))

/* PSY-Q gte_op12: two hazard nops, then OP with sf=1. */
#define gte_op12_psyq() do { Psyz_GteCommand(0x4B78000Cu & 0x01ffffffu); } while (0)

/* PSY-Q gte_SetBackColor: BK from three colour components scaled by 16. */
#define gte_SetBackColor(r0, r1, r2) \
    asm volatile("sll $12,%0,4\n\t" \
                 "sll $13,%1,4\n\t" \
                 "sll $14,%2,4\n\t" \
                 "ctc2 $12,$13\n\t" \
                 "ctc2 $13,$14\n\t" \
                 "ctc2 $14,$15" \
                 : : "r"(r0), "r"(r1), "r"(r2) : "$12", "$13", "$14")

/* PSY-Q gte_SetLightMatrix: the five packed words of L11..L33. */
#define gte_SetLightMatrix(r0) do { const u8 *pe_gte_src = (const u8 *)(r0); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 5; ++pe_gte_i) { u32 pe_gte_word; memcpy(&pe_gte_word, pe_gte_src + 0 + pe_gte_i * 4, 4); Psyz_GteCtrlWrite(8 + pe_gte_i, pe_gte_word); } } while (0)

/* PSY-Q gte_ncct: two hazard nops, then NCCT. */
#define gte_ncct() Psyz_GteCommand(0x4B18043Fu & 0x01ffffffu)

/* PSY-Q gte_strgb3: store RGB0..RGB2 to three addresses. */
#define gte_strgb3(r0, r1, r2) \
    asm volatile("swc2 $20,0(%0)\n\t" \
                 "swc2 $21,0(%1)\n\t" \
                 "swc2 $22,0(%2)" \
                 : : "r"(r0), "r"(r1), "r"(r2) : "memory")

/* PSY-Q gte_stlvnl: store MAC1..3 as a long vector. */
#define gte_stlvnl(r0) do { u8 *pe_gte_dst = (u8 *)(r0); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u32 pe_gte_word = Psyz_GteDataRead(25 + pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 4, &pe_gte_word, 4); } } while (0)

/* Store the whole GTE matrix: R11..R33 as five packed words (PSY-Q
 * gte_ReadRotMatrix) followed by TRX, TRY, TRZ at +20. */
#define gte_ReadMatrix(r0) do { u8 *pe_gte_dst = (u8 *)(r0); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 8; ++pe_gte_i) { u32 pe_gte_word = Psyz_GteCtrlRead(pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 4, &pe_gte_word, 4); } } while (0)

/* PSY-Q gte_ldv3c: V0..V2 from three consecutive short vectors. */
#define gte_ldv3c(r0) do { const u8 *pe_gte_vectors = (const u8 *)(r0); gte_ldv0(pe_gte_vectors); gte_ldv1(pe_gte_vectors + 8); gte_ldv2(pe_gte_vectors + 16); } while (0)

/* PSY-Q gte_stsxy3c / gte_stsz3c: SXY0..2 and SZ1..3 to consecutive words. */
#define gte_stsxy3c(r0) do { u8 *pe_gte_dst = (u8 *)(r0); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u32 pe_gte_word = Psyz_GteDataRead(12 + pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 4, &pe_gte_word, 4); } } while (0)
#define gte_stsz3c(r0) do { u8 *pe_gte_dst = (u8 *)(r0); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 3; ++pe_gte_i) { u32 pe_gte_word = Psyz_GteDataRead(17 + pe_gte_i); memcpy(pe_gte_dst + pe_gte_i * 4, &pe_gte_word, 4); } } while (0)

/* PSY-Q gte_SetColorMatrix: the five packed words of LR1..LB3. */
#define gte_SetColorMatrix(r0) do { const u8 *pe_gte_src = (const u8 *)(r0); unsigned pe_gte_i; for (pe_gte_i = 0; pe_gte_i < 5; ++pe_gte_i) { u32 pe_gte_word; memcpy(&pe_gte_word, pe_gte_src + 0 + pe_gte_i * 4, 4); Psyz_GteCtrlWrite(16 + pe_gte_i, pe_gte_word); } } while (0)

#endif
