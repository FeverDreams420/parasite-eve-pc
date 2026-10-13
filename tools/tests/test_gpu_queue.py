import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import function_text, host_unit  # noqa: E402


class GpuQueueTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_state_and_callback_order(self):
        self.assertNotRegex(function_text("SetGraphQueue"), r'\b(?:asm|__asm__)\b')
        source = host_unit("SetGraphQueue")
        harness = source + r'''
#include <assert.h>
#include <stdarg.h>
GpuDebugState g_GpuSystemState;
char g_SetGraphQueFormat[] = "queue";
static int events, expectedOld, expectedNew, mutateOnPrint;
static int reset(int mode) {
    assert(mode == 1);
    assert(g_GpuSystemState.queueState.queue == expectedOld);
    events = events * 10 + 2;
    return 0;
}
static void print(char *message, ...) {
    va_list args;
    int argument;
    va_start(args, message);
    argument = va_arg(args, int);
    va_end(args);
    assert(message == g_SetGraphQueFormat);
    assert(argument == expectedNew);
    events = events * 10 + 1;
    if (mutateOnPrint) g_GpuSystemState.queueState.queue = expectedNew;
}
static GpuCallbacks callbacks;
GpuCallbacks *g_GpuCallbacks = &callbacks;
GpuDebugPrintf g_GpuDebugPrintf = print;
PsyqInterruptHandler DMACallback(int channel, PsyqInterruptHandler callback) {
    assert(channel == 2 && callback == 0);
    assert(g_GpuSystemState.queueState.queue == (unsigned char)expectedNew);
    events = events * 10 + 3;
    return 0;
}
static void check(int old, int mode, int debug, int mutate, int expectedEvents) {
    g_GpuSystemState.queueState.queue = expectedOld = old;
    g_GpuSystemState.queueState.debugLevel = debug;
    expectedNew = mode;
    mutateOnPrint = mutate;
    events = 0;
    assert(SetGraphQueue(mode) == old);
    assert(events == expectedEvents);
    assert(g_GpuSystemState.queueState.queue == (unsigned char)mode);
}
int main(void) {
    callbacks.reset = reset;
    check(0, 0, 0, 0, 0);
    check(0, 1, 0, 0, 23);
    check(1, 1, 2, 0, 1);
    check(1, 0, 2, 0, 123);
    check(255, 0, 1, 0, 23);
    check(0, 300, 0, 0, 23);
    check(0, 1, 2, 1, 1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "queue-test"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2",
                 "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
