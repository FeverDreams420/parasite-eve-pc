import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
ROOM = 'src/overlays/room_m350_cruiser'
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import find_definition, host_unit, psx_compile  # noqa: E402
FUNCTION = 'RoomM350_RushCloudEmitter'


class AttachedCloudTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(GteShortVector)==8 ? 1:-1];
typedef char b[sizeof(GteMatrix)==32 ? 1:-1];
typedef char c[OFF(GteMatrix,t)==20 ? 1:-1];
typedef char d[sizeof(RoomM350CloudParticle)==20 ? 1:-1];
typedef char e[OFF(RoomM350CloudParticle,position)==4 ? 1:-1];
typedef char f[OFF(RoomM350CloudParticle,size)==10 ? 1:-1];
typedef char g[OFF(RoomM350EffectInstance,transforms)==0x238 ? 1:-1];
typedef char h[OFF(RoomM350EffectActor,instance)==8 ? 1:-1];
typedef char i[OFF(RoomM350EffectEmitter,pool)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_helper_order_and_boundaries(self):
        source = host_unit(FUNCTION, under=ROOM)
        # Transform stubs verify CPU-side contracts, not GTE arithmetic.
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n' + source + r'''
typedef GteShortVector Vector;
typedef GteMatrix Matrix;
typedef RoomM350CloudParticle Particle;
FieldFxObjectPrefix *g_FieldFxOwner;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
unsigned char D_8019A8BE;
/* The retail halfword names are fields of the sprite parameter block. */
RenderEffectParameters g_FieldFxSpriteParams;
#define SIGNED(field) (*(short *)&g_FieldFxSpriteParams.field)
#define UNIT SIGNED(parameter00)
#define D_800F336A SIGNED(parameter02)
#define D_800F336C SIGNED(palette)
#define D_800F336E SIGNED(parameter06)
#define D_800F3370 g_FieldFxSpriteParams.tpage
#define D_800F3372 SIGNED(parameter0A)
#define D_800F3374 g_FieldFxSpriteParams.depth
#define D_800F3376 SIGNED(extent_x)
#define D_800F3378 SIGNED(extent_y)
unsigned short g_FieldFxTpageSlots[3],g_FieldFxTpageTable[65536];
#define g_FieldFxTpageSlot2 g_FieldFxTpageSlots[2]
static RoomM350EffectActor actors[2];
static RoomM350EffectInstance instances[2];
static RoomM350EffectEmitter emitters[2];
static Matrix transforms[2][16],*rotation;
static Particle particles[2];
static int pools[2],allocs,randoms,rotations,applications,failAt;
static uint32_t seed;
static int word(uint32_t x) { int result; memcpy(&result,&x,4); return result; }
static int half(uint32_t x) { x &= 65535; return x<32768 ? (int)x:(int)x-65536; }
static uint32_t rng(int n) { return seed+(uint32_t)n*0x87654321u; }
int RoomM350_RushCloudCallback(int e,Particle *p) { return 0; }
int FieldFx_InitCallbackList(char *p,int stride,int count,FieldFxCallbackListCallback cb) {
    assert(p==(char *)&pools[0] && stride==20 && count==16 && (void *)cb==(void *)RoomM350_RushCloudCallback);
    return -123;
}
void *FieldFx_AcquireCallbackSlot(char *p) {
    int i=allocs++;
    assert(i<2 && p==(char *)&pools[i] && randoms==i*5 && applications==i);
    D_8019A8BE=255;
    return i==failAt ? 0:&particles[i];
}
int Inv_ScrambleGrid(void) {
    int n=randoms++,i=n/5;
    assert(allocs==i+1);
    if(n%5==4) {
        assert(applications==i+1);
        g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[1];
        g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[1];
    }
    return word(rng(n));
}
GteMatrix *RotMatrixYXZ(GteShortVector *v,GteMatrix *m) {
    int i=rotations++;
    assert(randoms==i*5+2 && applications==i);
    assert(v->x==half(rng(i*5)<<4) && v->y==half(rng(i*5+1)<<4) && v->z==0);
    rotation=m;
    return m;
}
GteShortVector *ApplyMatrixSV(const GteMatrix *m,const GteShortVector *v,GteShortVector *out) {
    int i=applications++;
    assert(m==rotation && rotations==i+1 && randoms==i*5+4);
    assert(v->x==0 && v->y==0 && v->z==word((rng(i*5+2)<<8)|rng(i*5+3))%128);
    assert(out==(Vector *)particles[i].position);
    out->x=half(seed); out->y=half(seed*7); out->z=half(seed*31);
    return out;
}
static void reset(void) {
    allocs=randoms=rotations=applications=0;
    D_8019A8BE=0; g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[0];
    g_FieldFxActiveTaskSlot=(FieldFxTaskSlot *)&emitters[0];
    memset(particles,0x5A,sizeof(particles));
}
int main(void) {
    int n,f,i,k,success;
    for(i=0;i<2;i++) {
        actors[i].instance=&instances[i]; instances[i].transforms=transforms[i];
        emitters[i].pool=&pools[i];
    }
    reset(); assert(RoomM350_RushCloudEmitter(0)==-123);
    for(n=1;n<256;n++) {
        reset(); D_8019A8BE=n;
        assert(RoomM350_RushCloudEmitter(1)==2 && allocs==0 && D_8019A8BE==n);
    }
    for(n=0;n<65536;n++) for(f=-1;f<2;f++) {
        reset(); seed=(uint32_t)n*65537u; failAt=f;
        assert(RoomM350_RushCloudEmitter(1)==0);
        success=f<0 ? 2:f;
        assert(allocs==(f<0 ? 2:f+1) && randoms==success*5);
        assert(rotations==success && applications==success && D_8019A8BE==255);
        for(i=0;i<2;i++) {
            if(i<success) {
                assert(particles[i].anchor==transforms[1][i ? 11:15].t);
                assert(particles[i].position[0]==half(seed));
                assert(particles[i].position[1]==half((uint32_t)(uint16_t)half(seed*7)*2+128));
                assert(particles[i].position[2]==half(seed*31));
                assert(particles[i].size==half((rng(i*5+4)<<4)+4096));
                for(k=0;k<2;k++) assert(particles[i].state.value.spawnReserved[k]==0x5A5A5A5A);
            } else {
                unsigned char *p=(unsigned char *)&particles[i];
                for(k=0;k<sizeof(Particle);k++) assert(p[k]==0x5A);
            }
        }
    }
    for(n=0;n<65536;n++) {
        g_FieldFxTpageSlot2=n; g_FieldFxTpageTable[n]=n^0xA35C;
        assert(RoomM350_RushCloudEmitter(2)==0 && D_800F3370==(unsigned short)(n^0xA35C));
        assert(UNIT==16 && D_800F336A==1 && D_800F3376==16 && D_800F3378==16);
        assert(D_800F336C==2 && D_800F336E==0 && D_800F3372==0 && D_800F3374==16);
    }
    reset(); assert(RoomM350_RushCloudEmitter(-1)==0 && RoomM350_RushCloudEmitter(3)==0 && allocs==0);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory)/'test.c'
            path.write_text(harness)
            binary = path.with_suffix('')
            result = subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing', str(path), '-o', str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            subprocess.run([str(binary)], check=True)
