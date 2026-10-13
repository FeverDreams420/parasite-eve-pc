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


class Room350EffectBallisticTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_motion_and_palette_rendering(self):
        source = host_unit("RoomM350_AreaDropletCallback", under=ROOM)
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag, D_8019A598[4];
GteSineWord rcossin_tbl[4096];
RenderEffectParameters g_FieldFxSpriteParams;
unsigned short g_FieldFxPaletteTable[8];
#define D_800F336C g_FieldFxSpriteParams.palette
#define D_800F336A g_FieldFxSpriteParams.parameter02
static short position[5];
static int lookups, draws, nextCounter, expectedScale, expectedShade, handleValue;
u_short GetClut(int x, int y) {
    unsigned int index = ((unsigned int)(nextCounter - 1) << 7) & 0xF80;
    assert(lookups++ == 0 && draws == 0 && x == 16);
    assert(y == g_FieldFxPaletteTable[D_800F336C] +
           (D_800F336C == 4 && g_FieldFxAltPaletteFlag ? 4 : 0));
    g_FieldFxCurrentAge = nextCounter; D_800F336A = expectedScale;
    rcossin_tbl[index].word = (int)(0x76540000U | (unsigned short)expectedShade);
    return handleValue;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *point, struct GteRotation *zero, int a,
                  int b, int scale, int handle, int flag, int shade,
                  struct RenderColor *colors) {
    assert(lookups == 1 && draws++ == 0 && point == position && zero == 0);
    assert(a == 12288 && b == 12288);
    assert(scale == expectedScale * (((nextCounter - 1) >> 1) & 7) + 200);
    assert(handle == (unsigned short)handleValue && flag == 1);
    assert(shade == expectedShade >> 5 && colors == D_8019A598);
}
int main(void) {
    const short values[] = {-32768, -33, -1, 0, 32, 32767};
    short expected[5];
    int counter, event, y, v, a, kind, special, h, s, i;
    for (counter = -16; counter <= 32; ++counter)
    for (event = -1; event <= 3; ++event)
    for (y = 0; y < 6; ++y) for (v = 0; v < 6; ++v) for (a = 0; a < 6; ++a) {
        int result = event == 1 && counter >= 16;
        if (event == 2) continue;
        position[0] = 11; position[1] = values[y]; position[2] = -22;
        position[3] = values[v]; position[4] = values[a];
        memcpy(expected, position, sizeof(position));
        if (event == 1 && counter < 16) {
            expected[1] = (short)(position[1] + position[3]);
            expected[3] = (short)(position[3] - position[4]);
        }
        g_FieldFxCurrentAge = counter; lookups = draws = 0;
        assert(RoomM350_AreaDropletCallback(event, position) == result);
        assert(lookups == 0 && draws == 0);
        assert(memcmp(position, expected, sizeof(position)) == 0);
    }
    for (counter = 0; counter < 32; ++counter)
    for (kind = 0; kind < 8; ++kind) for (special = 0; special < 2; ++special)
    for (h = 0; h < 2; ++h) for (s = 0; s < 6; ++s) {
        for (i = 0; i < 8; ++i) g_FieldFxPaletteTable[i] = h ? 65535 : i * 100;
        g_FieldFxCurrentAge = counter; nextCounter = counter + 1;
        D_800F336C = kind; g_FieldFxAltPaletteFlag = special; D_800F336A = 999;
        expectedScale = values[s]; expectedShade = values[5-s];
        handleValue = h ? -1 : 0x12345;
        memcpy(expected, position, sizeof(position));
        lookups = draws = 0;
        assert(RoomM350_AreaDropletCallback(2, position) == 0);
        assert(lookups == 1 && draws == 1);
        assert(memcmp(position, expected, sizeof(position)) == 0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-effect-ballistic"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
