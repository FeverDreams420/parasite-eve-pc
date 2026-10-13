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


class Room350EffectConfiguredPulseTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_configuration_and_pre_post_clut_samples(self):
        source = host_unit("RoomM350_AreaPlayerPulseCallback", under=ROOM)
        harness = '#include <assert.h>\n#include <stddef.h>\n#include <stdint.h>\n#include <string.h>\n' + source + r'''
int g_FieldFxCurrentAge;
/* The retail halfword names are fields of the sprite parameter block; the
 * signed ones are read through signed views as the old globals were. */
RenderEffectParameters g_FieldFxSpriteParams;
#define SIGNED(field) (*(short *)&g_FieldFxSpriteParams.field)
#define UNIT SIGNED(parameter00)
#define D_800F336A SIGNED(parameter02)
#define D_800F336C g_FieldFxSpriteParams.palette
#define D_800F336E SIGNED(parameter06)
#define D_800F3370 g_FieldFxSpriteParams.tpage
#define D_800F3372 SIGNED(parameter0A)
#define D_800F3374 g_FieldFxSpriteParams.depth
#define D_800F3376 SIGNED(extent_x)
#define D_800F3378 SIGNED(extent_y)
unsigned short g_FieldFxTpageSlot2, g_FieldFxPaletteEntry2, g_FieldFxTpageTable[65536];
/* Size samples are the low halfword, shades the high halfword of a word. */
GteSineWord rcossin_tbl[4096];
#define HALVES ((short *)rcossin_tbl)
int D_8019A59C[4];
static unsigned char object[16];
static int cluts, draws, before, after, sample, shade, scale;
static unsigned short handle, paletteIndex, clutY;
_Static_assert(offsetof(RenderEffectParameters, parameter02) == 2, "scale alias");
_Static_assert(sizeof(GteSineWord) == 4, "signed high halfword view");
static unsigned tableIndex(int frame) { return (((uint32_t)frame - 1) << 9 & 0x3E00) / 2; }
static int signedHalf(int value) {
    unsigned v = (unsigned)value & 65535;
    return v < 32768 ? (int)v : (int)v - 65536;
}
u_short GetClut(int x, int y) {
    assert(cluts++ == 0 && draws == 0 && x == 0 && y == clutY);
    assert(UNIT == 32 && D_800F336A == 2);
    assert(D_800F3376 == 32 && D_800F3378 == 32 && D_800F336C == 2);
    assert(D_800F336E == 0 && D_800F3372 == 0 && D_800F3374 == 32);
    assert(D_800F3370 == (unsigned short)(65535 - paletteIndex));
    /* These mutations test capture/reload order, not the actual helper's effects. */
    g_FieldFxCurrentAge = after;
    D_800F336A = scale;
    HALVES[tableIndex(before)] = 12345;
    HALVES[tableIndex(after) + 1] = shade;
    g_FieldFxTpageSlot2 ^= 65535;
    g_FieldFxPaletteEntry2 ^= 65535;
    return handle;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *position, struct GteRotation *unused,
                   int width, int height, int tile, int clut, int flag,
                   int brightness, struct RenderColor *colors) {
    unsigned phase = (((uint32_t)after - 1) >> 2) & 3;
    int expectedShade = shade >= 0 ? shade / 32 : -((-shade + 31) / 32);
    assert(draws++ == 0 && cluts == 1 && (void *)position == object && unused == 0);
    assert(width == signedHalf(sample * 2 + 4096) && height == width);
    assert(tile == scale * (int)phase + 192 && clut == handle && flag == 1);
    assert(brightness == expectedShade && (int *)colors == D_8019A59C);
}
static void reset(void) {
    cluts = draws = 0;
    g_FieldFxCurrentAge = before;
    UNIT = -999; D_800F336A = -998;
    D_800F336E = -997; D_800F3372 = -996; D_800F3374 = -995;
    D_800F3376 = -994; D_800F3378 = -993; D_800F336C = 65535; D_800F3370 = 65534;
    g_FieldFxTpageSlot2 = paletteIndex; g_FieldFxPaletteEntry2 = clutY;
    g_FieldFxTpageTable[paletteIndex] = 65535 - paletteIndex;
    HALVES[tableIndex(before)] = sample;
    HALVES[tableIndex(after) + 1] = 23456;
}
int main(void) {
    const int frames[] = {-1, 0, 1, 4, 7, 8, 16, 31, 32, 33, INT32_MIN, INT32_MAX};
    const int samples[] = {-32768, -16384, -2048, -33, -1, 0, 1, 16384, 32767};
    const int scales[] = {-32768, -1, 0, 1, 2, 123, 32767};
    const unsigned short palettes[] = {0, 1, 255, 32768, 65535};
    int i, j, k, l, m, n, event;
    unsigned char untouched[16];
    memset(object, 0x5A, sizeof(object)); memset(untouched, 0x5A, sizeof(untouched));
    for (i = 0; i < 12; ++i) {
        before = frames[i]; reset();
        assert(RoomM350_AreaPlayerPulseCallback(1, object) == (before >= 8));
        assert(cluts == 0 && draws == 0 && UNIT == -999 && D_800F336A == -998);
    }
    for (i = 0; i < 12; ++i) for (j = 0; j < 12; ++j)
    for (k = 0; k < 9; ++k) for (l = 0; l < 9; ++l)
    for (m = 0; m < 7; ++m) for (n = 0; n < 5; ++n) {
        before = frames[i]; after = frames[j]; sample = samples[k]; shade = samples[l];
        scale = scales[m]; paletteIndex = palettes[n]; clutY = palettes[(n + 1) % 5];
        handle = palettes[(n + 2) % 5]; reset();
        assert(RoomM350_AreaPlayerPulseCallback(2, object) == 0 && cluts == 1 && draws == 1);
        assert(memcmp(object, untouched, sizeof(object)) == 0);
    }
    for (event = -3; event <= 4; ++event) {
        if (event == 1 || event == 2) continue;
        reset();
        assert(RoomM350_AreaPlayerPulseCallback(event, object) == 0 && cluts == 0 && draws == 0);
        assert(UNIT == -999 && D_800F336A == -998);
        assert(D_800F3370 == 65534 && D_800F3376 == -994 && D_800F3378 == -993);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-configured-pulse"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-fwrapv", "-fno-strict-aliasing",
                 "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
