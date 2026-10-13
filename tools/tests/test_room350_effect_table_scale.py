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


class Room350EffectTableScaleTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_threshold_and_post_lookup_fields(self):
        source = host_unit("RoomM350_OrbTableFlashCallback", under=ROOM)
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
RenderEffectParameters g_FieldFxSpriteParams;
GteSineWord rcossin_tbl[4096];
RenderColor D_8019A3D0;
#define D_800F336A g_FieldFxSpriteParams.parameter02
#define D_800F336C g_FieldFxSpriteParams.palette
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag;
unsigned short g_FieldFxPaletteTable[8];
unsigned char D_8019A4EC[32];
static short position[4];
static int lookups, draws, nextCounter, expectedSize, expectedScale, expectedShade;
static unsigned int multiplier;
static int handleValue;
u_short GetClut(int x, int y) {
    unsigned int index = ((unsigned int)(nextCounter - 1) << 7) & 0xF80;
    assert(lookups++ == 0 && draws == 0 && x == 0);
    assert(y == g_FieldFxPaletteTable[D_800F336C] +
           (D_800F336C == 4 && g_FieldFxAltPaletteFlag ? 8 : 4));
    g_FieldFxCurrentAge = nextCounter; D_800F336A = expectedScale;
    D_8019A4EC[(nextCounter - 1) / 2] = multiplier;
    rcossin_tbl[index].word = (int)(0x76540000U | (unsigned short)expectedShade);
    position[3] = expectedSize;
    return handleValue;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *point, struct GteRotation *zero,
                   int a, int b, int scale, int handle, int flag, int shade,
                   struct RenderColor *colors) {
    assert(lookups == 1 && draws++ == 0 && point == position && zero == 0);
    assert(a == expectedSize && b == expectedSize);
    assert(scale == expectedScale * (int)multiplier + 128);
    assert(handle == (unsigned short)handleValue && flag == 1);
    assert(shade == expectedShade >> 5 && colors == &D_8019A3D0);
}
int main(void) {
    const short values[] = {-32768, -33, -1, 0, 32, 32767};
    const unsigned int weights[] = {0, 1, 127, 128, 255};
    short before[4];
    int counter, event, kind, special, h, s, w, i;
    for (counter = -16; counter <= 32; ++counter)
    for (event = -1; event <= 3; ++event) {
        if (event == 2) continue;
        g_FieldFxCurrentAge = counter; lookups = draws = 0;
        assert(RoomM350_OrbTableFlashCallback(event, (Particle *)0) == (event == 1 && counter >= 8));
        assert(lookups == 0 && draws == 0);
    }
    for (counter = 0; counter < 32; ++counter)
    for (kind = 0; kind < 8; ++kind) for (special = 0; special < 2; ++special)
    for (h = 0; h < 2; ++h) for (s = 0; s < 6; ++s) for (w = 0; w < 5; ++w) {
        for (i = 0; i < 8; ++i) g_FieldFxPaletteTable[i] = h ? 65535 : i * 100;
        memset(D_8019A4EC, 37, sizeof(D_8019A4EC));
        g_FieldFxCurrentAge = counter; nextCounter = counter + 1;
        D_800F336C = kind; g_FieldFxAltPaletteFlag = special; D_800F336A = 999;
        expectedScale = values[s]; expectedShade = values[5-s];
        expectedSize = values[(s+1)%6]; multiplier = weights[w];
        handleValue = h ? -1 : 0x12345;
        position[0] = 11; position[1] = -22; position[2] = 33; position[3] = 999;
        memcpy(before, position, sizeof(position)); before[3] = expectedSize;
        lookups = draws = 0;
        assert(RoomM350_OrbTableFlashCallback(2, (Particle *)position) == 0);
        assert(lookups == 1 && draws == 1);
        assert(memcmp(position, before, sizeof(position)) == 0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-effect-table-scale"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
