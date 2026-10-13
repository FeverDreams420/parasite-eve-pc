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
FUNCTION = 'RoomM273_HandTrailHitCallback'


class ColoredRisingSprite(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Particle)==8 && sizeof(Color)==4 &&
    (unsigned long)&((Particle *)0)->y==2 &&
    (unsigned long)&((Particle *)0)->z==4 &&
    (unsigned long)&((Particle *)0)->speed==6 &&
    (unsigned long)&((Color *)0)->unknown==3 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n#include <string.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
Color g_RoomM273HandTrailHitColor;
int g_FieldFxCurrentAge, g_FieldFxAltPaletteFlag;
GteSineWord rcossin_tbl[4096];
unsigned short g_FieldFxPaletteTable[8];
RenderEffectParameters g_FieldFxSpriteParams;
static Particle particle;
static Color wantColor;
static int draws,cluts,wantPalette,wantSize,wantShade;
static unsigned int shadeOffset;
u_short GetClut(int x, int y) {
    assert(cluts++==0 && !draws && x==0 && y==wantPalette);
    g_FieldFxCurrentAge=999;
    memset(rcossin_tbl,0,sizeof(rcossin_tbl));
    rcossin_tbl[shadeOffset/4].word=0x12348001;
    memset(&g_RoomM273HandTrailHitColor,0,sizeof(g_RoomM273HandTrailHitColor));
    return 0xF123;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *point, struct GteRotation *rotation,
                  int w, int h, int tile, int clut, int flag, int shade,
                  struct RenderColor *colorArg) {
    Color *color=(Color *)colorArg;
    assert(cluts==1 && draws++==0 && (void *)point==&particle && rotation==0);
    assert(w==wantSize && h==wantSize && tile==102);
    assert(clut==0xF123 && flag==1 && shade==wantShade);
    assert(color!=&g_RoomM273HandTrailHitColor && memcmp(color,&wantColor,4)==0);
    color->unknown=0;
}
int main(void) {
    int times[]={INT_MIN,-1,0,1,15,16,INT_MAX};
    unsigned short values[]={0,1,32767,32768,65535};
    int frames[]={-33,-4,-1,0,1,3,4,15,16,31,32,63,64};
    int i,j,k,f,kind,variant;
    for(i=0;i<7;++i) for(j=0;j<5;++j) for(k=0;k<5;++k) {
        g_FieldFxCurrentAge=times[i]; particle.x=12; particle.z=-34;
        particle.y=values[j]; particle.speed=values[k];
        assert(RoomM273_HandTrailHitCallback(1,&particle)==(times[i]>=16));
        assert(particle.x==12 && particle.z==-34);
        assert(particle.y==(unsigned short)(times[i]<16 ? values[j]-values[k]:values[j]));
        assert(particle.speed==(unsigned short)(times[i]<16 ? values[k]+1:values[k]));
    }
    assert(!draws && !cluts);
    for(kind=0;kind<8;++kind) for(variant=0;variant<2;++variant)
        for(f=0;f<13;++f) for(j=0;j<5;++j) {
            unsigned int sizeOffset=((unsigned int)frames[f]<<8)&0x3F00;
            short sizeSample=(short)values[j];
            shadeOffset=((unsigned int)frames[f]<<9)&0x3E00;
            memset(rcossin_tbl,0,sizeof(rcossin_tbl));
            memcpy((char *)rcossin_tbl+sizeOffset,&sizeSample,2);
            g_FieldFxCurrentAge=frames[f]+1;
            wantSize=sizeSample*2;
            wantShade=(short)0x8001>>6;
            g_FieldFxSpriteParams.palette=kind; g_FieldFxAltPaletteFlag=variant;
            g_FieldFxPaletteTable[kind]=65535;
            wantPalette=65535+(kind==4 && variant ? 9:5);
            g_RoomM273HandTrailHitColor.r=1; g_RoomM273HandTrailHitColor.g=128;
            g_RoomM273HandTrailHitColor.b=255; g_RoomM273HandTrailHitColor.unknown=37;
            wantColor=g_RoomM273HandTrailHitColor;
            particle.x=12; particle.y=65530; particle.z=-34; particle.speed=8;
            cluts=draws=0;
            assert(RoomM273_HandTrailHitCallback(2,&particle)==0 && cluts==1 && draws==1);
            assert(particle.x==12 && particle.y==65530 && particle.z==-34 && particle.speed==8);
        }
    assert(RoomM273_HandTrailHitCallback(0,&particle)==0);
    assert(RoomM273_HandTrailHitCallback(-1,&particle)==0);
    assert(RoomM273_HandTrailHitCallback(INT_MAX,&particle)==0);
    assert(draws==1 && cluts==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing',
                            str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
