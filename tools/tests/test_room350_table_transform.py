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

CONTROLLER = "RoomM350_OrbTableFlashEmitter"


class TableTransformTests(unittest.TestCase):
    @unittest.skipUnless((ROOT / "tools/old-gcc/cc1").is_file() and shutil.which("mipsel-none-elf-as"), "PSX tools unavailable")
    def test_target_layout(self):
        unit = find_definition(CONTROLLER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Particle)==8 ? 1:-1];
typedef char b[sizeof(Entry)==2 ? 1:-1];
typedef char c[OFF(Instance,frame)==22 ? 1:-1];
typedef char d[OFF(Instance,transforms)==0x238 ? 1:-1];
typedef char e[sizeof(Transform)==32 ? 1:-1];
typedef char f[OFF(Transform,position)==20 ? 1:-1];
typedef char g[OFF(Actor,instance)==8 ? 1:-1];
typedef char h[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which("cc"), "host compiler unavailable")
    def test_table_and_post_allocation_reads(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM))
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n' + source + r'''
FieldFxObjectPrefix *g_FieldFxOwner;
FieldActor *g_RoomM350OrbActor;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
unsigned char D_8019A804;
Entry g_RoomM350OrbTableFlashEntries[13][2];
#define ENTRY(n) (((Entry *)g_RoomM350OrbTableFlashEntries)[n])
GteSineWord rcossin_tbl[4096];
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlots[16];
u16 g_FieldFxTpageTable[65536];
static Actor actors[2];
static Instance instances[3];
static Transform transforms[3][256];
static RoomM350Emitter emitters[2];
static Particle particles[2],expected[2];
static int pools[2],calls,setups,setupResult,failAt,frame,indexAfter,alternate,sizeWord;
static int half(uint32_t x) { x &=65535; return x<32768 ? (int)x:(int)x-65536; }
int RoomM350_OrbTableFlashCallback(int e,short *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[0] && stride==8 && count==10);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_OrbTableFlashCallback);
    setups++; return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int i=calls++;
    assert(i<2 && p==(char *)&pools[i ? 1:0]);
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1]; g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[1];
    g_RoomM350OrbActor=(FieldActor *)&instances[2];
    D_8019A804=255; instances[1].frame=65535;
    ENTRY(frame*2+i).index=indexAfter;
    ENTRY(frame*2+i).alternate=alternate;
    rcossin_tbl[(frame-7)*128].word=sizeWord;
    return i==failAt ? 0:&particles[i];
}
static void reset(void) {
    calls=setups=0; g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[0];
    g_RoomM350OrbActor=(FieldActor *)&instances[0];
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0]; D_8019A804=0;
    memset(particles,0x5A,sizeof(particles)); memcpy(expected,particles,sizeof(particles));
    memset(g_RoomM350OrbTableFlashEntries,0,sizeof(g_RoomM350OrbTableFlashEntries));
}
static void run(int f,int stop,int first,int second,int failure) {
    int active=!stop && f>=7 && f<=12;
    int expectedCalls=0, successes=0,i,j;
    reset(); frame=f; instances[0].frame=f; D_8019A804=stop; failAt=failure;
    if(active) {
        ENTRY(f*2).index=first; ENTRY(f*2+1).index=second;
        ENTRY(f*2).alternate=!alternate; ENTRY(f*2+1).alternate=!alternate;
        if(first) { expectedCalls=1; if(failure!=0) { successes=1;
            if(second) { expectedCalls=2; if(failure!=1) successes=2; }
        } }
    }
    assert(RoomM350_OrbTableFlashEmitter(1)==(stop ? 2:0));
    assert(calls==expectedCalls && setups==0);
    for(i=0;i<successes;i++) {
        for(j=0;j<3;j++) expected[i].position[j]=half(transforms[alternate ? 2:1][indexAfter].position[j]);
        expected[i].size=half((uint32_t)sizeWord+2048);
    }
    assert(memcmp(expected,particles,sizeof(particles))==0);
    assert(D_8019A804==(calls ? 255:stop));
}
int main(void) {
    int i,j,k,f,a,b,failure;
    const int words[]={0,-2048,32767,-32768,INT32_MAX,INT32_MIN,-1};
    for(i=0;i<3;i++) {
        instances[i].transforms=transforms[i];
        for(j=0;j<256;j++) for(k=0;k<3;k++) transforms[i][j].position[k]=(i+1)*100000+j*333+k*17777;
    }
    actors[0].instance=&instances[0]; actors[1].instance=&instances[1];
    emitters[0].pool=&pools[0]; emitters[1].pool=&pools[1];
    indexAfter=1; alternate=1; sizeWord=123;
    for(f=0;f<65536;f++) { run(f,0,1,1,2); run(f,255,1,1,2); }
    for(f=7;f<=12;f++) for(a=0;a<2;a++) for(b=0;b<2;b++)
        for(failure=0;failure<3;failure++) run(f,0,a,b,failure);
    /* Includes index becoming zero AFTER the pre-allocation sentinel check. */
    for(indexAfter=0;indexAfter<256;indexAfter++) for(alternate=0;alternate<2;alternate++)
        for(j=0;j<7;j++) for(f=7;f<=12;f++) { sizeWord=words[j]; run(f,0,1,1,2); }
    for(i=0;i<65536;i++) { sizeWord=i; indexAfter=i&255; alternate=i&1; run(7,0,1,1,2); }
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(); assert(RoomM350_OrbTableFlashEmitter(0)==setupResult && setups==1 && calls==0);
    }
    for(i=0;i<65536;i++) {
        /* Texture slot 11 of g_FieldFxTpageSlots is the halfword at 0x800E11FA. */
        reset(); g_FieldFxTpageSlots[11]=i; g_FieldFxTpageTable[i]=65535-i;
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
        assert(RoomM350_OrbTableFlashEmitter(2)==0 && calls==0 && setups==0);
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2);
        assert(g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
        assert(g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==1);
        assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0);
        assert(g_FieldFxSpriteParams.tpage==65535-i);
    }
    for(i=-3;i<=5;i++) if(i<0 || i>2) {
        reset(); assert(RoomM350_OrbTableFlashEmitter(i)==0 && calls==0 && setups==0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "transforms"
            result = subprocess.run(["cc", "-std=gnu11", "-O2", "-fwrapv", "-fno-strict-aliasing", "-x", "c", "-", "-o", str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
