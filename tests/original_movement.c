#include "pe1/entity_frame_update.h"
#include "pe1/entity_allocation.h"
#include "pe1/script_state.h"
#include "pe1/script_queue.h"
#include "pe1/scene_flags.h"
#include <stdio.h>
/* Controlled input/dispatch tables are fixtures; game code is unchanged. */
int (*g_ScriptQueueOpcodeHandlers[8192])(int **);
SceneFlagRecord *g_EntityInputRulesByMode[18];
static unsigned rule_calls;
static void input(struct BattleEntity *ctx,int *mode)
{
    FieldActor *actor=(FieldActor *)ctx;(void)mode;
    ++rule_calls;actor->accel_x=actor->pad_0AC[5];
}
int main(void)
{
    FieldActor actor={0};FieldActorState state={0};
    SceneFlagRecord rules[]={{0x20,0,0,input},{0,0,0,0}};
    u32 script[]={10|(2u<<13)|(1u<<17),0,5,3,2|(1u<<13),0,1};
    ScriptNode *node;
    Script_InitNodePool();Script_InitNodeFreeList();
    node=Script_AllocNode(script,0);actor.script_node_lists[0]=node;
    actor.state=&state;actor.pos_x=100;actor.rot_y=7;
    actor.flags=2;actor.gravity_x=2;actor.motion_x=4;actor.delta_x=5;
    g_PlayerEntity=&actor;g_EntityInputRulesByMode[0]=rules;
    g_ScriptQueueOpcodeHandlers[10]=Script_AssignValue;
    g_ScriptQueueOpcodeHandlers[2]=Entity_SetCurrentNodeActive;
    Entity_IntegratePositionFull(&actor);
    if (actor.base_x!=100 || actor.saved_rot_y!=7 || actor.motion_x!=9 ||
        actor.pos_x!=114 || actor.pad_0AC[5]!=3 || rule_calls!=1) return 1;
    actor.base_x=999;g_GameStateFlags=0x100;
    node->current.script=script;node->active=1;actor.pad_0AC[5]=0;
    Entity_IntegratePositionConditional(&actor);
    if (actor.pos_x!=114 || actor.base_x!=999 || actor.pad_0AC[5]!=3) return 2;
    g_GameStateFlags=0;actor.script_node_lists[0]=0;
    actor.flags=0;actor.accel_x=0;actor.motion_x=1;actor.delta_x=0;actor.pos_x=INT32_MAX;
    Entity_IntegratePositionConditional(&actor);
    if (actor.pos_x!=INT32_MIN || actor.base_x!=INT32_MAX) return 3;
    g_FieldMoveLock=1;
    Entity_IntegratePositionFull(&actor);
    if (rule_calls!=1) return 4;
    g_FieldMoveLock=0;state.flags=0x80;actor.mode=1;
    g_EntityInputRulesByMode[17]=rules;
    Entity_IntegratePositionFull(&actor);
    if (rule_calls!=2) return 5;
    {
        u32 second[]={10|(2u<<13)|(1u<<17),0,5,20,2|(1u<<13),0,1};
        u32 third[]={10|(2u<<13)|(1u<<17),0,5,30,2|(1u<<13),0,1};
        actor.script_node_lists[0]=node;node->current.script=script;node->active=1;
        actor.script_node_lists[1]=Script_AllocNode(second,0);
        actor.script_node_lists[2]=Script_AllocNode(third,0);
        state.flags=0;actor.mode=0;actor.flags=0;actor.pos_x=0;
        actor.motion_x=actor.accel_x=actor.delta_x=0;
        Entity_IntegratePositionFull(&actor);
        if (actor.pad_0AC[5]!=30 || actor.accel_x!=30 || actor.pos_x!=30) return 6;
    }
    g_PlayerEntity=0;g_FieldMoveLock=0;
    puts("Original callback-before-motion order, freeze, input locks and word wrap passed");
    return 0;
}
