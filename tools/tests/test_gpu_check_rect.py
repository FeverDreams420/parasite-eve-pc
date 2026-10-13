import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import function_text, host_unit  # noqa: E402


class CheckRectTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_diagnostics_boundaries_and_no_mutation(self):
        # Register pins are matching debt: they may only disappear.
        self.assertLessEqual(function_text("checkRECT").count('asm("$3")'), 1)
        source = host_unit("checkRECT")
        # The retail MIPS callback is intentionally unprototyped.  Route the
        # host build through a variadic shim so arm64 passes its arguments in
        # the ABI locations inspected below.
        source = 'extern void test_debug(char *, ...);\n' + source
        source = source.replace('g_GpuDebugPrintf(format, name);',
                                'test_debug(format, name);')
        source = source.replace('g_GpuDebugPrintf(g_RectCoordsFormat, x, y, width, height);',
                                'test_debug(g_RectCoordsFormat, x, y, width, height);')
        harness = source + r'''
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
GpuDebugState g_GpuSystemState;
char g_BadRectFormat[] = "invalid", g_RectNameFormat[] = "all", g_RectCoordsFormat[] = "fields";
static char name[] = "test";
static RECT before;
static int calls;
void test_debug(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    if (calls == 0) {
        assert(format == (g_GpuSystemState.queueState.debugLevel == 1
            ? g_BadRectFormat : g_RectNameFormat));
        assert(va_arg(ap, char *) == name);
    } else {
        assert(calls == 1 && format == g_RectCoordsFormat);
        assert(va_arg(ap, int) == before.x);
        assert(va_arg(ap, int) == before.y);
        assert(va_arg(ap, int) == before.w);
        assert(va_arg(ap, int) == before.h);
    }
    va_end(ap);
    ++calls;
}
int main(void) {
    static const short values[] = {-32768, -1, 0, 1, 239, 240, 241,
                                    319, 320, 321, 32767};
    int a, b, c, d, level;
    g_GpuSystemState.width = 320;
    g_GpuSystemState.height = 240;
    for (level = 0; level < 5; ++level) {
        g_GpuSystemState.queueState.debugLevel = level == 4 ? 255 : level;
        for (a = 0; a < 11; ++a) for (b = 0; b < 11; ++b)
        for (c = 0; c < 11; ++c) for (d = 0; d < 11; ++d) {
            RECT rect = {values[a], values[b], values[c], values[d]};
            int invalid = rect.w > 320 || rect.w + rect.x > 320 ||
                rect.y > 240 || rect.y + rect.h > 240 || rect.w <= 0 ||
                rect.x < 0 || rect.y < 0 || rect.h <= 0;
            before = rect;
            calls = 0;
            checkRECT(name, &rect);
            assert(calls == ((level == 2 || (level == 1 && invalid)) ? 2 : 0));
            assert(__builtin_memcmp(&rect, &before, sizeof(rect)) == 0);
        }
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "check-rect-test"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-I", str(ROOT / "include"),
                 "-I", str(ROOT / "tools/m2c"), "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
