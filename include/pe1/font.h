#ifndef PE1_FONT_H
#define PE1_FONT_H

#include "common.h"
#include "pe1/disc_load_state.h"

/* Partial glyph-table layout recovered from the slot lookup at 0x80038BC0.
 * The first list begins at +4; the second list's count is at +0x1C.
 * Names describe observed lookups, not an identified original file format. */
typedef struct FontGlyphGroups {
    unsigned char unknown[2];
    unsigned char count;
    unsigned char codes[24];
} FontGlyphGroups;

typedef struct FontGlyphSlots {
    unsigned char count;
    unsigned char indices[100];
    unsigned char groupKeys[100];
    unsigned char unknownC9[100];
} FontGlyphSlots;

/* The disc buffer has one shared code count followed by 328-byte records.
 * The selected in-memory table prefixes one record with that shared count. */
typedef struct FontGlyphRecord {
    FontGlyphGroups groups;
    FontGlyphSlots slots;
} FontGlyphRecord;

typedef struct FontGlyphTable {
    unsigned char codeCount;  /* glyph codes in the loaded set */
    FontGlyphGroups groups;
    FontGlyphSlots slots;
} FontGlyphTable;

/* The selection byte is nine bytes before the table pointer. Navigation
 * derives its address from the pointer member's address in retail code. */
typedef struct FontGlyphSelectionState {
    unsigned char active;     /* set by the shuffled load, cleared by reset */
    unsigned char codeIndex;  /* index of the loaded code in g_FontGlyphCodeSequence */
    unsigned char code;       /* glyph code last passed to the loader */
    unsigned char selected;
    unsigned char loadFailed; /* set when the code index runs past the table */
    unsigned char unknown05[3];
    u32 seed;                 /* rand() seed of the code shuffle, 0 = none */
    FontGlyphTable *table;
} FontGlyphSelectionState;

PE1_STATIC_ASSERT(sizeof(FontGlyphRecord) == 328, font_glyph_record_size);
PE1_STATIC_ASSERT(sizeof(FontGlyphTable) == 329, font_glyph_table_size);

PE1_STATIC_ASSERT(PE1_OFFSETOF(FontGlyphTable, groups.codes) == 4,
                  font_group_codes_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FontGlyphTable, slots) == 0x1C,
                  font_slots_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FontGlyphSlots, groupKeys) == 0x65,
                  font_slot_group_keys_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FontGlyphSelectionState, selected) == 3,
                  font_selection_index_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FontGlyphSelectionState, seed) == 8,
                  font_selection_seed_offset);
#if UINTPTR_MAX == UINT32_MAX /* Historical runtime layout; not a glyph format. */
PE1_STATIC_ASSERT(PE1_OFFSETOF(FontGlyphSelectionState, table) == 0xC,
                  font_selection_table_offset);
PE1_STATIC_ASSERT(sizeof(FontGlyphSelectionState) == 0x10,
                  font_selection_state_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FontGlyphLoadState, buffer) == 0x94,
                  font_glyph_load_buffer_offset);
#endif

extern FontGlyphSelectionState g_FontSelectionState;
extern FontGlyphTable g_FontActiveGlyphTable;
extern unsigned char *g_SceneLoadScratchBuffer[];
extern u16 g_PeImageSectorTable[];

/* Historical name: returns a slot index, or 0xFF; performs no drawing.
 * A null table selects the current one (g_FontSelectionState.table). */
unsigned char Render_DrawTextDigit(FontGlyphTable *table, unsigned char mode);

int Render_GetOrLoadFontGlyph(unsigned char action);
unsigned char Render_FindFontGlyphSlot(void);
unsigned char Render_StepFontLoad(void);

/* Byte views of g_FontSelectionState.codeIndex and .selected. The step load
 * and the shuffled and by-code loads keep retail's instruction order only
 * when these bytes are read outside the record. */
extern unsigned char g_FontSelectionCodeIndex, g_FontSelectionSelectedSlot;
/* The code list: codes from index 2 on are shuffled by
 * Render_LoadFontGlyphAlt. */
extern unsigned char g_FontGlyphCodeSequence[];
int Render_LoadFontGlyph(int code);
unsigned char Render_SetFontGlyphByCode(unsigned char code);
int Render_LoadFontGlyphAlt(void);
int Render_GetFontSlotCode(int index);
int Render_ConsumeFontShuffleSeed(void);

/* Selection state queries and resets used by the map selection, the frame
 * loop and the script; the names predate the identification of the state. */
int Menu_IsEquipSlotActive(void);
int Menu_ConsumeEquipSlotFlag(void);
int Menu_GetEquipSlotStateOrIndex(void);
int Menu_FindSelectedEquipSlotItem(void);
int Menu_ResetEquipSlotState(void);

#endif
