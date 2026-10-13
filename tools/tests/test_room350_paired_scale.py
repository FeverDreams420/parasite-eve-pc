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

CONTROLLER = "RoomM350_HandFlashEmitter"


class PairedScaleTests(unittest.TestCase):
    @unittest.skipUnless((ROOT / "tools/old-gcc/cc1").is_file() and shutil.which("mipsel-none-elf-as"), "PSX tools unavailable")
    def test_target_layout(self):
        unit = find_definition(CONTROLLER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Particle)==4 ? 1:-1];
typedef char b[sizeof(Vector)==8 ? 1:-1];
typedef char c[OFF(Instance,animation)==14 ? 1:-1];
typedef char d[OFF(Instance,frame)==22 ? 1:-1];
typedef char e[OFF(Instance,previousFrame)==26 ? 1:-1];
typedef char f[OFF(Actor,instance)==8 ? 1:-1];
typedef char g[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which("cc"), "host compiler unavailable")
    def test_crossing_precedes_completion(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM))
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
FieldFxObjectPrefix *g_FieldFxOwner;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
GteShortVector g_RoomM350HandPoints[2];
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlots[16];
u16 g_FieldFxTpageTable[65536];
static Actor actor;
static Instance instance;
static RoomM350Emitter emitters[2];
static int pools[2], calls, setups, failAt, setupResult;
static Particle particles[2], expected[2];
int RoomM350_HandFlashCallback(int e, void **p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[0] && stride==4 && count==2);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_HandFlashCallback);
    setups++;
    return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *pool) {
    int index=calls++;
    assert(index<2 && pool==(char *)&pools[index ? 1:0]);
    /* Verify pool reload and absence of a post-allocation frame check. */
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1];
    instance.frame=65535; instance.previousFrame=65535; instance.animation=0;
    return index==failAt ? 0:&particles[index];
}
static void reset(void) {
    calls=setups=0;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actor;
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0];
    memset(particles,0x5A,sizeof(particles));
    memcpy(expected,particles,sizeof(particles));
}
static void run(int animation,int frame,int previous,int failure) {
    int active=animation==11 && frame>=5 && previous<5;
    int result=animation==11 && !active && frame>=41 ? 2:0;
    int i, successes=active ? (failure<2 ? failure:2):0;
    reset(); failAt=failure;
    instance.animation=animation; instance.frame=frame; instance.previousFrame=previous;
    assert(RoomM350_HandFlashEmitter(1)==result);
    assert(calls==(active ? (failure==0 ? 1:2):0) && setups==0);
    for(i=0;i<successes;i++) expected[i].position=&g_RoomM350HandPoints[i];
    assert(memcmp(expected,particles,sizeof(particles))==0);
}
int main(void) {
    int i,j,k;
    int edges[]={0,4,5,40,41,32767,32768,65535};
    actor.instance=&instance;
    emitters[0].pool=&pools[0]; emitters[1].pool=&pools[1];
    for(i=0;i<65536;i++) for(k=0;k<3;k++) {
        run(11,i,0,k);
        run(11,i,5,k);
        run(11,5,i,k);
        run(11,65535,i,k);
    }
    for(i=0;i<256;i++) for(j=0;j<8;j++) for(k=0;k<3;k++) run(i,edges[j],0,k);
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(); assert(RoomM350_HandFlashEmitter(0)==setupResult && setups==1 && calls==0);
    }
    for(i=0;i<65536;i++) {
        /* The tpage slot is entry 11 of the texture slot table (0x800E11FA). */
        reset(); g_FieldFxTpageSlots[11]=i; g_FieldFxTpageTable[i]=65535-i;
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
        assert(RoomM350_HandFlashEmitter(2)==0 && calls==0 && setups==0);
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2);
        assert(g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
        assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0);
        assert(g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==1);
        assert(g_FieldFxSpriteParams.tpage==65535-i);
    }
    for(i=-3;i<=5;i++) if(i<0 || i>2) {
        reset(); assert(RoomM350_HandFlashEmitter(i)==0 && calls==0 && setups==0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "paired"
            result = subprocess.run(["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing", "-x", "c", "-", "-o", str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
