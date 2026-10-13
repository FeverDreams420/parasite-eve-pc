import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import find_definition, host_unit, psx_compile  # noqa: E402

ROOM = 'src/overlays/room_m273_liberty'
FUNCTION = 'RoomM273_SweepSplashCallback'


class RisingSprite(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(GteShortVector)==8 &&
    (unsigned long)&((GteShortVector *)0)->y==2 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
typedef GteShortVector Vector;
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag;
unsigned short g_FieldFxPaletteTable[8];
RenderEffectParameters g_FieldFxSpriteParams;
short D_8019AD68[8];
GteSineWord rcossin_tbl[4096];
static Vector position;
static int cluts, draws, expectedPalette, expectedTile, expectedShade, mutate;
u_short GetClut(int x, int y) {
    assert(cluts++==0 && draws==0 && x==64 && y==expectedPalette);
    if (mutate) { g_FieldFxCurrentAge=999; g_FieldFxSpriteParams.parameter02=-2; }
    return 0xF123;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p, struct GteRotation *rotation,
                  int w, int h, int tile, int clut, int flag, int shade,
                  struct RenderColor *color) {
    assert(cluts==1 && draws++==0 && p==&position && rotation==0);
    assert(w==4096 && h==4096 && tile==expectedTile);
    assert(clut==0xF123 && flag==1 && shade==expectedShade && color==0);
}
int main(void) {
    int i, kind, variant, frame, index;
    int timers[]={INT_MIN,0,15,16,INT_MAX};
    for (i=0;i<5;++i) {
        g_FieldFxCurrentAge=timers[i]; position.x=12; position.y=-32768;
        position.z=34; position.pad=56;
        assert(RoomM273_SweepSplashCallback(1,&position)==(timers[i]>=16));
        assert(position.y==(timers[i]>=16 ? -32768:32762));
        assert(position.x==12 && position.z==34 && position.pad==56);
    }
    assert(!cluts && !draws);
    for (i=0;i<8;++i) D_8019AD68[i]=i+3;
    for (kind=0;kind<8;++kind) for (variant=0;variant<2;++variant)
        for (i=0;i<=16;++i) {
            g_FieldFxCurrentAge=i; frame=i-1;
            g_FieldFxSpriteParams.palette=kind; g_FieldFxAltPaletteFlag=variant;
            g_FieldFxPaletteTable[kind]=65535;
            expectedPalette=65535+(kind==4 && variant ? 4:0);
            g_FieldFxSpriteParams.parameter02=2; mutate=variant;
            expectedTile=44+(variant ? -2:2)*D_8019AD68[frame/4];
            index=(((unsigned int)frame<<9)&0x3E00)/4;
            rcossin_tbl[index].word=variant ? 0x1234FFE0:0x56780040;
            expectedShade=variant ? -1:2;
            cluts=draws=0;
            assert(RoomM273_SweepSplashCallback(2,&position)==0 && cluts==1 && draws==1);
        }
    assert(RoomM273_SweepSplashCallback(0,&position)==0);
    assert(RoomM273_SweepSplashCallback(-1,&position)==0);
    assert(RoomM273_SweepSplashCallback(INT_MAX,&position)==0);
    assert(cluts==1 && draws==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
