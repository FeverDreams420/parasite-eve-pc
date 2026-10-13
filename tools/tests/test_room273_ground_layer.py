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
FUNCTION = 'RoomM273_SwayGroundCallback'


class GroundLayerSprite(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Vector)==8 && __alignof__(Vector)==2 &&
    (unsigned long)&((Vector *)0)->y==2 &&
    (unsigned long)&((Vector *)0)->pad==6 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
int g_FieldFxCurrentAge,g_FieldFxAltPaletteFlag;
short D_800966EE[8192];
unsigned short g_FieldFxPaletteTable[8];
RoomFloorY g_RoomFloorY[1];
RenderEffectParameters g_FieldFxSpriteParams;
GteRotation g_RoomM273FloorRotation;
unsigned char D_8019AE18[8],g_RoomM273BlackColor[8];
static Vector input={12,-34,56,78};
static Vector *local;
static int stage,expectedPalette,expectedSize,expectedShade,sizeIndex,shadeIndex;
u_short GetClut(int x,int y) {
    assert(stage++==0 && x==0 && y==expectedPalette);
    g_FieldFxCurrentAge=999;
    D_800966EE[sizeIndex]=111; D_800966EE[shadeIndex]=222;
    return 0xF123;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p, struct GteRotation *rotation,
                  int w, int h, int tile, int clut, int flag, int shade,
                  struct RenderColor *color) {
    assert(stage++==1 && p!=&input && !rotation);
    assert(p->x==12 && p->y==-34 && p->z==56 && p->pad==78);
    assert(w==expectedSize && h==expectedSize && tile==100 && clut==0xF123);
    assert(flag==1 && shade==expectedShade && !color);
    local=p; p->x=-111; p->y=222; p->z=-333; p->pad=444;
    g_RoomFloorY->raw=0xFEDC;
}
void FieldFx_DrawStarFan(struct GteShortVector *p,int w,int h,int mode,
                  struct GteRotation *rotation,int sx,int sy,
                  struct RenderColor *a,struct RenderColor *b,int shade,int flag) {
    assert(stage++==2 && p==local && w==384 && h==384 && mode==8);
    assert(p->x==-111 && p->y==(short)0xFEDC && p->z==-333 && p->pad==444);
    assert(rotation==&g_RoomM273FloorRotation && sx==4096 && sy==4096);
    assert((void *)a==D_8019AE18 && (void *)b==g_RoomM273BlackColor && shade==expectedShade && flag==1);
}
int main(void) {
    int times[]={INT_MIN,-1,0,31,32,INT_MAX};
    int frames[]={-2,-1,0,1,31,32,127,128,4096};
    short samples[]={-32768,-4097,-4096,-1,0,32767};
    int i,j,k,kind,variant;
    unsigned int phase;
    for(i=0;i<6;++i) {
        g_FieldFxCurrentAge=times[i];
        assert(RoomM273_SwayGroundCallback(1,&input)==(times[i]>=32));
    }
    assert(stage==0);
    for(kind=0;kind<8;++kind) for(variant=0;variant<2;++variant)
        for(i=0;i<9;++i) for(j=0;j<6;++j) for(k=0;k<6;++k) {
            g_FieldFxCurrentAge=frames[i]+1; phase=(unsigned int)frames[i]<<5;
            sizeIndex=(phase&4095)*2; shadeIndex=((phase+1024)&4095)*2;
            D_800966EE[sizeIndex]=samples[j]; D_800966EE[shadeIndex]=samples[k];
            expectedSize=(short)(samples[j]*2+4096);
            expectedShade=(short)((unsigned int)(samples[k]+4096)>>5);
            g_FieldFxSpriteParams.palette=kind; g_FieldFxAltPaletteFlag=variant; g_FieldFxPaletteTable[kind]=65535;
            expectedPalette=65535+(kind==4 && variant ?8:4);
            stage=0; g_RoomFloorY->raw=123;
            assert(RoomM273_SwayGroundCallback(2,&input)==0 && stage==3);
            assert(input.x==12 && input.y==-34 && input.z==56 && input.pad==78);
        }
    assert(RoomM273_SwayGroundCallback(0,&input)==0 && RoomM273_SwayGroundCallback(-1,&input)==0);
    assert(stage==3);
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
