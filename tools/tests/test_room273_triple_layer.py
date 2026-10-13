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
FUNCTION = 'RoomM273_SweepFlashCallback'


class TripleLayer(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
typedef char layout[sizeof(Vector)==8 && sizeof(int)==4 &&
    (unsigned long)&((Vector *)0)->x==0 &&
    (unsigned long)&((Vector *)0)->y==2 &&
    (unsigned long)&((Vector *)0)->z==4 &&
    (unsigned long)&((Vector *)0)->pad==6 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_behavior(self):
        source = '#include <assert.h>\n#include <limits.h>\n' + host_unit(FUNCTION, path=find_definition(FUNCTION, ROOM)) + r'''
int g_FieldFxCurrentAge,g_FieldFxAltPaletteFlag;
GteSineWord rcossin_tbl[4096];
unsigned short g_FieldFxPaletteTable[8];
RoomFloorY g_RoomFloorY[1];
RenderEffectParameters g_FieldFxSpriteParams;
unsigned char g_RoomM273BlackColor[4],D_8019AD74[4],D_8019AD78[4];
static int stage,frame,sample1,sample2,expectedPalette,sampleIndex;
static Vector input,original,*positionPtr,*rotationPtr;
static int shade(int sample) { return sample>>21; }
u_short GetClut(int x,int y) {
    assert(stage++==0 && x==80 && y==expectedPalette);
    g_FieldFxCurrentAge=19;
    rcossin_tbl[sampleIndex].word=0x12345678;
    __builtin_memset(&input,0xEE,sizeof(input));
    return 0xFEDC;
}
void FieldFx_DrawShapeQuads(struct GteShortVector *p,struct GteRotation *rotation,int w,int h,int tile,
                  int clut,int flag,int color,struct RenderColor *extra) {
    Vector *r=(Vector *)rotation;
    assert(stage++==1 && p!=&input && r!=p);
    assert(__builtin_memcmp(p,&original,sizeof(original))==0);
    assert(r->x==0 && r->y==0 && r->z==(short)((unsigned int)frame<<8) && r->pad==0);
    assert(w==(short)sample1*3+2048 && h==w && tile==74);
    assert(clut==0xFEDC && flag==1 && color==shade(sample1) && !extra);
    positionPtr=p; rotationPtr=r;
    r->x=11; r->y=12; r->z=-257; r->pad=13;
    p->x=21; p->y=22; p->z=23; p->pad=24;
    rcossin_tbl[sampleIndex].word=sample2;
    g_FieldFxCurrentAge=27;
}
void FieldFx_DrawStarFan(struct GteShortVector *p,int w,int h,int count,struct GteRotation *rotation,
                  int sx,int sy,struct RenderColor *a,struct RenderColor *b,int color,int flag) {
    Vector *r=(Vector *)rotation;
    assert(p==positionPtr && r==rotationPtr && flag==1 && color==shade(sample2));
    if(stage++==2) {
        assert(w==512 && h==512 && count==16 && sx==(short)sample2+1024 && sy==sx);
        assert((void *)a==g_RoomM273BlackColor && (void *)b==D_8019AD74);
        assert(r->x==11 && r->y==12 && r->z==-129 && r->pad==13);
        assert(p->x==21 && p->y==22 && p->z==23 && p->pad==24);
        r->x=101; r->y=102; r->z=103; r->pad=104;
        p->x=31; p->y=32; p->z=33; p->pad=34;
        g_RoomFloorY->raw=0x8123;
        rcossin_tbl[sampleIndex].word=0;
        g_FieldFxCurrentAge=39;
    } else {
        assert(stage==4 && w==768 && h==768 && count==8 && sx==4096 && sy==4096);
        assert((void *)a==D_8019AD78 && (void *)b==g_RoomM273BlackColor);
        assert(r->x==1024 && r->y==0 && r->z==(short)((unsigned int)frame<<8) && r->pad==1);
        assert(p->x==31 && p->y==(short)0x8123 && p->z==33 && p->pad==34);
    }
}
int main(void) {
    int counters[]={0,1,2,15,16,64,128,257};
    unsigned int samples[]={0,0x7FFFFFFF,0x80008000,0xFFFF0001,0x0020FFFF};
    int i,j,k,kind,variant;
    for(i=0;i<8;++i) for(j=0;j<5;++j) for(k=0;k<5;++k)
        for(kind=3;kind<=4;++kind) for(variant=0;variant<2;++variant) {
            g_FieldFxCurrentAge=counters[i]; frame=counters[i]-1;
            sampleIndex=(((unsigned int)frame<<8)&0x3F00)/4;
            sample1=(int)samples[j]; sample2=(int)samples[k];
            __builtin_memset(rcossin_tbl,0x5A,sizeof(rcossin_tbl)); rcossin_tbl[sampleIndex].word=sample1;
            g_FieldFxSpriteParams.palette=kind; g_FieldFxAltPaletteFlag=variant;
            g_FieldFxPaletteTable[kind]=65534; expectedPalette=65534+(kind==4 && variant ? 4:0);
            input.x=-32768; input.y=32767; input.z=-1; input.pad=0x1234; original=input;
            stage=0;
            assert(RoomM273_SweepFlashCallback(2,&input)==0 && stage==4);
        }
    stage=0;
    for(i=-2;i<=17;++i) { g_FieldFxCurrentAge=i; assert(RoomM273_SweepFlashCallback(1,0)==(i>=16)); }
    g_FieldFxCurrentAge=INT_MIN; assert(RoomM273_SweepFlashCallback(1,0)==0);
    g_FieldFxCurrentAge=INT_MAX; assert(RoomM273_SweepFlashCallback(1,0)==1);
    assert(RoomM273_SweepFlashCallback(0,0)==0 && RoomM273_SweepFlashCallback(3,0)==0);
    assert(RoomM273_SweepFlashCallback(INT_MIN,0)==0 && RoomM273_SweepFlashCallback(INT_MAX,0)==0 && stage==0);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'behavior.c'
            path.write_text(source)
            exe = path.with_suffix('')
            subprocess.run(['cc', '-std=gnu11', '-O2', str(path), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
