/* ASSEMBLER: GNU */
/* Psy-Q LIBETC VSYNC.OBJ: VSync, v_wait. */

#include "common.h"
#include "pe1/psyq_api_internal.h"
#include "pe1/psyq_bios.h"

#include "vsync_externs_1.h"

void v_wait(int target, int frames);

int VSync(int mode) {
    u32 status;
    int elapsed;
    register int target asm("$2");
    int frames;
    int next;
    volatile u32 sample;

    status = *g_VSyncGpuStatusRegister;
    do {
        sample = *g_VSyncHcountRegister;
    } while (sample != *g_VSyncHcountRegister);
    elapsed = (sample - g_VSyncLastHcount) & 0xFFFF;
    if (mode < 0) return g_VSyncCount;
    if (mode == 1) return elapsed;
    if (mode > 0) {
        target = g_VSyncLastCount - 1;
        target += mode;
    } else {
        target = g_VSyncLastCount;
    }
    frames = 0;
    if (mode > 0) frames = mode - 1;
    v_wait(target, frames);
    status = *g_VSyncGpuStatusRegister;
    next = g_VSyncCount;
    next++;
    v_wait(next, 1);
    if (status & 0x400000) {
        while (!((status ^ *g_VSyncGpuStatusRegister) & 0x80000000)) {}
    }
    g_VSyncLastCount = g_VSyncCount;
    do {
        g_VSyncLastHcount = *g_VSyncHcountRegister;
    } while (g_VSyncLastHcount != *g_VSyncHcountRegister);
    return elapsed;
}

void v_wait(int arg0, int arg1) {
    volatile int timeout = arg1 << 15;

    if (g_VSyncCount < arg0) {
        do {
            if (--timeout == -1) {
                puts(g_VSyncTimeoutText);
                ChangeClearPAD(0);
                ChangeClearRCnt(3, 0);
                return;
            }
        } while (g_VSyncCount < arg0);
    }
}
