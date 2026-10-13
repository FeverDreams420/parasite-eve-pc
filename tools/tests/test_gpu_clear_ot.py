import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import function_text, host_unit  # noqa: E402


class ClearOtTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_forward_links_last_entry_and_debug(self):
        # The link-mask register pin was replaced by staged masks.
        self.assertNotRegex(function_text("ClearOTag"), r'\basm\s*\(\s*"\$')
        source = host_unit("ClearOTag")
        harness = source + r'''
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
GpuDebugState g_GpuSystemState;
char g_ClearOTagFormat[] = "ot";
u32 g_GpuOtTerminatorTarget, g_GpuOtTerminatorWord;
static u32 storage[130];
static int calls, expectedCount;
static void debug(char *message, ...) {
    va_list args;
    void *ot;
    int count;
    va_start(args, message);
    ot = va_arg(args, void *);
    count = va_arg(args, int);
    va_end(args);
    assert(message == g_ClearOTagFormat && ot == storage + 1 && count == expectedCount);
    assert(storage[1] == 0xA5ABCDEF);
    ++calls;
}
GpuDebugPrintf g_GpuDebugPrintf = debug;
int main(void) {
    int level, count, i;
    for (level = 0; level <= 3; ++level) {
        g_GpuSystemState.queueState.debugLevel = level;
        for (count = 1; count <= 128; ++count) {
            u32 *result;
            expectedCount = count;
            calls = 0;
            for (i = 0; i < 130; ++i) storage[i] = 0xA5ABCDEF;
            g_GpuOtTerminatorTarget = 0x12345678;
            g_GpuOtTerminatorWord = 0;
            result = ClearOTag(storage + 1, count);
            assert(result == storage + count);
            assert(calls == (level >= 2));
            assert(storage[0] == 0xA5ABCDEF && storage[count + 1] == 0xA5ABCDEF);
            for (i = 1; i < count; ++i)
                assert(storage[i] == ((uintptr_t)(storage + i + 1) & 0xFFFFFF));
            assert(storage[count] == ((uintptr_t)&g_GpuOtTerminatorWord & 0xFFFFFF));
            assert(g_GpuOtTerminatorWord == (0x04000000 | ((uintptr_t)&g_GpuOtTerminatorTarget & 0xFFFFFF)));
            assert(g_GpuOtTerminatorTarget == 0x12345678);
        }
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "clear-ot-test"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-Wno-pointer-to-int-cast",
                 "-Wno-int-to-pointer-cast", "-I", str(ROOT / "include"),
                 "-I", str(ROOT / "tools/m2c"), "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
