#include "pe1/entity_allocation.h"
#include "pe1/render_buffers.h"

/* Original block allocator from allocation.c, using native addresses. */
unsigned char *g_RenderScratchBufferBase[1];

void *Entity_AllocBlock(u32 size) {
    s32 blocks;
    u8 slot;
    u32 count;
    u8 *scratch;
    u32 offset;
    u32 end;
    s32 free;
    u32 used;

    blocks = 8;
    if (size <= ENTITY_ALLOCATION_BLOCK_SIZE) {
        blocks = 1;
    } else if (size <= ENTITY_ALLOCATION_BLOCK_SIZE * 2) {
        blocks = 2;
    } else if (size <= ENTITY_ALLOCATION_BLOCK_SIZE * 4) {
        blocks = 4;
    }
    slot = 0;
    count = blocks & 0xFF;
    scratch = g_RenderScratchBufferBase[0];
    if (!scratch) return 0;
    while (slot < 0x10U) {
        offset = slot;
        if (g_EntityAllocationTable[slot].address == 0) {
            free = 1;
            if (slot < slot + count) {
                end = blocks + slot;
                if (end > 16) return 0;
                do {
                    EntityAllocationBlock *run = &g_EntityAllocationTable[offset];
                    if (run->address != 0) {
                        free = 0;
                        slot += blocks;
                        break;
                    }
                    offset++;
                } while (offset < end);
            }
            if (free != 0) {
                uintptr_t address;

                g_EntityAllocationTable[slot].blockCount = count;
                address = (uintptr_t)(scratch + slot * ENTITY_ALLOCATION_BLOCK_SIZE);
                g_EntityAllocationTable[slot].address = address;
                return (void *)address;
            }
        } else {
            used = g_EntityAllocationTable[slot].blockCount;
            if (count < used) {
                slot += used;
            } else {
                slot += blocks;
            }
        }
    }
    return 0;
}

void Entity_FreeAllocationBlock(uintptr_t arg0) {
    int i;

    i = 0;
    while ((unsigned int)(i & 0xFF) < 0x10U) {
        if (g_EntityAllocationTable[i & 0xFF].address == arg0) {
            g_EntityAllocationTable[i & 0xFF].address = 0;
            return;
        }
        i++;
    }
}

