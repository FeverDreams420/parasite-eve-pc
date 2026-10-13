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
FUNCTION = 'RoomM350_RushController'


class CloudControllerTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(State)==12 ? 1:-1];
typedef char b[OFF(State,speed)==4 && OFF(State,targetSpeed)==6 ? 1:-1];
typedef char c[OFF(State,heightPhase)==8 && OFF(State,mode)==10 ? 1:-1];
typedef char d[OFF(Actor,instance)==8 ? 1:-1];
typedef char e[OFF(FieldActorState,progress)==8 && OFF(FieldActorState,action_state)==24 ? 1:-1];
typedef char f[OFF(Instance,mode)==14 && OFF(Instance,action)==15 ? 1:-1];
typedef char g[OFF(Instance,anim)==20 && OFF(Instance,pos_x)==40 ? 1:-1];
typedef char h[OFF(Instance,rot_y)==58 && OFF(Instance,render_object.matrices)==568 ? 1:-1];
typedef char i[sizeof(Matrix)==32 && OFF(Matrix,translation)==20 ? 1:-1];
typedef char j[sizeof(Vector)==8 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory)/'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_state_machine_and_helper_boundaries(self):
        source = host_unit(FUNCTION, under=ROOM)
        # Alias the separately named target globals to their offsets in the
        # g_RoomM350RushMatrix matrix.
        matrix_globals = ''
        for i in range(1, 9):
            matrix_globals += f'#define D_{0x8019A870+i*2:08X} g_RoomM350RushMatrix.m[{i//3}][{i%3}]\n'
        for i in range(3):
            matrix_globals += f'#define D_{0x8019A884+i*4:08X} g_RoomM350RushMatrix.t[{i}]\n'
        declarations = ('extern short D_8019A872,D_8019A874,D_8019A876,D_8019A878,'
                        'D_8019A87A,D_8019A87C,D_8019A87E,D_8019A880;')
        self.assertIn(declarations, source)
        source = source.replace(declarations, matrix_globals)
        declarations = 'extern int D_8019A884,D_8019A888,D_8019A88C;'
        self.assertIn(declarations, source)
        source = source.replace(declarations, '')
        # Transform and angle helpers are boundary stubs, not GTE emulation.
        harness = '#include <assert.h>\n#include <stdint.h>\n#include <string.h>\n#include <limits.h>\n' + source + r'''
FieldFxObjectPrefix *g_FieldFxOwner;
Instance *g_PlayerEntity;
unsigned char D_8019A8BE,g_RoomM350RushPlayerHit;
volatile short D_8019A89E;
short g_RoomM350RushState,D_8019A8BA,D_8019A8BC;
RoomFloorY g_RoomFloorY[1];
#define D_800942EC g_RoomFloorY[0].y
GteSineWord rcossin_tbl[4096];
#define TABLE ((short *)rcossin_tbl)
GteMatrix g_RoomM350RushMatrix;
#define testMatrix g_RoomM350RushMatrix
/* Actor coordinates are the pos_x/pos_y/pos_z triple. */
#define POS(actor) (&(actor).pos_x)
static Actor actor,otherActor;
static Instance instance,otherInstance,players[2];
static Matrix transforms;
static FieldActorState owner;
static unsigned char action;
static State state,expected;
static int stage,stopped,turns,angleResult,turnResult,mutate,expectedSpeed;
static int expectedYaw,expectedPosition[3],expectedFrame,expectedRepeat,expectedAction,expectedStop,expectedAux;
static int word(uint32_t x) { int r; memcpy(&r,&x,4); return r; }
static int half(uint32_t x) { x&=65535; return x<32768 ? (int)x:(int)x-65536; }
int Asset_Find08w(int id,int asset,int x,int y,int z) {
    assert(id==0x5CB && asset==owner.progress);
    assert(x==(short)transforms.translation[0] && y==(short)transforms.translation[1] && z==(short)transforms.translation[2]);
    assert(state.speed==0 && state.targetSpeed==0 && state.heightPhase==0 && state.mode==0);
    assert(D_8019A8BE==0 && g_RoomM350RushPlayerHit==0 && D_8019A89E==0);
    return -9876;
}
void Akao_Cmd_21(int handle,int zero) {
    assert(handle==state.asset && zero==0 && D_8019A8BE==1 && stage==0);
    assert(!instance.state || action==4); stopped++;
}
s32 FieldFx_GetDistanceXZ(Vec3Fixed *a,Vec3Fixed *b) {
    assert(stage++==0 && a==POS(players[0]) && b==POS(instance));
    assert(instance.anim.fixed==(uint32_t)expectedFrame && g_RoomM350RushState==expectedRepeat);
    if(mutate) { g_PlayerEntity=&players[1]; g_FieldFxOwner=(FieldFxObjectPrefix *)&otherActor; instance.anim.fixed=0; }
    return -123456;
}
int FieldFx_VecToAngle(Vec3Fixed *a,Vec3Fixed *b) {
    assert(stage++==1 && a==POS(players[mutate]) && b==POS(instance));
    assert(D_8019A8BA==(short)-123456);
    return angleResult;
}
int FieldFx_TurnToward(int yaw,int angle,int step) {
    assert(stage==2 && yaw==instance.rot_y && angle==(angleResult&4095) && step==80);
    assert(D_8019A8BC==(angleResult&4095)); turns++;
    return turnResult;
}
int rcos(int yaw) {
    assert(stage++==2 && yaw==expectedYaw+2048 && state.speed==expectedSpeed);
    if(mutate) instance.rot_y=-456;
    return 0x7FFF8123;
}
int rsin(int yaw) {
    assert(stage++==3 && yaw==(mutate ? -456:expectedYaw)+2048);
    if(mutate) state.speed=-1234;
    return -123456789;
}
GteShortVector *ApplyMatrixSV(const GteMatrix *m,const GteShortVector *in,GteShortVector *out) {
    assert(stage++==4 && m==&testMatrix && in==out);
    assert(in->x==0 && in->y==0 && in->z==(mutate ? -1234:expectedSpeed));
    assert(m->m[0][0]==half(0x7FFF8123) && m->m[0][2]==half((uint32_t)-123456789));
    assert(m->m[2][0]==half(123456789) && m->m[2][2]==half(0x7FFF8123));
    assert(m->m[1][1]==4096 && m->m[0][1]==0 && m->m[1][0]==0);
    assert(m->m[1][2]==0 && m->m[2][1]==0);
    assert(m->t[0]==0 && m->t[1]==0 && m->t[2]==0);
    out->x=-32768; out->z=32767;
    if(mutate) {
        POS(instance)[0]=INT_MAX; POS(instance)[1]=INT_MIN; POS(instance)[2]=-123;
        state.heightPhase=-2; D_800942EC=-32768;
    }
    return out;
}
static void reset(void) {
    memset(&instance,0,sizeof(instance)); memset(&state,0,sizeof(state));
    actor.instance=(struct RoomM350Instance *)&instance;
    otherActor.instance=(struct RoomM350Instance *)&otherInstance;
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actor;
    instance.state=&owner; owner.action_state=&action; owner.progress=123;
    instance.render_object.matrices=&transforms;
    action=1; g_PlayerEntity=&players[0]; stage=stopped=turns=0;
    state.asset=1234; state.heightPhase=16;
    instance.mode=9; instance.action=40; instance.anim.fixed=31u<<16; instance.rot_y=321;
    POS(instance)[0]=INT_MAX; POS(instance)[1]=123456789; POS(instance)[2]=INT_MIN;
    g_RoomM350RushState=1; D_8019A8BE=77; g_RoomM350RushPlayerHit=88; D_8019A89E=99;
    D_800942EC=123; angleResult=321; turnResult=-321; mutate=0;
}
static void run(void) {
    int frame=(short)(instance.anim.fixed>>16),phase,mode,delta,result=0,k,expectedTurns=0;
    memcpy(&expected,&state,sizeof(state)); expectedYaw=instance.rot_y;
    expectedFrame=instance.anim.fixed; expectedRepeat=g_RoomM350RushState; expectedAction=action;
    expectedStop=D_8019A8BE; expectedAux=g_RoomM350RushPlayerHit;
    for(k=0;k<3;k++) expectedPosition[k]=POS(instance)[k];
    if(instance.mode!=9) goto verify;
    if(instance.state && action==1) expectedAction=2;
    if(expectedRepeat>0) {
        if(frame>=30) { expectedFrame=0x50000; expectedRepeat=half((uint16_t)expectedRepeat-1); }
    } else if(frame>=instance.action-1) {
        if(instance.state) expectedAction=4;
        expectedStop=1; result=1; goto verify;
    }
    mode=state.mode; delta=((angleResult&4095)-(uint16_t)instance.rot_y)&4095;
    if(mode==0) {
        if(state.speed<=1280) { expectedYaw=half(turnResult); expectedTurns=1; }
        if(state.speed<=state.targetSpeed && ((expectedYaw-(angleResult&4095)+64)&4095)<=256) {
            expected.mode=1; expected.targetSpeed=1536; expectedAux=0;
        }
    } else if(mode==1) {
        if(state.targetSpeed>=state.speed && ((delta-1536)&4095)<1024) {
            expected.mode=0; expected.targetSpeed=512;
        }
    } else expected.targetSpeed=0;
    if(expected.speed<expected.targetSpeed) expected.speed=half((uint16_t)expected.speed+128);
    else if(expected.speed>expected.targetSpeed) expected.speed=half((uint16_t)expected.speed-32);
    expectedSpeed=expected.speed;
    phase=state.heightPhase;
    if(mutate) {
        expected.speed=-1234; phase=-2;
        expectedPosition[0]=INT_MAX; expectedPosition[1]=INT_MIN; expectedPosition[2]=-123;
    }
    expectedPosition[0]=word((uint32_t)expectedPosition[0]+((uint32_t)-32768<<12));
    expectedPosition[2]=word((uint32_t)expectedPosition[2]+((uint32_t)32767<<12));
    if(phase<16) {
        int sample=TABLE[(((uint32_t)(phase+1)<<8)&0x3F00)/2];
        int height=(4096-sample)/8-128;
        expected.heightPhase=half(phase+1);
        expectedPosition[1]=word((uint32_t)((mutate ? -32768:D_800942EC)-height)<<16);
    } else if(frame>=31) {
        int floor=word((uint32_t)D_800942EC<<16);
        expected.mode=2;
        expectedPosition[1]=floor<expectedPosition[1] ? word((uint32_t)expectedPosition[1]-0x100000):floor;
    }
verify:
    assert(RoomM350_RushController(1,&state)==result);
    assert(memcmp(&state,&expected,sizeof(state))==0);
    assert(action==expectedAction && D_8019A8BE==expectedStop && g_RoomM350RushPlayerHit==expectedAux);
    assert(g_RoomM350RushState==expectedRepeat && stopped==result && turns==expectedTurns);
    assert(stage==(result || instance.mode!=9 ? 0:5));
    for(k=0;k<3;k++) assert(POS(instance)[k]==expectedPosition[k]);
}
int main(void) {
    int n,m,j;
    static const int targets[]={-32768,-1536,-1,0,512,1280,1536,32767};
    for(n=0;n<8192;n++) TABLE[n]=half((uint32_t)n*12345+0x8000);
    reset(); transforms.translation[0]=INT_MIN; transforms.translation[1]=INT_MAX; transforms.translation[2]=-65537;
    assert(RoomM350_RushController(0,&state)==0 && state.asset==-9876);
    for(n=0;n<65536;n++) for(m=0;m<3;m++) for(j=0;j<8;j++) {
        reset(); state.speed=half(n); state.targetSpeed=targets[j]; state.mode=m;
        angleResult=n; turnResult=n^0x8765; run();
    }
    for(n=0;n<65536;n++) for(m=-1;m<=1;m++) {
        reset(); instance.anim.fixed=(uint32_t)n<<16; instance.action=n&255; g_RoomM350RushState=m;
        state.mode=2; instance.state=n&1 ? &owner:0; run();
    }
    for(n=0;n<65536;n++) for(m=0;m<2;m++) {
        reset(); state.heightPhase=half(n); state.mode=2; mutate=m;
        POS(instance)[1]=word((uint32_t)n*65537); D_800942EC=half(n); run();
    }
    for(n=0;n<256;n++) { reset(); instance.mode=n; run(); }
    reset(); assert(RoomM350_RushController(-1,&state)==0 && RoomM350_RushController(2,&state)==0 && stage==0);
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
