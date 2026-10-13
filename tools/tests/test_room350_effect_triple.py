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


class Room350EffectTripleTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_signed_samples_palette_and_reload_order(self):
        source = host_unit("RoomM350_HandFlashCallback", under=ROOM)
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
RenderEffectParameters g_FieldFxSpriteParams;
GteSineWord rcossin_tbl[4096];
#define D_800F336A g_FieldFxSpriteParams.parameter02
#define D_800F336C g_FieldFxSpriteParams.palette
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag;
unsigned short g_FieldFxPaletteTable[8];
static int point, replacement, lookups, draws, nextCounter, expectedSize;
static int expectedScale, expectedShade, handleValue;
static void *object;
u_short GetClut(int x, int y) {
    unsigned int index = ((unsigned int)(nextCounter - 1) << 7) & 0xF80;
    assert(lookups++ == 0 && draws == 0 && x == 0);
    assert(y == g_FieldFxPaletteTable[D_800F336C] +
           (D_800F336C == 4 && g_FieldFxAltPaletteFlag ? 8 : 4));
    /* Deliberate stub mutations test reloads, not real GetClut side effects. */
    memset(rcossin_tbl, 0, sizeof(rcossin_tbl));
    g_FieldFxCurrentAge = nextCounter; D_800F336A = expectedScale;
    rcossin_tbl[index].word = (int)(0x76540000U | (unsigned short)expectedShade);
    object = &replacement;
    return handleValue;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p, struct GteRotation *zero, int a,
                   int b, int scale, int handle, int flag, int shade,
                   struct RenderColor *colors) {
    assert(lookups == 1 && draws++ == 0 && p == &replacement && zero == 0);
    assert(a == expectedSize && b == expectedSize);
    assert(scale == expectedScale * 6 + 128);
    assert(handle == (unsigned short)handleValue && flag == 1);
    assert(shade == expectedShade >> 5 && colors == 0);
}
int main(void) {
    const short values[] = {-32768, -33, -1, 0, 32, 32767};
    int counter, event, kind, special, h, s, i;
    for (counter = -16; counter <= 80; ++counter)
    for (event = -1; event <= 3; ++event) {
        if (event == 2) continue;
        g_FieldFxCurrentAge = counter; lookups = draws = 0;
        assert(RoomM350_HandFlashCallback(event, 0) == (event == 1 && counter >= 16));
        assert(lookups == 0 && draws == 0);
    }
    for (counter = -16; counter <= 80; ++counter)
    for (kind = 0; kind < 8; ++kind) for (special = 0; special < 2; ++special)
    for (h = 0; h < 2; ++h) for (s = 0; s < 6; ++s) {
        unsigned int offset = ((unsigned int)(counter - 1) << 8) & 0x3F00;
        memset(rcossin_tbl, 0, sizeof(rcossin_tbl));
        memcpy((char *)rcossin_tbl + offset, &values[s], sizeof(short));
        for (i = 0; i < 8; ++i) g_FieldFxPaletteTable[i] = h ? 65535 : i * 100;
        g_FieldFxCurrentAge = counter; nextCounter = counter + 1;
        D_800F336C = kind; g_FieldFxAltPaletteFlag = special; D_800F336A = 999;
        expectedSize = values[s] * 3;
        expectedScale = values[s]; expectedShade = values[5-s];
        handleValue = h ? -1 : 0x12345;
        object = &point; lookups = draws = 0;
        assert(RoomM350_HandFlashCallback(2, &object) == 0);
        assert(lookups == 1 && draws == 1);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-effect-triple"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
