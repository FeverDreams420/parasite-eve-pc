/* PSY-Q LIBGTE FGO_01: RotMatrix. */

#include "pe1/gte_sine_table.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_nop.h"

/* Q12 rotation with the original wrap and rounding behavior. */
GteMatrix *RotMatrix(GteShortVector *angles, GteMatrix *matrix) {
    GteMatrix *result;
    unsigned long long product;
    unsigned low;
    s32 cx;
    s32 cy;
    s32 cz;
    s32 sx;
    s32 negative_sy;
    s32 sz;
    s32 value;
    s32 angle;
    s32 temp;
    s32 word;
    /* Decode packed sine/cosine for each signed angle. */
    angle = angles->x;
    result = matrix;
    word = angle & 0xFFF;
    if (angle < 0) {
    angle = 0U - angle;
    if (angle >= 0)
        angle &= 0xfff;

    temp = angle << 2;
    word = GTE_SINE_WORD_AT_OFFSET(temp);
    PE1_NOP_MEMORY_DEP("r", word);
    temp = word << 16;
    temp = temp >> 16;
    sx = 0U - temp;
    cx = word >> 16;
    } else {
    temp = word << 2;
    word = GTE_SINE_WORD_AT_OFFSET(temp);
    PE1_NOP_MEMORY_DEP("r", word);
    temp = word << 16;
    sx = temp >> 16;
    cx = word >> 16;
    }
x_ready:;
    angle = angles->y;
    word = angle & 0xFFF;
    if (angle < 0) {
    angle = 0U - angle;
    if (angle >= 0)
        angle &= 0xfff;

    temp = angle << 2;
    word = GTE_SINE_WORD_AT_OFFSET(temp);
    PE1_NOP_MEMORY_DEP("r", word);
    negative_sy = word << 16;
    negative_sy = negative_sy >> 16;
    value = 0U - negative_sy;
    cy = word >> 16;
    } else {
    temp = word << 2;
        word = GTE_SINE_WORD_AT_OFFSET(temp);
        PE1_NOP_MEMORY_DEP("r", word);
        value = word << 16;
        value = value >> 16;
        negative_sy = 0U - value;
        cy = word >> 16;
    }
y_ready:;
    product = GTE_MULTU(cy, sx);
    angle = angles->z;
    matrix->m[0][2] = value;
    temp = low;
    word = 0U - temp;
    value = word >> 12;
    product = GTE_MULTU(cy, cx);
    matrix->m[1][2] = value;
    word = angle & 0xFFF;
    if (angle < 0) {
    temp = low;
    value = temp >> 12;
    matrix->m[2][2] = value;
    angle = 0U - angle;
    if (angle >= 0)
        angle &= 0xfff;

    temp = angle << 2;
    word = GTE_SINE_WORD_AT_OFFSET(temp);
    PE1_NOP_MEMORY_DEP("r", word);
    temp = word << 16;
    temp = temp >> 16;
    sz = 0U - temp;
    cz = word >> 16;
    } else {
        angle = low;
        value = angle >> 12;
        matrix->m[2][2] = value;
        temp = word << 2;
        word = GTE_SINE_WORD_AT_OFFSET(temp);
        PE1_NOP_MEMORY_DEP("r", word);
        temp = word << 16;
        sz = temp >> 16;
        cz = word >> 16;
    }
z_ready:;
    product = GTE_MULTU(cz, cy);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    value = angle >> 12;
    matrix->m[0][0] = value;
    product = GTE_MULTU(sz, cy);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    value = 0U - angle;
    angle = value >> 12;
    product = GTE_MULTU(cz, negative_sy);
    matrix->m[0][1] = angle;
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    temp = angle >> 12;
    product = GTE_MULTU(temp, sx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    value = angle >> 12;
    PE1_NOP_MEMORY_DEP("r", value);
    product = GTE_MULTU(sz, cx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    word = angle >> 12;
    angle = word - value;
    product = GTE_MULTU(temp, cx);
    matrix->m[1][0] = angle;
    PE1_NOP_MEMORY_DEP("x", product);
    value = low;
    angle = value >> 12;
    PE1_NOP_MEMORY_DEP("r", angle);
    product = GTE_MULTU(sz, sx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    value = low;
    word = value >> 12;
    value = word + angle;
    product = GTE_MULTU(sz, negative_sy);
    matrix->m[2][0] = value;
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    temp = angle >> 12;
    product = GTE_MULTU(temp, sx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    value = angle >> 12;
    PE1_NOP_MEMORY_DEP("r", value);
    product = GTE_MULTU(cz, cx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    word = angle >> 12;
    angle = word + value;
    product = GTE_MULTU(temp, cx);
    matrix->m[1][1] = angle;
    PE1_NOP_MEMORY_DEP("x", product);
    value = low;
    angle = value >> 12;
    PE1_NOP_MEMORY_DEP("r", angle);
    product = GTE_MULTU(cz, sx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    value = low;
    word = value >> 12;
    value = word - angle;
    matrix->m[2][1] = value;
    PE1_NOP_MEMORY_DEP("r", value);
    return result;
}
