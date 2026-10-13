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

CONTROLLER = 'RoomM350_BodyDebrisEmitter'
# The GTE stays a boundary in the host harness: keep pe1/gte.h's asm out and
# route its macros to recording stubs.
HOST_GTE = r'''
#include "pe1/gte_types.h"
#define PE1_GTE_H
static void load_rotation(void *);
static void load_translation(void *);
static void load_vector(void *);
static void transform(void);
static void store_ir(void *);
#define gte_ldrotmatrix(p) load_rotation(p)
#define gte_ldtransmatrix(p) load_translation(p)
#define gte_ldv0(p) load_vector(p)
#define gte_rtv0tr_mac() transform()
#define gte_stsv(p) store_ir(p)
'''


class FadingModelTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(CONTROLLER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(State)==4 && OFF(State,delay)==2 ? 1:-1];
typedef char b[sizeof(Particle)==20 && OFF(Particle,velocity)==8 ? 1:-1];
typedef char c[OFF(Particle,acceleration)==10 && OFF(Particle,brightness)==12 ? 1:-1];
typedef char d[OFF(Particle,size)==14 && OFF(Particle,phase)==16 && OFF(Particle,counter)==18 ? 1:-1];
typedef char e[OFF(Instance,render_object.header)==0x1B4 && OFF(Instance,render_object.matrices)==0x238 ? 1:-1];
typedef char f[OFF(RenderObjectHeader,part_count)==2 && OFF(FieldActorState,progress)==8 ? 1:-1];
typedef char g[OFF(Actor,instance)==8 && OFF(RoomM350Emitter,pool)==8 ? 1:-1];
typedef char h[sizeof(Matrix)==32 && OFF(Matrix,translation)==20 && sizeof(Vector)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_state_rng_and_gte_boundaries(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM), prefix=HOST_GTE)
        harness = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
