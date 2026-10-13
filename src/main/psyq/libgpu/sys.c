/* ASSEMBLER: GNU */
/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -mno-split-addresses */
/* Psy-Q LIBGPU SYS.OBJ, part 1 of 11: ResetGraph, SetGraphDebug,
 * SetGraphQueue, GetGraphDebug, DrawSyncCallback, SetDispMask, DrawSync,
 * ClearImage, ClearImage2, LoadImage, StoreImage, MoveImage, ClearOTag,
 * ClearOTagR, DrawPrim, DrawOTag, PutDrawEnv, DrawOTagEnv and GetDrawEnv. SYS
 * stays in eleven units because neighbouring parts need different compilers:
 * sys.c, sys_3.c, sys_6.c, sys_8.c and sys_10.c only match with GCC 2.8.1 and
 * -mno-split-addresses, sys_2.c, sys_4.c, sys_7.c, sys_9.c and sys_11.c only
 * with GCC 2.7.2 (the return delay slot), and sys_5.c only with -O1 or
 * -fno-schedule-insns. */

#include "common.h"
#include "pe1/gpu_callbacks.h"
#include "pe1/gpu_state.h"
#include "pe1/psyq_bios.h"
#include "pe1/psyq_callbacks.h"
#include "pe1/psyq_gpu.h"
#include "include_asm.h"

#include "sys_externs.h"

void GPU_cw(unsigned int);
int Gpu_InitDmaQueue(int);

int ResetGraph(int mode) {
    GpuSystemState *state;
    void *drawCache;
    volatile unsigned char *gpuType;
    switch (mode & 7) {
    case 0:
    case 3:
        {
            char *format;
            unsigned char *version;

            format = g_ResetGraphJtbFormat;
            version = g_GpuDefaultCallbacks;
            printf(format, version, (GpuSystemState *)&g_GpuSystemState);
        }
    case 5:
        state = (GpuSystemState *)&g_GpuSystemState;
        gpuType = &state->status.type;
        GPU_memset((unsigned char *)state, 0, sizeof(*state));
        ResetCallback();
        GPU_cw((unsigned int)g_GpuCallbacks & 0xFFFFFF);
        *gpuType = Gpu_InitDmaQueue(mode);
        drawCache = &state->drawCache;
        state->status.queueState.queue = 1;
        state->status.width = g_GpuTypeWidthTable[*gpuType][0];
        state->status.height = g_GpuTypeHeightTable[*gpuType][0];
        GPU_memset(drawCache, -1, sizeof(state->drawCache));
        GPU_memset((unsigned char *)&state->displayCache, -1, sizeof(state->displayCache));
        return *gpuType;
    default:
        if (g_GraphDebug[0] >= 2) g_GpuDebugPrintf(g_ResetGraphModeFormat, mode);
        return g_GpuCallbacks->reset(1);
    }
}

int SetGraphDebug(int debugLevel) {
    u8 *currentDebugLevel;
    int result;
    GpuDebugPrintf debugPrint;
    int currentLevel;
    int type;
    int reverse;
    int oldDebugLevel;

    currentDebugLevel = &g_GpuSystemState.queueState.debugLevel;
    /* Preserve the shared base used for the adjacent GPU state bytes. */
        oldDebugLevel = *currentDebugLevel;
    *currentDebugLevel = debugLevel;
    result = oldDebugLevel;

    if ((u8)debugLevel == 0) {
        return result;
    }

    debugPrint = g_GpuDebugPrintf;
    /* Keep the callback load ahead of its arguments. */
        currentLevel = currentDebugLevel[0];
    type = currentDebugLevel[-2];
    reverse = currentDebugLevel[1];
    /* Materialize the byte arguments before the format string address. */
        debugPrint(g_SetGraphDebugFormat, currentLevel, type, reverse);
    result = oldDebugLevel;
    return result;
}

int SetGraphQueue(int mode)
{
    GpuQueueState *state = &g_GpuSystemState.queueState;
    int previous = state->queue;
    if (state->debugLevel >= 2) {
        /* Keep the two prior-state paths: stock GCC merges the calls after
         * choosing the retail register lifetimes. Both log exactly once. */
        if (previous) {
            g_GpuDebugPrintf(g_SetGraphQueFormat, mode);
        } else {
            g_GpuDebugPrintf(g_SetGraphQueFormat, mode);
        }
    }
    if (mode != state->queue) {
        g_GpuCallbacks->reset(1);
        state->queue = mode;
        DMACallback(2, 0);
    }
    return previous;
}

int GetGraphDebug(void) {
    return g_GpuSystemState.queueState.debugLevel;
}

