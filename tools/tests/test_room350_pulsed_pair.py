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

SPAWNER = 'RoomM350_HandSparkEmitter'


class PulsedPairTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(SPAWNER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Vector)==8 ? 1:-1];
typedef char b[sizeof(Matrix)==32 && OFF(Matrix,position)==20 ? 1:-1];
typedef char c[sizeof(Particle)==8 && OFF(Particle,reserved)==6 ? 1:-1];
typedef char d[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_emission_count_rng_and_reload(self):
        source = host_unit(SPAWNER, path=find_definition(SPAWNER, under=ROOM))
        # Stubs validate CPU-side helper contracts, not rotation/GTE arithmetic.
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n#include <limits.h>\n' + source + r'''
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
GteShortVector g_RoomM350HandPoints[2];
int g_FieldFxCurrentAge;
short D_800966EE[8192];
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlot2;
u16 g_FieldFxTpageTable[65536];
static RoomM350Emitter emitters[2];
static int pools[2],allocs,randoms,rotations,applications,failAt;
static Particle particles[36],expected[36];
static const GteMatrix *rotation;
static uint32_t seed;
static int word(uint32_t x) { int r; memcpy(&r,&x,4); return r; }
static int half(uint32_t x) { x&=65535; return x<32768 ? (int)x:(int)x-65536; }
static uint32_t rng(int n) { return seed+(uint32_t)n*0x87654321u; }
int RoomM350_HandSparkCallback(int e,Particle *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[0] && stride==8 && count==64);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_HandSparkCallback); return -123;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int i=allocs++;
    assert(i<36 && p==(char *)&pools[i&1] && randoms==i*4 && applications==i);
    g_FieldFxCurrentAge=INT_MAX;
    return i==failAt ? 0:&particles[i];
}
int Inv_ScrambleGrid(void) { return word(rng(randoms++)); }
GteMatrix *RotMatrixYXZ(GteShortVector *v,GteMatrix *m) {
    int i=rotations++;
    assert(randoms==i*4+2 && applications==i && allocs==i+1);
    assert(v->x==half(rng(i*4)<<4) && v->y==half(rng(i*4+1)<<4) && v->z==0);
    rotation=m;
    return m;
}
GteShortVector *ApplyMatrixSV(const GteMatrix *m,const GteShortVector *v,GteShortVector *out) {
    int i=applications++,j;
    int combined=word((rng(i*4+2)<<8)|rng(i*4+3));
    assert(m==rotation && randoms==i*4+4 && rotations==i+1);
    assert(v->x==0 && v->y==0 && v->z==combined%192+64);
    out->x=half(seed+i); out->y=half(seed*7+i); out->z=half(seed*31+i);
    for(j=0;j<3;j++) {
        ((short *)&g_RoomM350HandPoints[i&1])[j]=half(seed*13+i*257+j);
        expected[i].position[j]=half((uint16_t)((short *)out)[j]+(uint16_t)((short *)&g_RoomM350HandPoints[i&1])[j]);
    }
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[(i+1)&1];
    memset(D_800966EE,0,sizeof(D_800966EE));
    return out;
}
static void reset(void) {
    allocs=randoms=rotations=applications=0; g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0];
    memset(particles,0x5A,sizeof(particles)); memcpy(expected,particles,sizeof(particles));
}
static void run(int timer,int sample,int failure) {
    int count=((sample+4096)/2048+1)*2,success,wanted;
    reset(); failAt=failure; g_FieldFxCurrentAge=timer;
    D_800966EE[((((uint32_t)timer<<7)+2048)&4095)*2]=sample;
    assert(RoomM350_HandSparkEmitter(1)==(timer>=33 ? 2:0));
    if(timer>=33 || count<=0) wanted=success=0;
    else { success=(failure>=0 && failure<count) ? failure:count; wanted=success+(success<count); }
    assert(allocs==wanted && randoms==success*4 && applications==success && rotations==success);
    assert(memcmp(particles,expected,sizeof(particles))==0);
}
int main(void) {
    int n,j;
    static const int timers[]={INT_MIN,-65537,-32768,-33,-1,0,1,31,32,33,INT_MAX};
    for(n=0;n<2;n++) emitters[n].pool=&pools[n];
    reset(); assert(RoomM350_HandSparkEmitter(0)==-123);
    for(n=0;n<65536;n++) for(j=0;j<3;j++) {
        seed=(uint32_t)n*65537; run(-1,half(n),j-1);
    }
    for(n=0;n<36;n++) for(j=0;j<sizeof(timers)/sizeof(timers[0]);j++) {
        seed=(uint32_t)n*0x81234567u; run(timers[j],32767,n);
    }
    for(n=0;n<65536;n++) {
        seed=(uint32_t)n*0x81234567u; run(half(n),4096,-1);
        g_FieldFxTpageSlot2=n; g_FieldFxTpageTable[n]=n^0xA357;
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
        assert(RoomM350_HandSparkEmitter(2)==0 && g_FieldFxSpriteParams.tpage==(unsigned short)(n^0xA357));
        assert(g_FieldFxSpriteParams.parameter00==16 && g_FieldFxSpriteParams.parameter02==1);
        assert(g_FieldFxSpriteParams.extent_x==16 && g_FieldFxSpriteParams.extent_y==16);
        assert(g_FieldFxSpriteParams.palette==2 && g_FieldFxSpriteParams.parameter06==0);
        assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0);
    }
    reset(); assert(RoomM350_HandSparkEmitter(-1)==0 && RoomM350_HandSparkEmitter(3)==0 && allocs==0);
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
