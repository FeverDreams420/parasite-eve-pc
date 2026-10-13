/* PSY-Q LIBGTE FGO_03: RotMatrixZYX. */

#include "pe1/gte_sine_table.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_nop.h"

/* Q12 rotation with the original wrap and rounding behavior. */
GteMatrix *RotMatrixZYX(GteShortVector *angles, GteMatrix *matrix) {
    GteMatrix *result;
    unsigned long long product;
    unsigned low;
    s32 cx;
    s32 cy;
    s32 cz;
    s32 sx;
    s32 sy;
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
    value = word << 16;
    value = value >> 16;
    sx = 0U - value;
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
    value = word << 16;
    value = value >> 16;
    sy = 0U - value;
    cy = word >> 16;
    } else {
    temp = word << 2;
        word = GTE_SINE_WORD_AT_OFFSET(temp);
        PE1_NOP_MEMORY_DEP("r", word);
        value = word << 16;
        sy = value >> 16;
        value = 0U - sy;
        cy = word >> 16;
    }
y_ready:;
    product = GTE_MULTU(sx, cy);
    angle = angles->z;
    matrix->m[2][0] = value;
    temp = low;
    value = temp >> 12;
    product = GTE_MULTU(cx, cy);
    matrix->m[2][1] = value;
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
    product = GTE_MULTU(cy, cz);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    value = angle >> 12;
    matrix->m[0][0] = value;
    product = GTE_MULTU(sz, cy);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    value = angle >> 12;
    PE1_NOP_MEMORY_DEP("r", value);
    product = GTE_MULTU(sx, sy);
    matrix->m[1][0] = value;
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    temp = angle >> 12;
    product = GTE_MULTU(temp, cz);
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
    angle = value - word;
    product = GTE_MULTU(cx, cz);
    matrix->m[0][1] = angle;
    PE1_NOP_MEMORY_DEP("x", product);
    value = low;
    angle = value >> 12;
    PE1_NOP_MEMORY_DEP("r", angle);
    product = GTE_MULTU(temp, sz);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    value = low;
    word = value >> 12;
    value = word + angle;
    product = GTE_MULTU(sy, cx);
    matrix->m[1][1] = value;
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    temp = angle >> 12;
    product = GTE_MULTU(temp, cz);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    value = angle >> 12;
    PE1_NOP_MEMORY_DEP("r", value);
    product = GTE_MULTU(sx, sz);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    angle = low;
    word = angle >> 12;
    angle = value + word;
    product = GTE_MULTU(sx, cz);
    matrix->m[0][2] = angle;
    PE1_NOP_MEMORY_DEP("x", product);
    value = low;
    angle = value >> 12;
    PE1_NOP_MEMORY_DEP("r", angle);
    product = GTE_MULTU(temp, sz);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    value = low;
    word = value >> 12;
    value = word - angle;
    matrix->m[1][2] = value;
    PE1_NOP_MEMORY_DEP("r", value);
    return result;
}
