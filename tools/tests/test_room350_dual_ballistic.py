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
FUNCTION = 'RoomM350_AreaDropletEmitter'


def controller_source():
    # The ballistic callback is reduced to a prototype; the test stubs it.
    return host_unit(FUNCTION, under=ROOM)


class DualBallisticTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Matrix)==32 ? 1:-1];
typedef char b[OFF(Matrix,position)==20 ? 1:-1];
typedef char c[OFF(Instance,transforms)==0x238 ? 1:-1];
typedef char d[OFF(Actor,instance)==8 ? 1:-1];
typedef char e[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
typedef char f[sizeof(Particle)==12 ? 1:-1];
typedef char g[OFF(Particle,velocityY)==6 ? 1:-1];
typedef char h[OFF(Particle,gravity)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_captured_matrices_fresh_fields_and_request(self):
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n#include <limits.h>\n' + controller_source() + r'''
FieldFxObjectPrefix *g_FieldFxOwner;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
/* The player is read through the same transform-owner view as the actor. */
FieldActor *g_PlayerEntity;
#define PLAYER ((Instance *)g_PlayerEntity)
unsigned char g_RoomM350AreaPlayerInside;
int g_FieldFxCurrentAge;
/* The retail halfword names are fields of the sprite parameter block. */
RenderEffectParameters g_FieldFxSpriteParams;
#define SIGNED(field) (*(short *)&g_FieldFxSpriteParams.field)
#define UNIT SIGNED(parameter00)
#define D_800F336A SIGNED(parameter02)
#define D_800F336C g_FieldFxSpriteParams.palette
#define D_800F336E SIGNED(parameter06)
#define D_800F3370 g_FieldFxSpriteParams.tpage
#define D_800F3372 SIGNED(parameter0A)
#define D_800F3374 g_FieldFxSpriteParams.depth
#define D_800F3376 SIGNED(extent_x)
#define D_800F3378 SIGNED(extent_y)
unsigned short g_FieldFxTpageSlot2,g_FieldFxTpageTable[65536];
static Actor actor;
static RoomM350Emitter emitters[2];
static Instance actorInstance,players[2];
static Matrix actorMatrices[2][34],playerMatrices[3],*captured;
static Particle particles[2],expected[2];
static int pools[2],allocs,randoms,setups,setupResult,failure,mutation,actorAttempt,active,part;
static uint32_t seed;
static int word(uint32_t x) { int result; memcpy(&result,&x,4); return result; }
static int half(uint32_t x) { x&=65535; return x<32768 ? (int)x:(int)x-65536; }
int RoomM350_AreaDropletCallback(int e,short *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int size,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[0] && size==12 && count==16 && (void *)cb==(void *)RoomM350_AreaDropletCallback);
    setups++; return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int i=allocs++,isActor=actorAttempt && i==0;
    assert(i<2 && p==(char *)&pools[i!=0]); active=i; part=0;
    captured=isActor ? &actorMatrices[0][33] : PLAYER->transforms;
    /* Pointer changes must not replace the transform captured before allocation. */
    actorInstance.transforms=actorMatrices[1];
    if(isActor) {
        g_PlayerEntity=(FieldActor *)&players[1];
        if(mutation) g_RoomM350AreaPlayerInside=mutation==1 ? 0:255;
    } else PLAYER->transforms=&playerMatrices[2];
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1];
    if(failure&(1<<i)) return 0;
    return &particles[i];
}
int Inv_ScrambleGrid(void) {
    uint32_t value=seed+(uint32_t)randoms*0x87654321u;
    int j; randoms++;
    if(part) assert(particles[active].x==expected[active].x && particles[active].y==expected[active].y);
    for(j=0;j<3;j++) captured->position[j]=word(0x7FFFFF00u+seed+j*17u+part*0x8001u);
    g_FieldFxCurrentAge=word(seed+randoms*31u);
    if(!part) {
        expected[active].x=half((uint32_t)captured->position[0]-(value*2-256));
        expected[active].y=half(captured->position[1]);
    } else {
        expected[active].z=half((uint32_t)captured->position[2]-(value*2-256));
        expected[active].velocityY=0;
        expected[active].gravity=(((uint32_t)g_FieldFxCurrentAge>>1)&1)+2;
    }
    part++; return word(value);
}
static void reset(void) {
    allocs=randoms=setups=0;
    actor.instance=(struct RoomM350Instance *)&actorInstance;
    actorInstance.transforms=actorMatrices[0];
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actor; g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0];
    players[0].transforms=&playerMatrices[0]; players[1].transforms=&playerMatrices[1];
    g_PlayerEntity=(FieldActor *)&players[0];
    memset(particles,0x5A,sizeof(particles)); memcpy(expected,particles,sizeof(particles));
}
static void run(int timer,int flag,int mask,int mode) {
    int playerAttempt,calls,successes,i;
    reset(); failure=mask; mutation=mode; g_FieldFxCurrentAge=timer; g_RoomM350AreaPlayerInside=flag;
    actorAttempt=timer<107 && (timer&1);
    playerAttempt=timer<107 && (actorAttempt && mode ? mode==2 : flag!=0);
    calls=actorAttempt+playerAttempt; successes=0;
    for(i=0;i<calls;i++) successes+=!(mask&(1<<i));
    if(!actorAttempt) g_FieldFxOwner=0;
    assert(RoomM350_AreaDropletEmitter(1)==(timer>=107 ? 2:0));
    assert(allocs==calls && randoms==successes*2 && setups==0);
    assert(memcmp(particles,expected,sizeof(particles))==0);
    assert(g_RoomM350AreaPlayerInside==(actorAttempt && mode ? (mode==2 ? 255:0):flag));
}
int main(void) {
    int i,f,m,k,event,flags[]={0,1,255},timers[]={INT_MIN,-1,0,105,106,107,INT_MAX};
    emitters[0].pool=&pools[0]; emitters[1].pool=&pools[1];
    for(i=0;i<65536;i++) for(f=0;f<4;f++) for(m=0;m<3;m++) for(k=0;k<3;k++) {
        seed=0xFEDC0000u|i; run(i-32768,flags[k],f,m);
    }
    for(i=0;i<7;i++) for(f=0;f<4;f++) { seed=(uint32_t)timers[i]; run(timers[i],1,f,0); }
    for(i=0;i<256;i++) { seed=i; run(3,i,0,0); run(2,i,0,0); }
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(); assert(RoomM350_AreaDropletEmitter(0)==setupResult && setups==1 && !allocs);
    }
    for(i=0;i<65536;i++) {
        reset(); g_FieldFxTpageSlot2=i; g_FieldFxTpageTable[i]=65535-i;
        assert(RoomM350_AreaDropletEmitter(2)==0 && !allocs && !setups);
        assert(UNIT==16 && D_800F336A==1 && D_800F3376==16 && D_800F3378==16);
        assert(D_800F336C==2 && D_800F336E==0 && D_800F3372==0 && D_800F3374==0 && D_800F3370==65535-i);
    }
    for(event=-3;event<=5;event++) if(event<0 || event>2) {
        reset(); assert(RoomM350_AreaDropletEmitter(event)==0 && !allocs && !setups && !randoms);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory)/'dual'
            result = subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing', '-x', 'c', '-', '-o', str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
