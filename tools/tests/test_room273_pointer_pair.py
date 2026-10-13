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
FUNCTION = 'RoomM273_SwayPointerEmitter'


class PeriodicPointerPair(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Position)==8 && sizeof(Effect)==4 &&
    (unsigned long)&((Effect *)0)->position==0 &&
    (unsigned long)&((RoomM273PoolContext *)0)->pool==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
RoomM273SwayState g_RoomM273SwayState;
short D_8019AE88;
int g_FieldFxCurrentAge;
unsigned short g_FieldFxTpageSlot11,g_FieldFxTpageTable[65536];
RenderEffectParameters g_FieldFxSpriteParams;
static RoomM273PoolContext context;
static Effect outputs[2];
static Position sentinel;
static int pool,initCalls,initResult,allocs,failAt;
int RoomM273_SwayPointerCallback(int mode,Effect *p) { assert(0); return 0; }
int FieldFx_InitCallbackList(char *p,int size,int count,FieldFxCallbackListCallback callback) {
    assert(p==(char *)&pool && size==4 && count==6 &&
           callback==(FieldFxCallbackListCallback)RoomM273_SwayPointerCallback);
    assert(D_8019AE88==0); ++initCalls; return initResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int i=allocs++;
    assert(p==(char *)&pool && i<2);
    if(i==failAt) return 0;
    return &outputs[i];
}
int main(void) {
    short stops[]={-32768,-1,0,1,32767};
    short states[]={-32768,14,15,16,32767};
    short counts[]={-32768,24,25,32767};
    int frames[]={INT_MIN,-9,-8,-1,0,1,7,8,16,INT_MAX};
    unsigned short indices[]={0,1,32768,65535};
    int s,a,b,f,i,stopped,eligible,success,calls;
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&context; context.pool=&pool;
    for(initResult=-1;initResult<=1;++initResult) {
        D_8019AE88=99; assert(RoomM273_SwayPointerEmitter(0)==initResult);
    }
    for(s=0;s<5;++s) for(a=0;a<5;++a) for(b=0;b<4;++b)
        for(f=0;f<10;++f) for(failAt=-1;failAt<2;++failAt) {
            D_8019AE88=stops[s]; g_RoomM273SwayState.animation=states[a]; g_RoomM273SwayState.frame=counts[b];
            g_FieldFxCurrentAge=frames[f]; allocs=0;
            outputs[0].position=outputs[1].position=&sentinel;
            stopped=stops[s]!=0 || (states[a]==15 && counts[b]>=25);
            assert(RoomM273_SwayPointerEmitter(1)==(stopped ?2:0));
            assert(D_8019AE88==(stops[s] ?stops[s]:(stopped ?1:0)));
            eligible=!stopped && !(frames[f]&7);
            calls=eligible ?(failAt==0 ?1:2):0;
            success=eligible ?(failAt<0 ?2:failAt):0;
            assert(allocs==calls);
            for(i=0;i<2;++i) {
                assert(outputs[i].position==(i<success ?&g_RoomM273SwayState.points[i]:&sentinel));
                if(i<success) {
                    g_RoomM273SwayState.points[i].x=0xA500+i;
                    assert(((Position *)outputs[i].position)->x==(short)(0xA500+i));
                }
            }
        }
    calls=allocs; D_8019AE88=91;
    for(i=0;i<4;++i) {
        g_FieldFxTpageSlot11=indices[i]; g_FieldFxTpageTable[indices[i]]=65535-i;
        g_FieldFxSpriteParams.parameter00=g_FieldFxSpriteParams.parameter02=g_FieldFxSpriteParams.palette=g_FieldFxSpriteParams.parameter06=0xAAAA;
        g_FieldFxSpriteParams.tpage=g_FieldFxSpriteParams.parameter0A=g_FieldFxSpriteParams.depth=0xAAAA;
        g_FieldFxSpriteParams.extent_x=g_FieldFxSpriteParams.extent_y=0xAAAA;
        assert(RoomM273_SwayPointerEmitter(2)==0 && allocs==calls && D_8019AE88==91);
        assert(g_FieldFxSpriteParams.parameter00==64 && g_FieldFxSpriteParams.parameter02==4 && g_FieldFxSpriteParams.palette==3);
        assert(g_FieldFxSpriteParams.parameter06==1 && g_FieldFxSpriteParams.tpage==65535-i && g_FieldFxSpriteParams.parameter0A==0);
        assert(g_FieldFxSpriteParams.depth==32 && g_FieldFxSpriteParams.extent_x==64 && g_FieldFxSpriteParams.extent_y==64);
    }
    assert(RoomM273_SwayPointerEmitter(-1)==0 && RoomM273_SwayPointerEmitter(3)==0);
    assert(RoomM273_SwayPointerEmitter(INT_MIN)==0 && RoomM273_SwayPointerEmitter(INT_MAX)==0);
    assert(initCalls==3 && allocs==calls && D_8019AE88==91);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', str(path), '-o', str(exe)],
                           check=True)
            subprocess.run([str(exe)], check=True)
