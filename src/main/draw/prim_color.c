#include "common.h"
#include "pe1/boot.h"

/* The original range scaler uses the low 16 bits of the frame generator.
 * Keep the signed product's high bits; an unsigned scale changes negative
 * ranges and the enemy scripts' choices. */
int Draw_PackPrimColor(int low, int high) {
    s32 range = (s32)((u32)high - (u32)low);
    s64 product = (s64)(u16)Draw_FlushPrimQueue() * range;
    return (s32)((u32)low + (u32)(product >> 16));
}
