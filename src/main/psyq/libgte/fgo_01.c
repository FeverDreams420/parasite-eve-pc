/* ASSEMBLER: GNU */
/* PSY-Q LIBGTE FGO_01: RotMatrix. */

#include "pe1/gte_sine_table.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_nop.h"

/* Q12 rotation with retail wrap/rounding order. Pins and empty HI/LO
 * scheduling constraints are tracked matching debt. The NOP slots reproduce
 * the handwritten LIBGTE schedule and are not debt.
 * Provenance: configs/USA/psyq_provenance.json (LIBGTE FGO objects).
 */
GteMatrix *RotMatrix(GteShortVector *angles, GteMatrix *matrix) {
    register GteMatrix *result;
    unsigned long long product;
    unsigned low;
    register s32 cx asm("$8");
    register s32 cy asm("$9");
    register s32 cz asm("$10");
    register s32 sx asm("$11");
    register s32 negative_sy asm("$12");
    register s32 sz asm("$13");
    register s32 value asm("$14");
    register s32 angle asm("$15");
    register s32 temp asm("$24");
    register s32 word asm("$25");
    /* Decode packed sine/cosine for each signed angle. */
    angle = angles->x;
    result = matrix;
    word = angle & 0xFFF;
    if (angle < 0) {
    angle = 0U - angle;
    if (angle < 0)
        asm volatile("" : : : "15");
    else
        angle &= 0xfff;
    asm volatile("" : : : "24");

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
    asm volatile("" : "=r"(cx) : "0"(cx));
    }
x_ready:;
    angle = angles->y;
    word = angle & 0xFFF;
    if (angle < 0) {
    angle = 0U - angle;
    if (angle < 0)
        asm volatile("" : : : "15");
    else
        angle &= 0xfff;
    asm volatile("" : : : "24");

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
        asm volatile("" : "=r"(cy) : "0"(cy));
    }
y_ready:;
    product = GTE_MULTU(cy, sx);
    angle = angles->z;
    matrix->m[0][2] = value;
    asm volatile("" : "=l"(low) : "x"(product));
    temp = low;
    asm volatile("" : "=r"(temp) : "0"(temp));
    word = 0U - temp;
    value = word >> 12;
    product = GTE_MULTU(cy, cx);
    asm volatile("" : : "x"(product) : "memory");
    matrix->m[1][2] = value;
    word = angle & 0xFFF;
    if (angle < 0) {
    asm volatile("" : "=l"(low) : "x"(product));
    temp = low;
    asm volatile("" : "=r"(temp) : "0"(temp));
    value = temp >> 12;
    matrix->m[2][2] = value;
    angle = 0U - angle;
    asm volatile("" : "=r"(angle) : "0"(angle));
    if (angle < 0)
        asm volatile("" : : : "15");
    else
        angle &= 0xfff;
    asm volatile("" : : : "24");

    temp = angle << 2;
    word = GTE_SINE_WORD_AT_OFFSET(temp);
    PE1_NOP_MEMORY_DEP("r", word);
    temp = word << 16;
    temp = temp >> 16;
    sz = 0U - temp;
    cz = word >> 16;
    } else {
    asm volatile("" : "=l"(low) : "x"(product));
        angle = low;
        asm volatile("" : "=r"(angle) : "0"(angle));
        value = angle >> 12;
        matrix->m[2][2] = value;
        asm volatile("" : : "m"(matrix->m[2][2]) : "memory");
        temp = word << 2;
        word = GTE_SINE_WORD_AT_OFFSET(temp);
        PE1_NOP_MEMORY_DEP("r", word);
        temp = word << 16;
        sz = temp >> 16;
        cz = word >> 16;
        asm volatile("" : "=r"(cz) : "0"(cz));
    }
z_ready:;
    product = GTE_MULTU(cz, cy);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    angle = low;
    asm volatile("" : "=r"(angle) : "0"(angle));
    value = angle >> 12;
    matrix->m[0][0] = value;
    asm volatile("" : : "m"(matrix->m[0][0]) : "memory");
    product = GTE_MULTU(sz, cy);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    angle = low;
    asm volatile("" : "=r"(angle) : "0"(angle));
    value = 0U - angle;
    angle = value >> 12;
    asm volatile("" : "=r"(angle) : "0"(angle));
    product = GTE_MULTU(cz, negative_sy);
    matrix->m[0][1] = angle;
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    angle = low;
    asm volatile("" : "=r"(angle) : "0"(angle));
    temp = angle >> 12;
    product = GTE_MULTU(temp, sx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    angle = low;
    asm volatile("" : "=r"(angle) : "0"(angle));
    value = angle >> 12;
    PE1_NOP_MEMORY_DEP("r", value);
    product = GTE_MULTU(sz, cx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    angle = low;
    asm volatile("" : "=r"(angle) : "0"(angle));
    word = angle >> 12;
    angle = word - value;
    asm volatile("" : "=r"(angle) : "0"(angle));
    product = GTE_MULTU(temp, cx);
    matrix->m[1][0] = angle;
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    value = low;
    asm volatile("" : "=r"(value) : "0"(value));
    angle = value >> 12;
    PE1_NOP_MEMORY_DEP("r", angle);
    product = GTE_MULTU(sz, sx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    value = low;
    asm volatile("" : "=r"(value) : "0"(value));
    word = value >> 12;
    value = word + angle;
    asm volatile("" : "=r"(value) : "0"(value));
    product = GTE_MULTU(sz, negative_sy);
    matrix->m[2][0] = value;
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    angle = low;
    asm volatile("" : "=r"(angle) : "0"(angle));
    temp = angle >> 12;
    product = GTE_MULTU(temp, sx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    angle = low;
    asm volatile("" : "=r"(angle) : "0"(angle));
    value = angle >> 12;
    PE1_NOP_MEMORY_DEP("r", value);
    product = GTE_MULTU(cz, cx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    angle = low;
    asm volatile("" : "=r"(angle) : "0"(angle));
    word = angle >> 12;
    angle = word + value;
    asm volatile("" : "=r"(angle) : "0"(angle));
    product = GTE_MULTU(temp, cx);
    matrix->m[1][1] = angle;
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    value = low;
    asm volatile("" : "=r"(value) : "0"(value));
    angle = value >> 12;
    PE1_NOP_MEMORY_DEP("r", angle);
    product = GTE_MULTU(cz, sx);
    PE1_NOP_MEMORY_DEP("x", product);
    PE1_NOP_MEMORY_DEP("x", product);
    asm volatile("" : "=l"(low) : "x"(product));
    value = low;
    asm volatile("" : "=r"(value) : "0"(value));
    word = value >> 12;
    value = word - angle;
    matrix->m[2][1] = value;
    PE1_NOP_MEMORY_DEP("r", value);
    return result;
}
