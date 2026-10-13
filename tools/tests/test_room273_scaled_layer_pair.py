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
FUNCTION = 'RoomM273_GrabPlacementScaledFanCallback'


class ScaledLayerPair(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Vector)==8 && sizeof(Effect)==12 && sizeof(int)==4 &&
    (unsigned long)&((Vector *)0)->x==0 &&
    (unsigned long)&((Vector *)0)->y==2 &&
    (unsigned long)&((Vector *)0)->z==4 &&
    (unsigned long)&((Vector *)0)->pad==6 &&
    (unsigned long)&((Effect *)0)->parameter==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <string.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
int g_FieldFxCurrentAge;
GteSineWord rcossin_tbl[4096];
RoomFloorY g_RoomFloorY[1];
unsigned char g_RoomM273BlackColor[4];
static Effect input,original;
static Vector *savedPosition,*savedRotation;
static int calls,frame,wantedSize,wantedShade;
void FieldFx_DrawStarFan(struct GteShortVector *p,int w,int h,int count,struct GteRotation *rotation,int sx,int sy,
                  struct RenderColor *a,struct RenderColor *b,int shade,int flag) {
    Vector *r=(Vector *)rotation;
    assert(w==128 && h==128 && count==12 && flag==1);
    assert((void *)a==g_RoomM273BlackColor && (void *)b==input.parameter);
    assert(sx==wantedSize && sy==sx && shade==wantedShade);
    if(calls++==0) {
        assert(p!=&input.position && r!=p);
        assert(memcmp(p,&original.position,sizeof(Vector))==0);
        assert(r->x==1024 && r->y==0 && r->z==(short)(frame*170) && r->pad==-1);
        savedPosition=p; savedRotation=r;
        p->x=21; p->y=22; p->z=23; p->pad=24;
        r->x=31; r->y=32; r->z=33; r->pad=34;
        g_RoomFloorY->raw=0x8123;
        g_FieldFxCurrentAge=7;
        memset(rcossin_tbl,0x55,sizeof(rcossin_tbl));
        wantedSize=(short)(wantedSize*3/2); wantedShade>>=1;
    } else {
        assert(calls==2 && p==savedPosition && r==savedRotation);
        assert(p->x==21 && p->y==(short)0x8123 && p->z==23 && p->pad==24);
        assert(r->x==31 && r->y==32 && r->z==33 && r->pad==34);
        g_RoomFloorY->raw=0x7654;
    }
}
int main(void) {
    int counters[]={-1,0,1,7,8,32,257,32769,65537};
    int lows[]={-32768,-4096,-2049,-1,0,1,4096,32767};
    int highs[]={-32768,-33,-1,0,31,32,32767};
    int scales[]={-16384,-4096,-1,0,1,4096,16384};
    int i,j,k,l;
    for(i=0;i<9;++i) for(j=0;j<8;++j) for(k=0;k<7;++k) for(l=0;l<7;++l) {
        unsigned int bits=((unsigned int)(unsigned short)highs[k]<<16)|(unsigned short)lows[j];
        int index;
        g_FieldFxCurrentAge=counters[i]; frame=(short)(counters[i]-1);
        index=(((unsigned int)frame<<9)&0x3E00)/4;
        memset(rcossin_tbl,0,sizeof(rcossin_tbl));
        memcpy((char *)rcossin_tbl + index * 4,&bits,4);
        memset(&input,0xA5,sizeof(input));
        input.position.x=-32768; input.position.y=32767; input.position.z=-1;
        input.position.pad=scales[l]; original=input;
        wantedSize=(short)(((long long)lows[j]*2+4096)*scales[l]/4096);
        wantedShade=highs[k]>>5;
        calls=0;
        assert(RoomM273_GrabPlacementScaledFanCallback(2,&input)==0 && calls==2);
        assert(memcmp(&input,&original,sizeof(input))==0);
    }
    calls=0;
    for(i=-2;i<=9;++i) { g_FieldFxCurrentAge=i; assert(RoomM273_GrabPlacementScaledFanCallback(1,0)==(i>=8)); }
    g_FieldFxCurrentAge=INT_MIN; assert(RoomM273_GrabPlacementScaledFanCallback(1,0)==0);
    g_FieldFxCurrentAge=INT_MAX; assert(RoomM273_GrabPlacementScaledFanCallback(1,0)==1);
    assert(RoomM273_GrabPlacementScaledFanCallback(0,0)==0 && RoomM273_GrabPlacementScaledFanCallback(3,0)==0);
    assert(RoomM273_GrabPlacementScaledFanCallback(INT_MIN,0)==0 && RoomM273_GrabPlacementScaledFanCallback(INT_MAX,0)==0 && calls==0);
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
