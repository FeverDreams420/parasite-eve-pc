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

CONTROLLER = "RoomM350_HandSweepTrapEmitter"


class PairedAnimationTests(unittest.TestCase):
    @unittest.skipUnless((ROOT / "tools/old-gcc/cc1").is_file() and shutil.which("mipsel-none-elf-as"), "PSX tools unavailable")
    def test_target_layout(self):
        unit = find_definition(CONTROLLER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(RoomM350SweepTrap)==8 ? 1:-1];
typedef char b[OFF(RoomM350SweepTrap,done)==4 ? 1:-1];
typedef char c[OFF(Instance,animation)==14 ? 1:-1];
typedef char d[OFF(Instance,frame)==22 ? 1:-1];
typedef char e[OFF(Instance,previousFrame)==26 ? 1:-1];
typedef char f[OFF(Instance,transforms)==0x238 ? 1:-1];
typedef char g[OFF(Transform,position)==20 ? 1:-1];
typedef char h[sizeof(Transform)==32 ? 1:-1];
typedef char i[OFF(RoomM350Actor,instance)==8 ? 1:-1];
typedef char j[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
typedef char k[OFF(RoomM350SoundOwner,soundMode)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which("cc"), "host compiler unavailable")
    def test_animation_allocation_and_sound(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM))
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
typedef RoomM350SweepTrap Particle;
FieldFxObjectPrefix *g_FieldFxOwner;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
GteShortVector g_RoomM350HandPoints[2];
RenderEffectParameters g_FieldFxSpriteParams;
static RoomM350Actor actors[2];
static Instance instances[2];
static RoomM350SoundOwner owners[2];
static Transform transforms[2];
static RoomM350Emitter emitters[2];
static int pools[2], calls, sounds, setups, failAt, setupResult;
static Particle particles[2], expected[2];
int RoomM350_HandSweepTrapCallback(int e, RoomM350SweepTrap *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[0] && stride==8 && count==2);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_HandSweepTrapCallback);
    setups++;
    return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *pool) {
    int index=calls++;
    assert(index<2 && pool==(char *)&pools[index ? 1:0]);
    /* Exercise reloads after allocation, including the failure path. */
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1];
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[1];
    return index==failAt ? 0:&particles[index];
}
int Asset_Find08w(int sound,int mode,int x,int y,int z) {
    assert(sound==0x5C6 && mode==-17);
    assert(x==-32768 && y==32767 && z==-1);
    assert(calls==(failAt==0 ? 1:2));
    sounds++;
    return 123;
}
static void reset(void) {
    calls=sounds=setups=0;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[0];
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0];
    memset(particles,0x5A,sizeof(particles));
    memcpy(expected,particles,sizeof(particles));
}
static void run(int animation,int frame,int previous,int failure) {
    int active=animation==11 && frame>=8 && frame<50 && previous<8;
    int i, successes=active ? (failure<2 ? failure:2):0;
    reset(); failAt=failure;
    instances[0].animation=animation; instances[0].frame=frame;
    instances[0].previousFrame=previous;
    assert(RoomM350_HandSweepTrapEmitter(1)==(animation==11 && frame>=50 ? 2:0));
    assert(calls==(active ? (failure==0 ? 1:2):0));
    assert(sounds==active && setups==0);
    for(i=0;i<successes;i++) {
        expected[i].anchor=&g_RoomM350HandPoints[i]; expected[i].done=0;
    }
    assert(memcmp(expected,particles,sizeof(particles))==0);
}
int main(void) {
    int i,j,k;
    int edges[]={0,7,8,49,50,32767,32768,65535};
    for(i=0;i<2;i++) {
        actors[i].instance=(struct RoomM350Instance *)&instances[i]; instances[i].owner=&owners[i];
        instances[i].transforms=&transforms[i]; emitters[i].pool=&pools[i];
    }
    owners[1].soundMode=-17;
    transforms[1].position[0]=0x12348000;
    transforms[1].position[1]=0x56787FFF;
    transforms[1].position[2]=0x1357FFFF;
    for(i=0;i<65536;i++) for(k=0;k<3;k++) {
        run(11,i,0,k);
        run(11,8,i,k);
    }
    for(i=0;i<256;i++) for(j=0;j<8;j++) for(k=0;k<3;k++) run(i,edges[j],0,k);
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(); assert(RoomM350_HandSweepTrapEmitter(0)==setupResult && setups==1 && calls==0 && sounds==0);
    }
    reset();
    memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
    assert(RoomM350_HandSweepTrapEmitter(2)==0 && calls==0 && sounds==0 && setups==0);
    assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2);
    assert(g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
    assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0);
    for(i=-3;i<=5;i++) if(i<0 || i>2) {
        reset(); assert(RoomM350_HandSweepTrapEmitter(i)==0 && calls==0 && sounds==0 && setups==0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "paired"
            result = subprocess.run(["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing", "-x", "c", "-", "-o", str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
