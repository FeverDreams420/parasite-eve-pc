#include "pe1/cdrom.h"
#include "pe1/font.h"

/* A failed poll restarts the whole load. Retail rebuilds the directory
 * address and the -1 sentinel on every pass, which a loop around the read
 * would hoist; the restart is a tail call, which GCC 2.7 turns into a jump
 * back to the function body without loop notes. The tail call needs an
 * int parameter: a u8 one is promoted at the call and fails the type
 * check of the tail-recursion test. */
int Render_LoadFontGlyph(int code)
{
    int status;
    int offset;
    int sourceOffset;
    int i;
    u8 *source;
    u8 *bytePtr;
    FontGlyphLoadState *state;
    u16 *range;
    state = &g_DiscLoadState;
    range = &g_PeImageSectorTable[79];
    do {
        status = CdRom_ReadSectorsFromLba(state->baseLba + range[0], state->buffer,
                                            range[1] - range[0]);
    } while (status == -1);
    while ((status = CdRom_PollReady()) != 0) {
        if (status == -1)
            return Render_LoadFontGlyph(code);
    }
    offset = (u8)code * sizeof(FontGlyphRecord);
    sourceOffset = offset + PE1_OFFSETOF(FontGlyphTable, groups.unknown[0]);
    source = g_SceneLoadScratchBuffer[0];
    i = 0;
    bytePtr = source + sourceOffset;
    g_FontActiveGlyphTable.groups.unknown[0] = *bytePtr;
    sourceOffset = offset + PE1_OFFSETOF(FontGlyphTable, groups.unknown[1]);
    bytePtr = source + sourceOffset;
    sourceOffset = offset + PE1_OFFSETOF(FontGlyphTable, groups.count);
    g_FontActiveGlyphTable.groups.unknown[1] = *bytePtr;
    g_FontActiveGlyphTable.groups.count = source[sourceOffset];
    sourceOffset = offset + PE1_OFFSETOF(FontGlyphTable, groups.codes);
    for (i = 0; i < 24; i++, sourceOffset++)
        g_FontActiveGlyphTable.groups.codes[i] = g_SceneLoadScratchBuffer[0][sourceOffset];
    g_FontActiveGlyphTable.slots.count = g_SceneLoadScratchBuffer[0][sourceOffset++];
    for (i = 0; i < 100; i++, sourceOffset++)
        g_FontActiveGlyphTable.slots.indices[i] = g_SceneLoadScratchBuffer[0][sourceOffset];
    for (i = 0; i < 100; i++, sourceOffset++)
        g_FontActiveGlyphTable.slots.groupKeys[i] = g_SceneLoadScratchBuffer[0][sourceOffset];
    for (i = 0; i < 100; i++, sourceOffset++)
        g_FontActiveGlyphTable.slots.unknownC9[i] = g_SceneLoadScratchBuffer[0][sourceOffset];
    return 0;
}
