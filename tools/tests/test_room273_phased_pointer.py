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
FUNCTION = 'RoomM273_HandRingBandCallback'


class PhasedPointerSprite(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Effect)==4 && sizeof(GteRotation)==8 &&
    (unsigned long)&((GteRotation *)0)->y==2 &&
    (unsigned long)&((GteRotation *)0)->z==4 &&
    (unsigned long)&((GteRotation *)0)->flags==6 &&
    (unsigned long)&((Object *)0)->value==58 &&
    (unsigned long)&((RoomM273EffectStateContext *)0)->state==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n#include <string.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
int g_FieldFxCurrentAge;
GteSineWord rcossin_tbl[4096];
FieldFxObjectPrefix *g_FieldFxOwner;
unsigned char D_8019ACDC[4],D_8019ACE0[4];
static Object object;
static RoomM273EffectStateContext context;
static Effect effect;
static int position,draws,wantSize,wantShade;
static unsigned short wantPhase;
void FieldFx_DrawRingBand(struct GteShortVector *p,int a,int b,int c,
                  struct GteRotation *params,int w,int h,
                  struct RenderColor *color,struct RenderColor *other,
                  int shade,int flag) {
    assert((void *)p==&position && a==32 && b==96 && c==8 && draws++==0);
    assert(params->x==0 && (unsigned short)params->y==object.value);
    assert((unsigned short)params->z==wantPhase && params->flags==1);
    assert(w==wantSize && h==wantSize && shade==wantShade && flag==1);
    assert((void *)color==D_8019ACDC && (void *)other==D_8019ACE0);
}
int main(void) {
    int frames[]={-257,-1,0,1,15,31,32,63,64,255,256,65536};
    int times[]={INT_MIN,-1,0,15,16,INT_MAX};
    unsigned short values[]={0,1,32767,32768,65535};
    int f,j,k,i;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&context;
    context.state.mode=(RoomM273EffectModeState *)&object; effect.position=&position;
    memset(object.unknown,0xA5,sizeof(object.unknown));
    for(i=0;i<6;++i) {
        g_FieldFxCurrentAge=times[i];
        assert(RoomM273_HandRingBandCallback(1,&effect)==(times[i]>=16));
    }
    assert(draws==0);
    for(f=0;f<12;++f) for(j=0;j<5;++j) for(k=0;k<5;++k) {
        unsigned int so=((unsigned int)frames[f]<<8)&0x3F00;
        unsigned int bo=((unsigned int)frames[f]<<9)&0x3E00;
        short sample=(short)values[j];
        g_FieldFxCurrentAge=frames[f]+1;
        rcossin_tbl[bo/4].word=0x12340000|values[k];
        memcpy((char *)rcossin_tbl+so,&sample,2);
        wantSize=sample*2+4096;
        wantShade=(short)rcossin_tbl[bo/4].word>>5;
        wantPhase=(unsigned int)frames[f]<<8;
        object.value=values[k]; draws=0;
        assert(RoomM273_HandRingBandCallback(2,&effect)==0 && draws==1);
        assert(effect.position==&position && object.value==values[k]);
        for(i=0;i<58;++i) assert(object.unknown[i]==0xA5);
    }
    assert(RoomM273_HandRingBandCallback(0,&effect)==0 && RoomM273_HandRingBandCallback(-1,&effect)==0);
    assert(RoomM273_HandRingBandCallback(INT_MAX,&effect)==0 && draws==1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing',
                            str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
