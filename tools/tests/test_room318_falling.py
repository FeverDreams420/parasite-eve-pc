import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import find_definition, host_unit, psx_compile  # noqa: E402

ROOM = 'src/overlays/room_m318_chrysler'
FUNCTION = 'RoomM318_SpiralDropSparkCallback'


class FallingSprite(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char layout_a[sizeof(RoomSpiralDropSparkRecord)==8 &&
    OFF(RoomSpiralDropSparkRecord,callback.speed)==6 ? 1:-1];
typedef char layout_b[sizeof(GteShortVector)==8 && OFF(GteShortVector,z)==4 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
typedef RoomSpiralDropSparkCallbackView Particle;
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag;
unsigned short g_FieldFxPaletteTable[8];
RenderEffectParameters g_FieldFxSpriteParams;
char D_8019948C[1];
static Particle particle;
static int generated, cluts, draws, expectedPalette, mutate;
void FieldFx_SampleColorTrack(void *table, void *out, int frame) {
    int *color = out;
    assert(table == D_8019948C && frame == g_FieldFxCurrentAge);
    assert(generated++ == 0 && cluts == 0 && draws == 0);
    color[0] = 123; color[1] = 456;
    if (mutate) { particle.x = 65535; particle.y = 32768; particle.z = 32767; }
}
u_short GetClut(int x, int y) {
    assert(generated == 1 && cluts++ == 0 && draws == 0);
    assert(x == 0 && y == expectedPalette);
    return 0xCAFE;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p, struct GteRotation *rotation,
                  int w, int h, int tile, int clut, int flag, int shade,
                  struct RenderColor *data) {
    int *color = data;
    assert(generated == 1 && cluts == 1 && draws++ == 0);
    assert((unsigned short)p->x == particle.x);
    assert((unsigned short)p->y == particle.y && (unsigned short)p->z == particle.z);
    assert(rotation == 0 && w == 2048 && h == 2048 && tile == 6);
    assert(clut == 0xCAFE && flag == 1 && shade == 128);
    assert(color[0] == 123 && color[1] == 456);
}
int main(void) {
    int frames[] = {INT_MIN, -1, 0, 23, 24, INT_MAX};
    unsigned short speeds[] = {0, 1, 32768, 65535};
    unsigned int i, j;
    int kind, variant;
    for (i=0; i<sizeof(frames)/sizeof(frames[0]); ++i)
        for (j=0; j<sizeof(speeds)/sizeof(speeds[0]); ++j) {
            particle.x=12; particle.y=3; particle.z=34; particle.speed=speeds[j];
            g_FieldFxCurrentAge=frames[i];
            assert(RoomM318_SpiralDropSparkCallback(1, (RoomSpiralDropSparkRecord *)&particle) == (frames[i]>=24));
            assert(particle.y == (unsigned short)(3-speeds[j]));
            assert(particle.speed == (unsigned short)(speeds[j]-1));
            assert(particle.x==12 && particle.z==34);
        }
    assert(!generated && !cluts && !draws);
    for (kind=0; kind<8; ++kind) for (variant=0; variant<2; ++variant) {
        generated=cluts=draws=0; mutate=variant;
        g_FieldFxSpriteParams.palette=kind; g_FieldFxAltPaletteFlag=variant;
        g_FieldFxPaletteTable[kind]=65535;
        expectedPalette=65535+(kind==4 && variant ? 10:6);
        assert(RoomM318_SpiralDropSparkCallback(2, (RoomSpiralDropSparkRecord *)&particle)==0);
        assert(generated==1 && cluts==1 && draws==1);
    }
    assert(RoomM318_SpiralDropSparkCallback(0, (RoomSpiralDropSparkRecord *)&particle)==0);
    assert(RoomM318_SpiralDropSparkCallback(-1, (RoomSpiralDropSparkRecord *)&particle)==0);
    assert(RoomM318_SpiralDropSparkCallback(INT_MAX, (RoomSpiralDropSparkRecord *)&particle)==0);
    assert(generated==1 && cluts==1 && draws==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
