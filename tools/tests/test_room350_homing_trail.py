import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import find_definition, host_unit, psx_compile  # noqa: E402

# Overlay function addresses repeat across rooms; search this room only.
ROOM = 'src/overlays/room_m350_cruiser'

CONTROLLER = 'RoomM350_OrbCallback'
# Keep pe1/gte.h's asm out of the host build; the GTE is a boundary here.
HOST_GTE = r'''
#include "pe1/gte_types.h"
#define PE1_GTE_H
static void check_vector(void *);
static void store_vector(void *);
#define gte_ldrotmatrix(p) ((void)(p))
#define gte_ldtransmatrix(p) ((void)(p))
#define gte_ldv0(p) check_vector(p)
#define gte_rtv0tr_mac() (gteCalls++)
#define gte_stsv(p) store_vector(p)
'''


class HomingTrailTests(unittest.TestCase):
    def test_cpu_behavior(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM), prefix=HOST_GTE)
        # The two queue counters sit after their queues; map them into
        # host-sized buffers so the pointer arithmetic stays in bounds.
        source = source.replace('extern short D_8019A7FE,D_8019A800,D_8019A802;',
                                'extern short D_8019A802;')
        source = source.replace('(unsigned int)count', '(uintptr_t)count')
        prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
static union { uint64_t align; unsigned char bytes[256]; } pointerQueue,positionQueue;
#define D_8019A7FE (*(short *)(pointerQueue.bytes+0x56))
#define D_8019A800 (*(short *)(positionQueue.bytes+0x48))
static int gteCalls,draws;
'''
        harness = prefix + source + r'''
