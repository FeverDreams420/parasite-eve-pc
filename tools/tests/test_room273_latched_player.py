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
FUNCTION = 'RoomM273_GrabPlayerPulseEmitter'


class LatchedPlayer(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(RoomM273SpritePoolEffect)==4 &&
    sizeof(RoomM273WorldPosition)==12 &&
    (unsigned long)&((RoomM273PoolContext *)0)->pool==8 &&
    (unsigned long)&((FieldActor *)0)->mode==14 &&
    (unsigned long)&((FieldActor *)0)->render_object.matrices==0x238 &&
    (unsigned long)&((RoomM273PlayerTransform *)0)->position==20 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <string.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
static RoomM273PoolContext context;
static FieldActor owner,replacement;
static RoomM273PlayerTransform transform;
static RoomM273SpritePoolEffect output;
static RoomM273WorldPosition sentinel;
static unsigned char pool[32];
FieldFxTaskSlot *g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&context;
FieldActor *g_PlayerEntity=&owner;
int g_FieldFxCurrentAge;
unsigned char D_8019AE5C;
unsigned short g_FieldFxTpageSlot11,g_FieldFxTpageTable[8];
RenderEffectParameters g_FieldFxSpriteParams;
static int init,allocations,fail;
int RoomM273_GrabPlayerPulseCallback(int mode, RoomM273SpritePoolEffect *state) { assert(0); return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback callback) {
    assert(init++==0 && p==(char *)pool && stride==4 && count==1 &&
           callback==(FieldFxCallbackListCallback)RoomM273_GrabPlayerPulseCallback);
    assert(D_8019AE5C==0); return 36;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    assert(p==(char *)pool && allocations++==0 && D_8019AE5C==0);
    g_PlayerEntity=&replacement;
    return fail ? 0 : &output;
}
int main(void) {
    int counters[]={INT_MIN,-1,0,19,20,INT_MAX};
    int latches[]={0,1,255};
    int i,l,k,f;
    context.pool=pool; replacement.render_object.matrices=(RenderMatrix *)&transform;
    D_8019AE5C=255;
    assert(RoomM273_GrabPlayerPulseEmitter(0)==36 && init==1);
    for(i=0;i<6;++i) for(l=0;l<3;++l) for(k=0;k<256;++k) for(f=0;f<2;++f) {
        int stopped=counters[i]>=20 || latches[l]!=0;
        int wanted=!stopped && k<4;
        g_FieldFxCurrentAge=counters[i]; D_8019AE5C=latches[l]; owner.mode=k;
        g_PlayerEntity=&owner; owner.render_object.matrices=0; allocations=0; fail=f;
        output.position=&sentinel;
        assert(RoomM273_GrabPlayerPulseEmitter(1)==(stopped ? 2:0) && allocations==wanted);
        assert(D_8019AE5C==(wanted && !f ? 1:latches[l]));
        assert(output.position==(wanted && !f ? &transform.position:&sentinel));
    }
    for(l=0;l<3;++l) for(i=0;i<8;++i) {
        int enabled=latches[l]!=0;
        D_8019AE5C=latches[l]; g_FieldFxTpageSlot11=i; g_FieldFxTpageTable[i]=65535-i;
        g_FieldFxSpriteParams.parameter00=g_FieldFxSpriteParams.parameter02=g_FieldFxSpriteParams.palette=g_FieldFxSpriteParams.parameter06=99;
        g_FieldFxSpriteParams.tpage=g_FieldFxSpriteParams.parameter0A=g_FieldFxSpriteParams.depth=g_FieldFxSpriteParams.extent_x=g_FieldFxSpriteParams.extent_y=99;
        assert(RoomM273_GrabPlayerPulseEmitter(2)==0);
        assert(g_FieldFxSpriteParams.parameter00==(enabled?32:99) && g_FieldFxSpriteParams.parameter02==(enabled?2:99));
        assert(g_FieldFxSpriteParams.palette==(enabled?3:99) && g_FieldFxSpriteParams.parameter06==(enabled?1:99));
        assert(g_FieldFxSpriteParams.tpage==(enabled?65535-i:99));
        assert(g_FieldFxSpriteParams.parameter0A==(enabled?0:99) && g_FieldFxSpriteParams.depth==(enabled?0:99));
        assert(g_FieldFxSpriteParams.extent_x==(enabled?32:99) && g_FieldFxSpriteParams.extent_y==(enabled?32:99));
        assert(D_8019AE5C==latches[l]);
    }
    allocations=0;
    assert(RoomM273_GrabPlayerPulseEmitter(-1)==0 && RoomM273_GrabPlayerPulseEmitter(3)==0);
    assert(RoomM273_GrabPlayerPulseEmitter(INT_MIN)==0 && RoomM273_GrabPlayerPulseEmitter(INT_MAX)==0);
    assert(allocations==0 && init==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
