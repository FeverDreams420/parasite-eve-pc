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


class Room350EffectPulseTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_pre_lookup_scale_and_post_lookup_shade(self):
        source = host_unit("RoomM350_HandPulseCallback", under=ROOM)
        # The two sine-table views alias: D_800966EE is the halfword at +2.
        source = source.replace('extern short D_800966EE[];',
            '#define D_800966EE ((short *)rcossin_tbl + 1)')
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag;
GteSineWord rcossin_tbl[4096];
#define table ((int *)rcossin_tbl)
RenderEffectParameters g_FieldFxSpriteParams;
#define D_800F336C g_FieldFxSpriteParams.palette
unsigned short g_FieldFxPaletteTable[8];
static Particle effect;
static int position, nextPosition, lookups, draws, handleValue;
static int nextCounter, expectedScale, expectedShade;
u_short GetClut(int x, int y) {
    unsigned int index = ((unsigned int)(nextCounter - 1) << 7) & 0xF80;
    assert(lookups++ == 0 && draws == 0 && x == 48);
    assert(y == g_FieldFxPaletteTable[D_800F336C] +
           (D_800F336C == 4 && g_FieldFxAltPaletteFlag ? 4 : 0));
    g_FieldFxCurrentAge = nextCounter;
    /* Different high half catches accidental recomputation of pre-call scale. */
    table[index] = (int)(0x7FF00000U | (unsigned short)expectedShade);
    effect.position = (Vector *)&nextPosition;
    return handleValue;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *point, struct GteRotation *context, int a,
                  int b, int c, int handle, int flag, int shade,
                  struct RenderColor *colors) {
    assert(lookups == 1 && draws++ == 0);
    assert(point == &nextPosition && context == &effect);
    assert(a == expectedScale && b == expectedScale && c == 110);
    assert(handle == (unsigned short)handleValue && flag == 1);
    assert(shade == expectedShade >> 5 && colors == &effect.color);
}
int main(void) {
    const int values[] = {-32768, -33, -1, 0, 32, 32767};
    Particle before;
    int counter, event, kind, special, h, s, i;
    for (counter = -16; counter <= 32; ++counter)
    for (event = -1; event <= 3; ++event) {
        if (event == 2) continue;
        g_FieldFxCurrentAge = counter; lookups = draws = 0;
        assert(RoomM350_HandPulseCallback(event, 0) == (event == 1 && counter >= 16));
        assert(lookups == 0 && draws == 0);
    }
    for (counter = 0; counter < 32; ++counter)
    for (kind = 0; kind < 8; ++kind) for (special = 0; special < 2; ++special)
    for (h = 0; h < 2; ++h) for (s = 0; s < 6; ++s) {
        unsigned int index = (((unsigned int)counter - 1) << 8) & 0x1F00;
        for (i = 0; i < 4096; ++i) table[i] = 0x12345678;
        for (i = 0; i < 8; ++i) g_FieldFxPaletteTable[i] = h ? 65535 : i * 100;
        g_FieldFxCurrentAge = counter; nextCounter = counter + 1;
        D_800F336C = kind; g_FieldFxAltPaletteFlag = special;
        D_800966EE[index] = values[s];
        expectedScale = values[s] * 4 + 2048; expectedShade = values[5-s];
        handleValue = h ? -1 : 0x12345;
        memset(&effect, 0xCC, sizeof(effect));
        effect.position = (Vector *)&position;
        memcpy(&before, &effect, sizeof(effect));
        before.position = (Vector *)&nextPosition;
        lookups = draws = 0;
        assert(RoomM350_HandPulseCallback(2, &effect) == 0);
        assert(lookups == 1 && draws == 1);
        assert(memcmp(&effect, &before, sizeof(effect)) == 0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-effect-pulse"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing",
                 "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
