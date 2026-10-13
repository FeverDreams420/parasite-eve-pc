int *g_DrawGpuReg2;

void Draw_InitGpuHwRegs(void) {
    unsigned int zero = 0;
    volatile int *ptr;
    unsigned int a;
    unsigned int b;
    unsigned int count;
    volatile int *ptr1;

    ptr = (int *)0x80070E0C;
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

    ptr1 = (int *)0x80070E04;
    g_DrawGpuReg2 = (int *)0x80070E08;
    a = zero | 0x40;
    b = zero | 0x10;
    *ptr1 = a;
    *g_DrawGpuReg2 = b;
}
