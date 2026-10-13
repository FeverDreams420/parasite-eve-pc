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

CONTROLLER = 'RoomM350_OrbImpactEmitter'


class PointerQueueTests(unittest.TestCase):
    @unittest.skipUnless((ROOT / "tools/old-gcc/cc1").is_file() and shutil.which("mipsel-none-elf-as"), "PSX tools unavailable")
    def test_target_layout(self):
        unit = find_definition(CONTROLLER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Particle)==8 ? 1:-1];
typedef char b[OFF(Particle,offset)==4 ? 1:-1];
typedef char c[OFF(Queue,count)==0x56 ? 1:-1];
typedef char d[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which("cc"), "host compiler unavailable")
    def test_queue_mutation_and_failure(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM))
        source = source.replace("extern short D_8019A7FE, D_8019A802;", "extern short D_8019A802;")
        # Bind the interior count symbol to real storage, preserving aliasing.
        head = 'int RoomM350_OrbImpactEmitter(int event) {'
        assert head in source
        source = source.replace(head, 'static Queue pending;\n#define D_8019A7FE pending.count\n' + head)
        harness = '#include <assert.h>\n#include <string.h>\n' + source + r'''
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
short D_8019A802;
unsigned char D_8019A804;
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlots[16];
u16 g_FieldFxTpageTable[65536];
static RoomM350Emitter emitters[2];
static int pools[2], calls, setups, setupResult, failAt, nextCount;
static Particle particles[4], expected[4];
static Vector positions[8];
int RoomM350_OrbImpactCallback(int e, Particle *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[0] && stride==8 && count==16);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_OrbImpactCallback);
    setups++; return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int index=calls++;
    assert(index<4 && p==(char *)&pools[index ? 1:0]);
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1];
    pending.count=nextCount;
    pending.positions[index]=&positions[index+4];
    return index==failAt ? 0:&particles[index];
}
static void reset(void) {
    int i;
    calls=setups=0; g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0];
    memset(particles,0x5A,sizeof(particles)); memcpy(expected,particles,sizeof(particles));
    for(i=0;i<4;i++) pending.positions[i]=&positions[i];
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
    assert(RoomM350_OrbImpactEmitter(1)==(stopped ? 2:0));
    assert(calls==expectedCalls && setups==0);
    assert(pending.count==(stopped ? count:0));
    for(i=0;i<successes;i++) {
        expected[i].position=&positions[i+4]; expected[i].offset=0;
    }
    assert(memcmp(expected,particles,sizeof(particles))==0);
    assert(D_8019A804==stop && D_8019A802==active);
}
int main(void) {
    int count,stop,active,replacement,failure,i;
    emitters[0].pool=&pools[0]; emitters[1].pool=&pools[1];
    for(count=-2;count<=4;count++) for(stop=0;stop<2;stop++)
        for(active=-1;active<=1;active++) for(replacement=-2;replacement<=4;replacement++)
            for(failure=0;failure<=4;failure++) run(count,stop,active,replacement,failure);
    /* Full signed count range, with immediate allocation failure for positive values. */
    for(count=-32768;count<=32767;count++) { run(count,0,1,0,0); run(count,255,0,0,0); }
    for(active=-32768;active<=32767;active++) run(1,255,active,0,0);
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(); pending.count=3;
        assert(RoomM350_OrbImpactEmitter(0)==setupResult && setups==1 && calls==0 && pending.count==3);
    }
    for(i=0;i<65536;i++) {
        /* Texture slot 11 of g_FieldFxTpageSlots is the halfword at 0x800E11FA. */
        reset(); pending.count=3; g_FieldFxTpageSlots[11]=i; g_FieldFxTpageTable[i]=65535-i;
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
        assert(RoomM350_OrbImpactEmitter(2)==0 && calls==0 && setups==0 && pending.count==3);
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2);
        assert(g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
        assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0);
        assert(g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==1);
        assert(g_FieldFxSpriteParams.tpage==65535-i);
    }
    for(i=-3;i<=5;i++) if(i<0 || i>2) {
        reset(); pending.count=3;
        assert(RoomM350_OrbImpactEmitter(i)==0 && calls==0 && setups==0 && pending.count==3);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "queue"
            result = subprocess.run(["cc", "-std=gnu11", "-O2", "-fno-strict-aliasing", "-x", "c", "-", "-o", str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
