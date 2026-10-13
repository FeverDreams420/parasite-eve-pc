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

CONTROLLER = 'RoomM350_HandSweepController'
# Keep pe1/gte.h's asm out of the host build; the COP2 transfers are a
# recorded boundary, not emulated.
HOST_GTE = r'''
#include "pe1/gte_types.h"
#define PE1_GTE_H
static void transfer(int,int),load_vector(void *),command(void);
static int read_ir(int);
#define gte_ctc2_0(x) transfer(0,x)
#define gte_ctc2_1(x) transfer(1,x)
#define gte_ctc2_2(x) transfer(2,x)
#define gte_ctc2_3(x) transfer(3,x)
#define gte_ctc2_4(x) transfer(4,x)
#define gte_ctc2_5(x) transfer(5,x)
#define gte_ctc2_6(x) transfer(6,x)
#define gte_ctc2_7(x) transfer(7,x)
#define gte_ldv0(p) load_vector(p)
#define gte_rtv0tr_mac() command()
#define gte_mfc2_9(x) ((x)=read_ir(0))
#define gte_mfc2_10(x) ((x)=read_ir(1))
#define gte_mfc2_11(x) ((x)=read_ir(2))
'''


class PairedTurnTests(unittest.TestCase):
    def test_layout(self):
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
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    def test_cpu_and_cop2_boundary(self):
        source = host_unit(CONTROLLER, path=find_definition(CONTROLLER, under=ROOM), prefix=HOST_GTE)
        source = source.replace('(unsigned int)base', '(uintptr_t)base')
        source = source.replace('extern short D_8019A77E[], g_RoomM350HandSweepRepeats;',
                                'extern short g_RoomM350HandSweepRepeats;')
        prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#define D_8019A77E ((short *)&g_RoomM350HandPoints[0].pad)
'''
        harness = prefix + source + r'''
