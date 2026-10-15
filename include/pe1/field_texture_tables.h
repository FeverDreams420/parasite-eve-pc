#ifndef PE1_FIELD_TEXTURE_TABLES_H
#define PE1_FIELD_TEXTURE_TABLES_H

#include "common.h"

/* Texture page slot table; g_FieldFxTpageSlot1 and g_FieldFxTpageSlot9 are entries 1 and 9. */
extern u16 g_FieldFxTpageSlots[16];
#define g_FieldFxTpageSlot1 (g_FieldFxTpageSlots[1])
#define g_FieldFxTpageSlot9 (g_FieldFxTpageSlots[9])
/* Palette (CLUT) table indexed by effect kind. */
extern u16 g_FieldFxPaletteTable[];
/* Slot table entries 2 and 3. */
#define g_FieldFxTpageSlot2 (g_FieldFxTpageSlots[2])
#define g_FieldFxTpageSlot3 (g_FieldFxTpageSlots[3])
/* Slot table entries 4 and 11 and palette table entry 3. */
#define g_FieldFxTpageSlot4 (g_FieldFxTpageSlots[4])
#define g_FieldFxTpageSlot11 (g_FieldFxTpageSlots[11])
#define g_FieldFxPaletteEntry3 (g_FieldFxPaletteTable[3])

/* View of one of those entries: a few effects read it as a one-field
 * record, which GCC orders after their particle-record stores. Effects that
 * index past an entry read the slot table itself (g_FieldFxTpageSlots[3] for
 * g_FieldFxTpageSlot3). */
typedef struct FieldTextureSlot {
    u16 value;
} FieldTextureSlot;
#define TEXTURE_SLOT_RECORD(slot) (((FieldTextureSlot *)&(slot))->value)

/* Texture page table indexed by a slot from g_FieldFxTpageSlots. */
extern u16 g_FieldFxTpageTable[4];
#define g_FieldFxTpageTableEntry1 (g_FieldFxTpageTable[1])
/* Selects the alternate palette for effect kind 4 when set. */
extern int g_FieldFxAltPaletteFlag;

#endif /* PE1_FIELD_TEXTURE_TABLES_H */
