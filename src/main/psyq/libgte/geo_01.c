/* PSY-Q LIBGTE GEO_01: rcos. */
/* Quarter-wave table views used by the original cosine object. */

#include "geo_01_externs.h"

int rcos(int angle) {
    int value;

    if (angle < 0) {
        angle = (int)(0U - (unsigned int)angle);
    }

    angle &= 0xFFF;
    if (angle < 0x801) {
        if (angle < 0x401) {
            return g_SineQuarterTable[0x400 - angle];
        }

        value = g_SineQuarterTable[angle - 0x400];
        return -value;
    }

    if (angle < 0xC01) {
        value = g_SineQuarterTable[0xC00 - angle];
        return -value;
    }

    return g_SineQuarterTable[angle - 0xC00];
}
