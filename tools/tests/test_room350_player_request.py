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

EMITTER = 'RoomM350_HandSweepHitEmitter'


class PlayerRequestTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(EMITTER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Particle)==8 ? 1:-1];
typedef char b[sizeof(RoomM350TransformMatrix)==32 ? 1:-1];
typedef char c[OFF(RoomM350TransformMatrix,position)==20 ? 1:-1];
typedef char d[OFF(Instance,transforms)==0x238 ? 1:-1];
typedef char e[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_request_retry_and_player_reload(self):
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n' + host_unit(EMITTER, path=find_definition(EMITTER, under=ROOM)) + r'''
typedef RoomM350TransformMatrix Transform;
/* The spawner reads the player through the transform-owner view. */
FieldActor *g_PlayerEntity;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
unsigned char g_RoomM350HandSweepDone,g_RoomM350HandSweepHit;
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlots[16];
u16 g_FieldFxTpageTable[65536];
static Instance players[2];
static Transform transforms[2];
static RoomM350Emitter emitter;
static Particle particle,expected;
static int pool,calls,setups,setupResult,fail;
static int half(uint32_t x) { x &=65535; return x<32768 ? (int)x:(int)x-65536; }
int RoomM350_HandSweepHitCallback(int e,short *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pool && stride==8 && count==4);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_HandSweepHitCallback);
    setups++; return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    assert(p==(char *)&pool); calls++;
    if(fail) return 0;
    g_PlayerEntity=(FieldActor *)&players[1]; g_RoomM350HandSweepDone=255; g_RoomM350HandSweepHit=123;
    return &particle;
}
static void reset(void) {
    calls=setups=0; g_PlayerEntity=(FieldActor *)&players[0];
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitter;
    g_RoomM350HandSweepDone=0; g_RoomM350HandSweepHit=1;
    memset(&particle,0x5A,sizeof(particle)); expected=particle;
}
static void verify(void) {
    expected.x=half(transforms[1].position[0]); expected.y=half(transforms[1].position[1]);
    expected.z=half(transforms[1].position[2]); expected.frame=0;
    assert(memcmp(&expected,&particle,sizeof(particle))==0);
    assert(g_RoomM350HandSweepDone==255 && g_RoomM350HandSweepHit==0);
}
int main(void) {
    int stop,request,failure,i,event;
    emitter.pool=&pool; players[0].transforms=&transforms[0]; players[1].transforms=&transforms[1];
    transforms[1].position[0]=0x12348000; transforms[1].position[1]=0x56787FFF; transforms[1].position[2]=-1;
    for(stop=0;stop<256;stop++) for(request=0;request<256;request++) for(failure=0;failure<2;failure++) {
        reset(); g_RoomM350HandSweepDone=stop; g_RoomM350HandSweepHit=request; fail=failure;
        assert(RoomM350_HandSweepHitEmitter(1)==(stop ? 2:0));
        assert(calls==(!stop && request!=0) && setups==0);
        if(!stop && request && !fail) verify();
        else {
            assert(g_RoomM350HandSweepDone==stop && g_RoomM350HandSweepHit==request);
            assert(memcmp(&expected,&particle,sizeof(particle))==0);
        }
    }
    reset(); fail=1;
    assert(RoomM350_HandSweepHitEmitter(1)==0 && calls==1 && g_RoomM350HandSweepHit==1);
    fail=0; assert(RoomM350_HandSweepHitEmitter(1)==0 && calls==2); verify();
    for(i=0;i<65536;i++) {
        reset(); fail=0;
        transforms[1].position[0]=0x12340000|i;
        transforms[1].position[1]=-i-1; transforms[1].position[2]=i*30001;
        assert(RoomM350_HandSweepHitEmitter(1)==0 && calls==1); verify();
    }
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(); assert(RoomM350_HandSweepHitEmitter(0)==setupResult && setups==1 && calls==0);
        assert(g_RoomM350HandSweepDone==0 && g_RoomM350HandSweepHit==1);
    }
    for(i=0;i<65536;i++) {
        /* Texture slot 11 of g_FieldFxTpageSlots is the halfword at 0x800E11FA. */
        reset(); g_FieldFxTpageSlots[11]=i; g_FieldFxTpageTable[i]=65535-i;
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
        assert(RoomM350_HandSweepHitEmitter(2)==0 && calls==0 && setups==0);
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2);
        assert(g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
        assert(g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==1);
        assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0);
        assert(g_FieldFxSpriteParams.tpage==65535-i && g_RoomM350HandSweepHit==1);
    }
    for(event=-3;event<=5;event++) if(event<0 || event>2) {
        reset(); assert(RoomM350_HandSweepHitEmitter(event)==0 && calls==0 && setups==0 && g_RoomM350HandSweepHit==1);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe=pathlib.Path(directory)/'request'
            result=subprocess.run(['cc','-std=gnu11','-O2','-fno-strict-aliasing','-x','c','-','-o',str(exe)],input=harness,text=True,capture_output=True)
            self.assertEqual(result.returncode,0,result.stderr)
            subprocess.run([str(exe)],check=True,timeout=20)
