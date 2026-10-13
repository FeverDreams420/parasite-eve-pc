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


def callback_source():
    return host_unit("RoomM350_OrbBurstCallback", under=ROOM)


class Room350EffectLayersTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_two_draws_and_inter_call_reloads(self):
        source = callback_source()
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
GteSineWord rcossin_tbl[4096];
RenderColor D_8019A3C8;
int g_FieldFxCurrentAge, D_8019A4DC[4], D_8019A4E0[4];
static int firstPoint, secondPoint, calls, initialCounter, nextCounter;
static int sizeValue, firstShade, secondShade;
static void *object;
static short *savedVector;
void FieldFx_DrawStarFan(struct GteShortVector *p, int a, int b, int mode,
                   struct GteRotation *rotation, int x, int y,
                   struct RenderColor *colorA, struct RenderColor *colorB,
                   int shade, int flag) {
    short *vector = (short *)rotation;
    unsigned int index = ((unsigned int)(nextCounter - 1) << 8) & 0xF00;
    assert(calls++ == 0 && p == &firstPoint && a == 320 && b == 320 && mode == 16);
    assert(x == sizeValue && y == sizeValue && shade == firstShade >> 5 && flag == 1);
    assert(colorA == D_8019A4DC && colorB == &D_8019A3C8);
    assert(vector[0] == 0 && vector[1] == 0 && vector[2] == (short)(initialCounter * 128));
    assert(vector[3] == 0);
    /* Deliberate mutations test inter-call dataflow, not actual helper effects. */
    savedVector = vector; vector[0] = -11; vector[1] = 22; vector[2] = -33; vector[3] = 44;
    object = &secondPoint; g_FieldFxCurrentAge = nextCounter;
    rcossin_tbl[index].word = (int)(0x76540000U | (unsigned short)secondShade);
}
void FieldFx_DrawRingBand(struct GteShortVector *p, int a, int b, int mode,
                   struct GteRotation *rotation, int x, int y,
                   struct RenderColor *colorA, struct RenderColor *colorB,
                   int shade, int flag) {
    short *vector = (short *)rotation;
    assert(calls++ == 1 && p == &secondPoint && a == 256 && b == 512 && mode == 16);
    assert(x == nextCounter * 512 + 2048 && y == x && shade == secondShade >> 5 && flag == 1);
    assert(colorA == &D_8019A3C8 && colorB == D_8019A4E0);
    assert(vector == savedVector);
    assert(vector[0] == -11 && vector[1] == 22 && vector[2] == -33 && vector[3] == 44);
}
int main(void) {
    const short values[] = {-32768, -33, -1, 0, 32, 32767};
    const int nextValues[] = {-16, -1, 0, 1, 8, 17, 32, 80};
    int counter, event, s, h, n;
    assert(sizeof(TableEntry) == 4);
    for (counter = -16; counter <= 80; ++counter)
    for (event = -1; event <= 3; ++event) {
        if (event == 2) continue;
        g_FieldFxCurrentAge = counter; calls = 0;
        assert(RoomM350_OrbBurstCallback(event, 0) == (event == 1 && counter >= 8));
        assert(calls == 0);
    }
    for (counter = -16; counter <= 80; ++counter)
    for (s = 0; s < 6; ++s) for (h = 0; h < 6; ++h) for (n = 0; n < 8; ++n) {
        unsigned int index = ((unsigned int)(counter - 1) << 7) & 0xF80;
        memset(rcossin_tbl, 0, sizeof(rcossin_tbl));
        initialCounter = counter; nextCounter = nextValues[n];
        sizeValue = values[s]; firstShade = values[h]; secondShade = values[5-h];
        rcossin_tbl[index].word = (int)(((unsigned int)(unsigned short)firstShade << 16) |
                                  (unsigned short)sizeValue);
        g_FieldFxCurrentAge = counter; object = &firstPoint; calls = 0;
        assert(RoomM350_OrbBurstCallback(2, &object) == 0);
        assert(calls == 2);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-effect-layers"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
