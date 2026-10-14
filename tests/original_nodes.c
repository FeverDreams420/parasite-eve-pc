#include "pe1/script_node.h"
#include "pe1/script_state.h"
#include "pe1/entity_allocation.h"
#include <stdio.h>
#include <string.h>
extern FieldActor *g_FieldActorListHead;
int main(void)
{
    u32 script[]={123};
    ScriptNode *nodes[72]; FieldActor actor={0};
    memset(g_ScriptNodes,0xaa,sizeof(g_ScriptNodes));
    for (unsigned i=0;i<16;++i) g_ScriptOperandTable[i]=42;
    Script_InitNodePool(); Script_InitNodeFreeList();
    for (unsigned i=0;i<72;++i) {
        nodes[i]=Script_AllocNode(script,i?nodes[i-1]:0);
        if (nodes[i]!=&g_ScriptNodes[i] || nodes[i]->current.script!=script ||
            nodes[i]->seq!=i || nodes[i]->active!=1 || nodes[i]->flags) return 1;
        if (i && (nodes[i]->prev!=nodes[i-1] || nodes[i-1]->next!=nodes[i])) return 2;
    }
    if (Script_AllocNode(script,0)) return 3;
    for (unsigned i=0;i<16;++i) if (g_ScriptOperandTable[i]) return 4;
    actor.script_node_lists[0]=nodes[0];
    g_FieldActorListHead=&actor;
    nodes[0]->flags=nodes[35]->flags=nodes[71]->flags=0x10;
    Script_GarbageCollectNodes();
    if (actor.script_node_lists[0]!=nodes[1] || nodes[1]->prev ||
        nodes[34]->next!=nodes[36] || nodes[36]->prev!=nodes[34] || nodes[70]->next) return 5;
    if (Script_AllocNode(script,0)!=nodes[71]) return 6;
    actor.flags=0x10;
    Script_GarbageCollectNodes();
    if (actor.script_node_lists[0]) return 7;
    g_ScriptNodeSeqCounter[0]=65535;
    if (Script_AllocNode(script,0)->seq!=65535 || g_ScriptNodeSeqCounter[0]) return 8;
    g_FieldActorListHead=0;
    puts("Original 72-node pool, insertion, exhaustion, collection and sequence wrap passed");
    return 0;
}
