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
FUNCTION = 'RoomM273_SweepRisingEmitter'


class TimedRisingEmitter(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Particle)==8 && __alignof__(Particle)==2 &&
    (unsigned long)&((Particle *)0)->y==2 &&
    (unsigned long)&((Particle *)0)->z==4 &&
    (unsigned long)&((Particle *)0)->speed==6 &&
    (unsigned long)&((RoomM273PoolContext *)0)->pool==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
unsigned char D_8019AF6A,D_8019AF69;
short D_8019AE84;
RoomM273SweepPoint g_RoomM273SweepPoint;
volatile unsigned short D_8019AEFE,D_8019AF00;
unsigned short g_FieldFxTpageSlot2,g_FieldFxTpageTable[65536];
RenderEffectParameters g_FieldFxSpriteParams;
static RoomM273PoolContext context;
static Particle output;
static int pool,initCalls,initResult,allocs,fail;
static short expectedCounter;
static unsigned short newY;
int RoomM273_SweepRisingCallback(int mode,Particle *p) { assert(0); return 0; }
int FieldFx_InitCallbackList(char *p,int size,int count,FieldFxCallbackListCallback callback) {
    assert(p==(char *)&pool && size==8 && count==12 &&
           callback==(FieldFxCallbackListCallback)RoomM273_SweepRisingCallback);
    assert(D_8019AE84==0);
    ++initCalls;
    return initResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    assert(p==(char *)&pool && allocs++==0 && D_8019AE84==expectedCounter);
    if(fail) return 0;
    g_RoomM273SweepPoint.x=65535; D_8019AEFE=newY; D_8019AF00=32768;
    return &output;
}
int main(void) {
    int stop,emit,i,j,c,calls,success;
    unsigned char flags[]={0,1,255};
    short counters[]={-32768,-1,0,1,2,32767};
    unsigned short values[]={0,255,256,32768,65535};
    unsigned short indices[]={0,1,32768,65535};
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&context; context.pool=&pool;
    for(initResult=-1;initResult<=1;++initResult) {
        D_8019AE84=99;
        assert(RoomM273_SweepRisingEmitter(0)==initResult);
    }
    for(stop=0;stop<3;++stop) for(emit=0;emit<3;++emit)
        for(c=0;c<6;++c) for(j=0;j<5;++j) for(fail=0;fail<2;++fail) {
            output.x=11; output.y=22; output.z=33; output.speed=44;
            g_RoomM273SweepPoint.x=1; D_8019AEFE=2; D_8019AF00=3; newY=values[j];
            D_8019AE84=counters[c];
            expectedCounter=(short)(counters[c]-1);
            D_8019AF6A=flags[emit]; D_8019AF69=flags[stop]; allocs=0;
            assert(RoomM273_SweepRisingEmitter(1)==(stop ?2:0));
            calls=!stop && emit && counters[c]<=0;
            success=calls && !fail;
            assert(allocs==calls);
            assert(D_8019AE84==(!stop && emit ? (success ?2:expectedCounter):counters[c]));
            assert((unsigned short)output.x==(success ?65535:11));
            assert(output.y==(success ?(unsigned short)(newY-256):22));
            assert((unsigned short)output.z==(success ?32768:33) && output.speed==(success ?0:44));
            assert(D_8019AF6A==flags[emit] && D_8019AF69==flags[stop]);
        }
    calls=allocs; D_8019AE84=91;
    for(i=0;i<4;++i) {
        g_FieldFxTpageSlot2=indices[i]; g_FieldFxTpageTable[indices[i]]=65535-i;
        g_FieldFxSpriteParams.parameter00=g_FieldFxSpriteParams.parameter02=g_FieldFxSpriteParams.palette=g_FieldFxSpriteParams.parameter06=0xAAAA;
        g_FieldFxSpriteParams.tpage=g_FieldFxSpriteParams.parameter0A=g_FieldFxSpriteParams.depth=0xAAAA;
        g_FieldFxSpriteParams.extent_x=g_FieldFxSpriteParams.extent_y=0xAAAA;
        assert(RoomM273_SweepRisingEmitter(2)==0 && allocs==calls && D_8019AE84==91);
        assert(g_FieldFxSpriteParams.parameter00==16 && g_FieldFxSpriteParams.parameter02==1 && g_FieldFxSpriteParams.palette==2);
        assert(g_FieldFxSpriteParams.parameter06==0 && g_FieldFxSpriteParams.tpage==65535-i && g_FieldFxSpriteParams.parameter0A==0);
        assert(g_FieldFxSpriteParams.depth==0 && g_FieldFxSpriteParams.extent_x==16 && g_FieldFxSpriteParams.extent_y==16);
    }
    assert(RoomM273_SweepRisingEmitter(-1)==0 && RoomM273_SweepRisingEmitter(3)==0);
    assert(RoomM273_SweepRisingEmitter(INT_MIN)==0 && RoomM273_SweepRisingEmitter(INT_MAX)==0);
    assert(initCalls==3 && allocs==calls && D_8019AE84==91);
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
