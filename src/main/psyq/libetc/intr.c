/* PSY-Q LIBETC INTR, part 1 of 2: ResetCallback, InterruptCallback,
 * DMACallback, VSyncCallback, VSyncCallbacks, StopCallback, RestartCallback,
 * CheckCallback, GetIntrMask, SetIntrMask. */
/* INTR stays in two units: these callbacks only match with GCC 2.7.2,
 * the interrupt manager in intr_2.c only with GCC 2.8.1 and
 * -mno-split-addresses. */

#include "pe1/psyq_api_internal.h"

#include "intr_externs.h"

int ResetCallback(void) {
    return g_EventCallbackTable->reset();
}

PsyqInterruptHandler InterruptCallback(int channel, PsyqInterruptHandler callback) {
    return g_EventCallbackTable->interrupt(channel, callback);
}

PsyqInterruptHandler DMACallback(int channel, PsyqInterruptHandler callback) {
    return g_EventCallbackTable->dma(channel, callback);
}

int VSyncCallback(PsyqInterruptHandler callback) {
    return (int)g_EventCallbackTable->vsync(4, callback);
}

PsyqInterruptHandler VSyncCallbacks(unsigned int channel, PsyqInterruptHandler callback) {
    return g_EventCallbackTable->vsync(channel, callback);
}

int StopCallback(void) {
    return g_EventCallbackTable->stop();
}

int RestartCallback(void) {
    return g_EventCallbackTable->restart();
}

int CheckCallback(void) {
    return g_IntrInCallback;
}

unsigned short GetIntrMask(void) {
    return *g_IntrMaskRegPtr;
}

unsigned short SetIntrMask(unsigned short mask) {
    unsigned short prev = *g_IntrMaskRegPtr;
    *g_IntrMaskRegPtr = mask;
    return prev;
}
