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

CONTROLLER = 'RoomM350_OrbTrailEmitter'


class PositionQueueTests(unittest.TestCase):
    @unittest.skipUnless((ROOT / "tools/old-gcc/cc1").is_file() and shutil.which("mipsel-none-elf-as"), "PSX tools unavailable")
    def test_target_layout(self):
        unit = find_definition(CONTROLLER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Particle)==12 ? 1:-1];
typedef char b[OFF(Particle,delay)==8 ? 1:-1];
typedef char c[OFF(Queue,count)==0x48 ? 1:-1];
typedef char d[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
typedef char e[OFF(Instance,yaw)==0x3A ? 1:-1];
typedef char f[OFF(Actor,instance)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which("cc"), "host compiler unavailable")
    def test_queue_copy_and_helper_reloads(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM))
        source = source.replace("extern short D_8019A800, D_8019A802;", "extern short D_8019A802;")
        # Bind the interior count symbol to real storage, preserving aliasing.
        head = 'int RoomM350_OrbTrailEmitter(int event) {'
        assert head in source
        source = source.replace(head, 'static Queue pending;\n#define D_8019A800 pending.count\n' + head)
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
FieldFxObjectPrefix *g_FieldFxOwner;
int g_FieldFxCurrentAge;
short D_8019A802;
unsigned char D_8019A804;
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlot3;
u16 g_FieldFxTpageTable[65536];
static RoomM350Emitter emitters[2];
static Actor actors[2];
static Instance instances[2];
static int pools[2], calls, setups, setupResult, failAt, nextCount, yaw, timer;
static Particle particles[8], expected[8];
static int half(int x) { unsigned int b=(unsigned int)x&65535; return b<32768 ? (int)b:(int)b-65536; }
int RoomM350_OrbTrailCallback(int e, Particle *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[0] && stride==12 && count==50);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_OrbTrailCallback);
    setups++; return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int index=calls++;
    assert(index<8 && p==(char *)&pools[index ? 1:0]);
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1]; g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[1];
    instances[1].yaw=yaw; g_FieldFxCurrentAge=timer;
    pending.count=nextCount;
    pending.positions[index].x=-32768+index;
    pending.positions[index].y=32767-index;
    pending.positions[index].z=-123-index;
    pending.positions[index].pad=999;
    return index==failAt ? 0:&particles[index];
}
static void reset(void) {
    calls=setups=0; g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0];
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[0];
    g_FieldFxCurrentAge=123; instances[0].yaw=123;
    memset(particles,0x5A,sizeof(particles)); memcpy(expected,particles,sizeof(particles));
    memset(pending.positions,0,sizeof(pending.positions));
}
static void run(int count,int stop,int active,int replacement,int failure) {
    int expectedCalls=0, successes=0, i;
    int stopped=stop && active==0;
    reset(); pending.count=count; D_8019A804=stop; D_8019A802=active;
    nextCount=replacement; failAt=failure;
    if(!stopped && count>0) {
        do {
            expectedCalls++;
            if(successes==failure) break;
            successes++;
        } while(successes<replacement);
    }
    assert(RoomM350_OrbTrailEmitter(1)==(stopped ? 2:0));
    assert(calls==expectedCalls && setups==0);
    assert(pending.count==(stopped ? count:0));
    for(i=0;i<successes;i++) {
        expected[i].position=pending.positions[i]; expected[i].position.pad=half(yaw);
        expected[i].delay=(unsigned int)timer%8;
    }
    assert(memcmp(expected,particles,sizeof(particles))==0);
    assert(D_8019A804==stop && D_8019A802==active);
}
int main(void) {
    int count,stop,active,replacement,failure,i,j;
    const int timers[]={0,1,7,8,-1,-8,0x7FFFFFFF,(-2147483647-1)};
    emitters[0].pool=&pools[0]; emitters[1].pool=&pools[1];
    actors[0].instance=&instances[0]; actors[1].instance=&instances[1];
    yaw=32768; timer=-1;
    for(count=-2;count<=8;count++) for(stop=0;stop<2;stop++)
        for(active=-1;active<=1;active++) for(replacement=-2;replacement<=8;replacement++)
            for(failure=0;failure<=8;failure++) run(count,stop,active,replacement,failure);
    for(count=-32768;count<=32767;count++) { run(count,0,1,0,0); run(count,255,0,0,0); }
    for(active=-32768;active<=32767;active++) run(1,255,active,0,0);
    for(i=0;i<65536;i++) for(j=0;j<8;j++) { yaw=i; timer=timers[j]; run(1,0,1,1,8); }
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(); pending.count=3;
        assert(RoomM350_OrbTrailEmitter(0)==setupResult && setups==1 && calls==0 && pending.count==3);
    }
    for(i=0;i<65536;i++) {
        reset(); pending.count=3; g_FieldFxTpageSlot3=i; g_FieldFxTpageTable[i]=65535-i;
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
        assert(RoomM350_OrbTrailEmitter(2)==0 && calls==0 && setups==0 && pending.count==3);
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2);
        assert(g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
        assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0);
        assert(g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==0);
        assert(g_FieldFxSpriteParams.tpage==65535-i);
    }
    for(i=-3;i<=5;i++) if(i<0 || i>2) {
        reset(); pending.count=3;
        assert(RoomM350_OrbTrailEmitter(i)==0 && calls==0 && setups==0 && pending.count==3);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "queue"
            result = subprocess.run(["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing", "-x", "c", "-", "-o", str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
