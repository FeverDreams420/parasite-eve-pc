import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/tests'))
from csource import find_definition, host_unit, psx_compile  # noqa: E402

ROOM = 'src/overlays/room_m273_liberty'
FUNCTION = 'RoomM273_HandGlowEmitter'


class PeriodicDualPools(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Vector)==8 &&
    (unsigned long)&((RoomM273PoolContext *)0)->pool==8 &&
    (unsigned long)&((RoomM273EffectStateContext *)0)->state==8 &&
    (unsigned long)&((State *)0)->kind==14 &&
    (unsigned long)&((State *)0)->value22==22 &&
    (unsigned long)&((RoomM273BossState *)0)->done==92 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        # The two addresses are aliases within the same PSX allocation.
        source = host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM))
        source = re.sub(r'extern (?:u8|unsigned char) D_8019AEF8;', '', source)
        self.assertIn('extern RoomM273BossState g_RoomM273HandTrailState;', source)
        source = source.replace('extern RoomM273BossState g_RoomM273HandTrailState;',
            'extern RoomM273BossState g_RoomM273HandTrailState;\n#define D_8019AEF8 g_RoomM273HandTrailState.done\n', 1)
        source = '#include <assert.h>\n#include <string.h>\n#include <limits.h>\n' + source + r'''
RoomM273BossState g_RoomM273HandTrailState;
static RoomM273PoolContext context;
static RoomM273EffectStateContext stateContext;
static State state,replacement;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&context;
FieldFxObjectPrefix *g_FieldFxOwner=(FieldFxObjectPrefix *)&stateContext;
static unsigned char primary[64],otherPrimary[64],secondary[64],otherSecondary[64];
void *g_RoomM273HandRingBandPool=secondary;
unsigned short g_FieldFxTpageSlot11,g_FieldFxTpageTable[8];
RenderEffectParameters g_FieldFxSpriteParams;
static Vector *primaryLinks[2],*secondaryLinks[2];
static int init,attempts,aux,updates,renders,failPrimary,failSecondary;
int func_8019A398(void) { assert(0); return 0; }
int RoomM273_HandRingBandCallback(void) { assert(0); return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback callback) {
    assert(init++==0 && p==(char *)primary && stride==4 && count==6 &&
           callback==(FieldFxCallbackListCallback)func_8019A398);
    return 48;
}
int FieldFx_AllocCallbackList(void *p,int offset,int stride,int count,void *callback) {
    assert(init++==1 && p==&g_RoomM273HandRingBandPool && offset==48 && stride==4 && count==6 &&
           callback==(void *)RoomM273_HandRingBandCallback);
    *(void **)p=secondary; return 32;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int n;
    if(p==primary || p==otherPrimary) {
        n=attempts++;
        assert(n<2 && p==(n==0 ? primary:otherPrimary) && aux==0);
        context.pool=otherPrimary;
        stateContext.state.mode=&replacement; state.value22=100; g_RoomM273HandTrailState.done=1;
        return n==failPrimary ? 0:&primaryLinks[n];
    }
    n=aux++;
    assert(n<2 && p==(n==0 ? secondary:otherSecondary));
    assert(attempts==(failPrimary<2 ? failPrimary+1:2));
    g_RoomM273HandRingBandPool=otherSecondary;
    return n==failSecondary ? 0:&secondaryLinks[n];
}
int FieldFx_UpdateCallbackList(void *p) { assert(p==g_RoomM273HandRingBandPool && updates++==0); return 0; }
int FieldFx_DrawCallbackList(char *p) {
    assert(p==g_RoomM273HandRingBandPool && renders++==0);
    g_FieldFxTpageSlot11=3; g_FieldFxTpageTable[3]=0xFEDC;
    return 0;
}
int main(void) {
    int timers[]={-32768,-16,-8,-1,0,1,7,8,16,24,32,34,35,40,32767};
    int stops[]={0,1,255};
    int s,k,t,f,g,n,active,pSuccess,sSuccess;
    context.pool=primary;
    assert(RoomM273_HandGlowEmitter(0)==80 && init==2 && g_RoomM273HandRingBandPool==secondary);
    for(s=0;s<3;++s) for(k=10;k<=12;++k) for(t=0;t<15;++t)
        for(f=0;f<=2;++f) for(g=0;g<=2;++g) {
            g_RoomM273HandTrailState.done=stops[s]; state.kind=k; state.value22=timers[t];
            stateContext.state.mode=&state; context.pool=primary; g_RoomM273HandRingBandPool=secondary;
            attempts=aux=updates=renders=0; failPrimary=f; failSecondary=g;
            memset(primaryLinks,0,sizeof(primaryLinks)); memset(secondaryLinks,0,sizeof(secondaryLinks));
            active=!stops[s] && k==11 && timers[t]<35 && !(timers[t]&7);
            pSuccess=active ? f:0; sSuccess=active ? g:0;
            assert(RoomM273_HandGlowEmitter(1)==(stops[s] ? 2:0));
            assert(attempts==(active ? (f<2 ? f+1:2):0));
            assert(aux==(active ? (g<2 ? g+1:2):0));
            assert(updates==(!stops[s] && k==11) && renders==0);
            for(n=0;n<2;++n) {
                assert(primaryLinks[n]==(n<pSuccess ? &g_RoomM273HandTrailState.hands[n]:0));
                assert(secondaryLinks[n]==(n<sSuccess ? &g_RoomM273HandTrailState.hands[n]:0));
            }
        }
    for(s=0;s<3;++s) {
        g_RoomM273HandTrailState.done=stops[s]; renders=0; g_FieldFxTpageSlot11=0; g_FieldFxTpageTable[0]=0;
        assert(RoomM273_HandGlowEmitter(2)==0 && renders==1);
        assert(g_FieldFxSpriteParams.parameter00==32 && g_FieldFxSpriteParams.parameter02==2 && g_FieldFxSpriteParams.extent_x==32 && g_FieldFxSpriteParams.extent_y==32);
        assert(g_FieldFxSpriteParams.palette==3 && g_FieldFxSpriteParams.parameter06==1 && g_FieldFxSpriteParams.parameter0A==0 && g_FieldFxSpriteParams.depth==0 && g_FieldFxSpriteParams.tpage==0xFEDC);
    }
    attempts=updates=renders=0;
    assert(RoomM273_HandGlowEmitter(-1)==0 && RoomM273_HandGlowEmitter(3)==0);
    assert(RoomM273_HandGlowEmitter(INT_MIN)==0 && RoomM273_HandGlowEmitter(INT_MAX)==0);
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
