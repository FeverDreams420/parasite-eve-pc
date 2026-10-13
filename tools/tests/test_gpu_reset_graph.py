import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import function_text, host_unit  # noqa: E402


class ResetGraphTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_reset_modes_order_layout_and_dimensions(self):
        self.assertNotRegex(function_text("ResetGraph"), r'\basm\s*\(\s*"\$')
        source = host_unit("ResetGraph")
        # Preserve the PSX state layout on the host (32-bit callback word).
        self.assertIn('void (*drawSyncCallback)();', source)
        source = source.replace('void (*drawSyncCallback)();',
                                'unsigned int drawSyncCallback;')
        # g_GpuSystemState heads the 128-byte SYS.OBJ state ResetGraph clears.
        source = source.replace(
            'extern GpuDebugState g_GpuSystemState;',
            'extern GpuSystemState g_TestGpuState;\n'
            '#define g_GpuSystemState (g_TestGpuState.status)')
        source = source.replace('printf', 'testPrintf')
        harness = source + r'''
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
GpuSystemState g_TestGpuState;
unsigned char g_GraphDebug[1], g_GpuDefaultCallbacks[4];
char g_ResetGraphJtbFormat[] = "reset", g_ResetGraphModeFormat[] = "soft";
unsigned short g_GpuTypeWidthTable[3][2];
unsigned short g_GpuTypeHeightTable[3][2] = {{241, 999}, {481, 999}, {513, 999}};
static GpuCallbacks callbacks;
GpuCallbacks *g_GpuCallbacks = &callbacks;
static int stage, prints, modeExpected, variantExpected, softCalls;
int testPrintf(const char *format, ...) {
    va_list ap;
    assert(stage == 0 && format == g_ResetGraphJtbFormat);
    va_start(ap, format);
    assert(va_arg(ap, unsigned char *) == g_GpuDefaultCallbacks);
    assert(va_arg(ap, unsigned char *) == (unsigned char *)&g_GpuSystemState);
    va_end(ap);
    ++prints;
    return 0;
}
static void debug(char *format, ...) {
    va_list args;
    int mode;
    va_start(args, format);
    mode = va_arg(args, int);
    va_end(args);
    assert(stage == 0 && format == g_ResetGraphModeFormat && mode == modeExpected);
    ++prints;
}
GpuDebugPrintf g_GpuDebugPrintf = debug;
void GPU_memset(unsigned char *ptr, int value, int size) {
    if (stage == 0) {
        assert((void *)ptr == (void *)&g_TestGpuState && value == 0 && size == 128);
        stage = 1;
    } else if (stage == 4) {
        assert((void *)ptr == (void *)&g_TestGpuState.drawCache && value == -1 && size == 0x5C);
        assert(g_GpuSystemState.type == variantExpected && g_GpuSystemState.queueState.queue == 1);
        assert(g_GpuSystemState.width == 321 + 100 * variantExpected);
        assert(g_GpuSystemState.height == g_GpuTypeHeightTable[variantExpected][0]);
        stage = 5;
    } else {
        assert(stage == 5 && (void *)ptr == (void *)&g_TestGpuState.displayCache && value == -1 && size == 0x14);
        stage = 6;
    }
    __builtin_memset(ptr, value, size);
}
int ResetCallback(void) { assert(stage == 1); stage = 2; return 0; }
void GPU_cw(unsigned int command) {
    assert(stage == 2 && command == ((uintptr_t)&callbacks & 0xFFFFFF));
    stage = 3;
}
int Gpu_InitDmaQueue(int mode) {
    assert(stage == 3 && mode == modeExpected);
    stage = 4;
    return variantExpected;
}
static int softReset(int mode) {
    assert(stage == 0 && mode == 1);
    ++softCalls;
    return -77;
}
int main(void) {
    static const int levels[] = {0, 1, 2, 255};
    int mode, level, variant;
    assert(sizeof(GpuSystemState) == 128 && offsetof(GpuSystemState, drawCache) == 16);
    assert(offsetof(GpuSystemState, displayCache) == 0x6C);
    callbacks.reset = softReset;
    for (variant = 0; variant < 3; ++variant)
        g_GpuTypeWidthTable[variant][0] = 321 + 100 * variant;
    for (mode = -8; mode < 24; ++mode) for (level = 0; level < 4; ++level)
    for (variant = 0; variant < 3; ++variant) {
        unsigned char before[128], expected[128] = {0};
        int full = (mode & 7) == 0 || (mode & 7) == 3 || (mode & 7) == 5;
        int result;
        __builtin_memset(&g_TestGpuState, 0xA5, sizeof(g_TestGpuState));
        g_GpuSystemState.queueState.debugLevel = g_GraphDebug[0] = levels[level];
        __builtin_memcpy(before, &g_TestGpuState, 128);
        stage = prints = softCalls = 0;
        modeExpected = mode; variantExpected = variant;
        result = ResetGraph(mode);
        if (full) {
            unsigned short width = 321 + 100 * variant;
            unsigned short height = g_GpuTypeHeightTable[variant][0];
            expected[0] = variant; expected[1] = 1;
            __builtin_memcpy(expected + 4, &width, 2); __builtin_memcpy(expected + 6, &height, 2);
            __builtin_memset(expected + 16, 255, 112);
            assert(result == variant && stage == 6 && softCalls == 0);
            assert(prints == ((mode & 7) != 5));
            assert(__builtin_memcmp(&g_TestGpuState, expected, 128) == 0);
        } else {
            assert(result == -77 && stage == 0 && softCalls == 1);
            assert(prints == (levels[level] >= 2));
            assert(__builtin_memcmp(&g_TestGpuState, before, 128) == 0);
        }
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "reset-graph-test"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-Wno-pointer-to-int-cast",
                 "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
