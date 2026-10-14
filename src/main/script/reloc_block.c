#include "common.h"
#include "pe1/entity_frame_update.h"

int DrawSync(int mode);
#include "pe1/reloc_block.h"
#include <stdlib.h>
#include <string.h>

RelocBlock *Script_RelocBlock(void *block) {
    static RelocBlock *resolved;
    u8 *bytes = block;
    u32 offset, count;
    if (!block) return NULL;
    if (block == resolved) {
        g_SceneDataTable1 = resolved;
        return resolved;
    }
    memcpy(&offset, bytes, sizeof(offset));
    memcpy(&count, bytes + 4, sizeof(count));
#if UINTPTR_MAX <= UINT32_MAX
    if (count > (SIZE_MAX - sizeof(RelocBlock)) / sizeof(u8 *)) return NULL;
#endif
    RelocBlock *next = malloc(sizeof(RelocBlock) + (size_t)count * sizeof(u8 *));
    if (!next) return NULL;
    next->u0.base = bytes + offset;
    next->count = count;
    for (u32 i = 0; i < count; ++i) {
        memcpy(&offset, bytes + 8 + (size_t)i * 4, sizeof(offset));
        next->offsets[i] = bytes + offset;
    }
    free(resolved);
    resolved = next;
    g_SceneDataTable1 = resolved;
    return resolved;
}

void Script_DrawSyncAndFlush(void) {
    unsigned int i;
    int offset;
    RelocBlock *block;

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
