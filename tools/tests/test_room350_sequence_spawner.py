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

SPAWNER = "RoomM350_OrbEmitter"


class SequenceSpawnerTests(unittest.TestCase):
    @unittest.skipUnless((ROOT / "tools/old-gcc/cc1").is_file() and shutil.which("mipsel-none-elf-as"), "PSX tools unavailable")
    def test_target_layout(self):
        unit = find_definition(SPAWNER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef RoomM350Particle Particle;
typedef RoomM350EffectInstance Instance;
typedef char a[sizeof(Particle)==28 ? 1:-1];
typedef char b[sizeof(GteShortVector)==8 ? 1:-1];
typedef char c[OFF(Instance,animation)==14 ? 1:-1];
typedef char d[OFF(Instance,frame)==22 ? 1:-1];
typedef char e[OFF(Instance,previousFrame)==26 ? 1:-1];
typedef char f[OFF(Instance,yaw)==58 ? 1:-1];
typedef char g[OFF(RoomM350EffectActor,instance)==8 ? 1:-1];
typedef char h[OFF(RoomM350EffectEmitter,pool)==8 ? 1:-1];
typedef char i[OFF(Particle,motion.value.spawn.phase)==12 ? 1:-1];
typedef char j[OFF(Particle,tail.value.spawn.size)==16 ? 1:-1];
typedef char k[OFF(Particle,tail.value.spawn.state)==24 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which("cc"), "host compiler unavailable")
    def test_initialization_and_crossing(self):
        harness = '#include <assert.h>\n#include <string.h>\n' + host_unit(SPAWNER, path=find_definition(SPAWNER, under=ROOM)) + r'''
typedef RoomM350Particle Particle;
FieldFxObjectPrefix *g_FieldFxOwner;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
GteShortVector g_RoomM350OrbSpawnPoint;
unsigned char D_8019A804;
short D_8019A802;
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlots[16];
u16 g_FieldFxTpageTable[65536];
static RoomM350EffectActor actors[2];
static RoomM350EffectInstance instances[2];
static RoomM350EffectEmitter emitter;
static int pool, calls, setups, setupResult, countAfterAllocation;
static Particle particle, expected;
static int half(int value) {
    unsigned int bits=(unsigned int)value & 65535;
    return bits<32768 ? (int)bits:(int)bits-65536;
}
int RoomM350_OrbCallback(int e, Particle *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pool && stride==28 && count==3);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_OrbCallback);
    setups++;
    return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    assert(p==(char *)&pool && calls++==0);
    /* Allocation is assumed successful by the target. Probe helper reloads. */
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[1]; D_8019A802=countAfterAllocation; D_8019A804=255;
    g_RoomM350OrbSpawnPoint.x=-32768; g_RoomM350OrbSpawnPoint.y=32767; g_RoomM350OrbSpawnPoint.z=-123; g_RoomM350OrbSpawnPoint.pad=99;
    return &particle;
}
static void reset(void) {
    calls=setups=0;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[0]; g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitter;
    D_8019A802=123; D_8019A804=0;
    memset(&particle,0x5A,sizeof(particle)); expected=particle;
    memset(&g_RoomM350OrbSpawnPoint,0,sizeof(g_RoomM350OrbSpawnPoint));
}
static void run(int animation,int frame,int previous,int stop) {
    int active=!stop && animation==13 && frame>=15 && previous<15;
    reset(); D_8019A804=stop;
    instances[0].animation=animation; instances[0].frame=frame; instances[0].previousFrame=previous;
    assert(RoomM350_OrbEmitter(1)==(stop ? 2:0));
    assert(calls==active && setups==0);
    if(active) {
        expected.position=g_RoomM350OrbSpawnPoint; expected.position.pad=16;
        expected.motion.value.spawn.speed=512;
        expected.motion.value.spawn.yaw=half(instances[1].yaw);
        expected.motion.value.spawn.phase=0;
        expected.tail.value.spawn.duration=16; expected.tail.value.spawn.frame=0;
        expected.tail.value.spawn.state=0;
        expected.tail.value.spawn.size=half(100-countAfterAllocation*20);
        assert(D_8019A802==half(countAfterAllocation+1) && D_8019A804==255);
    } else assert(D_8019A802==123 && D_8019A804==stop);
    assert(memcmp(&expected,&particle,sizeof(particle))==0);
}
int main(void) {
    int i,j;
    int edges[]={0,14,15,32767,32768,65535};
    emitter.pool=&pool;
    actors[0].instance=&instances[0]; actors[1].instance=&instances[1];
    instances[0].yaw=1; instances[1].yaw=65535;
    countAfterAllocation=5;
    for(i=0;i<65536;i++) { run(13,i,0,0); run(13,15,i,0); }
    for(i=0;i<256;i++) for(j=0;j<6;j++) {
        run(i,edges[j],0,0); run(13,edges[j],0,i);
    }
    for(i=-32768;i<=32767;i++) {
        countAfterAllocation=i; instances[1].yaw=(unsigned short)i;
        run(13,15,0,0);
    }
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(); assert(RoomM350_OrbEmitter(0)==setupResult && setups==1 && calls==0);
        assert(D_8019A802==123);
    }
    for(i=0;i<65536;i++) {
        /* Texture slot 3 of g_FieldFxTpageSlots is the halfword at 0x800E11EA. */
        reset(); g_FieldFxTpageSlots[3]=i; g_FieldFxTpageTable[i]=65535-i;
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
        assert(RoomM350_OrbEmitter(2)==0 && calls==0 && setups==0);
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2);
        assert(g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
        assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==64);
        assert(g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==0);
        assert(g_FieldFxSpriteParams.tpage==65535-i && D_8019A802==123);
    }
    for(i=-3;i<=5;i++) if(i<0 || i>2) {
        reset(); assert(RoomM350_OrbEmitter(i)==0 && calls==0 && setups==0);
        assert(memcmp(&expected,&particle,sizeof(particle))==0 && D_8019A802==123);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "sequence"
            result = subprocess.run(["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing", "-x", "c", "-", "-o", str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