FieldFxObjectPrefix *g_FieldFxOwner;
FieldActor *g_RoomM350HandActor,*g_PlayerEntity;
GteShortVector g_RoomM350HandPoints[2];
short g_RoomM350HandSweepRepeats;
GteSineWord rcossin_tbl[4096];
unsigned char g_RoomM350HandSweepDone,g_RoomM350HandSweepHit;
static Actor actor,otherActor;
static Instance instance,model,otherModel,player,other;
static Owner owner;
static u8 action[1];
static Matrix matrices[16],otherMatrices[16];
static int phase,iteration,angles,turns,expectedSpeed,expectedBias;
static void transfer(int index,int value) {
    assert(phase++==index);
    assert(value==(iteration ? 200:100)+index);
}
static void load_vector(void *p) {
    Vector *v=p;
    assert(phase++==8 && v->x==(iteration ? 112:-112) && v->y==0 && v->z==-80);
    /* Target leaves pad uninitialized; do not read it. */
}
static void command(void) {
    assert(phase++==9);
    g_RoomM350HandActor=&otherModel;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&otherActor;
}
static int read_ir(int index) {
    int value=index==0 ? -32768 : index==1 ? 32767 : -123;
    assert(phase++==10+index);
    if(index==2) { phase=0; iteration++; }
    return value;
}
int FieldFx_VecToAngle(Vec3Fixed *a,Vec3Fixed *b) {
    assert(iteration==2 && angles++==0);
    assert((void *)a==player.render_object.model_matrix.translation);
    assert((void *)b==instance.render_object.model_matrix.translation);
    instance.rot_y=-777;
    return 0x12348001;
}
int FieldFx_TurnToward(int current,int target,int speed) {
    assert(turns++==0 && angles==1 && current==-777);
    assert(target==(int16_t)(0x8001+expectedBias) && speed==expectedSpeed);
    return -1234;
}
static void reset(void) {
    memset(&instance,0,sizeof(instance));
    actor.instance=(struct RoomM350Instance *)&instance;
    otherActor.instance=(struct RoomM350Instance *)&other;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actor;
    instance.state=&owner; owner.action_state=action; owner.core_flags=0x13579;
    /* mode is the animation id; action holds the clip length. */
    instance.mode=11; instance.action=42; instance.rot_y=123;
    model.render_object.matrices=matrices; otherModel.render_object.matrices=otherMatrices;
    g_RoomM350HandActor=&model; g_PlayerEntity=&player;
    g_RoomM350HandSweepDone=0; g_RoomM350HandSweepHit=93; action[0]=1;
    phase=iteration=angles=turns=0;
    memset(g_RoomM350HandPoints,0x55,sizeof(g_RoomM350HandPoints));
}
int main(void) {
    int frame,repeat,index,sample,doTurn,result,i,animation,state;
    for(i=0;i<8;i++) {
        ((int *)&matrices[11])[i]=100+i;
        ((int *)&otherMatrices[15])[i]=200+i;
    }
    reset(); assert(RoomM350_HandSweepController(0)==0 && iteration==0);
    assert(owner.core_flags==(0x13579|0x40000000) && g_RoomM350HandSweepRepeats==3);
    assert(g_RoomM350HandSweepDone==0 && g_RoomM350HandSweepHit==0);
    for(frame=-32768;frame<=32767;frame++) for(repeat=-1;repeat<=3;repeat++) {
        reset(); g_RoomM350HandSweepRepeats=repeat;
        instance.anim.fixed=((uint32_t)(uint16_t)frame<<16)|123;
        index=((frame-28)*170)&0xFFE;
        sample=(int16_t)(frame*17); rcossin_tbl[index].part.sin=sample;
        expectedSpeed=sample/32;
        expectedBias=((repeat-(frame>=40))&1) ? 128:-128;
        doTurn=repeat>0 && frame>=28;
        result=repeat<=0 && frame>=41;
        assert(RoomM350_HandSweepController(1)==result && iteration==2 && phase==0);
        assert(angles==doTurn && turns==doTurn);
        assert(g_RoomM350HandSweepRepeats==repeat-(repeat>0 && frame>=40));
        assert((uint32_t)instance.anim.fixed==(repeat>0 && frame>=40 ? 0x30000 : ((uint32_t)(uint16_t)frame<<16)|123));
        assert(instance.rot_y==(doTurn ? -1234:123));
        assert(action[0]==(result ? 4:2) && g_RoomM350HandSweepDone==result && g_RoomM350HandSweepHit==93);
        for(i=0;i<2;i++) assert(g_RoomM350HandPoints[i].x==-32768 && g_RoomM350HandPoints[i].y==32767 && g_RoomM350HandPoints[i].z==-123 && g_RoomM350HandPoints[i].pad==i);
    }
    for(animation=0;animation<256;animation++) for(state=0;state<256;state++) {
        reset(); instance.mode=animation; action[0]=state;
        instance.anim.fixed=41u<<16; g_RoomM350HandSweepRepeats=0; result=animation==11;
        assert(RoomM350_HandSweepController(1)==result && iteration==2 && angles==0);
        assert(action[0]==(result ? 4:state==1 ? 2:state));
    }
    for(frame=-2;frame<=44;frame++) {
        reset(); instance.state=0; instance.anim.fixed=(uint32_t)(uint16_t)frame<<16; g_RoomM350HandSweepRepeats=0;
        assert(RoomM350_HandSweepController(1)==(frame>=41) && iteration==2 && action[0]==1);
    }
    for(i=-3;i<=4;i++) if(i!=0 && i!=1) {
        reset(); g_RoomM350HandSweepRepeats=123;
        assert(RoomM350_HandSweepController(i)==0 && iteration==0 && g_RoomM350HandSweepRepeats==123 && action[0]==1);
    }
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory)/'paired'
            result = subprocess.run(['cc','-std=gnu11','-O2','-fno-strict-aliasing','-x','c','-','-o',str(exe)], input=harness, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=20)


if __name__ == '__main__':
    unittest.main()
