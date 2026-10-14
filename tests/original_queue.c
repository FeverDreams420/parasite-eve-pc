#include "pe1/script_queue.h"
#include "pe1/script_state.h"
#include "pe1/script_expression.h"
#include "pe1/entity_allocation.h"
#include "pe1/player_entity.h"
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
    puts("Original interpreter operand addressing, yield, delay and player gating passed");
    return 0;
}
