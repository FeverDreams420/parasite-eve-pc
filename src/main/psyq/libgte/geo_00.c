/* PSY-Q LIBGTE GEO_00: rsin and sin_1. */

#include "geo_00_externs.h"

int sin_1(int angle);

int rsin(int angle) {
    if (angle < 0) {
        return -sin_1((-angle) & 0xFFF);
    }

    return sin_1(angle & 0xFFF);
}

int sin_1(int angle) {
    int value;

    if (angle < 0x801) {
        if (angle < 0x401) {
            return g_SineQuarterTable[angle];
        }

        return g_SineQuarterTable[0x800 - angle];
    }

    if (angle < 0xC01) {
        value = g_SineQuarterTable[angle - 0x800];
        return -value;
    }

    value = g_SineQuarterTable[0x1000 - angle];
    return -value;
}
