import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
ROOM = 'src/overlays/room_m350_cruiser'
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import find_definition, host_unit, psx_compile  # noqa: E402
FUNCTION = 'RoomM350_RushCloudCallback'


class AttachedCloudCallbackTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(GteShortVector)==8 ? 1:-1];
typedef char b[sizeof(RoomM350CloudParticle)==20 ? 1:-1];
typedef char c[OFF(RoomM350CloudParticle,position)==4 ? 1:-1];
typedef char d[OFF(RoomM350CloudParticle,size)==10 ? 1:-1];
typedef char e[OFF(RoomM350CloudParticle,state.value.active.animatedSize)==12 ? 1:-1];
typedef char f[OFF(RoomM350CloudParticle,state.value.active.firstShade)==14 ? 1:-1];
typedef char g[OFF(RoomM350CloudParticle,state.value.active.secondShade)==16 ? 1:-1];
typedef char h[OFF(RoomM350CloudParticle,state.value.active.reserved12)==18 ? 1:-1];
typedef char i[sizeof(GteSineWord)==4 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_update_and_draw_boundaries(self):
        source = host_unit(FUNCTION, under=ROOM)
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n#include <limits.h>\n' + source + r'''
typedef RoomM350CloudParticle Particle;
int g_FieldFxCurrentAge,g_FieldFxAltPaletteFlag;
/* Sine words read as halfwords: sin at even, cos at odd indices. */
GteSineWord rcossin_tbl[4096];
#define TABLE ((short *)rcossin_tbl)
RenderEffectParameters g_FieldFxSpriteParams;
#define D_800F336A (*(short *)&g_FieldFxSpriteParams.parameter02)
#define D_800F336C g_FieldFxSpriteParams.palette
unsigned short g_FieldFxPaletteTable[65536];
RenderColor D_8019A3D0;
static Particle particle;
static int anchor[3],draws,cluts,first,mutate;
static short expectedPosition[3];
static int word(uint32_t x) { int result; memcpy(&result,&x,4); return result; }
static int half(uint32_t x) { x &= 65535; return x<32768 ? (int)x:(int)x-65536; }
static int asr5(int x) { return x>=0 ? x/32:-((-x+31)/32); }
u_short GetClut(int x,int y) {
    int expected=g_FieldFxPaletteTable[D_800F336C];
    if(D_800F336C==4 && g_FieldFxAltPaletteFlag) expected+=4;
    assert(x==32 && y==expected && draws==cluts);
    cluts++;
    if(mutate) {
        particle.size=-123; particle.state.value.active.animatedSize=234;
        particle.state.value.active.firstShade=-321; particle.state.value.active.secondShade=-432;
        D_800F336A=-765;
    }
    return (unsigned short)(0xFFFF-cluts);
}
void FieldFx_DrawShapeQuads(struct GteShortVector *v,struct GteRotation *r,int width,int height,
                   int texture,int clut,int one,int shade,struct RenderColor *color) {
    int firstDraw=first && draws==0;
    assert(cluts==draws+1 && r==0 && one==1 && color==&D_8019A3D0);
    assert(v->x==expectedPosition[0] && v->y==expectedPosition[1] && v->z==expectedPosition[2]);
    assert(width==(firstDraw ? particle.size:particle.state.value.active.animatedSize) && height==width);
    assert(shade==(firstDraw ? particle.state.value.active.firstShade:particle.state.value.active.secondShade));
    assert(texture==D_800F336A*2+216 && clut==(unsigned short)(0xFFFF-cluts));
    draws++;
    if(mutate) {
        D_800F336C=4; g_FieldFxAltPaletteFlag=-1; g_FieldFxPaletteTable[4]=65535;
        particle.position[0]=123; anchor[1]=INT_MAX;
    }
}
static void update(int timer,int size) {
    Particle before;
    int shade,product;
    memset(&particle,0x5A,sizeof(particle)); particle.size=size;
    memcpy(&before,&particle,sizeof(particle));
    g_FieldFxCurrentAge=timer;
    assert(RoomM350_RushCloudCallback(1,&particle)==(timer>=8 ? 1:0));
    if(timer<8) {
        shade=TABLE[1+((((uint32_t)timer*3)<<8)&0x3F00)/2];
        product=TABLE[(((uint32_t)timer<<9)&0x3E00)/2]*size;
        before.state.value.active.firstShade=asr5(shade);
        before.state.value.active.secondShade=half(128-((uint32_t)timer<<4));
        before.state.value.active.animatedSize=half(word((uint32_t)product*3)/4096);
    }
    assert(memcmp(&before,&particle,sizeof(particle))==0);
}
int main(void) {
    int n,j,k;
    static const int sizes[]={-32768,-32767,-4096,-1,0,1,4096,32767};
    static const int timers[]={INT_MIN,INT_MIN+1,-65537,-65536,-32769,-1,0,7,8,32768,65535,INT_MAX};
    for(n=0;n<8192;n++) TABLE[n]=half((uint32_t)n*12345+0x8000);
    for(n=0;n<65536;n++) for(j=0;j<8;j++) update(half(n),sizes[j]);
    for(n=0;n<65536;n++) for(j=0;j<sizeof(timers)/sizeof(timers[0]);j++) update(timers[j],half(n));
    for(n=0;n<65536;n++) for(mutate=0;mutate<2;mutate++) {
        memset(&particle,0x5A,sizeof(particle));
        particle.anchor=anchor; particle.state.value.active.firstShade=half(n);
        particle.size=half((uint32_t)n*13); particle.state.value.active.animatedSize=half((uint32_t)n*17);
        particle.state.value.active.secondShade=half((uint32_t)n*29);
        for(k=0;k<3;k++) {
            anchor[k]=word((uint32_t)n*0x1234567u+(uint32_t)k*0x7FFFFFFFu);
            particle.position[k]=half((uint32_t)n*31+k);
            expectedPosition[k]=half((uint32_t)anchor[k]+(uint16_t)particle.position[k]);
        }
        D_800F336C=n; g_FieldFxPaletteTable[n]=n^0xA357; g_FieldFxAltPaletteFlag=n&1;
        D_800F336A=half(n); first=particle.state.value.active.firstShade>0; draws=cluts=0;
        assert(RoomM350_RushCloudCallback(2,&particle)==0 && draws==1+first && cluts==draws);
    }
    assert(RoomM350_RushCloudCallback(0,0)==0 && RoomM350_RushCloudCallback(-1,0)==0 && RoomM350_RushCloudCallback(3,0)==0);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory)/'test.c'
            path.write_text(harness)
            binary = path.with_suffix('')
            result = subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing', str(path), '-o', str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            subprocess.run([str(binary)], check=True)
