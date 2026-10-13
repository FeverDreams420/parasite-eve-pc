import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import find_definition, host_unit, psx_compile  # noqa: E402

FUNCTION = 'RoomM273_SweepDropHitCallback'


class SizedSprite(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Vector)==8 &&
    (unsigned long)&((Vector *)0)->z==4 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION) + r'''
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag;
GteSineWord rcossin_tbl[4096];
RenderEffectParameters g_FieldFxSpriteParams;
unsigned short g_FieldFxPaletteTable[8];
static Vector position={12,34,56,78};
static int cluts,draws,expectedPalette,expectedSize,shadeIndex;
u_short GetClut(int x, int y) {
    assert(cluts++==0 && !draws && x==0 && y==expectedPalette);
    g_FieldFxCurrentAge=999;
    rcossin_tbl[shadeIndex].word=0x1234FFE0;
    return 0xF123;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p, struct GteRotation *rotation,
                  int w, int h, int tile, int clut, int flag, int shade,
                  struct RenderColor *color) {
    assert(cluts==1 && draws++==0 && p==&position && rotation==0);
    assert(w==expectedSize && h==expectedSize && tile==102);
    assert(clut==0xF123 && flag==1 && shade==-1 && color==0);
}
int main(void) {
    int times[]={INT_MIN,-1,0,7,8,INT_MAX};
    int words[]={0,0x12347FFF,-2049,0x1234FFFF};
    int i,j,kind,variant,frame,sizeIndex;
    for(i=0;i<6;++i) {
        g_FieldFxCurrentAge=times[i];
        assert(RoomM273_SweepDropHitCallback(1,&position)==(times[i]>=8));
    }
    assert(!cluts && !draws);
    for(kind=0;kind<8;++kind) for(variant=0;variant<2;++variant)
        for(i=0;i<=8;++i) for(j=0;j<4;++j) {
            g_FieldFxCurrentAge=i; frame=i-1;
            sizeIndex=(((unsigned int)frame<<9)&0x3E00)/4;
            shadeIndex=(((unsigned int)frame<<10)&0x3C00)/4;
            rcossin_tbl[sizeIndex].word=words[j];
            expectedSize=(short)(words[j]+2048);
            g_FieldFxSpriteParams.palette=kind; g_FieldFxAltPaletteFlag=variant;
            g_FieldFxPaletteTable[kind]=65535;
            expectedPalette=65535+(kind==4 && variant ? 9:5);
            cluts=draws=0;
            assert(RoomM273_SweepDropHitCallback(2,&position)==0 && cluts==1 && draws==1);
        }
    assert(RoomM273_SweepDropHitCallback(0,&position)==0);
    assert(RoomM273_SweepDropHitCallback(-1,&position)==0);
    assert(RoomM273_SweepDropHitCallback(INT_MAX,&position)==0);
    assert(cluts==1 && draws==1);
    assert(position.x==12 && position.y==34 && position.z==56 && position.pad==78);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=c99', '-O2', str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
