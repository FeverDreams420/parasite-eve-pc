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
FUNCTION = 'RoomM273_SwayGroundRingEmitter'


class PairedEmitter(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Entry)==8 &&
    (unsigned long)&((Entry *)0)->emitter.flags==6 &&
    (unsigned long)&((EmitterState *)0)->stopped==30 &&
    (unsigned long)&((RoomM273PoolContext *)0)->pool==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
RenderEffectParameters g_FieldFxSpriteParams;
#define memset __builtin_memset
struct SceneAssetBlob *D_800B0E64[1];
void *D_8019AF6C;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
EmitterState D_8019AF84;
static RoomM273PoolContext context;
static int asset,pool,stage,initResult,allocs,failAt;
static Entry outputs[2];
void *Asset_FindTable08ByU32Key(void *owner,int id) {
    assert(stage++==0 && owner==&context && (unsigned int)id==0xC5541704);
    return &asset;
}
void FieldFx_ModelScrollUv(union FieldTexturedModel *handle,u8 a,u8 b) {
    assert(stage++==1 && (void *)handle==&asset && D_8019AF6C==&asset && !a && !b);
    context.pool=&pool;
}
int RoomM273_SwayGroundRingCallback(int mode,GteShortVector *p) { assert(0); return 0; }
int FieldFx_InitCallbackList(char *p,int size,int count,FieldFxCallbackListCallback callback) {
    assert(stage++==2 && p==(char *)&pool && size==8 && count==4);
    assert(callback==(FieldFxCallbackListCallback)RoomM273_SwayGroundRingCallback);
    return initResult;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int index=allocs++;
    assert(p==(char *)&pool && index<2);
    if(index==failAt) return 0;
    return &outputs[index];
}
int main(void) {
    int mask,fail,i,next,successful,expectedCalls,stopped;
    D_800B0E64[0]=(struct SceneAssetBlob *)&context;
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&context;
    for(initResult=-1;initResult<=1;++initResult) {
        stage=0; context.pool=0;
        assert(RoomM273_SwayGroundRingEmitter(0)==initResult && stage==3);
    }
    for(mask=0;mask<4;++mask) for(fail=-1;fail<2;++fail) {
        memset(&D_8019AF84,0xA6,sizeof(D_8019AF84));
        memset(outputs,0x55,sizeof(outputs));
        D_8019AF84.stopped=0;
        for(i=0;i<2;++i) {
            D_8019AF84.entries[i].emitter.x=65535-i;
            D_8019AF84.entries[i].emitter.y=32768+i;
            D_8019AF84.entries[i].emitter.z=123+i;
            D_8019AF84.entries[i].emitter.flags=0xA5A0 | ((mask>>i)&1);
        }
        allocs=0; failAt=fail;
        assert(RoomM273_SwayGroundRingEmitter(1)==0);
        next=successful=expectedCalls=stopped=0;
        for(i=0;i<2;++i) {
            int active=(mask>>i)&1;
            if(active && !stopped) {
                ++expectedCalls;
                if(next==fail) stopped=1;
                else {
                    Entry *out=&outputs[next];
                    assert(out->emitter.x==65535-i && out->emitter.y==32768+i && out->emitter.z==123+i);
                    assert(out->emitter.flags==0);
                    ++successful;
                }
                ++next;
            }
            assert(D_8019AF84.entries[i].emitter.flags==
                   (unsigned short)(0xA5A0 | (active && stopped)));
        }
        assert(allocs==expectedCalls);
        for(i=successful;i<2;++i) assert(outputs[i].emitter.flags==0x5555);
        for(i=0;i<14;++i) assert(D_8019AF84.unknown[i]==0xA6);
        D_8019AF84.stopped=255;
        assert(RoomM273_SwayGroundRingEmitter(1)==2 && allocs==expectedCalls);
    }
    g_FieldFxSpriteParams.parameter0A=-1; g_FieldFxSpriteParams.depth=123;
    assert(RoomM273_SwayGroundRingEmitter(2)==0 && !g_FieldFxSpriteParams.parameter0A && !g_FieldFxSpriteParams.depth);
    assert(RoomM273_SwayGroundRingEmitter(-1)==0 && RoomM273_SwayGroundRingEmitter(3)==0);
    assert(stage==3);
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
