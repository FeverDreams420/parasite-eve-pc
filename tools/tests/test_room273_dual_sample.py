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
FUNCTION = 'RoomM273_SwayPointerCallback'


class DualSampleSprite(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Effect)==4 &&
    (unsigned long)&((Effect *)0)->position==0 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag;
GteSineWord rcossin_tbl[4096];
unsigned short g_FieldFxPaletteTable[8];
RenderEffectParameters g_FieldFxSpriteParams;
static int before,after;
static Effect effect;
static int cluts,draws,expectedPalette,expectedSize,expectedShade;
static int sizeIndex,shadeIndex;
u_short GetClut(int x, int y) {
    assert(cluts++==0 && !draws && x==0 && y==expectedPalette);
    g_FieldFxCurrentAge=999;
    rcossin_tbl[sizeIndex].word=0;
    rcossin_tbl[shadeIndex].word=0;
    effect.position=(Position *)&after;
    return 0xF123;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p, struct GteRotation *rotation,
                  int w, int h, int tile, int clut, int flag, int shade,
                  struct RenderColor *color) {
    assert(cluts==1 && draws++==0 && (void *)p==&after && rotation==0);
    assert(w==expectedSize && h==expectedSize && tile==64);
    assert(clut==0xF123 && flag==1 && shade==expectedShade && color==0);
}
int main(void) {
    int times[]={INT_MIN,-1,0,15,16,INT_MAX};
    int frames[]={-32769,-1,0,1,15,16,63,64,32767,32768,65536};
    int words[]={0,0x12347FFF,0x12348000,0x1234FFFF};
    int i,j,k,kind,variant,frame;
    for(i=0;i<6;++i) {
        g_FieldFxCurrentAge=times[i];
        assert(RoomM273_SwayPointerCallback(1,&effect)==(times[i]>=16));
    }
    assert(!cluts && !draws);
    for(kind=0;kind<8;++kind) for(variant=0;variant<2;++variant)
        for(i=0;i<11;++i) for(j=0;j<4;++j) for(k=0;k<4;++k) {
            g_FieldFxCurrentAge=frames[i]+1; frame=(short)frames[i];
            sizeIndex=(((unsigned int)frame<<8)&0x3F00)/4;
            shadeIndex=(((unsigned int)frame<<9)&0x3E00)/4;
            rcossin_tbl[sizeIndex].word=words[j];
            rcossin_tbl[shadeIndex].word=words[k];
            expectedSize=(short)(rcossin_tbl[sizeIndex].word+2048);
            expectedShade=(short)rcossin_tbl[shadeIndex].word >> 6;
            g_FieldFxSpriteParams.palette=kind; g_FieldFxAltPaletteFlag=variant;
            g_FieldFxPaletteTable[kind]=65535;
            expectedPalette=65535+(kind==4 && variant ? 7:3);
            effect.position=(Position *)&before; cluts=draws=0;
            assert(RoomM273_SwayPointerCallback(2,&effect)==0 && cluts==1 && draws==1);
        }
    assert(RoomM273_SwayPointerCallback(0,&effect)==0);
    assert(RoomM273_SwayPointerCallback(-1,&effect)==0);
    assert(RoomM273_SwayPointerCallback(INT_MAX,&effect)==0);
    assert(cluts==1 && draws==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', str(path), '-o', str(exe)],
                           check=True)
            subprocess.run([str(exe)], check=True)