''' + source + r'''
FieldFxObjectPrefix *g_FieldFxOwner;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
unsigned char g_RoomM350BodyDone;
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlots[16];
u16 g_FieldFxTpageTable[65536];
static Actor actors[3];
static Instance instances[3];
static RenderObjectHeader models[3];
static FieldActorState owners[3];
static Matrix matrices[3][256],*rotation;
static RoomM350Emitter emitters[2];
static int pools[2],allocs,randoms,scrambles,sounds,cosines,sines,gteStage,fail,mutate;
static int selected,size,halfSize,initialCount,initialDelay,sample;
static State state;
static Particle particle,expected;
static Vector *vector;
static uint32_t seed;
static int word(uint32_t x) { int r; memcpy(&r,&x,4); return r; }
static int half(uint32_t x) { x&=65535; return x<32768 ? (int)x:(int)x-65536; }
static int shift(int x,int n) { return x>=0 ? x/(1<<n):-((-(int64_t)x+(1<<n)-1)/(1<<n)); }
static int rng(int i) { return word(seed+(uint32_t)i*0x87654321u); }
int RoomM350_BodyDebrisCallback(int e,Particle *p) { return 0; }
int Asset_Find08w(int id,int asset,int x,int y,int z) {
    sounds++;
    if(id==0x549) {
        assert(asset==owners[0].progress && x==(short)matrices[0][0].translation[0]);
        assert(y==(short)matrices[0][0].translation[1] && z==(short)matrices[0][0].translation[2]);
        g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1]; state.count=123; state.delay=456; g_RoomM350BodyDone=255;
    } else {
        assert(id==0x5FE && asset==owners[mutate ? 2:0].progress && gteStage==7);
        assert(x==expected.x && y==expected.y && z==expected.z);
        assert(state.count==half((uint16_t)(mutate ? -1:initialCount)+1) && state.delay==0);
    }
    return -999;
}
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[1] && stride==20 && count==10);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_BodyDebrisCallback);
    assert(sounds==1 && state.count==0 && state.delay==0 && g_RoomM350BodyDone==0); return -17;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    assert(p==(char *)&pools[0] && allocs++==0 && randoms==0);
    return fail ? 0:&particle;
}
int rand(void) {
    int i=randoms++;
    assert(allocs==1 && i<4);
    if(i==0 && mutate) state.count=-32768;
    if(i==1) assert(particle.y==expected.y && particle.size==size && particle.counter==0);
    return rng(i);
}
int Inv_ScrambleGrid(void) {
    assert(randoms==1 && scrambles++==0 && particle.brightness==64 && particle.size==size);
    if(mutate) g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[1];
    return sample;
}
int rcos(int angle) { assert(randoms==3 && cosines++==0 && angle==rng(2)); return -12345; }
int rsin(int angle) { assert(randoms==4 && sines++==0 && angle==rng(3)); return 23456; }
static void load_rotation(void *p) {
    Matrix *m=p;
    assert(gteStage++==0 && randoms==4 && cosines==1 && sines==1);
    assert(m->rotation[0][0]==-12345 && m->rotation[0][2]==23456);
    assert(m->rotation[2][0]==-23456 && m->rotation[2][2]==-12345 && m->rotation[1][1]==4096);
    assert(m->rotation[0][1]==0 && m->rotation[1][0]==0 && m->rotation[1][2]==0 && m->rotation[2][1]==0);
    assert(m->translation[0]==0 && m->translation[1]==0 && m->translation[2]==0); rotation=m;
}
static void load_translation(void *p) { assert(gteStage++==1 && p==rotation); }
static void load_vector(void *p) {
    Vector *v=p;
    assert(gteStage++==2 && v->x==0 && v->z==half(shift((rng(1)%96+64)*halfSize,12)));
    /* Target does not initialize vector Y or pad; never read them here. */
    vector=v;
}
static void transform(void) {
    assert(gteStage++==3 && vector);
    if(mutate) { g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[2]; state.count=-1; }
}
static void store_ir(void *p) {
    Vector *v=p;
    assert(gteStage==4 && v==vector); gteStage+=3;
    v->x=-32768; v->y=123; v->z=32767;
}
static void reset(void) {
    allocs=randoms=scrambles=sounds=cosines=sines=gteStage=0;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[0];
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0]; g_RoomM350BodyDone=0;
    memset(&particle,0x5A,sizeof(particle)); memcpy(&expected,&particle,sizeof(particle));
}
static void run(int count,int delay,int modelCount,int failure,int mutation,int stop) {
    int next=half((uint16_t)delay+1),success,i,which=mutation ? 1:0;
    reset(); state.count=initialCount=count; state.delay=initialDelay=delay;
    fail=failure; mutate=mutation; g_RoomM350BodyDone=stop;
    models[which].part_count=modelCount;
    success=!stop && next>=4 && !failure;
    size=half(((uint32_t)(mutate ? -32768:count)<<7)+4096); halfSize=shift(size,1);
    selected=((sample&255)*(modelCount-1))/256+1;
    if(success) {
        expected.x=half((uint32_t)matrices[which][selected].translation[0]-32768);
        expected.y=half((uint32_t)matrices[which][selected].translation[1]-shift((sample&127)*size,13));
        expected.z=half((uint32_t)matrices[which][selected].translation[2]+32767);
        expected.velocity=expected.acceleration=expected.counter=0;
        expected.brightness=64; expected.size=size;
    }
    assert(RoomM350_BodyDebrisEmitter(1,&state)==(stop ? 2:0));
    assert(allocs==(!stop && next>=4) && randoms==(success ? 4:0) && scrambles==success);
    assert(sounds==success && gteStage==(success ? 7:0));
    assert(memcmp(&particle,&expected,sizeof(particle))==0);
    if(!success) assert(state.count==count && state.delay==(stop ? delay:next));
}
int main(void) {
    int n,m,j;
    for(n=0;n<3;n++) {
        actors[n].instance=(struct RoomM350Instance *)&instances[n]; instances[n].state=&owners[n];
        instances[n].render_object.header=&models[n];
        instances[n].render_object.matrices=matrices[n]; owners[n].progress=100+n;
        for(m=0;m<256;m++) for(j=0;j<3;j++) matrices[n][m].translation[j]=word(0x7FFFFF00u+(uint32_t)m*1234567+n*321+j);
    }
    for(n=0;n<2;n++) emitters[n].pool=&pools[n];
    reset(); assert(RoomM350_BodyDebrisEmitter(0,&state)==-17);
    for(n=0;n<65536;n++) for(m=0;m<2;m++) {
        seed=(uint32_t)n*65537; sample=word(seed);
        run(half(n),half(n),n&255,m,0,0);
        run(half(n),3,n&255,0,m,0);
    }
    for(n=0;n<256;n++) for(m=0;m<256;m++) {
        seed=(uint32_t)n*0x81234567u+m; sample=word(seed&0xFFFFFF00u)|m;
        run(32767,3,n,0,0,0);
    }
    for(n=1;n<256;n++) run(-1,32767,255,0,0,n);
    for(n=0;n<65536;n++) {
        /* Texture slot 11 of g_FieldFxTpageSlots is the halfword at 0x800E11FA. */
        g_FieldFxTpageSlots[11]=n; g_FieldFxTpageTable[n]=n^0xA357;
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
        assert(RoomM350_BodyDebrisEmitter(2,&state)==0 && g_FieldFxSpriteParams.tpage==(unsigned short)(n^0xA357));
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2);
        assert(g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
        assert(g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==1);
        assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==32);
    }
    reset(); assert(RoomM350_BodyDebrisEmitter(-1,&state)==0 && RoomM350_BodyDebrisEmitter(3,&state)==0 && sounds==0);
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
