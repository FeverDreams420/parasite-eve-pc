#ifndef PE1_DISC_LOAD_STATE_H
#define PE1_DISC_LOAD_STATE_H

#include "common.h"

/* The disc-load view begins at the PE.IMG base LBA. Its destination pointer
 * is also read as g_SceneLoadScratchBuffer[0] while copying the selected glyph
 * table. */
typedef struct FontGlyphLoadState {
    u32 baseLba;
    u8 unknown04[0x90];
    u8 *buffer;
} FontGlyphLoadState;

/* PE.IMG base LBA, and the font loader's destination pointer. The process
 * manager, the boot disc table, the scene loader and the memcard video
 * player read sectors relative to baseLba. */
extern FontGlyphLoadState g_DiscLoadState;

#endif /* PE1_DISC_LOAD_STATE_H */