void *DrawSyncCallback(void *callback) {
    void *previous;

    if (g_GpuSystemState.queueState.debugLevel >= 2) {
        g_GpuDebugPrintf(g_DrawSyncCallbackFormat, callback);
    }

    previous = (void *)g_GpuSystemState.drawSyncCallback;
    g_GpuSystemState.drawSyncCallback = (void (*)())callback;
    return previous;
}

void SetDispMask(int mask) {
    u8 *debug = g_GraphDebug;
    u8 *clearPointer;
    GpuCallbacks *callbacks;
    int command;

    if (*debug >= 2) {
        g_GpuDebugPrintf(g_SetDispMaskFormat, mask);
    }

    clearPointer = debug + 0x6A;
    if (mask == 0) {
        GPU_memset(clearPointer, -1, 20);
    }

    command = 0x03000000;
    asm("" : "+r"(command));
    callbacks = g_GpuCallbacks;
    if (mask == 0) {
        command |= 1;
    } else {
        command = 0x03000000;
    }
    callbacks->callback10(command);
}

int DrawSync(int arg0) {
    void (*fn)(char *, int);
    GpuCallbacks *callbacks;

    if (g_GpuSystemState.queueState.debugLevel >= 2) {
        fn = g_GpuDebugPrintf;
        fn(g_DrawSyncFormat, arg0);
    }

    callbacks = g_GpuCallbacks;
    return callbacks->callback3C(arg0);
}

void checkRECT(char *name, RECT *rect) {
    char *format;
    switch (g_GpuSystemState.queueState.debugLevel) {
    case 1:
        if (rect->w > g_GpuSystemState.width || rect->w + rect->x > g_GpuSystemState.width ||
            rect->y > g_GpuSystemState.height || rect->y + rect->h > g_GpuSystemState.height ||
            rect->w <= 0 || rect->x < 0 || rect->y < 0 || rect->h <= 0) {
            format = g_BadRectFormat;
            break;
        }
        return;
    case 2:
        format = g_RectNameFormat;
        break;
    default:
        return;
    }
    g_GpuDebugPrintf(format, name);
    {
        int x = rect->x;
        int y = rect->y;
        int width = rect->w;
        register int height asm("$3") = rect->h;
        g_GpuDebugPrintf(g_RectCoordsFormat, x, y, width, height);
    }
}

void checkRECT(char *message, RECT *rect);

int ClearImage(RECT *rect, u8 r, u8 g, u8 b) {
    GpuCallbacks *callbacks;
    u32 color;

    checkRECT(g_ClearImageName, rect);
    color = (b << 16) | (g << 8) | r;
    callbacks = g_GpuCallbacks;
    return callbacks->addque2(callbacks->clr, rect, 8, color);
}

int ClearImage2(RECT *rect, u8 r, u8 g, u8 b) {
    GpuCallbacks *callbacks;
    u32 color;

    checkRECT(g_ClearImage2Name, rect);
    color = 0x80000000 | (b << 16) | (g << 8) | r;
    callbacks = g_GpuCallbacks;
    return callbacks->addque2(callbacks->clr, rect, 8, color);
}

int LoadImage(RECT *rect, void *pixels) {
    GpuCallbacks *callbacks;

    checkRECT(g_LoadImageName, rect);
    callbacks = g_GpuCallbacks;
    return callbacks->addque2(callbacks->u20.load, rect, 8, pixels);
}

int StoreImage(RECT *rect, void *pixels) {
    GpuCallbacks *callbacks;

    checkRECT(g_StoreImageName, rect);
    callbacks = g_GpuCallbacks;
    return callbacks->addque2(callbacks->u1c.store, rect, 8, pixels);
}

int MoveImage(RECT *rect, int x, int y) {
    checkRECT(g_MoveImageName, rect);
    if (rect->w == 0 || rect->h == 0) {
        return -1;
    }
    {
        register u32 *packetData asm("$3");
        GpuCallbacks *callbacks;
        register u32 sourcePosition asm("$5");
        u32 *packet;
        int packetSize;
        u32 destinationPosition;
        register u32 destinationX asm("$4");
        u32 size;

        destinationPosition = y << 16;
        packetData = g_GpuMoveImagePacket;
        asm("" : "+r"(packetData));
        destinationX = x & 0xFFFF;
        destinationPosition |= destinationX;
        sourcePosition = ((GpuRectWords *)rect)->position;
        callbacks = g_GpuCallbacks;
        asm("" : "+r"(sourcePosition), "+r"(callbacks));
        packetSize = 20;
        asm("" : "+r"(packetSize));
        packetData[1] = destinationPosition;
        *packetData = sourcePosition;
        size = ((GpuRectWords *)rect)->size;
        packet = packetData - 2;
        packetData[2] = size;
        asm("" : : "r"(size) : "memory");
        return callbacks->addque2(callbacks->u18.moveImage, packet, packetSize,
                                  0);
    }
}