typedef RoomM350Particle Particle;
FieldFxObjectPrefix *g_FieldFxOwner;
FieldActor *g_PlayerEntity;
int g_FieldFxCurrentAge,g_FieldFxAltPaletteFlag;
GteSineWord rcossin_tbl[4096];
short D_8019A802;
RenderEffectParameters g_FieldFxSpriteParams;
u16 g_FieldFxPaletteTable[65536];
RoomFloorY g_RoomFloorY[1];
GteRotation D_8019A3C0;
RenderColor D_8019A4E4,D_8019A3C8;
static RoomM350EffectActor actor;
static RoomM350EffectInstance instance;
static FieldActor player;
static RoomM350EffectOwner owner,playerOwner;
static Particle particle;
static int sqrtResult;
static int steering,expectedStep,expectedYaw,turnCalls;
static void check_vector(void *p) {
    GteShortVector *v=p;
    assert(v->x==0 && v->y==0 && v->z==(short)(0u-(unsigned short)particle.tail.value.active.speed));
    /* The fourth halfword is uninitialized in the target. */
}
static void store_vector(void *p) {
    GteShortVector *v=p;
    assert(v==&particle.position);
    v->x=11; v->y=22; v->z=33;
}
int FieldFx_VecToAngle(Vec3Fixed *a,Vec3Fixed *b) {
    assert((void *)a==player.render_object.model_matrix.translation);
    assert((void *)b==instance.transform.t);
    return 123;
}
int FieldFx_TurnToward(int a,int b,int c) {
    assert(b==123); turnCalls++;
    if(steering) { assert(a==-321 && c==expectedStep); g_FieldFxCurrentAge=7; }
    return 456;
}
int Math_IntSqrt(int n) { return sqrtResult; }
GteMatrix *RotMatrixYXZ(GteShortVector *v,GteMatrix *m) {
    assert(v==&particle.motion.value.rotation);
    if(steering) assert(v->x==-1 && v->y==(short)expectedYaw && v->z==987);
    memset(m,0,sizeof(*m));
    return m;
}
u_short GetClut(int x,int y) { assert(x==32 && y==104); return 0x5678; }
void FieldFx_DrawShapeQuads(struct GteShortVector *p,struct GteRotation *a,int b,int c,int d,int e,int f,int g,struct RenderColor *h) {
    assert(draws++==0 && p==&particle.position && a==0 && b==4096 && c==4096);
    assert(d==108 && e==0x5678 && f==1 && g==77 && h==0);
    particle.position.x=-5; particle.position.z=9; particle.tail.value.active.brightness=88;
}
void FieldFx_DrawStarFan(struct GteShortVector *v,int a,int b,int c,struct GteRotation *d,int e,int f,struct RenderColor *g,struct RenderColor *h,int i,int j) {
    assert(draws++==1 && v->x==-5 && v->y==-123 && v->z==9);
    assert(a==192 && b==192 && c==8 && d==&D_8019A3C0 && e==4096 && f==4096);
    assert(g==&D_8019A4E4 && h==&D_8019A3C8 && i==88 && j==1);
}
static void reset(void) {
    memset(&particle,0,sizeof(particle)); memset(&owner,0,sizeof(owner));
    memset(&pointerQueue,0,sizeof(pointerQueue)); memset(&positionQueue,0,sizeof(positionQueue));
    actor.instance=&instance; instance.owner=&owner;
    /* The player's leading state pointer is read as an effect owner. */
    player.state=(FieldActorState *)&playerOwner;
    playerOwner.status=0; player.mode=4;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actor; g_PlayerEntity=&player;
    g_FieldFxCurrentAge=0; D_8019A802=9; gteCalls=draws=turnCalls=steering=0; sqrtResult=128;
}
int main(void) {
    int n,frame,hit,expected;
    for(n=0;n<65536;n++) {
        reset(); g_FieldFxCurrentAge=48; D_8019A802=n;
        assert(RoomM350_OrbCallback(1,&particle)==1);
        assert((unsigned short)D_8019A802==(unsigned short)(n-1) && gteCalls==0 && D_8019A800==0);
    }
    for(frame=-32768;frame<=32767;frame++) {
        reset(); particle.tail.value.active.frame=frame; particle.tail.value.active.hit=1;
        particle.position.pad=321;
        assert(RoomM350_OrbCallback(1,&particle)==0 && gteCalls==0);
        assert(particle.tail.value.active.frame==(frame<48 ? (short)(frame+1):frame));
        assert(D_8019A800==(frame<48));
        expected=frame>=36 && frame<48 && !(frame&3);
        assert(D_8019A7FE==expected);
        if(frame<48) assert(!memcmp(positionQueue.bytes,&particle.position,8));
    }
    for(n=0;n<=4;n++) for(hit=0;hit<2;hit++) {
        reset(); particle.tail.value.active.frame=0; particle.motion.value.rotation.x=64;
        particle.tail.value.active.speed=100;
        D_8019A7FE=n; sqrtResult=hit ? 127:128;
        assert(RoomM350_OrbCallback(1,&particle)==0 && gteCalls==1);
        assert(particle.tail.value.active.hit==hit);
        assert(particle.tail.value.active.frame==(hit ? 41:1));
        assert(particle.tail.value.active.speed==(hit ? 0:100));
        assert(owner.flags==(hit ? 0x80000000u:0) && playerOwner.status==(hit ? 0x4000:0));
        expected=hit ? (n+2<4 ? n+2:4):n;
        assert(D_8019A7FE==expected && D_8019A800==1);
        for(frame=n;frame<expected;frame++) assert(((Particle **)pointerQueue.bytes)[frame]==&particle);
    }
    for(n=-32768;n<=32767;n++) {
        int second=(short)(n*13);
        reset(); steering=1; player.mode=0;
        particle.motion.value.rotation.x=-1; particle.motion.value.rotation.y=-321;
        particle.motion.value.rotation.z=987;
        particle.tail.value.active.speed=n; rcossin_tbl[0].word=(unsigned short)n;
        rcossin_tbl[2055].word=(unsigned short)second;
        expectedStep=n/4; expectedYaw=456+80-(second*160)/4096;
        assert(RoomM350_OrbCallback(1,&particle)==0 && gteCalls==1 && turnCalls==1);
        assert(g_FieldFxCurrentAge==7 && particle.tail.value.active.frame==1);
        assert(particle.tail.value.active.hit==0);
    }
    reset(); particle.tail.value.active.brightness=77; g_FieldFxSpriteParams.palette=4; g_FieldFxAltPaletteFlag=1;
    g_FieldFxPaletteTable[4]=100; g_RoomFloorY[0].raw=(unsigned short)-123;
    assert(RoomM350_OrbCallback(2,&particle)==0 && draws==2);
    reset(); particle.tail.value.active.frame=48;
    assert(RoomM350_OrbCallback(2,&particle)==0 && draws==0);
    for(n=-3;n<=4;n++) if(n!=1 && n!=2) {
        reset(); assert(RoomM350_OrbCallback(n,&particle)==0 && gteCalls==0 && draws==0 && D_8019A802==9);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory)/'homing'
            result = subprocess.run(['cc','-std=gnu11','-O2','-fwrapv','-fno-strict-aliasing','-x','c','-','-o',str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)

    def test_psx_layout(self):
        unit = find_definition(CONTROLLER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(GteShortVector)==8 ? 1:-1];
typedef char b[sizeof(GteMatrix)==32 ? 1:-1];
typedef char c[sizeof(RoomM350Particle)==28 ? 1:-1];
typedef char d[OFF(RoomM350Particle,motion.value.rotation)==8 ? 1:-1];
typedef char e[OFF(RoomM350Particle,tail.value.active.speed)==16 ? 1:-1];
typedef char f[OFF(RoomM350Particle,tail.value.active.brightness)==20 ? 1:-1];
typedef char g[OFF(RoomM350Particle,tail.value.active.frame)==22 ? 1:-1];
typedef char h[OFF(RoomM350Particle,tail.value.active.hit)==24 ? 1:-1];
typedef char i[OFF(RoomM350EffectOwner,status)==0x4C ? 1:-1];
typedef char j[OFF(RoomM350EffectInstance,animation)==14 ? 1:-1];
typedef char k[OFF(RoomM350EffectInstance,transform.t)==0x1FC ? 1:-1];
typedef char l[OFF(RoomM350EffectActor,instance)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)


if __name__ == '__main__':
    unittest.main()
