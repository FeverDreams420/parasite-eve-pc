#ifndef PE1_ENTITY_ALLOCATION_H
#define PE1_ENTITY_ALLOCATION_H

#include "common.h"

/* Sixteen allocation slots: a run length followed by its scratch-buffer address.
 * A zero address marks a free slot. */
typedef struct EntityAllocationBlock {
    u32 blockCount;
    s32 address;
} EntityAllocationBlock;

PE1_STATIC_ASSERT(sizeof(EntityAllocationBlock) == 8, entity_allocation_block_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EntityAllocationBlock, address) == 4,
                  entity_allocation_block_address);
extern EntityAllocationBlock g_EntityAllocationTable[16];
/* Bytes per allocation block in the render scratch buffer. */
#define ENTITY_ALLOCATION_BLOCK_SIZE 0x47E0

/* An allocation slot addressed by its byte offset into the table. */
typedef union EntityAllocationAddress {
    EntityAllocationBlock *entry;
    u32 word;
} EntityAllocationAddress;

/* Entity pool storage cleared by Entity_ResetAllPools besides the script
 * variable bank (g_ScriptVariableBank): the 14 0x280-byte actor records at
 * g_EntityPool (D_800BEA94 is the free-list link word of the first) and the
 * pool's trailing record. */
extern char g_EntityPool[];
extern char D_800BEA94[];
extern int D_800C0B14[];
extern int g_FieldMoveLock;

/* Claims scratch-buffer slots for a model allocation of `size` bytes and
 * returns its address. */
void *Entity_AllocBlock(u32 size);

/* Field entity pool (entity/pool.c): the free-record list, the number
 * of live records and the serial number of the next allocated entity. */
struct FieldActor;
extern struct FieldActor *g_EntityFreeListHead;
extern u16 g_EntityFreePoolCount;
extern u32 g_EntitySerialCounter;

#endif
