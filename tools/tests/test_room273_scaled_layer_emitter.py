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
FUNCTION = 'RoomM273_GrabPlacementScaledFanEmitter'


class ScaledLayerEmitter(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Effect)==12 &&
    (unsigned long)&((Effect *)0)->position.x==0 &&
    (unsigned long)&((Effect *)0)->position.y==2 &&
    (unsigned long)&((Effect *)0)->position.z==4 &&
    (unsigned long)&((Effect *)0)->position.pad==6 &&
    (unsigned long)&((Effect *)0)->parameter==8 &&
    (unsigned long)&((RoomM273PoolContext *)0)->pool==8 &&
    (unsigned long)&((RoomM273EffectStateContext *)0)->state==8 &&
    (unsigned long)&((RoomM273ScaledLayerOwner *)0)->transform==0x238 &&
    (unsigned long)&((Transform *)0)->x==0x594 &&
    (unsigned long)&((Transform *)0)->y==0x598 &&
    (unsigned long)&((Transform *)0)->z==0x59C ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <string.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
static RoomM273PoolContext context;
static RoomM273EffectStateContext state,replacement;
static RoomM273ScaledLayerOwner owner;
static Transform transform;
static Effect output;
static unsigned char pool[128];
FieldFxTaskSlot *g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&context;
FieldFxObjectPrefix *g_FieldFxOwner=(FieldFxObjectPrefix *)&state;
int g_FieldFxCurrentAge;
short D_800966EE[8192];
GteSineWord rcossin_tbl[4096];
static int init,allocations,fail,newCounter;
int RoomM273_GrabPlacementScaledFanCallback(int mode, Effect *input) { assert(0); return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback callback) {
    assert(init++==0 && p==(char *)pool && stride==12 && count==9 &&
           callback==(FieldFxCallbackListCallback)RoomM273_GrabPlacementScaledFanCallback);
    return 140;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    assert(p==(char *)pool && allocations++==0);
    g_FieldFxOwner=(FieldFxObjectPrefix *)&replacement; g_FieldFxCurrentAge=newCounter;
    return fail ? 0 : &output;
}
int main(void) {
    int counters[]={-12345,-1,0,1,7,11,12,12345};
    int samples[]={-32768,-4096,-103,-102,-1,0,1,102,103,4096,32767};
    int i,j,k,f;
    context.pool=pool; replacement.state.scaled_layer=&owner; owner.transform=&transform;
    transform.x=65535; transform.y=-65537; transform.z=32768;
    assert(RoomM273_GrabPlacementScaledFanEmitter(0)==140 && init==1);
    for(i=0;i<8;++i) for(j=0;j<11;++j) for(k=0;k<11;++k) for(f=0;f<2;++f) {
        int sampleIndex=(int)(((long long)counters[i]*1024/12)&0xFFF)*2;
        Effect before;
        newCounter=counters[i]; g_FieldFxCurrentAge=0; g_FieldFxOwner=(FieldFxObjectPrefix *)&state; state.state.scaled_layer=0;
        memset(rcossin_tbl,0x55,sizeof(rcossin_tbl));
        memset(D_800966EE,0xAA,sizeof(D_800966EE));
        ((short *)rcossin_tbl)[sampleIndex]=samples[j]; D_800966EE[sampleIndex]=samples[k];
        memset(&output,0xA5,sizeof(output)); before=output; fail=f; allocations=0;
        assert(RoomM273_GrabPlacementScaledFanEmitter(1)==0 && allocations==1);
        if(f) assert(memcmp(&output,&before,sizeof(output))==0);
        else {
            assert(output.position.x==-1 && output.position.y==-1 && output.position.z==-32768);
            assert(output.position.pad==samples[k] && output.parameter[0]==40);
            assert(output.parameter[1]==(unsigned char)((long long)samples[j]*40/4096));
            assert(output.parameter[2]==0 && output.parameter[3]==0xA5);
        }
    }
    fail=1;
    for(i=-2;i<=13;++i) {
        allocations=0; g_FieldFxCurrentAge=i;
        assert(RoomM273_GrabPlacementScaledFanEmitter(1)==(i>=12 ? 2:0) && allocations==(i<12));
    }
    allocations=0; g_FieldFxCurrentAge=INT_MAX;
    assert(RoomM273_GrabPlacementScaledFanEmitter(1)==2 && allocations==0);
    assert(RoomM273_GrabPlacementScaledFanEmitter(2)==0 && RoomM273_GrabPlacementScaledFanEmitter(3)==0);
    assert(RoomM273_GrabPlacementScaledFanEmitter(INT_MIN)==0 && RoomM273_GrabPlacementScaledFanEmitter(INT_MAX)==0 && allocations==0);
    assert(init==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
