#include "pe1/entity_frame_update.h"
#include "pe1/reloc_block.h"

int DrawSync(int mode);

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