u32 *ClearOTag(u32 *ot, int count) {
    u32 mask;
    u32 linkMask;
    u32 *result;
    u32 *terminator;
    u32 address;
    u32 command;

    if (g_GpuSystemState.queueState.debugLevel >= 2) {
        g_GpuDebugPrintf(g_ClearOTagFormat, ot, count);
    }

    if (--count) {
        do {
            --count;
            linkMask = 0xFFFFFF;
            ((u8 *)ot)[3] = 0;
            mask = 0xFF000000;
            *ot = (*ot & mask) | (PSX_ADDRESS(ot + 1) & linkMask);
            ++ot;
        } while (count);
    }

    mask = 0xFFFFFF;
    result = ot;
    PE1_COMPILER_LAUNDER(result);
    terminator = &g_GpuOtTerminatorWord;
    address = PSX_ADDRESS(&g_GpuOtTerminatorTarget);
    address &= mask;
    command = 0x04000000;
    address |= command;
    *terminator = address;
    terminator = (u32 *)(PSX_ADDRESS(terminator) & mask);
    *result = PSX_ADDRESS(terminator);
    return result;
}

u32 *ClearOTagR(u32 *ot, int count) {
    u32 mask;
    u32 *result;
    u32 *terminator;
    u32 address;
    u32 command;

    if (g_GpuSystemState.queueState.debugLevel >= 2) {
        g_GpuDebugPrintf(g_ClearOTagRFormat, ot, count);
    }

    g_GpuCallbacks->clearOTag(ot, count);

    mask = 0xFFFFFF;
    result = ot;
    PE1_COMPILER_LAUNDER(result);
    terminator = &g_GpuOtTerminatorWord;
    address = PSX_ADDRESS(&g_GpuOtTerminatorTarget);
    address &= mask;
    command = 0x04000000;
    address |= command;
    *terminator = address;
    terminator = (u32 *)(PSX_ADDRESS(terminator) & mask);
    *result = PSX_ADDRESS(terminator);
    return result;
}

int DrawPrim(void *primitive) {
    u8 *bytes = primitive;
    int word_count = bytes[3];

    g_GpuCallbacks->callback3C(0);
    return g_GpuCallbacks->cb14(bytes + 4, word_count);
}

void Gpu_SetDrawEnvBack(DR_ENV *, DRAWENV *);
void *memcpy(void *, const void *, unsigned int);

int DrawOTag(u32 *table)
{
    if (g_GpuSystemState.queueState.debugLevel >= 2) {
        g_GpuDebugPrintf(g_DrawOTagFormat, table);
    }
    {
        GpuCallbacks *callbacks = g_GpuCallbacks;
        register void *argument asm("$5") = table;
        int count;
        /* Keep the dispatch load before the independently materialized zeros. */
        asm volatile("" : "+r"(callbacks) : "r"(argument));
        count = 0;
        asm volatile("" : "+r"(count));
        return callbacks->addque2(callbacks->u18.packet, argument, count, 0);
    }
}

DRAWENV *PutDrawEnv(DRAWENV *environment)
{
    unsigned char *state = g_GraphDebug;
    DR_ENV *packet;
    void *(*copy)(void *, const void *, unsigned int) = memcpy;
    if (state[0] >= 2) {
        g_GpuDebugPrintf(g_PutDrawEnvFormat, environment);
    }
    packet = &environment->dr_env;
    Gpu_SetDrawEnvBack(packet, environment);
    packet->tag |= 0xFFFFFF;
    g_GpuCallbacks->addque2(g_GpuCallbacks->u18.packet, packet, 0x40, 0);
    copy(state + 14, environment, sizeof(DRAWENV));
    return environment;
}

void DrawOTagEnv(void *next, DRAWENV *environment)
{
    unsigned char *state = g_GraphDebug;
    DR_ENV *packet;
    void *(*copy)(void *, const void *, unsigned int) = memcpy;
    if (state[0] >= 2) {
        g_GpuDebugPrintf(g_DrawOTagEnvFormat, next, environment);
    }
    packet = &environment->dr_env;
    Gpu_SetDrawEnvBack(packet, environment);
    packet->tag = (packet->tag & 0xFF000000) | (PSX_ADDRESS(next) & 0xFFFFFF);
    g_GpuCallbacks->addque2(g_GpuCallbacks->u18.packet, packet, 0x40, 0);
    copy(state + 14, environment, sizeof(DRAWENV));
}

DRAWENV *GetDrawEnv(DRAWENV *arg0) {
    void *(*fn)(void *, const void *, unsigned int);

    fn = memcpy;
    fn(arg0, &((GpuSystemState *)&g_GpuSystemState)->drawCache, sizeof(DRAWENV));
    return arg0;
}
