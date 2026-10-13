import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
ROOM = 'src/overlays/room_m350_cruiser'
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import find_definition, host_unit, psx_compile  # noqa: E402
FUNCTION = 'RoomM350_DropController'


class AnimationEventTests(unittest.TestCase):
    @unittest.skipUnless((ROOT/'tools/old-gcc/cc1').is_file() and shutil.which('mipsel-none-elf-as'), 'PSX tools unavailable')
    def test_target_layout(self):
        unit = find_definition(FUNCTION, ROOM)
        source = unit.read_text() + r'''
#define OFF(t,f) ((unsigned long)&((t *)0)->f)
typedef char a[sizeof(Events)==32 && sizeof(Event)==4 ? 1:-1];
typedef char b[OFF(FieldActorState,action_state)==0x18 && OFF(Actor,instance)==8 ? 1:-1];
typedef char c[OFF(Instance,mode)==0xE && OFF(Instance,action)==0xF ? 1:-1];
typedef char d[OFF(Instance,anim)==0x14 && OFF(Instance,anim_prev)==0x18 ? 1:-1];
typedef char e[OFF(Instance,pos_x)==0x28 && OFF(Instance,rot_y)==0x3A ? 1:-1];
typedef char f[OFF(Instance,render_object.model_matrix)==0x1E8 && OFF(RenderMatrix,translation)==0x14 ? 1:-1];
typedef char g[sizeof(Vector)==8 && sizeof(RenderMatrix)==32 ? 1:-1];
'''
        with tempfile.TemporaryDirectory() as directory:
            result = psx_compile(source, pathlib.Path(directory) / 'layout.o', unit)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    @unittest.skipUnless(shutil.which('cc'), 'host compiler unavailable')
    def test_events_and_helper_boundaries(self):
        source = host_unit(FUNCTION, under=ROOM)
        # The globals below alias one record and one table on the target;
        # drop their separate declarations and map them onto shared storage.
        for declaration in (r'extern unsigned char D_8019A854;',
                            r'extern unsigned char g_RoomM350DropBurstCount;',
                            r'extern unsigned char g_RoomM350DropDone;',
                            r'extern int D_8019A858;',
                            r'extern short D_8019A85C,g_RoomM350DropRepeats,g_RoomM350DropLungeTimer,g_RoomM350DropHitCooldown;',
                            r'extern GteSineWord rcossin_tbl\[\];',
                            r'extern short D_800966EE\[\];'):
            source, count = re.subn(declaration, '', source)
            self.assertGreater(count, 0, declaration)
        prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
static union { int words[8]; short halves[16]; unsigned char bytes[32]; } globals;
static union { int words[4097]; short halves[8194]; } trig;
#define D_8019A854 (globals.bytes[0])
#define D_8019A858 (globals.words[1])
#define g_RoomM350DropBurstCount (globals.bytes[5])
#define D_8019A85C (globals.halves[4])
#define g_RoomM350DropRepeats (globals.halves[5])
#define g_RoomM350DropLungeTimer (globals.halves[11])
#define g_RoomM350DropHitCooldown (globals.halves[12])
#define g_RoomM350DropDone (globals.bytes[26])
#define rcossin_tbl ((GteSineWord *)trig.words)
#define D_800966EE (trig.halves+1)
'''
        harness = prefix + source + r'''
