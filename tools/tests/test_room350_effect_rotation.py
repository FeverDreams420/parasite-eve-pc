import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
ROOM = 'src/overlays/room_m350_cruiser'
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import host_unit  # noqa: E402


class Room350EffectRotationTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_threshold_rotation_and_signed_fields(self):
        source = host_unit("RoomM350_DropSplashCallback", under=ROOM)
        harness = '#include <assert.h>\n' + source + r'''
int g_FieldFxCurrentAge, D_8019A628[2];
RenderColor D_8019A3C8;
GteSineWord rcossin_tbl[4096];
#define SINE ((short *)rcossin_tbl)
static int calls, expectedScale, expectedShade, contextData;
void FieldFx_DrawStarFan(struct GteShortVector *context, int a, int b, int c,
                  struct GteRotation *turn, int scale0, int scale1,
                  struct RenderColor *first, struct RenderColor *second,
                  int shade, int flag) {
    short *rotation = (short *)turn;
    assert(calls++ == 0 && (void *)context == &contextData);
    assert(a == 384 && b == 384 && c == 10 && flag == 1);
    assert(rotation[0] == 1024 && rotation[1] == 0 && rotation[3] == 1);
    assert(rotation[2] == (short)((g_FieldFxCurrentAge * 4096) / 20));
    assert(scale0 == expectedScale && scale1 == expectedScale);
    assert(first == &D_8019A3C8 && (int *)second == D_8019A628);
    assert(shade == expectedShade >> 5);
}
int main(void) {
    const int values[] = {-32768, -1025, -33, -32, -1, 0, 1, 31, 32, 32767};
    int counter, event, s, t, i;
    assert(sizeof(EffectEntry) == 4);
    for (counter = -16; counter <= 128; ++counter) {
        g_FieldFxCurrentAge = counter;
        for (event = -2; event < 5; ++event) {
            if (event == 2) continue;
            calls = 0;
            assert(RoomM350_DropSplashCallback(event, 0) == (event == 1 && counter >= 16));
            assert(calls == 0);
        }
    }
    for (counter = 0; counter <= 128; ++counter)
    for (s = 0; s < 10; ++s) for (t = 0; t < 10; ++t) {
        unsigned int index = (((unsigned int)counter - 1) * 256 & 0x3F00) / 2;
        for (i = 0; i < 8192; ++i) SINE[i] = 12345;
        g_FieldFxCurrentAge = counter;
        expectedScale = values[s]; expectedShade = values[t];
        SINE[index] = expectedScale;
        SINE[index + 1] = expectedShade;
        calls = 0;
        assert(RoomM350_DropSplashCallback(2, &contextData) == 0);
        assert(calls == 1);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-effect-rotation"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing",
                 "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
