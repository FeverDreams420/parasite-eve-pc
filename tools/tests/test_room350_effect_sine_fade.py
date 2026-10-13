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


class Room350EffectSineFadeTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_wave_fade_and_two_render_passes(self):
        source = host_unit("RoomM350_BodyRingCallback", under=ROOM)
        harness = '#include <assert.h>\n#include <stddef.h>\n#include <stdint.h>\n#include <string.h>\n' + source + r'''
typedef GteShortVector Vector;
typedef RoomM350SineParticle Particle;
FieldFxObjectPrefix *g_FieldFxOwner;
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag;
RenderEffectParameters g_FieldFxSpriteParams;
#define D_800F336C g_FieldFxSpriteParams.palette
unsigned short g_FieldFxPaletteTable[65536];
RenderColor D_8019A738, D_8019A734;
static RoomM350EffectActor actors[2];
static RoomM350EffectInstance instances[2];
/* The second pass draws at the owner's world height (transform.t[1]). */
#define WORLD_Y(i) instances[i].transform.t[1]
static Particle particle, expected;
static int beforeFrame, afterWaveFrame, afterRenderFrame, waveResult, amplitudeAfterWave;
static int waves, cluts, firstDraws, secondDraws, kind, palette, special;
static short renderSize, renderBrightness;
static unsigned short handle;
_Static_assert(sizeof(Vector) == 8 && sizeof(Particle) == 24, "effect layout");
_Static_assert(offsetof(Particle, controls) == 8, "controls offset");
_Static_assert(offsetof(Particle, brightness) == 16, "brightness offset");
_Static_assert(offsetof(Particle, size) == 18, "size offset");
_Static_assert(offsetof(Particle, amplitude) == 20, "amplitude offset");
_Static_assert(offsetof(RoomM350EffectActor, instance) == 8, "instance pointer offset");
static int signedHalf(uint32_t value) {
    value &= 65535;
    return value < 32768 ? (int)value : (int)value - 65536;
}
static int signedWord(uint32_t value) {
    return value <= INT32_MAX ? (int)value : (int)((int64_t)value - 4294967296LL);
}
static int sizeReference(int wave, int amplitude) {
    uint32_t low = (uint32_t)((uint64_t)(uint32_t)wave * (uint32_t)amplitude);
    int64_t product = signedWord(low);
    int64_t shifted = product >= 0 ? product / 4096 : -((-product + 4095) / 4096);
    return signedHalf((uint32_t)shifted);
}
int rsin(int angle) {
    assert(waves++ == 0 && angle == signedWord((uint32_t)beforeFrame << 4));
    /* Mutations test post-helper reads, not actual rsin side effects. */
    g_FieldFxCurrentAge = afterWaveFrame;
    particle.amplitude = amplitudeAfterWave;
    return waveResult;
}
u_short GetClut(int x, int y) {
    int add = kind == 4 && special ? 7 : 3;
    assert(cluts++ == 0 && waves == 0 && firstDraws == 0 && x == 0 && y == palette + add);
    particle.size = renderSize; particle.brightness = renderBrightness;
    D_800F336C = 65535; g_FieldFxAltPaletteFlag = 0;
    return handle;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *position, struct GteRotation *controls,
                   int w, int h, int tile, int clut, int flag, int brightness,
                   struct RenderColor *colors) {
    assert(firstDraws++ == 0 && cluts == 1 && secondDraws == 0);
    assert((void *)position == &particle && (void *)controls == particle.controls);
    assert(w == renderSize && h == renderSize && tile == 128 && clut == handle);
    assert(flag == 1 && brightness == renderBrightness && colors == 0);
    particle.position.x = -32768; particle.position.z = 32767;
    particle.size = -1234; particle.brightness = -99;
    g_FieldFxOwner = (FieldFxObjectPrefix *)&actors[1]; g_FieldFxCurrentAge = afterRenderFrame;
    expected = particle;
}
void FieldFx_DrawStarFan(struct GteShortVector *position, int w, int h, int mode,
                   struct GteRotation *texture, int sx, int sy,
                   struct RenderColor *colorsA, struct RenderColor *colorsB,
                   int brightness, int flag) {
    assert(secondDraws++ == 0 && firstDraws == 1);
    assert(position->x == -32768 && position->y == signedHalf(WORLD_Y(1)));
    assert(position->z == 32767);
    /* position.pad is deliberately uninitialized and must not be inspected. */
    assert(w == 384 && h == 384 && mode == 10 && sx == -1234 && sy == -1234);
    assert(texture->x == 1024 && texture->y == signedHalf((uint32_t)afterRenderFrame * 192));
    assert(texture->z == 0 && texture->flags == 1);
    assert(colorsA == &D_8019A738 && colorsB == &D_8019A734 && brightness == -99 && flag == 1);
}
static void reset(void) {
    waves = cluts = firstDraws = secondDraws = 0;
    memset(&particle, 0x5A, sizeof(particle));
    g_FieldFxCurrentAge = beforeFrame; g_FieldFxOwner = (FieldFxObjectPrefix *)&actors[0];
    D_800F336C = kind; g_FieldFxAltPaletteFlag = special; g_FieldFxPaletteTable[kind] = palette;
}
static void runUpdate(int brightness) {
    int faded = afterWaveFrame >= 8 ? signedHalf((uint32_t)brightness - 8) : brightness;
    reset(); particle.brightness = brightness; expected = particle;
    expected.amplitude = amplitudeAfterWave;
    expected.size = sizeReference(waveResult, amplitudeAfterWave);
    expected.brightness = faded;
    assert(RoomM350_BodyRingCallback(1, &particle) == (afterWaveFrame >= 8 && faded < 16));
    assert(waves == 1 && cluts == 0 && firstDraws == 0 && secondDraws == 0);
    assert(memcmp(&particle, &expected, sizeof(particle)) == 0);
}
int main(void) {
    const int frames[] = {-1, 0, 7, 8, 9, 32768, INT32_MIN, INT32_MAX};
    const int waveValues[] = {-4096, -1, 0, 4096, INT32_MIN, INT32_MAX};
    const int kinds[] = {0, 3, 4, 5, 255, 32768, 65535};
    const int flags[] = {0, 1, -1};
    const int values[] = {-32768, -1, 0, 1, 15, 16, 23, 24, 128, 32767};
    const unsigned short rows[] = {0, 1, 255, 32768, 65535};
    int i, j, k, l, m, n, value, event;
    actors[0].instance = &instances[0]; actors[1].instance = &instances[1];
    WORLD_Y(0) = 123; WORLD_Y(1) = 100000;
    /* Every signed amplitude, including product wrap and negative arithmetic shifts. */
    for (i = 0; i < 6; ++i) for (value = -32768; value <= 32767; ++value) {
        beforeFrame = frames[i]; afterWaveFrame = 7; waveResult = waveValues[i];
        amplitudeAfterWave = value; runUpdate(128);
    }
    /* Every brightness for both sides of the frame threshold, including halfword wrap. */
    for (i = 0; i < 8; ++i) for (value = -32768; value <= 32767; ++value) {
        beforeFrame = frames[(i + 1) % 8]; afterWaveFrame = frames[i];
        waveResult = -4096; amplitudeAfterWave = 32767; runUpdate(value);
    }
    for (i = 0; i < 7; ++i) for (j = 0; j < 3; ++j) for (k = 0; k < 5; ++k)
    for (l = 0; l < 10; ++l) for (m = 0; m < 10; ++m) for (n = 0; n < 8; ++n) {
        kind = kinds[i]; special = flags[j]; palette = rows[k]; handle = rows[(k + 1) % 5];
        renderSize = values[l]; renderBrightness = values[m]; afterRenderFrame = frames[n];
        reset(); assert(RoomM350_BodyRingCallback(2, &particle) == 0);
        assert(waves == 0 && cluts == 1 && firstDraws == 1 && secondDraws == 1);
        assert(memcmp(&particle, &expected, sizeof(particle)) == 0);
    }
    for (event = -3; event <= 4; ++event) {
        if (event == 1 || event == 2) continue;
        reset(); expected = particle;
        assert(RoomM350_BodyRingCallback(event, &particle) == 0 && waves == 0 && cluts == 0);
        assert(memcmp(&particle, &expected, sizeof(particle)) == 0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "room350-sine-fade"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-fwrapv", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
