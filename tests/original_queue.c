#include "pe1/script_queue.h"
#include "pe1/script_state.h"
#include "pe1/script_expression.h"
#include "pe1/entity_allocation.h"
#include "pe1/player_entity.h"
#include "pe1/pad_state.h"
#include "pe1/boot.h"
#include "pe1/field_movement.h"
#include "pe1/geom_state.h"
#include "pe1/game_state.h"
#include "pe1/battle_level_up.h"
#include "pe1/field_collision.h"
#include "pe1/aya.h"
#include "pe1/inventory_slots.h"
#include "pe1/inventory_state.h"
#include "pe1/floor_polygon.h"
#include <stdio.h>
extern FieldActor *g_CurrentEntity;
/* Override selected resident entries to observe traversal in this fixture. */
static unsigned calls;
static int record(int **args)
{
    if (args[0]!=(int *)&g_CurrentEntity->pad_0AC[5] || args[1]!=(int *)&g_ScriptVariableBank[7] ||
        args[2]!=(int *)&g_ScriptOperandTable[3] || args[3]!=&g_ScriptNodeActiveFlags[9]) return 0;
    ++calls; *args[0]=11;*args[1]=22;*args[2]=33;*args[3]=44;
    return 1;
}
static int yield(int **args) { (void)args; ++calls; return 0; }
static int six(int **args)
{
    if (*args[0]!=1 || *args[4]!=5 || args[5]!=(int *)&g_ScriptVariableBank[9]) return 0;
    *args[5]=99; ++calls; return 0;
}
int main(void)
{
    FieldActor actor={0};
    /* Four operands: actor local, script variable, operand, node flag. */
    u32 script[]={1|(4u<<13)|((1u|(2u<<3)|(3u<<6)|(4u<<9))<<17),0,5,7,3,9,2,0};
    ScriptNode *node;
    Script_InitNodePool(); Script_InitNodeFreeList();
    node=Script_AllocNode(script,0);g_ScriptNodePool=node;g_CurrentEntity=&actor;
    g_ScriptQueueOpcodeHandlers[1]=record;g_ScriptQueueOpcodeHandlers[2]=yield;
    Script_RunQueue();
    if (calls!=2 || actor.pad_0AC[5]!=11 || g_ScriptVariableBank[7]!=22 ||
        g_ScriptOperandTable[3]!=33 || g_ScriptNodeActiveFlags[9]!=44 ||
        node->current.script!=script+8) return 1;
    node->current.script=script;node->active=2;calls=0;g_ScriptNodePool=node;
    Script_RunQueue();
    if (calls || node->active!=1 || node->current.script!=script) return 2;
    node->flags=0x10;g_ScriptNodePool=node;Script_RunQueue();
    if (calls || node->active!=1) return 3;
    node->flags=0;g_GameStateFlags=0x100;g_ScriptNodePool=node;Script_RunQueue();
    if (calls) return 4;
    g_PlayerEntity=&actor;g_ScriptNodePool=node;Script_RunQueue();
    if (calls!=2) return 5;
    g_GameStateFlags=0;g_PlayerEntity=0;
    {
        u32 extended[]={3|(6u<<13),2,1,2,3,4,5,9};
        node->current.script=extended;node->active=1;calls=0;
        g_ScriptQueueOpcodeHandlers[3]=six;g_ScriptNodePool=node;
        Script_RunQueue();
        if (calls!=1 || g_ScriptVariableBank[9]!=99 || node->current.script!=extended+8) return 6;
    }
    {
        u32 arithmetic[]={9|(4u<<13)|((2u<<3)<<17),0,0,7,0x7fffffff,1,2,0};
        node->current.script=arithmetic;node->active=1;calls=0;
        g_ScriptQueueOpcodeHandlers[9]=Script_EvalExpr;g_ScriptNodePool=node;
        Script_RunQueue();
        if (g_ScriptVariableBank[7]!=0x80000000 || calls!=1 || node->current.script!=arithmetic+8) return 7;
    }
    {
        u32 flow[]={10|(2u<<13)|(2u<<17),0,12,77,2|(1u<<13),0,3};
        node->current.script=flow;node->active=1;g_ScriptNodePool=node;
        g_ScriptQueueOpcodeHandlers[10]=Script_AssignValue;
        g_ScriptQueueOpcodeHandlers[2]=Entity_SetCurrentNodeActive;
        Script_RunQueue();
        if (g_ScriptVariableBank[12]!=77 || node->active!=3 ||
            g_ScriptCursorAddress!=(uintptr_t)(flow+7)) return 8;
    }
    {
        u32 jump[]={1u<<13,0,6,2|(1u<<13),0,1};
        actor.script_base=(u8 *)jump;
        node->current.script=jump;node->active=1;g_ScriptNodePool=node;
        g_ScriptQueueOpcodeHandlers[0]=Script_JumpToEntityOffset;
        Script_RunQueue();
        if (node->current.script!=jump+6 || node->active!=1) return 9;
    }
    {
        u32 death[]={1,0};
        node->current.script=death;node->active=1;g_ScriptNodePool=node;
        g_ScriptQueueOpcodeHandlers[1]=Entity_MarkCurrentDead;
        Script_RunQueue();
        if (!(actor.flags&0x10)) return 10;
    }
    {
        u32 fork_script[]={18|(1u<<13),0,12,2|(1u<<13),0,1,2|(1u<<13),0,5};
        ScriptNode *child;
        actor.flags=0;actor.script_base=(u8 *)fork_script;
        node->flags=0;node->current.script=fork_script;node->active=1;g_ScriptNodePool=node;
        g_ScriptQueueOpcodeHandlers[18]=Script_SpawnChildNode;
        Script_RunQueue();
        child=node->next;
        if (!child || child->current.script!=fork_script+9 || child->active!=5 ||
            node->current.script!=fork_script+6 || node->active!=1) return 11;
    }
    {
        u32 branch[]={5|(2u<<13),0,0,12,10|(2u<<13)|(2u<<17),0,20,99,2|(1u<<13),0,1};
        /* Skip assignment when zero, jumping to the yield at halfword 16. */
        branch[3]=16;actor.script_base=(u8 *)branch;node->next=0;
        node->current.script=branch;node->active=1;g_ScriptNodePool=node;
        g_ScriptQueueOpcodeHandlers[5]=Script_JumpIfZero;
        g_ScriptVariableBank[20]=0;Script_RunQueue();
        if (g_ScriptVariableBank[20] || node->current.script!=branch+11) return 12;
        branch[2]=1;node->current.script=branch;node->active=1;g_ScriptNodePool=node;
        Script_RunQueue();
        if (g_ScriptVariableBank[20]!=99) return 13;
    }
    for(unsigned mode=0;mode<4;++mode) {
        FieldActor owner={0};
        ScriptNode self={.seq=2,.active=1},peer={.seq=77,.flags=0x8000},other={.seq=88,.flags=0x2000};
        owner.script_node_lists[0]=&self;owner.script_node_lists[1]=&peer;owner.script_node_lists[2]=&other;
        self.next=&peer;peer.next=&other;
        u32 all[]={4,0,32,0};
        u32 by_id[]={24|(1u<<13),0,mode==1?0x10002:mode==2?0x1004d:99,32,0};
        self.current.script=mode?by_id:all;
        g_CurrentEntity=&owner;g_ScriptNodePool=&self;g_GameStateFlags=0;
        if(g_ScriptQueueOpcodeHandlers[4]!=Entity_MarkNodeFree ||
           g_ScriptQueueOpcodeHandlers[24]!=Entity_FindNodeById) return 14;
        Script_RunQueue();
        if(!(self.flags&0x10) || peer.flags!=(mode==0||mode==2?0x8010:0x8000) ||
           other.flags!=(mode==0?0x2010:0x2000) ||
           self.current.script!=(mode?by_id+(mode==1?3:5):all+4)) return 15;
    }
    {
        FieldActor owner={.flags=0x80000000};
        u32 script[]={19|(1u<<13),0,0x2000,45|(1u<<13),0,0x82,
                      43|(1u<<13),0,2,44|(1u<<13)|(2u<<17),0,22,
                      25|(1u<<13)|(2u<<17),0,23,32,0};
        ScriptNode task={.current.script=script,.active=1,.seq=0x9876};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;g_FieldMoveLock=0x400;
        g_ScriptVariableBank[22]=g_ScriptVariableBank[23]=0;
        Script_RunQueue();
        if(owner.flags!=0x80002000 || g_FieldMoveLock!=0x480 ||
           g_ScriptVariableBank[22]!=0x480 || g_ScriptVariableBank[23]!=0x9876 ||
           task.flags!=0x10 || task.current.script!=script+17) return 16;
    }
    for(int bit=-2;bit<66;++bit) {
        FieldActor owner={0};
        u32 modes=(2u|(2u<<6))<<17;
        u32 script[]={42|(2u<<13)|(2u<<17),0,24,(u32)bit,
                      41|(3u<<13)|modes,0,24,(u32)bit,25,
                      40|(2u<<13)|(2u<<17),0,24,(u32)bit,
                      41|(3u<<13)|modes,0,24,(u32)bit,26,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        g_ScriptVariableBank[24]=0x55555555;g_ScriptVariableBank[25]=g_ScriptVariableBank[26]=0xffffffff;
        Script_RunQueue();
        u32 mask=UINT32_C(1)<<((unsigned)bit&31);
        if(g_ScriptVariableBank[24]!=(0x55555555u&~mask) ||
           g_ScriptVariableBank[25]!=mask || g_ScriptVariableBank[26] ||
           task.flags!=0x10 || task.current.script!=script+20) return 17;
    }
    const u32 frame_requests[]={0,3,7,0x10003};
    for(unsigned i=0;i<4;++i) {
        FieldActor owner={.flags=0x8000,.action=6};owner.anim.fixed=-65536;
        u32 script[]={47|(1u<<13),0,frame_requests[i],48,0,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        u16 target=(u16)frame_requests[i];if(target>6) target=6;
        if(owner.anim_frame_target!=target || owner.flags!=0x8200 || task.active!=1 ||
           task.current.script!=script+3 || task.flags) return 18;
        owner.anim.fixed=(int)target*65536;g_ScriptNodePool=&task;Script_RunQueue();
        if(task.current.script!=script+5 || task.active!=1 || task.flags) return 19;
        g_ScriptNodePool=&task;Script_RunQueue();
        if(task.current.script!=script+7 || task.flags!=0x10) return 20;
    }
    for(unsigned variant=0;variant<5;++variant) for(unsigned field=0;field<5;++field) {
        FieldActor live={.type_id=7,.sub_id=3,.mode=123,.flags=0x80000000,.action=4};
        live.anim.fixed=-2*65536+123;
        FieldActor dead=live,owner={.type_id=8};dead.flags|=0x10;dead.next=&live;
        if(variant==2) { owner=live;owner.flags|=0x10; }
        u32 script[]={14|(4u<<13)|((2u<<9)<<17),0,field,variant>=3?0:7,variant==1?99:3,28,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_PlayerEntity=variant==4?NULL:&live;g_FieldActorListHead=&dead;
        g_ScriptNodePool=&task;g_GameStateFlags=0;g_ScriptVariableBank[28]=0x1234;
        Script_RunQueue();
        u32 expected[]={123,variant==2?0x80000010u:0x80000000u,(u32)-2,4,0x1234};
        if(g_ScriptVariableBank[28]!=(variant==1||variant==4?UINT32_MAX:expected[field]) ||
           task.flags!=0x10 || task.current.script!=script+8) return 21;
    }
    for(unsigned variant=0;variant<4;++variant) for(unsigned vector=0;vector<8;++vector) {
        FieldActor live={.type_id=7,.sub_id=3,
            .pos_x=11,.pos_y=-12,.pos_z=13,.base_x=21,.base_y=-22,.base_z=23,
            .motion_x=31,.motion_y=-32,.motion_z=33,.accel_x=41,.accel_y=-42,.accel_z=43,
            .gravity_x=51,.gravity_y=-52,.gravity_z=53,.rot_x=61,.rot_y=62,.rot_z=63,
            .delta_x=71,.delta_y=-72,.delta_z=73};
        FieldActor dead=live,owner={0};dead.flags=0x10;dead.next=&live;
        u32 modes=((2u<<9)|(2u<<12))<<17;
        u32 script[]={94|(7u<<13)|modes,2|(2u<<3),vector,
                      variant==1||variant==3?0:variant==2?99:7,3,29,30,31,32,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_PlayerEntity=variant==3?NULL:&live;g_FieldActorListHead=&dead;
        g_ScriptNodePool=&task;g_GameStateFlags=0;
        for(unsigned i=29;i<=32;++i) g_ScriptVariableBank[i]=0x1234;
        Script_RunQueue();
        int expected[3]={0x1234,0x1234,0x1234};
        if(variant<2 && vector<7) {
            expected[0]=11+10*vector;expected[1]=vector==5?62:-(12+10*(int)vector);expected[2]=13+10*vector;
        }
        if(g_ScriptVariableBank[29]!=(u32)expected[0] || g_ScriptVariableBank[30]!=(u32)expected[1] ||
           g_ScriptVariableBank[31]!=(u32)expected[2] || g_ScriptVariableBank[32]!=(variant<2?1u:UINT32_MAX) ||
           task.flags!=0x10 || task.current.script!=script+11) return 22;
    }
    const int directions[][3]={{-100,0,1024},{0,100,2048},{100,0,3072},{0,-100,4096}};
    const int yaws[]={0,512,4095};
    for(unsigned variant=0;variant<4;++variant) for(unsigned direction=0;direction<4;++direction)
        for(unsigned yaw=0;yaw<3;++yaw) {
        FieldActor live={.type_id=7,.sub_id=3,.pos_x=directions[direction][0]*65536,
                         .pos_z=directions[direction][1]*65536};
        FieldActor dead=live,owner={.rot_y=yaws[yaw]};dead.flags=0x10;dead.next=&live;
        u32 script[]={69|(3u<<13)|((2u<<6)<<17),0,
                      variant==1||variant==3?0:variant==2?99:7,3,33,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_PlayerEntity=variant==3?NULL:&live;g_FieldActorListHead=&dead;
        g_ScriptNodePool=&task;g_GameStateFlags=0;g_ScriptVariableBank[33]=0x1234;
        Script_RunQueue();
        int angle=directions[direction][2]-yaws[yaw];if(angle<0) angle+=4096;
        if(g_ScriptVariableBank[33]!=(variant<2?(u32)angle:UINT32_MAX) ||
           task.flags!=0x10 || task.current.script!=script+7) return 23;
    }
    for(unsigned variant=0;variant<4;++variant) for(unsigned signs=0;signs<8;++signs) {
        FieldActor live={.type_id=7,.sub_id=3,.pos_x=-21,.pos_y=22,.pos_z=-23};
        FieldActor dead=live,owner={.pos_x=signs&1?11:-11,.pos_y=signs&2?12:-12,.pos_z=signs&4?13:-13};
        dead.flags=0x10;dead.next=&live;
        u32 script[]={84|(3u<<13)|((2u<<6)<<17),0,
                      variant==1||variant==3?0:variant==2?99:7,3,34,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_PlayerEntity=variant==3?NULL:&live;g_FieldActorListHead=&dead;
        g_ScriptNodePool=&task;g_GameStateFlags=0;g_ScriptVariableBank[34]=0x1234;
        Script_RunQueue();
        u32 distance=(signs&1?32:10)+(signs&2?10:34)+(signs&4?36:10);
        if(g_ScriptVariableBank[34]!=(variant<2?distance:UINT32_MAX) ||
           task.flags!=0x10 || task.current.script!=script+7) return 24;
    }
    {
        FieldActor owner={.flags=0x8000,.pos_x=10*65536,.pos_y=20*65536,.pos_z=30*65536,.gravity_y=-2*65536};
        u32 script[]={23|(4u<<13),0,6*65536,10*65536,14*65536,2,22,0,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(owner.flags!=0x8002 || owner.motion_x!=2*65536 || owner.motion_y!=2*65536 ||
           owner.motion_z!=8*65536 || task.current.script!=script+6 || task.active!=1 || task.flags) return 25;
        owner.flags&=~2u;g_ScriptNodePool=&task;Script_RunQueue();
        if(task.current.script!=script+10 || task.flags!=0x10) return 26;
    }
    for(unsigned selector=1;selector<=4;++selector) {
        u32 entries[10]={0};
        FieldActor owner={.script_base=(u8 *)entries,.script_cursor_1a0=entries+1,.script_cursor_19c=entries+2};
        u32 script[]={20|(2u<<13),0,selector,6,2|(1u<<13),0,1,
                      20|(2u<<13),0,selector,UINT32_MAX,32,0};
        ScriptNode task={.current.script=script,.active=1,.next_value=(intptr_t)(entries+4)};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(owner.script_cursor_1a0!=(selector==1?entries+3:entries+1) ||
           owner.script_cursor_19c!=(selector==2?entries+3:entries+2) ||
           task.next_value!=(intptr_t)(selector==3?entries+3:entries+4) ||
           task.current.script!=script+7 || task.active!=1) return 27;
        g_ScriptNodePool=&task;Script_RunQueue();
        if(owner.script_cursor_1a0!=(selector==1?NULL:entries+1) ||
           owner.script_cursor_19c!=(selector==2?NULL:entries+2) ||
           task.next_value!=(selector==3?0:(intptr_t)(entries+4)) ||
           task.current.script!=script+13 || task.flags!=0x10) return 28;
    }
    for(unsigned flags=0;flags<16;++flags) {
        FieldActor owner={0};
        u32 script[]={240|(1u<<13)|(2u<<17),0,35,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        g_FieldMoveLock=(s32)(0x80000000u|flags);g_ScriptVariableBank[35]=0x1234;
        Script_RunQueue();
        if(g_ScriptVariableBank[35]!=(flags&1?0u:1u) || (u32)g_FieldMoveLock!=(0x80000000u|flags) ||
           task.current.script!=script+5 || task.flags!=0x10) return 29;
    }
    for(unsigned mode=0;mode<4;++mode) for(unsigned held=0;held<2;++held) {
        FieldActor owner={0};
        u32 script[]={17|(3u<<13)|((2u<<6)<<17),0,mode,0x80000000,36,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        g_FieldPadBits=g_FieldPadPressed=g_FieldPadReleased=held?0x80000000u:0;
        g_FieldPadHeldFrameCounts[31]=123;g_ScriptVariableBank[36]=0x1234;
        Script_RunQueue();
        if(g_ScriptVariableBank[36]!=(held?(mode==3?123u:1u):0u) ||
           task.current.script!=script+7 || task.flags!=0x10) return 30;
    }
    for(unsigned bit=0;bit<32;++bit) for(unsigned held=0;held<2;++held) {
        FieldActor owner={0};u32 mask=UINT32_C(1)<<bit;
        u32 script[]={17|(3u<<13)|((2u<<6)<<17),0,3,mask,36,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        g_FieldPadBits=held?mask:0;
        for(unsigned i=0;i<32;++i) g_FieldPadHeldFrameCounts[i]=100+i;
        g_ScriptVariableBank[36]=0x1234;
        Script_RunQueue();
        u32 operand=held&&bit!=31?31-bit:mask;
        if(g_ScriptVariableBank[36]!=(held?100+bit:0) || script[3]!=operand ||
           task.current.script!=script+7 || task.flags!=0x10) return 31;
    }
    for(unsigned player=0;player<2;++player) for(unsigned vector=0;vector<8;++vector) {
        FieldActor owner={.flags=0x80};
        u32 modes=((2u<<3)|(2u<<6)|(2u<<9))<<17;
        u32 script[]={11|(4u<<13),0,vector,65547,(u32)-22,65549,
                      12|(4u<<13)|modes,0,vector,37,38,39,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_PlayerEntity=player?&owner:NULL;g_ScriptNodePool=&task;g_GameStateFlags=0;
        g_GeomScroll.state.flags=0x40;
        for(unsigned i=37;i<=39;++i) g_ScriptVariableBank[i]=0x1234;
        Script_RunQueue();
        if(g_ScriptVariableBank[37]!=(vector<7?(vector==5?11u:65547u):0x1234u) ||
           g_ScriptVariableBank[38]!=(vector<7?(u32)-22:0x1234u) ||
           g_ScriptVariableBank[39]!=(vector<7?(vector==5?13u:65549u):0x1234u) ||
           g_GeomScroll.state.flags!=(player&&vector==0?0xc0:0x40) ||
           task.current.script!=script+14 || task.flags!=0x10) return 32;
        if(vector==0 && (owner.base_x!=owner.pos_x || owner.base_y!=owner.pos_y || owner.base_z!=owner.pos_z)) return 33;
    }
    const int speeds[]={0,16,4096,-16,-4096,65536};
    for(unsigned player=0;player<2;++player) for(unsigned i=0;i<6;++i) {
        FieldActor owner={0};
        u32 script[]={27|(1u<<13),0,(u32)speeds[i],32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_PlayerEntity=player?&owner:NULL;g_ScriptNodePool=&task;g_GameStateFlags=0;
        g_GeomScroll.state.position.focusHeight=0x1234;
        Script_RunQueue();
        int focus=((speeds[i]>>4)*3*128)/4096;
        if(owner.move_speed!=(u16)(speeds[i]>>4) ||
           g_GeomScroll.state.position.focusHeight!=(player?focus:0x1234) ||
           task.current.script!=script+5 || task.flags!=0x10) return 34;
    }
    {
        FieldActor owner={.type_id=8,.sub_id=3,.flags=0x80002000};
        u32 query=14|(4u<<13)|((2u<<9)<<17);
        u32 script[]={30,0,query,0,1,8,3,40,50,0,query,0,1,8,3,41,
                      55,0,query,0,1,8,3,42,57,0,query,0,1,8,3,43,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(g_ScriptVariableBank[40]!=0x80002080 || g_ScriptVariableBank[41]!=0x80002000 ||
           g_ScriptVariableBank[42]!=0x80006000 || g_ScriptVariableBank[43]!=0x80002000 ||
           owner.flags!=0x80002000 || task.current.script!=script+34 || task.flags!=0x10) return 35;
    }
    const int interp_rates[]={0,1,32,-2,0x10020};
    for(unsigned player=0;player<2;++player) for(unsigned i=0;i<5;++i) {
        FieldActor owner={.flags=0x8040};owner.render_object.flags_9C=0x40;
        g_PlayerBattleEntity=(RenderObjectEntity){.flags_9C=0x80};
        u32 script[]={147|(1u<<13),0,(u32)interp_rates[i],32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_PlayerEntity=player?&owner:NULL;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        int duration=(s16)interp_rates[i];if(!duration) duration=1;
        if(owner.render_object.fade_duration!=(s8)duration || owner.render_object.fade_red_step!=(u8)(128/duration) ||
           owner.render_object.flags_9C!=0x42 || owner.flags!=0x8040 ||
           g_PlayerBattleEntity.flags_9C!=(player?0x82:0x80) ||
           (player && g_PlayerBattleEntity.fade_duration!=(s8)duration) ||
           task.current.script!=script+5 || task.flags!=0x10) return 36;
    }
    const int game_modes[]={-1,0,1,2,6,0x7fffffff};
    for(unsigned i=0;i<6;++i) {
        FieldActor owner={0};
        u32 script[]={148|(1u<<13)|(2u<<17),0,44,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        g_BattleModeState=game_modes[i];g_ScriptVariableBank[44]=0x1234;
        Script_RunQueue();
        if(g_ScriptVariableBank[44]!=(u32)game_modes[i] || g_BattleModeState!=game_modes[i] ||
           task.current.script!=script+5 || task.flags!=0x10) return 37;
    }
    {
        FieldActor owner={0};
        u32 script[]={170,0,2|(1u<<13),0,1,171,0,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;g_GameState.flags=0x80000001;
        Script_RunQueue();
        if(g_GameState.flags!=0x80002001 || task.current.script!=script+5 || task.active!=1 || task.flags) return 38;
        g_ScriptNodePool=&task;Script_RunQueue();
        if(g_GameState.flags!=0x80000001 || task.current.script!=script+9 || task.flags!=0x10) return 39;
    }
    {
        u16 entries[16]={0};FieldActor owner={.script_base=(u8 *)entries};
        u32 script[]={172|(2u<<13),0,3,0x12345,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(g_EntityCollisionWallSlot!=(short *)(entries+3) || g_EntityCollisionWallParam!=(s16)0x12345 ||
           task.current.script!=script+6 || task.flags!=0x10) return 40;
    }
    {
        FieldActor owner={0};
        u32 script[]={174,0,44|(1u<<13)|(2u<<17),0,45,173,0,44|(1u<<13)|(2u<<17),0,46,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;g_FieldMoveLock=(s32)0x80000081u;
        Script_RunQueue();
        if(g_ScriptVariableBank[45]!=0x80000085 || g_ScriptVariableBank[46]!=0x80000081 ||
           (u32)g_FieldMoveLock!=0x80000081 || task.current.script!=script+12 || task.flags!=0x10) return 41;
    }
    {
        FieldActor owner={.type_id=8,.sub_id=3,.flags=0x80002000};
        u32 query=14|(4u<<13)|((2u<<9)<<17);
        u32 script[]={223,0,query,0,1,8,3,47,224,0,query,0,1,8,3,48,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(g_ScriptVariableBank[47]!=0x80002002 || g_ScriptVariableBank[48]!=0x80002000 ||
           owner.flags!=0x80002000 || task.current.script!=script+18 || task.flags!=0x10) return 42;
    }
    const int sine[]={0,65536,0,-65536},cosine[]={65536,0,-65536,0};
    for(unsigned quarter=0;quarter<4;++quarter) {
        FieldActor owner={0};
        u32 script[]={215|(2u<<13)|((2u<<3)<<17),0,quarter*1024,49,
                      216|(2u<<13)|((2u<<3)<<17),0,quarter*1024,50,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(g_ScriptVariableBank[49]!=(u32)sine[quarter] || g_ScriptVariableBank[50]!=(u32)cosine[quarter] ||
           task.current.script!=script+10 || task.flags!=0x10) return 43;
    }
    const int atan_cases[][3]={{0,1,0},{1,0,1024},{0,-1,2048},{-1,0,-1024}};
    for(unsigned i=0;i<4;++i) {
        FieldActor owner={0};
        u32 script[]={217|(3u<<13)|((2u<<6)<<17),0,(u32)atan_cases[i][0],(u32)atan_cases[i][1],51,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(g_ScriptVariableBank[51]!=(u32)atan_cases[i][2] ||
           task.current.script!=script+7 || task.flags!=0x10) return 44;
    }
    const u32 pop_words[]={0,1,0x80000000,0xffffffff,0x55555555,0xaaaaaaaa,0x7fffffff,0xff00ff00};
    const u32 pop_counts[]={0,1,1,32,16,16,31,16};
    for(unsigned i=0;i<8;++i) {
        FieldActor owner={0};
        u32 script[]={222|(2u<<13)|((2u<<3)<<17),0,pop_words[i],52,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(g_ScriptVariableBank[52]!=pop_counts[i] || task.current.script!=script+6 || task.flags!=0x10) return 45;
    }
    const int mesh_levels[]={0,96,127,128,-1,0x160};
    for(unsigned i=0;i<6;++i) {
        FieldActor owner={0};
        u32 script[]={225|(1u<<13),0,(u32)mesh_levels[i],32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        g_GeomScroll.state.position.meshTintLevel=1;g_GeomScroll.state.position.focusHeight=123;
        Script_RunQueue();
        if(g_GeomScroll.state.position.meshTintLevel!=(s8)mesh_levels[i] ||
           g_GeomScroll.state.position.focusHeight!=123 || task.current.script!=script+5 || task.flags!=0x10) return 46;
    }
    {
        FieldActor owner={.type_id=8,.sub_id=3,.flags=0x80002000};
        u32 query=14|(4u<<13)|((2u<<9)<<17);
        u32 script[]={220,0,query,0,1,8,3,53,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(owner.flags!=0x81002000 || g_ScriptVariableBank[53]!=0x81002000 ||
           task.current.script!=script+10 || task.flags!=0x10) return 47;
    }
    const unsigned capacities[]={0,10,50},bonuses[]={0,3,60};
    for(unsigned i=0;i<3;++i) for(unsigned j=0;j<3;++j) {
        unsigned limit=capacities[i]+bonuses[j];if(limit>50) limit=50;
        g_AyaSaveState.inventory_slot_count=capacities[i];g_InvBonusSlotCount=bonuses[j];
        for(unsigned slot=0;slot<50;++slot) g_AyaInventoryItems[slot]=slot>=limit||slot%3==0?1:0;
        FieldActor owner={0};
        u32 script[]={229|(1u<<13)|(2u<<17),0,54,228|(1u<<13)|(2u<<17),0,55,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(g_ScriptVariableBank[54]!=limit || g_ScriptVariableBank[55]!=(limit+2)/3 ||
           g_InvItemPtr!=g_AyaInventoryItems || g_InvSlotLimit!=(int)limit ||
           task.current.script!=script+8 || task.flags!=0x10) return 48;
    }
    {
        u16 entries[32]={0};FieldActor owner={.script_base=(u8 *)entries};
        u32 script[]={227|(2u<<13),0,3,0x10003,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if((void *)g_FloorPolygonVertices!=(void *)(entries+3) || g_FloorPolygonVertexCount!=3 ||
           task.current.script!=script+6 || task.flags!=0x10) return 49;
    }
    const int rotations[][3]={{4095,2,1},{4095,1,4096},{0,-1,4095},{0,8193,4097},{0,1024,1024},{-1,0,4095}};
    for(unsigned i=0;i<6;++i) {
        FieldActor owner={.rot_x=-123,.rot_y=rotations[i][0],.rot_z=321};
        u32 modes=((2u<<3)|(2u<<6)|(2u<<9))<<17;
        u32 script[]={196|(1u<<13),0,(u32)rotations[i][1],12|(4u<<13)|modes,0,5,56,57,58,32,0};
        ScriptNode task={.current.script=script,.active=1};
        g_CurrentEntity=&owner;g_ScriptNodePool=&task;g_GameStateFlags=0;
        Script_RunQueue();
        if(g_ScriptVariableBank[56]!=(u32)-123 || g_ScriptVariableBank[57]!=(u32)rotations[i][2] ||
           g_ScriptVariableBank[58]!=321 || task.current.script!=script+11 || task.flags!=0x10) return 50;
    }
    puts("Original interpreter operand addressing, yield, delay and player gating passed");
    return 0;
}
