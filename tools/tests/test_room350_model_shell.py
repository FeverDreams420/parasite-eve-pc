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

SPAWNER = 'RoomM350_OrbHandSparkEmitter'


class ModelShellTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(SPAWNER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Vector)==8 ? 1:-1];
typedef char b[sizeof(Matrix)==32 ? 1:-1];
typedef char c[OFF(Matrix,position)==20 ? 1:-1];
typedef char d[sizeof(Particle)==12 ? 1:-1];
typedef char e[OFF(Particle,x)==4 ? 1:-1];
typedef char f[OFF(Particle,size)==10 ? 1:-1];
typedef char g[OFF(FieldActor,render_object.matrices)==0x238 ? 1:-1];
typedef char h[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_rng_transform_boundaries_and_reload(self):
        source = host_unit(SPAWNER, path=find_definition(SPAWNER, under=ROOM))
        # Helper stubs check CPU behavior, not rotation/GTE arithmetic.
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n#include <limits.h>\n' + source + r'''
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
FieldActor *g_RoomM350OrbActor;
int g_FieldFxCurrentAge;
GteSineWord rcossin_tbl[4096];
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlot2;
u16 g_FieldFxTpageTable[65536];
static RoomM350Emitter emitters[2];
static FieldActor instances[2];
static RenderMatrix transforms[2][16];
static const GteMatrix *rotationMatrix;
static Particle particles[2],expected[2];
static int pools[2],allocs,randoms,rotations,applications,setups,setupResult,failAt;
static uint32_t seed;
static int signed_word(uint32_t word) { int result; memcpy(&result,&word,4); return result; }
static int half(uint32_t x) { x&=65535; return x<32768 ? (int)x:(int)x-65536; }
static int asr1(uint32_t x) { return (int)((int64_t)signed_word(x)/2-(signed_word(x)<0 && (x&1))); }
static uint32_t rng(int call) { return seed+(uint32_t)call*0x1234567u; }
int RoomM350_OrbHandSparkCallback(int e,Particle *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[0] && stride==12 && count==64);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_OrbHandSparkCallback);
    setups++; return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int i=allocs++;
    assert(i<2 && p==(char *)&pools[i] && randoms==i*7 && applications==i);
    if(i==failAt) return 0;
    return &particles[i];
}
int Inv_ScrambleGrid(void) {
    int call=randoms++,i=call/7,part=call%7,j;
    assert(allocs==i+1);
    if(part>=4) {
        assert(applications==i+1 && rotations==i+1);
        for(j=0;j<3;j++) assert((&particles[i].x)[j]==(&expected[i].x)[j]);
        assert(particles[i].size==expected[i].size);
        g_FieldFxCurrentAge=INT_MAX;
        if(part==6) expected[i].color=signed_word((uint32_t)asr1(rng(i*7+4)) |
            ((uint32_t)asr1(rng(i*7+5))<<8) | ((uint32_t)asr1(rng(i*7+6))<<16));
    }
    return signed_word(rng(call));
}
GteMatrix *RotMatrixYXZ(GteShortVector *v,GteMatrix *m) {
    int i=rotations++;
    assert(randoms==i*7+2 && applications==i);
    assert(v->x==half(rng(i*7)<<4) && v->y==half(rng(i*7+1)<<4) && v->z==0);
    rotationMatrix=m; memset(m,0xB7,sizeof(*m));
    return m;
}
GteShortVector *ApplyMatrixSV(const GteMatrix *m,const GteShortVector *v,GteShortVector *out) {
    int i=applications++,j,timer,index;
    int combined=signed_word((rng(i*7+2)<<8)|rng(i*7+3));
    uint32_t tableWord=0x7FFFFFF0u+(seed&65535);
    assert(randoms==i*7+4 && rotations==i+1 && m==rotationMatrix);
    assert(v->x==0 && v->y==0 && v->z==combined%192+128);
    out->x=half(seed); out->y=half(seed*7); out->z=half(seed*31); out->pad=123;
    g_RoomM350OrbActor=&instances[1]; g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1];
    g_FieldFxCurrentAge=signed_word(seed+(uint32_t)i*0x13579u);
    timer=signed_word((uint32_t)g_FieldFxCurrentAge<<11); index=(timer/40)&4095;
    rcossin_tbl[index].word=signed_word(tableWord); expected[i].size=half(tableWord+4096);
    for(j=0;j<3;j++) (&expected[i].x)[j]=half((uint32_t)((short *)out)[j]+
        (uint32_t)transforms[1][i ? 11:15].translation[j]);
    return out;
}
static void reset(void) {
    allocs=randoms=rotations=applications=setups=0;
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0]; g_RoomM350OrbActor=&instances[0]; g_FieldFxCurrentAge=40;
    memset(particles,0x5A,sizeof(particles)); memcpy(expected,particles,sizeof(particles));
}
int main(void) {
    int i,j,k,f,event,edge,timers[]={INT_MIN,-1,0,40,41,INT_MAX};
    emitters[0].pool=&pools[0]; emitters[1].pool=&pools[1];
    instances[0].render_object.matrices=transforms[0];
    instances[1].render_object.matrices=transforms[1];
    for(i=0;i<2;i++) for(j=0;j<16;j++) for(k=0;k<3;k++)
        transforms[i][j].translation[k]=signed_word(0x7FFF8000u+i*0x3456u+j*123u+k*17u);
    for(i=0;i<65536;i++) for(f=0;f<3;f++) {
        reset(); failAt=f; seed=0xFEDC0000u|i;
        assert(RoomM350_OrbHandSparkEmitter(1)==0 && setups==0);
        assert(allocs==(f<2 ? f+1:2) && randoms==f*7 && rotations==f && applications==f);
        assert(memcmp(particles,expected,sizeof(particles))==0);
    }
    for(i=0;i<65536;i++) {
        reset(); failAt=0; g_FieldFxCurrentAge=i-32768;
        assert(RoomM350_OrbHandSparkEmitter(1)==(g_FieldFxCurrentAge>=41 ? 2:0));
        assert(allocs==(g_FieldFxCurrentAge<41) && !randoms);
    }
    for(edge=0;edge<6;edge++) {
        reset(); failAt=2; g_FieldFxCurrentAge=timers[edge]; seed=(uint32_t)timers[edge];
        assert(RoomM350_OrbHandSparkEmitter(1)==(timers[edge]>=41 ? 2:0));
        assert(allocs==(timers[edge]>=41 ? 0:2));
        assert(memcmp(particles,expected,sizeof(particles))==0);
    }
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(); assert(RoomM350_OrbHandSparkEmitter(0)==setupResult && setups==1 && !allocs);
    }
    for(i=0;i<65536;i++) {
        reset(); g_FieldFxTpageSlot2=i; g_FieldFxTpageTable[i]=65535-i;
        assert(RoomM350_OrbHandSparkEmitter(2)==0 && !allocs && !setups);
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams)); g_FieldFxTpageSlot2=i;
        assert(RoomM350_OrbHandSparkEmitter(2)==0 && !allocs && !setups);
        assert(g_FieldFxSpriteParams.parameter00==16 && g_FieldFxSpriteParams.parameter02==1);
        assert(g_FieldFxSpriteParams.extent_x==16 && g_FieldFxSpriteParams.extent_y==16);
        assert(g_FieldFxSpriteParams.palette==2 && g_FieldFxSpriteParams.parameter06==0 && g_FieldFxSpriteParams.parameter0A==0);
        assert(g_FieldFxSpriteParams.depth==0 && g_FieldFxSpriteParams.tpage==65535-i);
    }
    for(event=-3;event<=5;event++) if(event<0 || event>2) {
        reset(); assert(RoomM350_OrbHandSparkEmitter(event)==0 && !allocs && !setups && !randoms);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory)/'shell'
            result = subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing', '-x', 'c', '-', '-o', str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
