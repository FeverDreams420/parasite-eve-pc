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
FUNCTION = 'RoomM273_HandTrailLandingEmitter'


class PairedBatch(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef RoomM273PulseEmitterView Effect;
typedef RoomM273PairedPulseBatch Batch;
typedef char layout[sizeof(Effect)==12 &&
    (unsigned long)&((Effect *)0)->size==6 &&
    (unsigned long)&((Effect *)0)->phase==8 &&
    (unsigned long)&((Effect *)0)->unknown10==10 &&
    (unsigned long)&((RoomM273PoolContext *)0)->pool==8 &&
    (unsigned long)&((Batch *)0)->y==16 &&
    (unsigned long)&((Batch *)0)->z==32 &&
    (unsigned long)&((Batch *)0)->count==48 &&
    (unsigned long)&((Batch *)0)->stopped==64 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        # These linker symbols alias members of a single PSX allocation.
        source = host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM))
        for old, new in (('extern unsigned char D_8019AEF8;',
                          'static RoomM273PairedPulseBatch storage;\n'
                          '#define D_8019AEF8 storage.stopped'),
                         ('extern short D_8019AEE8;',
                          '#define D_8019AEE8 storage.count')):
            self.assertIn(old, source)
            source = source.replace(old, new)
        source = '#include <assert.h>\n#include <string.h>\n#include <limits.h>\n' + source + r'''
typedef RoomM273PulseEmitterView Effect;
static RoomM273PoolContext context;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&context;
static unsigned char primary[128],otherPrimary[128],secondary[128],otherSecondary[128];
void *g_RoomM273HandTrailLandingFanPool=secondary;
unsigned short g_FieldFxTpageSlot3,g_FieldFxTpageTable[8];
RenderEffectParameters g_FieldFxSpriteParams;
static Effect records[8],*links[8];
static int init,attempts,aux,updates,renders,failAt,mutateCount;
int RoomM273_HandTrailLandingPulseCallback(int mode, RoomM273PulseInput *input) { assert(0); return 0; }
int RoomM273_HandTrailLandingFanCallback(void) { assert(0); return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback callback) {
    assert(init++==0 && p==(char *)primary && stride==12 && count==10 &&
           callback==(FieldFxCallbackListCallback)RoomM273_HandTrailLandingPulseCallback);
    return 120;
}
int FieldFx_AllocCallbackList(void *p,int offset,int stride,int count,void *callback) {
    assert(init++==1 && p==&g_RoomM273HandTrailLandingFanPool && offset==120 && stride==4 && count==10 &&
           callback==(void *)RoomM273_HandTrailLandingFanCallback);
    *(void **)p=secondary; return 40;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int n;
    if(p==secondary || p==otherSecondary) {
        n=aux++;
        assert(n<8 && attempts==n+1 && p==(n==0 ? secondary:otherSecondary));
        assert(records[n].x==0xA5A5 && records[n].phase==0xA5A5 && links[n]==0);
        storage.x[n]=65535-n; storage.y[n]=n; storage.z[n]=32768+n;
        if(mutateCount && n==0) storage.count=3;
        g_RoomM273HandTrailLandingFanPool=otherSecondary;
        return &links[n];
    }
    n=attempts++;
    assert(n<8 && p==(n==0 ? primary:otherPrimary));
    context.pool=otherPrimary;
    return n==failAt ? 0:&records[n];
}
int FieldFx_UpdateCallbackList(void *p) {
    assert(p==g_RoomM273HandTrailLandingFanPool && updates++==0);
    if(!storage.stopped) assert(storage.count==0);
    return 0;
}
int FieldFx_DrawCallbackList(char *p) {
    assert(p==g_RoomM273HandTrailLandingFanPool && renders++==0);
    g_FieldFxTpageSlot3=3; g_FieldFxTpageTable[3]=0xFEDC;
    return 0;
}
int main(void) {
    int counts[]={-32768,-1,0,1,3,8};
    int stops[]={0,1,255};
    int s,c,f,n,successes,active;
    context.pool=primary;
    assert(RoomM273_HandTrailLandingEmitter(0)==160 && init==2 && g_RoomM273HandTrailLandingFanPool==secondary);
    for(s=0;s<3;++s) for(c=0;c<6;++c) for(f=0;f<=8;++f) {
        storage.stopped=stops[s]; storage.count=counts[c];
        context.pool=primary; g_RoomM273HandTrailLandingFanPool=secondary;
        attempts=aux=updates=renders=0; failAt=f;
        memset(records,0xA5,sizeof(records)); memset(links,0,sizeof(links));
        active=!stops[s] && counts[c]>0;
        successes=active ? (f<counts[c] ? f:counts[c]):0;
        assert(RoomM273_HandTrailLandingEmitter(1)==(stops[s] ? 2:0));
        assert(attempts==successes+(active && f<counts[c]));
        assert(aux==successes && updates==1 && renders==0);
        assert(storage.count==(stops[s] ? counts[c]:0));
        assert(storage.stopped==stops[s]);
        for(n=0;n<8;++n) {
            assert(records[n].unknown10==0xA5A5);
            if(n<successes) {
                assert(records[n].x==65535-n && records[n].y==(unsigned short)(n-128));
                assert(records[n].z==32768+n && records[n].size==128 && records[n].phase==0);
                assert(links[n]==&records[n]);
            } else assert(records[n].x==0xA5A5 && records[n].phase==0xA5A5 && links[n]==0);
        }
    }
    storage.stopped=0; storage.count=1; mutateCount=1; failAt=8;
    context.pool=primary; g_RoomM273HandTrailLandingFanPool=secondary;
    attempts=aux=updates=0;
    memset(records,0xA5,sizeof(records)); memset(links,0,sizeof(links));
    assert(RoomM273_HandTrailLandingEmitter(1)==0 && attempts==3 && aux==3 && storage.count==0);
    for(s=0;s<3;++s) {
        storage.stopped=stops[s]; renders=0;
        g_FieldFxTpageSlot3=0; g_FieldFxTpageTable[0]=0;
        assert(RoomM273_HandTrailLandingEmitter(2)==0 && renders==1);
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2 && g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
        assert(g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==0 && g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0 && g_FieldFxSpriteParams.tpage==0xFEDC);
    }
    attempts=updates=renders=0;
    assert(RoomM273_HandTrailLandingEmitter(-1)==0 && RoomM273_HandTrailLandingEmitter(3)==0);
    assert(RoomM273_HandTrailLandingEmitter(INT_MIN)==0 && RoomM273_HandTrailLandingEmitter(INT_MAX)==0);
    assert(!attempts && !updates && !renders && init==2);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
