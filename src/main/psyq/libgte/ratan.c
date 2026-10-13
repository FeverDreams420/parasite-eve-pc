/* PSY-Q LIBGTE RATAN: ratan2. */

#include "common.h"
#include "pe1/gte_types.h"

/* A function section avoids GAS 2.7's mandatory 16-byte .text padding. */
int ratan2(int y, int x) {
    int negative_x = 0;
    int negative_y = 0;
    int index;
    int angle;
    int result;
    int y_less_than_x;
    if (x < 0) { negative_x = 1; x = (int)(0u - x); }
    if (y < 0) { negative_y = 1; y = (int)(0u - y); }
    y_less_than_x = y < x;
    if (x == 0) {
        result = 0;
        if (y == 0) return result;
        y_less_than_x = y < x;
    }
    if (y_less_than_x) {
        if (y & 0x7FE00000) {
            y = y / (x >> 10);
            index = y << 1;
            /* Keep the shift before the jump; GAS fills its delay slot. */
        }
        else {
            y = (int)(y << 10) / x;
            index = y << 1;
        }
        {
            GteAtanEntryAddress at;
            at.entry = atan_table;
            at.word += index;
            angle = *at.entry;
        }
    } else {
        if (x & 0x7FE00000) {
            y = x / (y >> 10);
            index = y << 1;
        }
        else {
            y = (int)(x << 10) / y;
            index = y << 1;
        }
        {
            GteAtanEntryAddress at;
            at.entry = atan_table;
            at.word += index;
            angle = *at.entry;
        }
        angle = 1024 - angle;
    }
    if (negative_x) angle = 2048 - angle;
    if (negative_y) angle = -angle;
    result = angle;
done:
    return result;
}
