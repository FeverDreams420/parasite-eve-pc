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

EMITTER = 'RoomM350_HandGlowEmitter'


class RampedPairTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(EMITTER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Particle)==8 ? 1:-1];
typedef char b[OFF(Particle,scale)==4 ? 1:-1];
typedef char c[sizeof(State)==4 ? 1:-1];
typedef char d[OFF(State,current)==2 ? 1:-1];
typedef char e[OFF(Instance,frame)==22 ? 1:-1];
typedef char f[OFF(Actor,instance)==8 ? 1:-1];
typedef char g[OFF(RoomM350Emitter,pool)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_signed_ramp_and_allocation_reloads(self):
        source = host_unit(EMITTER, path=find_definition(EMITTER, under=ROOM))
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n#include <limits.h>\n' + source + r'''
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
FieldFxObjectPrefix *g_FieldFxOwner;
int g_FieldFxCurrentAge;
GteShortVector g_RoomM350HandPoints[2];
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxTpageSlot2;
u16 g_FieldFxTpageTable[65536];
static RoomM350Emitter emitters[2];
static Actor actor;
static Instance instance;
static State state,reference;
static Particle particles[2],expected[2];
static int pools[2],calls,setups,setupResult,failAt,mutate;
static int half(uint32_t x) { x&=65535; return x<32768 ? (int)x:(int)x-65536; }
int RoomM350_HandGlowCallback(int e,Particle *p) { return 0; }
int FieldFx_InitCallbackList(char *pool,int size,int count,FieldFxCallbackListCallback cb) {
    assert(pool==(char *)&pools[0] && size==8 && count==6);
    assert(cb==(FieldFxCallbackListCallback)RoomM350_HandGlowCallback);
    assert(state.target==16384 && state.current==0); setups++; return setupResult;
}
void *FieldFx_AcquireCallbackSlot(char *pool) {
    int i=calls++;
    assert(i<2 && pool==(char *)&pools[mutate && i>0]);
    assert(memcmp(&state,&reference,sizeof(state))==0);
    if(mutate) {
        state.current=half((unsigned short)state.current+0x8001u);
        state.target=-777; reference=state;
        g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1]; g_FieldFxCurrentAge=7; instance.frame=65535;
    }
    if(i==failAt) return 0;
    expected[i].position=&g_RoomM350HandPoints[i]; expected[i].scale=state.current;
    return &particles[i];
}
static void reset(int target,int current) {
    calls=setups=0; state.target=target; state.current=current; reference=state;
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0]; g_FieldFxOwner=(FieldFxObjectPrefix *)&actor;
    memset(particles,0x5A,sizeof(particles)); memcpy(expected,particles,sizeof(particles));
}
static void run(int target,int current,unsigned frame,int timer,int failure,int mutation) {
    int result=0,emitting=0;
    reset(target,current); instance.frame=frame; g_FieldFxCurrentAge=timer; failAt=failure; mutate=mutation;
    if(!target && current<=0) result=2;
    else {
        if(target && frame>=41) reference.target=0;
        if(!(timer&7)) {
            int delta=reference.current<reference.target ? 4096 : reference.current>reference.target ? -4096:0;
            reference.current=half((uint32_t)reference.current+delta);
            emitting=reference.current!=0;
        }
    }
    /* A zero target must not dereference the actor, even while fading. */
    if(!target) g_FieldFxOwner=0;
    assert(RoomM350_HandGlowEmitter(1,&state)==result && setups==0);
    assert(calls==(emitting ? (failure<2 ? failure+1:2):0));
    assert(memcmp(&state,&reference,sizeof(state))==0);
    assert(memcmp(particles,expected,sizeof(particles))==0);
}
int main(void) {
    int i,k,f,m,phase,event,targets[]={0,1,-1,16384,32767,-32768};
    emitters[0].pool=&pools[0]; emitters[1].pool=&pools[1]; actor.instance=&instance;
    for(i=0;i<65536;i++) for(k=0;k<6;k++) for(f=0;f<3;f++) for(m=0;m<2;m++)
        run(targets[k],half(i),40,8,f,m);
    for(i=0;i<65536;i++) {
        run(16384,4096,i,8,2,0);
        run(half(i),half(i),40,8,2,0);
        run(half(i),-32768,40,0,2,0);
    }
    for(phase=0;phase<8;phase++) {
        run(16384,0,40,phase,2,0); run(16384,4096,41,phase,2,0);
        run(0,0,65535,phase,2,0); run(0,-1,65535,phase,2,0);
        run(16384,32767,40,INT_MIN+phase,2,0);
    }
    for(setupResult=-2;setupResult<=2;setupResult++) {
        reset(-123,456); assert(RoomM350_HandGlowEmitter(0,&state)==setupResult && setups==1 && calls==0);
    }
    for(i=0;i<65536;i++) {
        reset(-123,456); g_FieldFxTpageSlot2=i; g_FieldFxTpageTable[i]=65535-i;
        memset(&g_FieldFxSpriteParams,0x9D,sizeof(g_FieldFxSpriteParams));
        assert(RoomM350_HandGlowEmitter(2,&state)==0 && !calls && !setups);
        assert(g_FieldFxSpriteParams.palette==2 && g_FieldFxSpriteParams.parameter06==0);
        assert(g_FieldFxSpriteParams.parameter00==16 && g_FieldFxSpriteParams.parameter02==1);
        assert(g_FieldFxSpriteParams.extent_x==16 && g_FieldFxSpriteParams.extent_y==16);
        assert(g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0 && g_FieldFxSpriteParams.tpage==65535-i);
        assert(state.target==-123 && state.current==456);
    }
    for(event=-3;event<=5;event++) if(event<0 || event>2) {
        reset(-123,456); assert(RoomM350_HandGlowEmitter(event,&state)==0 && !calls && !setups);
        assert(state.target==-123 && state.current==456);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory)/'ramp'
            result = subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing', '-x', 'c', '-', '-o', str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)
