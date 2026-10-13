import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import find_definition, host_unit, psx_compile  # noqa: E402

# Overlay function addresses repeat across rooms; search this room only.
ROOM = 'src/overlays/room_m350_cruiser'

CALLBACK = 'RoomM350_OrbHandSparkCallback'


class SizedLayersTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(CALLBACK, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[OFF(Particle,x)==4 ? 1:-1];
typedef char b[OFF(Particle,size)==10 ? 1:-1];
typedef char c[sizeof(GteSineWord)==4 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_signed_math_and_helper_reloads(self):
        source = host_unit(CALLBACK, path=find_definition(CALLBACK, under=ROOM))
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n#include <limits.h>\n' + source + r'''
int g_FieldFxCurrentAge,g_FieldFxAltPaletteFlag;
GteSineWord rcossin_tbl[4096];
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxPaletteTable[65536];
int D_8019A3CC[1];
RenderColor D_8019A3C8;
static Particle particle,afterClut;
static int calls,wantSize,wantPalette,nextTimer,lastTimer,nextScale;
static unsigned short clut;
static int half(uint32_t x) { x&=65535; return x<32768 ? (int)x:(int)x-65536; }
static int asr5(int x) { return x>=0 ? x/32 : -((-x+31)/32); }
static unsigned short trigHalf(unsigned timer,int high) {
    uint32_t word=(uint32_t)rcossin_tbl[((timer-1)<<7)&0xF80].word;
    return high ? word>>16 : word;
}
u_short GetClut(int x,int y) {
    assert(calls++==0 && x==48 && y==wantPalette);
    g_FieldFxCurrentAge=nextTimer; g_FieldFxSpriteParams.parameter02=nextScale;
    particle.size=~particle.size; afterClut=particle;
    return clut;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p,struct GteRotation *rotation,int sx,int sy,int texture,int palette,int one,int shade,struct RenderColor *owner) {
    assert(calls++==1 && (void *)p==&particle.x && (void *)owner==&particle);
    assert(rotation==0 && sx==wantSize && sy==wantSize && texture==nextScale*5+216);
    assert(palette==clut && one==1 && shade==asr5(half(trigHalf(nextTimer,1))));
    g_FieldFxCurrentAge=lastTimer;
}
void FieldFx_DrawStarFan(struct GteShortVector *p,int w,int h,int mode,struct GteRotation *vector,int sx,int sy,struct RenderColor *a,struct RenderColor *b,int shade,int one) {
    unsigned index=(((unsigned)lastTimer-1)<<8)&0xF00;
    assert(calls++==2 && (void *)p==&particle.x && w==96 && h==96 && mode==8 && !vector);
    assert(sx==wantSize && sy==wantSize && (void *)a==D_8019A3CC && b==&D_8019A3C8 && one==1);
    assert(shade==asr5(half(rcossin_tbl[index].word)));
}
int main(void) {
    int i,j,event,trigger,timers[]={INT_MIN,-32769,-1,0,1,7,8,9,INT_MAX};
    int sizes[]={-32768,-4097,-1,0,1,4095,32767};
    for(i=0;i<65536;i++) g_FieldFxPaletteTable[i]=65535-i;
    for(i=0;i<4096;i++) {
        uint32_t word=((uint32_t)(i*1877+5)<<16)|(uint16_t)(i*3119+7);
        memcpy(&rcossin_tbl[i],&word,4);
    }
    for(i=0;i<65536;i++) for(j=0;j<7;j++) {
        unsigned index;
        uint32_t word;
        calls=0; g_FieldFxCurrentAge=i-32768; particle.size=sizes[j];
        g_FieldFxSpriteParams.palette=i; g_FieldFxAltPaletteFlag=j&1;
        wantPalette=g_FieldFxPaletteTable[i]+(i==4 && g_FieldFxAltPaletteFlag ? 4:0);
        index=((((unsigned)g_FieldFxCurrentAge-1)<<7)&0xF80);
        word=((uint32_t)(uint16_t)(i*19)<<16)|(uint16_t)i;
        memcpy(&rcossin_tbl[index],&word,4);
        wantSize=(int)((int64_t)half(i)*sizes[j]/4096);
        nextTimer=g_FieldFxCurrentAge+3; lastTimer=g_FieldFxCurrentAge+11;
        nextScale=half(i); clut=i;
        assert(RoomM350_OrbHandSparkCallback(2,&particle)==0 && calls==3);
        assert(memcmp(&particle,&afterClut,sizeof(particle))==0);
    }
    for(trigger=0;trigger<9;trigger++) for(event=-3;event<=5;event++) if(event!=2) {
        calls=0; g_FieldFxCurrentAge=timers[trigger]; afterClut=particle;
        assert(RoomM350_OrbHandSparkCallback(event,&particle)==(event==1 && g_FieldFxCurrentAge>=8));
        assert(calls==0 && memcmp(&particle,&afterClut,sizeof(particle))==0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory)/'layers'
            result = subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing', '-x', 'c', '-', '-o', str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
