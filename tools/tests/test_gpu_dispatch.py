import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import function_closure, host_unit  # noqa: E402


class GpuDispatchTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_ready_states_timeout_and_callback_order(self):
        self.assertNotRegex(function_closure("DrawOTag2"), r'\b(?:asm|__asm__)\b')
        source = host_unit("DrawOTag2")
        harness = source + r'''
#include <assert.h>
#include <stdarg.h>
GpuDebugState g_GpuSystemState;
char g_DrawOTagFormat[] = "dispatch";
int g_GpuDmaTimeoutDeadline, g_GpuDmaWaitLoopCounter;
static volatile unsigned int dma;
static unsigned int gpu;
volatile unsigned int *g_GpuDmaChcrPtr = &dma;
unsigned int *g_GpuGp1Ptr = &gpu;
static GpuCallbacks callbacks;
GpuCallbacks *g_GpuCallbacks = &callbacks;
static int packet, polls, idleAt, readyAt, timeoutAt, events, vsyncCalls;
static void debug(char *message, ...) {
    va_list args;
    void *argument;
    va_start(args, message);
    argument = va_arg(args, void *);
    va_end(args);
    assert(message == g_DrawOTagFormat && argument == &packet);
    assert(events == 0);
    events = 1;
}
GpuDebugPrintf g_GpuDebugPrintf = debug;
int VSync(int mode) {
    assert(mode == -1 && ++vsyncCalls == 1);
    assert(events == (g_GpuSystemState.queueState.debugLevel >= 2 ? 1 : 0));
    events = events * 10 + 2;
    return 123;
}
int Gpu_DmaTimeoutCheck(void) {
    assert(g_GpuDmaTimeoutDeadline == 363);
    assert(g_GpuDmaWaitLoopCounter == 0);
    assert(++polls <= 10);
    if (polls == timeoutAt) return -1;
    if (polls >= idleAt) dma = 0;
    if (polls >= readyAt) gpu = 0x04000000;
    return 0;
}
void Gpu_RestoreDmaCallback(void) { assert(0); }
PsyqInterruptHandler DMACallback(int channel, PsyqInterruptHandler callback) {
    assert(channel == 2 && callback == Gpu_RestoreDmaCallback);
    assert(dma == 0 && gpu == 0x04000000);
    assert(events == (g_GpuSystemState.queueState.debugLevel >= 2 ? 12 : 2));
    events = events * 10 + 3;
    return 0;
}
static int send(void *argument) {
    assert(argument == &packet);
    assert(events == (g_GpuSystemState.queueState.debugLevel >= 2 ? 123 : 23));
    events = events * 10 + 4;
    return 77;
}
static void check(int debugLevel, int idle, int ready, int timeout) {
    g_GpuSystemState.queueState.debugLevel = debugLevel;
    idleAt = idle;
    readyAt = ready;
    timeoutAt = timeout;
    dma = idle ? 0x01000000 : 0;
    gpu = ready ? 0 : 0x04000000;
    g_GpuDmaTimeoutDeadline = g_GpuDmaWaitLoopCounter = -99;
    events = polls = vsyncCalls = 0;
    assert(DrawOTag2(&packet) == (timeout ? -1 : 0));
    assert(polls == (timeout ? timeout : (idle > ready ? idle : ready)));
    assert(g_GpuDmaTimeoutDeadline == 363 && g_GpuDmaWaitLoopCounter == 0);
    assert(events == (timeout ? (debugLevel >= 2 ? 12 : 2) :
                              (debugLevel >= 2 ? 1234 : 234)));
}
int main(void) {
    callbacks.u18.moveImage = send;
    check(0, 0, 0, 0);
    check(1, 2, 0, 0);
    check(2, 0, 3, 0);
    check(3, 2, 4, 0);
    check(0, 8, 0, 2);
    check(2, 0, 8, 2);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "dispatch-test"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-I", str(ROOT / "include"),
                 "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
