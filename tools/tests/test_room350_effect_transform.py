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


class Room350EffectTransformTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_signed_offset_transform_and_render_reloads(self):
        source = host_unit("RoomM350_AreaSparkCallback", under=ROOM)
        harness = '#include <assert.h>\n#include <stddef.h>\n#include <string.h>\n' + source + r'''
RenderEffectParameters g_FieldFxSpriteParams;
GteSineWord rcossin_tbl[4096];
#define D_800F336A g_FieldFxSpriteParams.parameter02
#define D_800F336C g_FieldFxSpriteParams.palette
FieldFxObjectPrefix *g_FieldFxOwner;
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag, D_8019A56C[4];
unsigned short g_FieldFxPaletteTable[8];
static RoomM350EffectActor actor;
static RoomM350EffectInstance instance;
/* The effect origin is the translation of the instance's matrix 33. */
static GteMatrix transforms[34];
#define ORIGIN transforms[33].t
static short object[4], expectedPosition[3];
static int lookups, draws, handleValue, expectedShade, sizeValue, renderScale;
_Static_assert(offsetof(RoomM350EffectActor, instance) == 8, "actor instance offset");
_Static_assert(sizeof(GteMatrix) * 33 + offsetof(GteMatrix, t) == 0x434, "origin offset");
u_short GetClut(int x, int y) {
    assert(lookups++ == 0 && draws == 0 && x == 32);
    assert(y == g_FieldFxPaletteTable[D_800F336C] +
           (D_800F336C == 4 && g_FieldFxAltPaletteFlag ? 4 : 0));
    /* Mutations test captured coordinates/shade and reloaded size/scale. */
    memset(ORIGIN, 0, sizeof(ORIGIN));
    memset(rcossin_tbl, 0, sizeof(rcossin_tbl));
    object[0] = 123; object[1] = 456; object[2] = 789;
    object[3] = sizeValue; D_800F336A = renderScale; g_FieldFxCurrentAge += 3;
    return handleValue;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p, struct GteRotation *zero, int a,
                   int b, int scale, int handle, int flag, int shade,
                   struct RenderColor *colors) {
    assert(lookups == 1 && draws++ == 0 && p != object && zero == 0);
    /* The fourth local halfword is not initialized by retail. */
    assert(memcmp(p, expectedPosition, sizeof(expectedPosition)) == 0);
    assert(a == sizeValue && b == sizeValue && scale == renderScale + 216);
    assert(handle == (unsigned short)handleValue && flag == 1);
    assert(shade == expectedShade && colors == D_8019A56C);
}
int main(void) {
    const short values[] = {-32768, -4097, -33, -1, 0, 32, 4097, 32767};
    int counter, event, kind, special, h, s, v, i;
    actor.instance = &instance; instance.transforms = transforms;
    g_FieldFxOwner = (FieldFxObjectPrefix *)&actor;
    for (counter = -16; counter <= 80; ++counter)
    for (event = -1; event <= 3; ++event) {
        if (event == 2) continue;
        g_FieldFxCurrentAge = counter; lookups = draws = 0;
        assert(RoomM350_AreaSparkCallback(event, 0) == (event == 1 && counter >= 8));
        assert(lookups == 0 && draws == 0);
    }
    for (counter = -16; counter <= 80; ++counter)
    for (kind = 0; kind < 8; ++kind) for (special = 0; special < 2; ++special)
    for (h = 0; h < 2; ++h) for (s = 0; s < 8; ++s) for (v = 0; v < 8; ++v) {
        unsigned int index = ((unsigned int)counter << 7) & 0xF80;
        for (i = 0; i < 8; ++i) g_FieldFxPaletteTable[i] = h ? 65535 : i * 100;
        memset(rcossin_tbl, 0, sizeof(rcossin_tbl));
        g_FieldFxCurrentAge = counter; D_800F336C = kind; g_FieldFxAltPaletteFlag = special;
        rcossin_tbl[index].word = (int)(((unsigned int)(unsigned short)values[s] << 16) |
                                  (unsigned short)values[7-s]);
        expectedShade = values[7-s] >> 5;
        for (i = 0; i < 3; ++i) {
            int product;
            object[i] = values[(v + i) % 8];
            ORIGIN[i] = (i - 1) * 100000 + counter;
            product = object[i] * values[s];
            expectedPosition[i] = (short)(((product + (product < 0 ? 4095 : 0)) >> 12) +
                                          ORIGIN[i]);
        }
        object[3] = 999; D_800F336A = 999;
        sizeValue = values[v]; renderScale = values[7-v];
        handleValue = h ? -1 : 0x12345;
        lookups = draws = 0;
        assert(RoomM350_AreaSparkCallback(2, object) == 0);
        assert(lookups == 1 && draws == 1);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-effect-transform"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)
