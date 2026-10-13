import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import find_definition, host_unit, psx_compile  # noqa: E402

ROOM = 'src/overlays/room_m273_liberty'
FUNCTION = 'RoomM273_GrabPlayerStreakEmitter'


class ThresholdPair(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(RoomM273PulseSeed)==8 &&
    (unsigned long)&((RoomM273PulseSeed *)0)->angle==2 &&
    (unsigned long)&((RoomM273PoolContext *)0)->pool==8 &&
    (unsigned long)&((RoomM273EffectStateContext *)0)->state==8 &&
    (unsigned long)&((RoomM273EffectModeState *)0)->kind==14 &&
    (unsigned long)&((RoomM273EffectModeState *)0)->value22==22 &&
    (unsigned long)&((RoomM273EffectModeState *)0)->value26==26 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
#define memcmp __builtin_memcmp
typedef RoomM273EffectModeState State;
typedef RoomM273PulseSeed Seed;
static RoomM273PoolContext context;
static RoomM273EffectStateContext stateContext;
static State state;
static Seed output[2];
static unsigned char pool[32],secondPool[32];
FieldFxTaskSlot *g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&context;
FieldFxObjectPrefix *g_FieldFxOwner=(FieldFxObjectPrefix *)&stateContext;
unsigned char D_8019AE9A;
Seed D_8019AC20[2];
unsigned short g_FieldFxTpageSlot11,g_FieldFxTpageTable[8];
RenderEffectParameters g_FieldFxSpriteParams;
static int init,allocations,randomCalls,failAt,randomValue;
int RoomM273_GrabPlayerStreakCallback(int mode, GteRotation *rotation) { assert(0); return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback callback) {
    assert(init++==0 && p==(char *)pool && stride==8 && count==4 &&
           callback==(FieldFxCallbackListCallback)RoomM273_GrabPlayerStreakCallback);
    return 60;
}
int Inv_ScrambleGrid(void) { assert(randomCalls++==0 && !allocations); return randomValue; }
void *FieldFx_AcquireCallbackSlot(char *p) {
    int index=allocations++;
    assert(randomCalls==1 && index<2 && p==(char *)(index ? secondPool:pool));
    context.pool=secondPool;
    return index==failAt ? 0 : &output[index];
}
int main(void) {
    int values[]={-32768,-1,0,1,32767};
    int stop,kind,i,j,f,r,n;
    context.pool=pool; stateContext.state.mode=&state;
    assert(RoomM273_GrabPlayerStreakEmitter(0)==60 && init==1);
    for(stop=0;stop<2;++stop) for(kind=8;kind<=10;++kind)
        for(i=0;i<5;++i) for(j=0;j<5;++j) for(f=0;f<3;++f) for(r=0;r<2;++r) {
            State before;
            int wanted=!stop && kind==9 && values[i]>0 && values[j]<=0;
            memset(&state,0xA5,sizeof(state));
            state.kind=kind; state.value22=values[i]; state.value26=values[j];
            before=state; D_8019AE9A=stop ? 255:0; context.pool=pool;
            memset(D_8019AC20,0x39,sizeof(D_8019AC20));
            D_8019AC20[0].angle=65530; D_8019AC20[1].angle=32768;
            memset(output,0xA5,sizeof(output));
            allocations=randomCalls=0; failAt=f; randomValue=r ? 255:0;
            assert(RoomM273_GrabPlayerStreakEmitter(1)==(stop ? 2:0));
            assert(randomCalls==wanted && allocations==(wanted ? (f<2 ? f+1:2):0));
            assert(memcmp(&state,&before,sizeof(state))==0);
            for(n=0;n<2;++n) {
                Seed expected;
                if(wanted && n<f) {
                    expected=D_8019AC20[n]; expected.angle+=randomValue*16;
                } else memset(&expected,0xA5,sizeof(expected));
                assert(memcmp(&output[n],&expected,sizeof(expected))==0);
            }
        }
    for(i=0;i<8;++i) {
        g_FieldFxTpageSlot11=i; g_FieldFxTpageTable[i]=65535-i;
        assert(RoomM273_GrabPlayerStreakEmitter(2)==0);
        assert(g_FieldFxSpriteParams.parameter00==16 && g_FieldFxSpriteParams.parameter02==1 && g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==1);
        assert(g_FieldFxSpriteParams.tpage==65535-i && g_FieldFxSpriteParams.parameter0A==3 && g_FieldFxSpriteParams.depth==0);
        assert(g_FieldFxSpriteParams.extent_x==16 && g_FieldFxSpriteParams.extent_y==64);
    }
    allocations=randomCalls=0;
    assert(RoomM273_GrabPlayerStreakEmitter(-1)==0 && RoomM273_GrabPlayerStreakEmitter(3)==0);
    assert(RoomM273_GrabPlayerStreakEmitter(INT_MIN)==0 && RoomM273_GrabPlayerStreakEmitter(INT_MAX)==0);
    assert(!allocations && !randomCalls && init==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
