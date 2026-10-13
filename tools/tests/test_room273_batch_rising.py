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
FUNCTION = 'RoomM273_SweepSplashEmitter'


class BatchRisingEmitter(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef RoomM273RisingBatchBuffer Batch;
typedef RoomM273PoolContext Context;
typedef char layout[sizeof(GteShortVector)==8 &&
    (unsigned long)&((Batch *)0)->y==24 &&
    (unsigned long)&((Batch *)0)->z==48 &&
    (unsigned long)&((Batch *)0)->count==72 &&
    (unsigned long)&((Batch *)0)->stopped==93 &&
    (unsigned long)&((Context *)0)->pool==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM))
        source = '#include <assert.h>\n#include <string.h>\n' + source + r'''
typedef RoomM273RisingBatchBuffer Batch;
typedef GteShortVector Vector;
RoomM273RisingBatchBuffer D_8019AF0C;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
unsigned short g_FieldFxTpageSlot3,g_FieldFxTpageTable[65536];
RenderEffectParameters g_FieldFxSpriteParams;
static RoomM273PoolContext context;
static Vector outputs[12];
static int pool,allocs,failAt,initCalls,initResult,mutate;
int RoomM273_SweepSplashCallback(int mode, GteShortVector *p) { assert(0); return 0; }
int FieldFx_InitCallbackList(char *p,int size,int count,FieldFxCallbackListCallback callback) {
    assert(p==(char *)&pool && size==8 && count==40 &&
           callback==(FieldFxCallbackListCallback)RoomM273_SweepSplashCallback);
    ++initCalls; return initResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int i=allocs++;
    assert(p==(char *)&pool && i<12);
    if(i==failAt) return 0;
    D_8019AF0C.x[i]=65535-i;
    D_8019AF0C.y[i]=i;
    D_8019AF0C.z[i]=32768+i;
    if(mutate) D_8019AF0C.count=1;
    return &outputs[i];
}
int main(void) {
    short counts[]={-32768,-1,0,1,2,12};
    unsigned short indices[]={0,1,32768,65535};
    int stop,c,i,calls,success,eligible;
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&context; context.pool=&pool;
    for(initResult=-1;initResult<=1;++initResult)
        assert(RoomM273_SweepSplashEmitter(0)==initResult);
    for(stop=0;stop<2;++stop) for(c=0;c<6;++c)
        for(failAt=-1;failAt<12;++failAt) {
            memset(&D_8019AF0C,0xA5,sizeof(D_8019AF0C));
            memset(outputs,0x55,sizeof(outputs));
            D_8019AF0C.count=counts[c]; D_8019AF0C.stopped=stop ?255:0;
            allocs=0; mutate=0;
            assert(RoomM273_SweepSplashEmitter(1)==(stop ?2:0));
            eligible=!stop && counts[c]>0;
            success=eligible ? (failAt>=0 && failAt<counts[c] ?failAt:counts[c]):0;
            calls=success+(eligible && failAt==success && failAt<counts[c]);
            assert(allocs==calls && D_8019AF0C.count==(stop ?counts[c]:0));
            for(i=0;i<12;++i) {
                if(i<success) {
                    assert((unsigned short)outputs[i].x==65535-i && (unsigned short)outputs[i].y==(unsigned short)(i-128));
                    assert((unsigned short)outputs[i].z==32768+i && outputs[i].pad==0);
                } else assert((unsigned short)outputs[i].x==0x5555 && (unsigned short)outputs[i].pad==0x5555);
            }
            assert(D_8019AF0C.stopped==(stop ?255:0));
            for(i=0;i<19;++i) assert(D_8019AF0C.reserved_4A[i]==0xA5);
        }
    D_8019AF0C.stopped=0; D_8019AF0C.count=12;
    allocs=0; failAt=-1; mutate=1;
    assert(RoomM273_SweepSplashEmitter(1)==0 && allocs==1 && D_8019AF0C.count==0);
    for(i=0;i<4;++i) {
        g_FieldFxTpageSlot3=indices[i]; g_FieldFxTpageTable[indices[i]]=65535-i;
        g_FieldFxSpriteParams.parameter06=g_FieldFxSpriteParams.parameter0A=g_FieldFxSpriteParams.depth=123;
        assert(RoomM273_SweepSplashEmitter(2)==0);
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2 && g_FieldFxSpriteParams.palette==3);
        assert(g_FieldFxSpriteParams.parameter06==0 && g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0);
        assert(g_FieldFxSpriteParams.tpage==65535-i && g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
    }
    assert(RoomM273_SweepSplashEmitter(-1)==0 && RoomM273_SweepSplashEmitter(3)==0 && initCalls==3 && allocs==1);
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
