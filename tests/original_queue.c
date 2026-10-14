#include "pe1/script_queue.h"
#include "pe1/script_state.h"
#include "pe1/script_expression.h"
#include "pe1/entity_allocation.h"
#include "pe1/player_entity.h"
#include <stdio.h>
extern FieldActor *g_CurrentEntity;
/* Controlled handler table is a test fixture, not a game implementation. */
int (*g_ScriptQueueOpcodeHandlers[8192])(int **);
u32 g_GameStateFlags;
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
    puts("Original interpreter operand addressing, yield, delay and player gating passed");
    return 0;
}
