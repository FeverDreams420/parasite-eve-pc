#ifndef PE1_GTE_SINE_TABLE_H
#define PE1_GTE_SINE_TABLE_H

#include "common.h"

/* Psy-Q's packed sine/cosine lookup words shared by the GTE rotation family
 * (sine in the low half, cosine in the high half). The room effects read a
 * word, its two halfwords, or the halves as 16-bit bitfields (an SImode
 * extract keeps the lh + sra form of the shifted reads). */
typedef union GteSineWord {
    s32 word;
    s16 half[2];
    struct {
        s16 sin;
        s16 cos;
    } part;
    struct {
        signed int sin : 16;
        signed int cos : 16;
    } bits;
} GteSineWord;

extern GteSineWord rcossin_tbl[];

/* The assembler rotation builders scale the 12-bit angle to a word offset in
 * a register of their own (kept live) and add it to the table base. */
#define GTE_SINE_WORD_AT_OFFSET(byte_offset) \
    (*(u32 *)((u8 *)rcossin_tbl + (byte_offset)))

#endif
