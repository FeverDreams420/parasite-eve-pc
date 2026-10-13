import pathlib
import subprocess
import tempfile
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import find_definition, host_unit, psx_compile  # noqa: E402

# Overlay function addresses repeat across rooms; search this room only.
ROOM = 'src/overlays/room_m350_cruiser'

CONTROLLER = 'RoomM350_OrbController'
# Keep pe1/gte.h's asm out of the host build; the stubs assert the COP2
# interface only and do not claim to emulate the GTE.
HOST_GTE = r'''
#include "pe1/gte_types.h"
#define PE1_GTE_H
static void load_rotation(void *), load_translation(void *), load_vector(void *);
static void command(void), store_ir(void *);
#define gte_ldrotmatrix(p) load_rotation(p)
#define gte_ldtransmatrix(p) load_translation(p)
#define gte_ldv0(p) load_vector(p)
#define gte_rtv0tr_mac() command()
#define gte_stsv(p) store_ir(p)
'''


class SequenceTests(unittest.TestCase):
    def test_target_layout(self):
        unit = find_definition(CONTROLLER, under=ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[OFF(Instance,mode)==14 ? 1:-1];
typedef char b[OFF(Instance,anim)==20 ? 1:-1];
typedef char c[OFF(Instance,rot_y)==58 ? 1:-1];
typedef char d[OFF(Instance,render_object.model_matrix.translation)==0x1FC ? 1:-1];
typedef char e[OFF(Instance,render_object.matrices)==0x238 ? 1:-1];
typedef char f[OFF(Owner,action_state)==24 ? 1:-1];
typedef char g[sizeof(Matrix)==32 ? 1:-1];
typedef char h[sizeof(Vector)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / "layout.o", unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    def test_cpu_logic_at_gte_boundary(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM), prefix=HOST_GTE)
        prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
'''
        harness = prefix + source + r'''
FieldFxObjectPrefix *g_FieldFxOwner;
FieldActor *g_RoomM350OrbActor,*g_PlayerEntity;
GteShortVector g_RoomM350OrbSpawnPoint;
short g_RoomM350OrbState;
short D_8019A7FE,D_8019A800,D_8019A802;
unsigned char D_8019A804,D_8019A805,D_8019A806;
GteSineWord rcossin_tbl[4096];
static Actor actor,otherActor;
static Instance instance,other,player;
static Owner owner;
static u8 action[1];
static Matrix matrices[18];
static int phase,angles,turns,expectedSpeed;
static void load_rotation(void *p) { assert(phase++==0 && p==&matrices[17]); }
static void load_translation(void *p) { assert(phase++==1 && p==&matrices[17]); }
static void load_vector(void *p) {
    Vector *v=p; assert(phase++==2 && v->x==0 && v->y==-144 && v->z==128);
}
static void command(void) {
    assert(phase++==3);
    g_FieldFxOwner=(FieldFxObjectPrefix *)&otherActor;
}
static void store_ir(void *p) {
    Vector *v=p;
    assert(phase==4 && v==&g_RoomM350OrbSpawnPoint); phase+=3;
    v->x=-32768; v->y=32767; v->z=-123;
}
int FieldFx_VecToAngle(Vec3Fixed *a,Vec3Fixed *b) {
    assert(phase==7 && angles++==0);
    assert((void *)a==player.render_object.model_matrix.translation);
    assert((void *)b==instance.render_object.model_matrix.translation);
    instance.rot_y=-777;
    return 0x12348001;
}
int FieldFx_TurnToward(int current,int target,int speed) {
    assert(turns++==0 && angles==1 && current==-777 && target==-32767 && speed==expectedSpeed);
    return -1234;
}
static void reset(void) {
    memset(&instance,0,sizeof(instance)); memset(&other,0,sizeof(other));
    actor.instance=(struct RoomM350Instance *)&instance;
    otherActor.instance=(struct RoomM350Instance *)&other;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actor;
    owner.action_state=action; owner.core_flags=0x13579;
    /* mode is the animation id; action holds the clip length. */
    instance.state=&owner; instance.mode=13; instance.action=30; instance.rot_y=123;
    other.render_object.matrices=matrices; g_RoomM350OrbActor=&other; g_PlayerEntity=&player;
    D_8019A804=D_8019A805=D_8019A806=0;
    D_8019A7FE=3; D_8019A800=4; D_8019A802=5;
    g_RoomM350OrbSpawnPoint.pad=987; phase=angles=turns=0; action[0]=1;
}
int main(void) {
    int frame,repeat,word,index,result,doTurn,event,state,animation;
    reset(); assert(RoomM350_OrbController(0)==0 && phase==0);
    assert(owner.core_flags==(0x13579|0x40000000) && g_RoomM350OrbState==2);
    assert(!D_8019A804 && !D_8019A805 && !D_8019A806 && !D_8019A7FE && !D_8019A800 && !D_8019A802);
    for(frame=-32768;frame<=32767;frame++) for(repeat=-1;repeat<=2;repeat++) {
        reset(); g_RoomM350OrbState=repeat; instance.anim.fixed=((uint32_t)(uint16_t)frame<<16)|123;
        index=((frame-19)*341)&4095; word=(int16_t)(frame*17);
        rcossin_tbl[index].word=(uint16_t)word; expectedSpeed=word<0 ? -((-word)/32):word/32;
        doTurn=repeat>0 && frame>=19;
        result=repeat<=0 && frame>=29;
        assert(RoomM350_OrbController(1)==result && phase==7);
        assert(angles==doTurn && turns==doTurn);
        assert(g_RoomM350OrbState==repeat-(repeat>0 && frame>=25));
        assert((uint32_t)instance.anim.fixed==(repeat>0 && frame>=25 ? 0x60000 : ((uint32_t)(uint16_t)frame<<16)|123));
        assert(instance.rot_y==(doTurn ? -1234:123));
        assert(action[0]==(result ? 4:2) && D_8019A804==result);
        assert(g_RoomM350OrbSpawnPoint.x==-32768 && g_RoomM350OrbSpawnPoint.y==32767 && g_RoomM350OrbSpawnPoint.z==-123 && g_RoomM350OrbSpawnPoint.pad==987);
    }
    for(event=-3;event<=5;event++) if(event<0 || event>1) {
        reset(); g_RoomM350OrbState=-9;
        assert(RoomM350_OrbController(event)==0 && phase==0 && angles==0 && turns==0);
        assert(action[0]==1 && g_RoomM350OrbState==-9 && D_8019A7FE==3 && D_8019A800==4 && D_8019A802==5);
    }
    for(animation=0;animation<256;animation++) for(state=0;state<256;state++) {
        reset(); instance.mode=animation; action[0]=state;
        instance.anim.fixed=30u<<16; g_RoomM350OrbState=0;
        result=animation==13;
        assert(RoomM350_OrbController(1)==result && phase==7 && angles==0 && turns==0);
        assert(action[0]==(result ? 4 : state==1 ? 2:state));
        assert(D_8019A804==result);
    }
    for(frame=-1;frame<=31;frame++) {
        reset(); instance.state=0; instance.anim.fixed=(uint32_t)(uint16_t)frame<<16; g_RoomM350OrbState=0;
        result=frame>=29;
        assert(RoomM350_OrbController(1)==result && phase==7 && action[0]==1);
        assert(D_8019A804==result && angles==0 && turns==0);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory)/'sequence'
            result = subprocess.run(['cc','-std=gnu11','-O2','-fwrapv','-fno-strict-aliasing','-x','c','-','-o',str(exe)],input=harness,text=True,capture_output=True)
            self.assertEqual(result.returncode,0,result.stderr)
            subprocess.run([str(exe)],check=True,timeout=20)


if __name__ == '__main__':
    unittest.main()
