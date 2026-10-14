#ifndef PE1_ENTITY_ALLOCATION_H
#define PE1_ENTITY_ALLOCATION_H

#include "common.h"
#include "pe1/field_actor.h"

/* Sixteen allocation slots: a run length followed by its scratch-buffer address.
 * A zero address marks a free slot. */
typedef struct EntityAllocationBlock {
    u32 blockCount;
    uintptr_t address;
} EntityAllocationBlock;



extern EntityAllocationBlock g_EntityAllocationTable[16];
/* Bytes per allocation block in the render scratch buffer. */
#define ENTITY_ALLOCATION_BLOCK_SIZE 0x47E0

/* Fourteen original actor slots, stored at the native actor stride. */
extern FieldActor g_EntityPool[14];
extern int g_FieldMoveLock;

/* Claims scratch-buffer slots for a model allocation of `size` bytes and
 * returns its address. */
void *Entity_AllocBlock(u32 size);
void Entity_FreeAllocationBlock(uintptr_t address);
void Entity_ResetAllPools(void);
void Entity_InitFreePool(void);

/* Field entity pool (entity/pool.c): the free-record list, the number
 * of live records and the serial number of the next allocated entity. */
struct FieldActor;
extern struct FieldActor *g_EntityFreeListHead;
extern u16 g_EntityFreePoolCount;
extern u32 g_EntitySerialCounter;

#endif
