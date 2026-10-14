#include "common.h"
#include <string.h>
#include "pe1/field_actor.h"
#include "pe1/reloc_block.h"
#include "pe1/script_node.h"
#include "pe1/script_state.h"

/* COMMON supplies small-data metadata to MASPSX: these words are reached
 * through $gp, every other extern stays absolute. */
ScriptNode *g_ScriptNodeFreeListHead;
unsigned short g_ScriptNodeSeqCounter[1];
RelocBlock *g_SceneDataTable1;
ScriptNode *g_ScriptNodePool;

/* Original task-node pool; scene relocation routines are in reloc_block.c. */

void Script_InitNodePool(void) {
    unsigned int i;
    u32 *ptr;
    g_ScriptNodePool = 0;
    g_ScriptNodeSeqCounter[0] = 0;
    g_ScriptNodeFreeListHead = 0;

    memset(g_ScriptNodes, 0, sizeof(g_ScriptNodes));

    g_ScriptCursorAddress = 0;

    i = 0;
    ptr = g_ScriptOperandTable;
    while (i < 16) {
        *ptr++ = 0;
        i++;
    }

    g_SceneDataTable1 = 0;
}

void Script_InitNodeFreeList(void) {
    unsigned short i;

    i = 0;
    g_ScriptNodePool = 0;
    g_ScriptNodeFreeListHead = g_ScriptNodes;

    do {
        g_ScriptNodes[i].next = &g_ScriptNodes[i + 1];
        i++;
    } while (i < 0x47);

    g_ScriptNodes[71].next = 0;

    i = 0;
    do {
        g_ScriptNodeActiveFlags[i] = 0;
        i++;
    } while (i < 0x40);
}

ScriptNode *Script_AllocNode(unsigned int *script, ScriptNode *arg1) {
    ScriptNode *node;
    ScriptNode *next;
    unsigned short seq;
    unsigned short value;

    node = g_ScriptNodeFreeListHead;
    if (!node) return 0;
    next = node->next;
    g_ScriptNodeFreeListHead = next;

    if (arg1 != 0) {
        node->prev = arg1;
        next = arg1->next;
        node->next = next;
        if (next != 0) {
            next->prev = node;
        }
        arg1->next = node;
    } else {
        node->prev = 0;
        node->next = 0;
    }

    seq = g_ScriptNodeSeqCounter[0];
    value = 1;
    node->trigger_value = 0;
    node->current.script = script;
    node->next_value = 0;
    node->active = value;
    node->flags = 0;
    value = seq + 1;
    node->seq = seq;
    g_ScriptNodeSeqCounter[0] = value;

    return node;
}

void Script_GarbageCollectNodes(void) {
    FieldActor *group;
    u8 i;

    group = g_FieldActorListHead;
    if (group != 0) {
        do {
            i = 0;
            do {
                ScriptNode *node;
                ScriptNode *next;

                node = group->script_node_lists[i];
                if (node != 0) {
                    do {
                        next = node->next;
                        if ((group->flags & 0x10) || (node->flags & 0x10)) {
                            if (node->prev != 0) {
                                node->prev->next = next;
                            } else {
                                group->script_node_lists[i] = next;
                            }

                            if (node->next != 0) {
                                node->next->prev = node->prev;
                            }

                            node->next = g_ScriptNodeFreeListHead;
                            g_ScriptNodeFreeListHead = node;
                        }

                        node = next;
                    } while (node != 0);
                }

                i++;
            } while (i < 3);

            group = group->next;
        } while (group != 0);
    }
}

/* Original linker-allocated node and operand storage. */
ScriptNode g_ScriptNodes[72];
u32 g_ScriptOperandTable[16];
