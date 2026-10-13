/* CC1_FLAGS: -G8 -fno-strength-reduce */
/* MASPSX_FLAGS: -G8 --use-comm-section */
/* MASPSX_FORCE_G0: 1 */

#include "common.h"
#include "pe1/entity_allocation.h"
#include "pe1/game_state.h"
#include "pe1/player_entity.h"
#include "pe1/script_node.h"
#include "pe1/script_queue.h"

/* A pool record cell as the word retail adds the row and column offsets to. */
typedef union EntityPoolAddress {
    char *bytes;
    int *words;
    unsigned int word;
} EntityPoolAddress;

/* Entity pool reset and free-list construction share the pool globals and the
 * 14-entry 0x280-byte entity record block at g_EntityPool. */

int g_FieldMoveLock;
FieldActor *g_EntityFreeListHead;
FieldActor *g_FieldActorListHead;
FieldActor *g_CurrentEntity;
u16 g_EntityFreePoolCount;
FieldActor *g_PlayerEntity;
u32 g_EntitySerialCounter;


void Entity_ResetAllPools(void)
{
    unsigned int i;
    unsigned int row;
    int *ptr;
    EntityPoolAddress base, cell;
    unsigned int col;
    register unsigned int offset;

    g_FieldMoveLock = 0;

    i = 0;
    ptr = (int *)g_ScriptVariableBank;
    do {
        *ptr = 0;
        i++;
        ptr++;
    } while (i < 0x200);

    i = 0;
    ptr = g_ScriptNodeActiveFlags;
    do {
        i++;
        *ptr = 0;
    } while (i < 0x40);

    i = 0;
    base.bytes = g_EntityPool;
    row = 0;
    do {
        col = 0;
        offset = row;
        do {
            cell.word = offset + base.word;
            *cell.words = 0;
            col++;
            offset += 4;
        } while (col < 0xA0);
        i++;
        row += 0x280;
    } while (i < 0xE);

    g_EntityFreeListHead = 0;
    g_FieldActorListHead = 0;
    g_CurrentEntity = 0;
    g_EntityFreePoolCount = 0;
    g_PlayerEntity = 0;
    g_EntitySerialCounter = 0;
    g_GameState.flags &= 0xFFFFCFFF;
}

void Entity_InitFreePool(void)
{
    int i;
    int offset;
    char *base;
    char *next;

    i = 0;
    base = g_EntityPool;
    next = base + 0x280;
    offset = 0;
    g_EntityFreePoolCount = 0;
    g_FieldActorListHead = 0;
    g_EntityFreeListHead = (FieldActor *)base;

    do {
        *(void **)(D_800BEA94 + offset) = next;
        next += 0x280;
        i++;
        offset += 0x280;
    } while ((unsigned int)i < 13);

    D_800C0B14[0] = 0;
    offset = 0;
    do {
        EntityAllocationAddress block;

        block.entry = g_EntityAllocationTable;
        block.word += offset;
        block.entry->address = 0;
        offset += sizeof(EntityAllocationBlock);
    } while ((unsigned int)offset < sizeof(g_EntityAllocationTable));

    g_PlayerEntity = 0;
}
