#include "common.h"
#include "pe1/boot.h"

/* Retail's negative tail reads 14 instruction words preceding its two
 * cursors. They are algorithm data on PC, not executable host memory. */
u32 g_DrawPrimGeneratorWords[33] = {
    0xAF0A0000,0x001F1825,0x0C01C35B,0x00A42822,0x3042FFFF,0x00450018,0x0003F825,
    0x00001012,0x00005810,0x00021402,0x000B5C00,0x004B1025,0x03E00008,0x00441020
};
int *g_DrawGpuReg2;

void Draw_InitGpuHwRegs(void) {
    unsigned int zero = 0;
    u32 *ptr;
    unsigned int a;
    unsigned int b;
    unsigned int count;
    u32 *ptr1;

    ptr = g_DrawPrimGeneratorWords + 16;
    a = zero | 1;
    ptr[0x10] = a;
    a = zero | 2;
    ptr[0xF] = a;
    count = zero | 0xE;
    do {
        a = ptr[0x10];
        b = ptr[0xF];
        ptr--;
        a += b;
        ptr[0xF] = a;
    } while (count != 0 && (count--, 1));

    ptr1 = g_DrawPrimGeneratorWords + 14;
    g_DrawGpuReg2 = (int *)(g_DrawPrimGeneratorWords + 15);
    a = zero | 0x40;
    b = zero | 0x10;
    *ptr1 = a;
    *g_DrawGpuReg2 = b;
}
