#include "common.h"
#include "pe1/battle_entity_anim.h"
#include "pe1/psyq_gpu.h"
#include "pe1/reloc_block.h"

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
