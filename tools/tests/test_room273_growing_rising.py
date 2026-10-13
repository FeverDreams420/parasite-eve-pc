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
FUNCTION = 'RoomM273_SweepRisingCallback'


class GrowingRisingSprite(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Particle)==8 &&
    (unsigned long)&((Particle *)0)->y==2 &&
    (unsigned long)&((Particle *)0)->z==4 &&
    (unsigned long)&((Particle *)0)->speed==6 ? 1:-1];
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
unsigned char D_8019AD70[4];
static Particle particle;
static int draws,cluts,wantPalette,wantSize,wantShade,wantTile;
u_short GetClut(int x, int y) {
    assert(cluts++==0 && !draws && x==16 && y==wantPalette);
    g_FieldFxCurrentAge=999;
    g_FieldFxSpriteParams.parameter02=-7;
    __builtin_memset(rcossin_tbl,0,sizeof(rcossin_tbl));
    return 0xF123;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p, struct GteRotation *rotation,
                  int w, int h, int tile, int clut, int flag, int shade,
                  struct RenderColor *color) {
    assert(cluts==1 && draws++==0 && p==&particle && rotation==0);
    assert(w==wantSize && h==wantSize && tile==wantTile);
    assert(clut==0xF123 && flag==1 && shade==wantShade && color==D_8019AD70);
}
int main(void) {
    int times[]={INT_MIN,-1,0,1,31,32,INT_MAX};
    unsigned short values[]={0,1,32767,32768,65535};
    int frames[]={-33,-4,-1,0,1,3,4,31,32,63,64};
    int i,j,k,f,kind,variant;
    for(i=0;i<7;++i) for(j=0;j<5;++j) for(k=0;k<5;++k) {
        g_FieldFxCurrentAge=times[i]; particle.x=12; particle.z=-34;
        particle.y=values[j]; particle.speed=values[k];
        assert(RoomM273_SweepRisingCallback(1,&particle)==(times[i]>=32));
        assert(particle.x==12 && particle.z==-34);
        assert(particle.y==(unsigned short)(times[i]<32 ? values[j]-values[k]:values[j]));
        assert(particle.speed==(unsigned short)(times[i]<32 ? values[k]+1:values[k]));
    }
    assert(!draws && !cluts);
    for(kind=0;kind<8;++kind) for(variant=0;variant<2;++variant)
        for(f=0;f<11;++f) for(j=0;j<5;++j) for(k=0;k<5;++k) {
            unsigned int shadeOffset=((unsigned int)frames[f]<<8)&0x3F00;
            unsigned int sizeOffset=((unsigned int)frames[f]<<7)&0x3F80;
            short sizeSample=(short)values[k];
            g_FieldFxCurrentAge=frames[f]+1;
            rcossin_tbl[shadeOffset/4].word=0x12340000|values[j];
            __builtin_memcpy((char *)rcossin_tbl+sizeOffset,&sizeSample,2);
            /* The two sample locations coincide for some wrapped frames. */
            wantShade=(short)rcossin_tbl[shadeOffset/4].word>>5;
            wantSize=sizeSample+8192;
            wantTile=-7*(frames[f]/4)+200;
            g_FieldFxSpriteParams.parameter02=123; g_FieldFxSpriteParams.palette=kind; g_FieldFxAltPaletteFlag=variant;
            g_FieldFxPaletteTable[kind]=65535;
            wantPalette=65535+(kind==4 && variant ? 4:0);
            particle.x=12; particle.y=65530; particle.z=-34; particle.speed=8;
            cluts=draws=0;
            assert(RoomM273_SweepRisingCallback(2,&particle)==0 && cluts==1 && draws==1);
            assert(particle.x==12 && particle.y==65530 && particle.z==-34 && particle.speed==8);
        }
    assert(RoomM273_SweepRisingCallback(0,&particle)==0);
    assert(RoomM273_SweepRisingCallback(-1,&particle)==0);
    assert(RoomM273_SweepRisingCallback(INT_MAX,&particle)==0);
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
