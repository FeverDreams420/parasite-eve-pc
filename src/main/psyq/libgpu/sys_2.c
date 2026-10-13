/* Psy-Q LIBGPU SYS.OBJ, part 2 of 11: PutDispEnv. */

#include "common.h"
#include "pe1/gpu_state.h"
#include "pe1/psyq_gpu.h"

#include "sys_2_externs.h"

int GetVideoMode(void);
void *memcpy(void *, const void *, unsigned);
#define SIGNED_HALF(p) ((short)*(volatile unsigned short *)(p))
/* Hides the cached rectangle address so the remaining halves load from a
 * copy instead of folded offsets. */
static inline unsigned short *OpaqueHalfPointer(unsigned short *p) {
    return p;
}

static inline void Gpu_SendDisplayRange(unsigned start, unsigned end,
                                       unsigned mask, unsigned shift, unsigned command) {
    unsigned y = (end & mask) << shift;
    unsigned x = start & mask;
    unsigned opcode = command;
    GpuCallbacks *ops = g_GpuCallbacks;
    x |= opcode;
    ops->callback10(y | x);
}

DISPENV *PutDispEnv(DISPENV *env) {
    DISPENV *e = env;
    unsigned char *state;
    GpuSystemState *system;
    unsigned mode;
    int vstart, vend;
    unsigned short *rect;
    int width;
    int span;
    int index, hstart, hend, clamp;
    state = &g_GpuSystemState.queueState.debugLevel;
    mode = 0x08000000;
    if (*state >= 2)
        g_GpuDebugPrintf(g_PutDispEnvFormat, e);
    {
        unsigned opcode = 0x05000000;
        unsigned y = (unsigned short)e->disp.y;
        unsigned x = (unsigned short)e->disp.x;
        GpuCallbacks *ops = g_GpuCallbacks;
        y = (y & 0x3ff) << 10;
        x = (x & 0x3ff) | opcode;
        ops->callback10(y | x);
    }
    system = ((GpuSystemState *)(state - PE1_OFFSETOF(GpuSystemState, status.queueState.debugLevel)));
    rect = (unsigned short *)&system->displayCache.disp;
    if (*(unsigned *)&system->displayCache.isinter != *(unsigned *)&e->isinter ||
        SIGNED_HALF(rect) != e->disp.x || SIGNED_HALF((rect = OpaqueHalfPointer(rect)) + 1) != e->disp.y ||
        SIGNED_HALF(rect + 2) != e->disp.w || SIGNED_HALF(rect + 3) != e->disp.h) {
        e->pad0 = GetVideoMode();
        if (e->pad0 == 1)
            mode |= 8;
        if (e->isrgb24)
            mode |= 0x10;
        if (e->isinter)
            mode |= 0x20;
        if (g_GpuSystemState.reverse)
            mode |= 0x80;
        width = e->disp.w;
        if (width <= 280) {
        } else if (width <= 352)
            mode |= 1;
        else if (width <= 400)
            mode |= 0x40;
        else if (width <= 560)
            mode |= 2;
        else
            mode |= 3;
        {
            int h = e->disp.h;
            int small;
            if (e->pad0)
                small = h <= 288;
            else
                small = h <= 256;
            if (!small)
                mode |= 0x24;
        }
        g_GpuCallbacks->callback10(mode);
        e->pad0 = 8;
    }
    rect = (unsigned short *)&g_GpuActiveDispEnv.screen;
    if (SIGNED_HALF(rect) != e->screen.x || SIGNED_HALF(rect + 1) != e->screen.y ||
        SIGNED_HALF(rect + 2) != e->screen.w || SIGNED_HALF(rect + 3) != e->screen.h ||
        e->pad0 == 8) {
        e->pad0 = GetVideoMode();
        {
            int y = e->screen.y;
            vstart = e->pad0 ? y + 19 : y + 16;
        }
        vend = vstart + (e->screen.h ? e->screen.h : 240);
        width = e->disp.w;
        index = 0;
        if (width > 280) {
            index = 1;
            if (width > 352) {
                index = 2;
                if (width > 400) {
                    index = 4;
                    if (width <= 560)
                        index = 3;
                }
            }
        }
        {
            int product = e->screen.x * g_GpuDotClockDividers[index];
            int video = e->pad0;
            int table = (video * 5 + index) * 2;
            int base = g_GpuHRangeTable[table];
            int screenWidth = e->screen.w;
            span = g_GpuHRangeEndTable[table] - base;
            hstart = base + product;
            if (screenWidth)
                span = (span * screenWidth) >> 8;
        }
        hend = hstart + span;
        if (e->pad0) {
            clamp = 540;
            if (hstart >= 540) {
                clamp = 3220;
                if (hstart <= 3220)
                    clamp = hstart;
            }
            hstart = clamp;
            clamp = hstart + 4 * g_GpuDotClockDividers[index];
            if (hend >= clamp)
                clamp = hend > 3260 ? 3260 : hend;
            hend = clamp;
            clamp = vstart < 19 ? 19 : vstart > 303 ? 303 : vstart;
            vstart = clamp;
            clamp = vstart + 2;
            if (vend >= clamp)
                clamp = vend > 305 ? 305 : vend;
        } else {
            clamp = 500;
            if (hstart >= 500) {
                clamp = 3250;
                if (hstart <= 3250)
                    clamp = hstart;
            }
            hstart = clamp;
            clamp = hstart + 4 * g_GpuDotClockDividers[index];
            if (hend >= clamp)
                clamp = hend > 3290 ? 3290 : hend;
            hend = clamp;
            clamp = vstart < 16 ? 16 : vstart > 257 ? 257 : vstart;
            vstart = clamp;
            clamp = vstart + 2;
            if (vend >= clamp)
                clamp = vend > 258 ? 258 : vend;
        }
        vend = clamp;
        Gpu_SendDisplayRange(hstart, hend, 0xfff, 12, 0x06000000);
        Gpu_SendDisplayRange(vstart, vend, 0x3ff, 10, 0x07000000);
    }
    /* An unprototyped call keeps the SDK memcpy call instead of builtin expansion.
     * Pointers and the word-sized count keep the same argument ABI. */
    ((void *(*)())memcpy)(&g_GpuActiveDispEnv, e, sizeof(DISPENV));
    return e;
}
