#include "common.h"
#include "pe1/battle_entity_anim.h"
#include "pe1/field_actor.h"
#include "pe1/psyq_gpu.h"
#include "pe1/reloc_block.h"
#include "pe1/script_node.h"
#include "pe1/script_state.h"

ScriptNode *g_ScriptNodeFreeListHead;
unsigned short g_ScriptNodeSeqCounter[1];
int g_SceneDataTable0;
RelocBlock *g_SceneDataTable1;
ScriptNode *g_ScriptNodePool;

/* Task-node pool and scene relocation-block setup. Pool initialization,
 * scene data relocation, the scene-map flush and free-list construction are
 * contiguous and share the node pool and g_SceneDataTable1 state. */

void Script_InitNodePool(void) {
    unsigned int i;
    unsigned int j;
    unsigned int rowOffset;
    ScriptNodeAddress base;
    u32 *ptr;

    i = 0;
    base.node = g_ScriptNodes;
    rowOffset = 0;

    g_ScriptNodePool = 0;
    g_ScriptNodeSeqCounter[0] = 0;
    g_ScriptNodeFreeListHead = 0;

    for (; i < 72; i++, rowOffset += sizeof(ScriptNode)) {
        unsigned int colOffset;
        ScriptNodeAddress cell;

        j = 0;
        colOffset = rowOffset;
        for (; j < 11; j++, colOffset += 4) {
            cell.word = colOffset + base.word;
            *cell.words = 0;
        }
    }

    g_SceneDataTable0 = 0;

    i = 0;
    ptr = g_ScriptOperandTable;
    while (i < 16) {
        *ptr++ = 0;
        i++;
    }

    g_SceneDataTable1 = 0;
}

/* Turn a loaded block's self-relative base and offsets into addresses;
 * a base already above 0x80000000 means the block was relocated before. */
RelocBlock *Script_RelocBlock(RelocBlock *block) {
    RelocAddress address;
    RelocU32 offset;
    RelocU32 i;
    RelocU32 count;
    RelocU32 *ptr;
    char frame[8];

    address.block = block;
    offset = address.block->u0.baseOffset;
    g_SceneDataTable1 = address.block;
    if (0x80000000U < offset) {
        return address.block;
    }

    i = 0;
    count = address.block->count;
    address.block->u0.baseOffset = address.word + offset;
    if (count != 0) {
        ptr = &address.block->u0.baseOffset;
        do {
            ptr[2] += address.word;
            i++;
            ptr++;
        } while (i < address.block->count);
    }

    return g_SceneDataTable1;
}

void Script_DrawSyncAndFlush(void) {
    unsigned int i;
    int offset;
    RelocBlock *block;
    char frame[8];

    DrawSync(0);
    block = g_SceneDataTable1;
    i = (unsigned char)block->u0.base[0] != 0;
    if (i != 0) {
        i = 0;
        offset = 1;
        do {
            Scene_LoadMap(block->u0.base + offset, 0, 1);
            block = g_SceneDataTable1;
            i++;
            offset += 2;
        } while (i < (unsigned char)block->u0.base[0]);
    }
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
