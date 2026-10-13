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

CALLBACK = 'RoomM350_RushFlareCallback'
# Keep pe1/gte.h's asm out of the host build; COP2 boundary stubs check call
# order and CPU logic, not GTE arithmetic.
HOST_GTE = r'''
#include "pe1/gte_types.h"
#define PE1_GTE_H
static void load_rotation(void *),load_translation(void *),load_vector(void *),command(void);
static void store_ir(void *);
#define gte_ldrotmatrix(p) load_rotation(p)
#define gte_ldtransmatrix(p) load_translation(p)
#define gte_ldv0(p) load_vector(p)
#define gte_rtv0tr_mac() command()
#define gte_stsv(p) store_ir(p)
'''


class TransformedFlareTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(CALLBACK, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(GteShortVector)==8 ? 1:-1];
typedef char b[sizeof(GteMatrix)==32 ? 1:-1];
typedef char c[OFF(GteMatrix,t)==20 ? 1:-1];
typedef char d[OFF(RoomM350FlareParticle,shade)==4 ? 1:-1];
typedef char e[OFF(FieldActor,rot_y)==58 ? 1:-1];
typedef char f[OFF(RoomM350EffectActor,instance)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_cpu_logic_at_gte_boundary(self):
        source = host_unit(CALLBACK, path=find_definition(CALLBACK, under=ROOM), prefix=HOST_GTE)
        prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
'''
        harness = prefix + source + r'''
typedef GteShortVector Vector;
typedef RoomM350FlareParticle Particle;
FieldFxObjectPrefix *g_FieldFxOwner;
GteMatrix g_RoomM350RushMatrix;
short g_RoomM350RushCenter[3];
int g_FieldFxCurrentAge,g_FieldFxAltPaletteFlag,D_8019A690[4];
GteSineWord rcossin_tbl[4096];
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxPaletteTable[65536];
static RoomM350EffectActor actor,otherActor;
static FieldActor instance,other;
static Particle particle,snapshot;
static Vector *vector;
static int step,ir[3],timerBefore,nextTimer,nextScale,nextShade,wantPalette;
static unsigned short clut;
static int half(uint32_t x) { x&=65535; return x<32768 ? (int)x:(int)x-65536; }
static int asr5(int x) { return x>=0 ? x/32 : -((-x+31)/32); }
static void load_rotation(void *p) { assert(step++==0 && p==&g_RoomM350RushMatrix); }
static void load_translation(void *p) { assert(step++==1 && p==&g_RoomM350RushMatrix); }
static void load_vector(void *p) {
    vector=p; assert(step++==2 && vector->x==0 && vector->y==particle.y && vector->z==particle.z);
    vector->pad=0x2468;
}
static void command(void) { assert(step++==3); g_FieldFxOwner=(FieldFxObjectPrefix *)&otherActor; }
static void store_ir(void *p) {
    Vector *v=p;
    assert(step==4 && v==vector); step+=3;
    v->x=ir[0]; v->y=ir[1]; v->z=ir[2];
}
u_short GetClut(int x,int y) {
    assert(step++==7 && x==0 && y==wantPalette);
    g_FieldFxCurrentAge=nextTimer; g_FieldFxSpriteParams.parameter02=nextScale; particle.shade=nextShade;
    snapshot=particle; return clut;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p,struct GteRotation *rotation,int size,int scale,int texture,int palette,int one,int shade,struct RenderColor *colors) {
    Vector *r=(Vector *)rotation;
    assert(step++==8 && p==vector && size==0x2200 && scale==4096);
    assert(p->x==half((uint32_t)ir[0]+g_RoomM350RushCenter[0]));
    assert(p->y==half((uint32_t)ir[1]+g_RoomM350RushCenter[1]));
    assert(p->z==half((uint32_t)ir[2]+g_RoomM350RushCenter[2]) && p->pad==0x2468);
    assert(r->x==half((uint32_t)timerBefore*384) && r->y==other.rot_y && r->z==0 && r->pad==1);
    assert(texture==nextScale*(nextTimer&7)+64 && palette==clut && one==1 && shade==nextShade);
    assert((void *)colors==&D_8019A690[nextTimer&3]);
}
int main(void) {
    int i,phase,event,result,edge,timers[]={INT_MIN,-32769,-1,0,7,8,INT_MAX};
    actor.instance=(RoomM350EffectInstance *)&instance;
    otherActor.instance=(RoomM350EffectInstance *)&other;
    for(i=0;i<65536;i++) g_FieldFxPaletteTable[i]=65535-i;
    for(i=0;i<65536;i++) {
        unsigned index; uint32_t word=0x5A5A0000u|i;
        g_FieldFxCurrentAge=i-32768; index=((unsigned)g_FieldFxCurrentAge<<8)&0xF00;
        memcpy(&rcossin_tbl[index].word,&word,4);
        particle.y=123; particle.z=-456; particle.shade=777; snapshot=particle; step=0;
        result=g_FieldFxCurrentAge>=8;
        assert(RoomM350_RushFlareCallback(1,&particle)==result && step==0);
        if(!result) snapshot.shade=asr5(half(i));
        assert(memcmp(&snapshot,&particle,sizeof(particle))==0);
        g_FieldFxCurrentAge=-1; memcpy(&rcossin_tbl[0xF00].word,&word,4);
        assert(RoomM350_RushFlareCallback(1,&particle)==0 && particle.shade==asr5(half(i)));
    }
    for(i=0;i<65536;i++) for(phase=0;phase<8;phase++) {
        step=0; g_FieldFxOwner=(FieldFxObjectPrefix *)&actor; g_FieldFxCurrentAge=timerBefore=i-32768;
        particle.y=half(i); particle.z=half(65535-i); particle.shade=321;
        ir[0]=i; ir[1]=-i-1; ir[2]=i*30001;
        g_RoomM350RushCenter[0]=half(i*7); g_RoomM350RushCenter[1]=half(i*11); g_RoomM350RushCenter[2]=half(i*19);
        other.rot_y=half(i*23); instance.rot_y=~other.rot_y;
        g_FieldFxSpriteParams.palette=i; g_FieldFxAltPaletteFlag=phase&1; g_FieldFxSpriteParams.parameter02=123;
        wantPalette=g_FieldFxPaletteTable[i]+(i==4 && g_FieldFxAltPaletteFlag ? 4:0);
        nextTimer=(i-32768)*8+phase; nextScale=half(i); nextShade=half(i*29); clut=i;
        assert(RoomM350_RushFlareCallback(2,&particle)==0 && step==9);
        assert(memcmp(&snapshot,&particle,sizeof(particle))==0);
    }
    for(edge=0;edge<7;edge++) for(event=-3;event<=5;event++) if(event!=2) {
        step=0; g_FieldFxCurrentAge=timers[edge]; snapshot=particle;
        assert(RoomM350_RushFlareCallback(event,&particle)==(event==1 && g_FieldFxCurrentAge>=8) && step==0);
        if(event!=1 || g_FieldFxCurrentAge>=8) assert(memcmp(&snapshot,&particle,sizeof(particle))==0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory)/'flare'
            result = subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing', '-x', 'c', '-', '-o', str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
