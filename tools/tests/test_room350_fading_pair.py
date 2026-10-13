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

CONTROLLER = 'RoomM350_AreaSparkEmitter'


class FadingPairTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(CONTROLLER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(State)==16 && OFF(State,count)==8 && OFF(State,brightness)==10 ? 1:-1];
typedef char b[OFF(State,fade)==12 && OFF(State,size)==14 ? 1:-1];
/* Particles are short vectors whose pad halfword holds the size. */
typedef char c[sizeof(GteShortVector)==8 && OFF(GteShortVector,pad)==6 ? 1:-1];
typedef char d[sizeof(GteMatrix)==32 && OFF(GteMatrix,t)==20 ? 1:-1];
typedef char e[OFF(RoomM350EffectInstance,transforms)==0x238 && OFF(RoomM350EffectInstance,owner)==0 ? 1:-1];
typedef char f[OFF(RoomM350EffectActor,instance)==8 && OFF(RoomM350EffectOwner,asset)==8 && OFF(RoomM350EffectEmitter,pool)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_setup_update_and_draw(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM))
        # Remove only MIPS register constraints; matrix helpers are boundaries,
        # not GTE emulation. Never inspect uninitialized vector/matrix padding.
        harness = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
''' + source + r'''
typedef GteShortVector Particle;
typedef GteMatrix Matrix;
typedef GteShortVector Vector;
FieldFxObjectPrefix *g_FieldFxOwner;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
unsigned char g_RoomM350AreaDone;
int g_FieldFxCurrentAge,g_FieldFxAltPaletteFlag;
GteSineWord rcossin_tbl[4096];
u16 g_FieldFxTpageTable[65536],g_FieldFxPaletteTable[65536];
u16 g_FieldFxTpageSlot2;
RenderEffectParameters g_FieldFxSpriteParams;
char D_8019A570[8];
static RoomM350EffectActor actors[2];
static RoomM350EffectInstance instances[2];
static RoomM350EffectOwner owners[2];
static RoomM350EffectEmitter emitters[2];
static Matrix transforms[2][34];
static const Matrix *activeMatrix;
static const Vector *activeVector;
static int pools[2],assets[2],allocs,randoms,rotations,applies,sounds,cluts,draws;
static int failAt,mutate,nextKind,special,initialTimer,expectedRotation;
static State state,expected;
static Particle particles[2],expectedParticles[2];
static uint32_t seed;
static int word(uint32_t x) { int r; memcpy(&r,&x,4); return r; }
static int half(uint32_t x) { x&=65535; return x<32768 ? (int)x:(int)x-65536; }
static int shift5(int x) { return x>=0 ? x/32:-((-x+31)/32); }
static int rng(int i) { return word(seed+(uint32_t)i*0x87654321u); }
int RoomM350_AreaSparkCallback(int e,Particle *p) { return 0; }
int Asset_Find08w(int id,int asset,int x,int y,int z) {
    /* The unit narrows the owner asset pointer to the int argument. */
    assert(sounds++==0 && id==0x5C9 && asset==(int)(intptr_t)owners[0].asset);
    assert(x==half(transforms[0][33].t[0]));
    assert(y==half(transforms[0][33].t[1]) && z==half(transforms[0][33].t[2]));
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1]; memset(&state,0x39,sizeof(state));
    return -999;
}
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    State wanted;
    memset(&wanted,0x39,sizeof(wanted)); memset(&wanted.rotation,0,sizeof(wanted.rotation));
    wanted.count=0; wanted.fade=0;
    assert(memcmp(&state,&wanted,sizeof(state))==0);
    assert(sounds==1 && p==(char *)&pools[1] && stride==8 && count==18);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_AreaSparkCallback);
    return -17;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int i=allocs++;
    assert(i<2 && p==(char *)&pools[mutate && i ? 1:0]);
    assert(randoms==i*4 && applies==i && rotations==i);
    assert(state.brightness==expected.brightness && state.size==expected.size);
    if(mutate) state.count=expected.count=half(0xFFFFu+i);
    return i==failAt ? 0:&particles[i];
}
int Inv_ScrambleGrid(void) {
    int i=randoms++;
    assert(i<8 && allocs==i/4+1);
    if(i%4==2) assert(activeVector->x==0 && activeVector->y==0);
    return rng(i);
}
GteMatrix *RotMatrixYXZ(GteShortVector *v,GteMatrix *m) {
    int i=rotations++;
    assert(randoms==i*4+2 && applies==i);
    assert(v->x==half((uint32_t)rng(i*4)<<4));
    assert(v->y==half((uint32_t)rng(i*4+1)<<4) && v->z==0);
    activeMatrix=m; activeVector=v;
    return m;
}
GteShortVector *ApplyMatrixSV(const GteMatrix *m,const GteShortVector *v,GteShortVector *out) {
    int i=applies++,combined=word(((uint32_t)rng(i*4+2)<<8)|(uint32_t)rng(i*4+3));
    Particle *p=out;
    assert(randoms==i*4+4 && rotations==i+1 && out==&particles[i]);
    assert(m==activeMatrix && v==activeVector && v->x==0 && v->y==0 && v->z==combined%44+320);
    p->x=expectedParticles[i].x=-32768+i;
    p->y=expectedParticles[i].y=32767-i;
    p->z=expectedParticles[i].z=-1234+i;
    if(mutate) { state.count=expected.count=i ? -32768:32767; g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1]; }
    expectedParticles[i].pad=half((uint32_t)state.count*128+2048);
    return out;
}
u_short GetClut(int x,int y) {
    int i=cluts++,kind=i ? nextKind:2;
    assert(i<2 && draws==i && x==32);
    assert(y==g_FieldFxPaletteTable[kind]+(kind==4 && special ? 4:0));
    assert(state.rotation.z==expectedRotation);
    state.size=i ? 32767:-32768; state.brightness=i ? -1:1023;
    g_FieldFxSpriteParams.parameter02=i ? -32768:32767;
    return word(0xABCDFFFFu+i);
}
void FieldFx_DrawShapeQuads(struct GteShortVector *pos,struct GteRotation *rotation,int sx,int sy,int scale,int clut,int flag,int brightness,struct RenderColor *colors) {
    Vector *v=pos;
    int i=draws++;
    assert(i<2 && cluts==i+1 && (void *)rotation==&state && flag==1 && (void *)colors==D_8019A570);
    assert(v->x==half(0xABCDEF01u) && v->y==half(0x76543210u) && v->z==-32768);
    assert(sx==(i ? 32767:-32768) && sy==sx && brightness==(i ? -1:1023));
    assert(scale==(i ? -32768:32767)+216 && clut==(i ? 0:65535));
    assert(state.rotation.z==expectedRotation);
    state.rotation.z=i ? -32768:12345;
    expectedRotation=half(0u-(uint16_t)state.rotation.z);
    g_FieldFxSpriteParams.palette=nextKind; g_FieldFxAltPaletteFlag=special; g_FieldFxPaletteTable[nextKind]=65535;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[1]; transforms[0][33].t[0]=55; g_FieldFxCurrentAge=999;
}
static void reset(void) {
    allocs=randoms=rotations=applies=sounds=cluts=draws=0;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[0]; g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0];
    memset(&state,0x5A,sizeof(state)); memset(particles,0x5A,sizeof(particles));
    memcpy(expectedParticles,particles,sizeof(particles));
    transforms[0][33].t[0]=word(0xABCDEF01u);
    transforms[0][33].t[1]=0x76543210;
    transforms[0][33].t[2]=word(0xFFFF8000u);
}
static void update(int count,int fade,int timer,int stop,int sample,int failure,int mutation) {
    int phase,expired,emits,index;
    reset(); state.count=count; state.fade=fade; expected=state;
    g_FieldFxCurrentAge=timer; g_RoomM350AreaDone=stop; failAt=failure; mutate=mutation;
    phase=stop ? fade*64+1024:timer<32 ? word((uint32_t)timer<<5):1024;
    expired=stop && phase>=2048; emits=!expired && count<16;
    if(stop) expected.fade=half((uint16_t)fade+1);
    index=(uint32_t)phase&4095; rcossin_tbl[index].word=sample;
    if(!expired) { expected.brightness=shift5(half(sample)); expected.size=half((uint32_t)sample<<2); }
    assert(RoomM350_AreaSparkEmitter(1,&state)==(expired ? 2:0));
    if(emits) expected.count=half((uint16_t)expected.count+1);
    assert(memcmp(&state,&expected,sizeof(state))==0);
    assert(allocs==(emits ? (failure==0 ? 1:2):0));
    assert(applies==(emits ? (failure<0 ? 2:failure):0));
    assert(rotations==applies && randoms==applies*4 && sounds==0 && draws==0);
    assert(memcmp(particles,expectedParticles,sizeof(particles))==0);
}
int main(void) {
    int n,j,k;
    const int timers[]={INT_MIN,-65537,-1,0,1,31,32,33,INT_MAX};
    const int kinds[]={0,2,4,32768,65535};
    for(n=0;n<2;n++) {
        actors[n].instance=&instances[n]; instances[n].owner=&owners[n];
        instances[n].transforms=transforms[n]; owners[n].asset=&assets[n]; emitters[n].pool=&pools[n];
    }
    reset(); assert(RoomM350_AreaSparkEmitter(0,&state)==-17);
    for(n=0;n<65536;n++) {
        seed=(uint32_t)n*65537;
        update(16,half(n),0,1,word(seed),-1,0);
        update(16,7,timers[n%9],0,word(seed),-1,0);
        for(j=-1;j<2;j++) update(half(n),-1,-123,0,word(seed),j,0);
        update(0,-3,31,0,word(seed),n%3-1,1);
    }
    for(n=1;n<256;n++) update(0,15,0,n,INT_MIN,-1,0);
    for(n=0;n<65536;n++) {
        reset(); g_FieldFxTpageSlot2=n; g_FieldFxTpageTable[n]=n^0xA537;
        initialTimer=word((uint32_t)n*65537); g_FieldFxCurrentAge=initialTimer;
        expectedRotation=half((uint32_t)initialTimer<<8);
        nextKind=kinds[n%5]; special=n&1; g_FieldFxAltPaletteFlag=special;
        assert(RoomM350_AreaSparkEmitter(2,&state)==0 && draws==2 && cluts==2);
        assert(state.rotation.z==expectedRotation && state.rotation.x==0x5A5A && state.rotation.y==0x5A5A);
        assert(g_FieldFxSpriteParams.tpage==(unsigned short)(n^0xA537));
        assert(g_FieldFxSpriteParams.parameter00==16 && g_FieldFxSpriteParams.extent_x==16 && g_FieldFxSpriteParams.extent_y==16);
        assert(g_FieldFxSpriteParams.parameter06==0 && g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0);
        assert(allocs==0 && sounds==0);
    }
    reset(); expected=state;
    assert(RoomM350_AreaSparkEmitter(-1,&state)==0 && RoomM350_AreaSparkEmitter(3,&state)==0);
    assert(RoomM350_AreaSparkEmitter(INT_MIN,0)==0 && RoomM350_AreaSparkEmitter(INT_MAX,0)==0);
    assert(memcmp(&state,&expected,sizeof(state))==0 && sounds==0 && allocs==0 && draws==0);
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