FieldFxObjectPrefix *g_FieldFxOwner;
Instance *g_PlayerEntity;
Events g_RoomM350DropEvents;
static Actor actors[2];
static Instance instances[2],player;
static FieldActorState owner;
/* Actor coordinates are the pos_x/pos_y/pos_z triple. */
#define POS(actor) (&(actor).pos_x)
static unsigned char status;
static short cursor;
static int angles,turns,moves,expectedSpeed,expectedZ,mutate;
static int word(uint32_t x) { int y; memcpy(&y,&x,4); return y; }
static int half(unsigned int x) { return (x&65535)<32768 ? (int)(x&65535):(int)(x&65535)-65536; }
static int floor64(int x) { return x>=0 ? x/64:-((-x+63)/64); }
int FieldFx_VecToAngle(Vec3Fixed *a,Vec3Fixed *b) {
    assert((void *)a==player.render_object.model_matrix.translation);
    assert((void *)b==instances[0].render_object.model_matrix.translation);
    assert(++angles==1);
    if (mutate) {
        g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[1];
        instances[0].rot_y=-777;
        memset(&g_RoomM350DropEvents,255,sizeof(g_RoomM350DropEvents));
    }
    return 0x12348001;
}
int FieldFx_TurnToward(int yaw,int angle,int speed) {
    assert(++turns==1 && angles==1 && angle==-32767 && speed==expectedSpeed);
    assert(yaw==(mutate ? -777:123));
    return 0x1234ABCD;
}
GteShortVector *ApplyMatrixSV(const GteMatrix *m,const GteShortVector *in,GteShortVector *b) {
    GteShortVector *a=b;
    assert(++moves==1 && (const void *)m==&instances[0].render_object.model_matrix && in==b);
    assert(a->x==0 && a->y==0 && a->z==expectedZ);
    /* Only the helper boundary is modeled, not GTE arithmetic. */
    a->x=-32768; a->y=111; a->z=32767;
    if (mutate) {
        g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[1];
        POS(instances[0])[0]=INT_MIN;
        POS(instances[0])[2]=INT_MAX;
        g_RoomM350DropLungeTimer=-32768;
    }
    return b;
}
static void reset(void) {
    int i;
    memset(&globals,0,sizeof(globals));
    memset(instances,0,sizeof(instances));
    memset(&player,0,sizeof(player));
    actors[0].instance=(struct RoomM350Instance *)&instances[0];
    actors[1].instance=(struct RoomM350Instance *)&instances[1];
    g_FieldFxOwner=(FieldFxObjectPrefix *)&actors[0]; g_PlayerEntity=&player;
    owner.action_state=&status; instances[0].state=&owner; status=1;
    instances[0].action=64; instances[0].rot_y=123;
    POS(instances[0])[0]=INT_MAX-12;
    POS(instances[0])[1]=42;
    POS(instances[0])[2]=INT_MIN+12;
    for (i=0;i<8;i++) {
        g_RoomM350DropEvents.entries[i].frame=i==7 ? 255:7+i*4;
        g_RoomM350DropEvents.entries[i].slot=i;
        g_RoomM350DropEvents.entries[i].value=100+i;
        g_RoomM350DropEvents.entries[i].pad=0xA5;
    }
    cursor=0; angles=turns=moves=mutate=0;
}
static void run(int frame,int previous,int start,int animation,int repeat,int recoil,int cooldown,int hasOwner,int mutation) {
    int phase=0,result,expectedResult=0,expectedCursor=start,i;
    int expectedStatus=hasOwner ? 2:1,expectedRepeat=repeat,expectedMotion=recoil;
    int expectedTime=frame<<16,expectedCooldown=half((unsigned int)cooldown-(cooldown!=0));
    unsigned char flags[8]={0};
    int done=0,ended=0,doTurn=0,doMove=0;
    reset(); cursor=start; mutate=mutation;
    instances[0].anim.parts.integer=frame;
    instances[0].anim_prev=(int)((uint32_t)previous<<16);
    instances[0].mode=animation;
    if (!hasOwner) instances[0].state=0;
    g_RoomM350DropRepeats=repeat; g_RoomM350DropLungeTimer=recoil; g_RoomM350DropHitCooldown=cooldown;
    if (animation==6) {
        if (half(frame)>=6 && half(previous)<6) flags[5]=5;
        done=1;
    } else if (repeat>0) {
        if (half(frame)>=31) { expectedTime=0x60000; expectedRepeat--; }
    } else if (half(frame)>=63) {
        if (hasOwner) expectedStatus=4;
        ended=1; expectedResult=1; done=1;
    }
    if (!done) {
        if (frame>=7 && frame<15) phase=15-frame;
        else if (frame>=19 && frame<27) phase=27-frame;
        doTurn=phase!=0;
        expectedSpeed=floor64(half((unsigned)trig.words[phase*256]));
        for (i=0;i<8;i++) {
            Event *e=&g_RoomM350DropEvents.entries[expectedCursor];
            if (half(frame)<e->frame || half(previous)>=e->frame) break;
            flags[e->slot]=e->value;
            if (++expectedCursor==8) expectedCursor=2;
        }
        assert(i<8);
        if (recoil>0) {
            doMove=1;
            expectedZ=D_800966EE[((8-recoil)*512 & 0x3E00)/2]/256;
            expectedMotion=mutation ? 32767:recoil-1;
        }
    }
    result=RoomM350_DropController(1,&cursor);
    assert(result==expectedResult && cursor==expectedCursor);
    assert(status==expectedStatus && g_RoomM350DropRepeats==expectedRepeat);
    assert(g_RoomM350DropHitCooldown==expectedCooldown && g_RoomM350DropLungeTimer==expectedMotion);
    assert(g_RoomM350DropDone==ended && memcmp(flags,globals.bytes,8)==0);
    assert(instances[0].anim.fixed==expectedTime);
    assert(angles==doTurn && turns==doTurn && moves==doMove);
    assert(instances[0].rot_y==(doTurn ? half(0xABCD):123));
    assert(POS(instances[0])[1]==42);
    if (doMove) {
        int x=mutation ? INT_MIN:INT_MAX-12;
        int z=mutation ? INT_MAX:INT_MIN+12;
        assert(POS(instances[0])[0]==word((uint32_t)x+0x80000000u));
        assert(POS(instances[0])[2]==word((uint32_t)z+0x7FFF0000u));
    }
}
int main(void) {
    int f,p,c,i;
    for (i=0;i<4097;i++) trig.words[i]=word(0xFEDC0000u+(unsigned)(i*211));
    reset(); memset(&globals,0x55,sizeof(globals)); cursor=4;
    assert(RoomM350_DropController(0,&cursor)==0 && cursor==0);
    for(i=0;i<8;i++) assert(globals.bytes[i]==0);
    assert(D_8019A85C==0 && g_RoomM350DropLungeTimer==0 && g_RoomM350DropHitCooldown==0 && g_RoomM350DropDone==0);
    assert(g_RoomM350DropRepeats==0x5555 && globals.bytes[27]==0x55 && status==1);
    reset(); cursor=3; g_RoomM350DropHitCooldown=7;
    assert(RoomM350_DropController(-1,&cursor)==0 && cursor==3 && g_RoomM350DropHitCooldown==7);
    assert(RoomM350_DropController(2,&cursor)==0 && status==1);
    reset(); cursor=7;
    instances[0].anim.parts.integer=10; instances[0].anim_prev=8<<16;
    g_RoomM350DropEvents.entries[7].frame=9;
    expectedSpeed=floor64(half((unsigned)trig.words[5*256]));
    assert(RoomM350_DropController(1,&cursor)==0 && cursor==2 && globals.bytes[7]==107);
    for (i=0;i<256;i++) {
        reset(); status=i; instances[0].mode=6;
        assert(RoomM350_DropController(1,&cursor)==0 && status==(i==1 ? 2:i));
    }
    for (f=0;f<=40;f++) for(p=0;p<=40;p++) for(c=0;c<8;c++)
        run(f,p,c,0,1,8,3,1,0);
    for (f=0;f<=64;f++) for(p=0;p<=8;p++) {
        run(f,p,0,6,0,8,-32768,1,0);
        run(f,p,0,0,0,0,0,0,0);
    }
    for (i=0;i<65536;i++) {
        trig.words[2048]=word(0xF00D0000u+(unsigned)i);
        run(7,6,0,0,0,0,1,1,1);
        D_800966EE[0]=half(i);
        run(16,15,3,0,0,8,1,1,1);
    }
    run(31,30,6,0,1,1,-1,1,1);
    run(65535,65534,0,0,0,-32768,32767,1,0);
    run(32768,32767,0,0,0,32767,0,0,0);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / 'test.c'
            path.write_text(harness)
            executable = path.with_suffix('')
            result = subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-strict-aliasing', '-fwrapv', str(path), '-o', str(executable)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
