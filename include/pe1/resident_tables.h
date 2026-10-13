#ifndef PE1_RESIDENT_TABLES_H
#define PE1_RESIDENT_TABLES_H

#include "common.h"

/*
 * Resident data pack at 0x800A8028: a 32 KiB block that Scene_LoadSceneData
 * reads from PE.IMG once at start-up. It begins with a directory of byte
 * offsets, each relative to the start of the pack, followed by the tables.
 * The lookups in src/main/text/tables.c resolve a table with
 * RESIDENT_TABLE(type, slot).
 */
typedef struct ResidentTableDirectory {
    /* 0x00 */ s32 reserved;
    /* 0x04 */ s32 text;             /* TextTableDirectory (text tables 0..4). */
    /* 0x08 */ s32 glyphDescriptors; /* DrawGlyphDescriptor[], 8 bytes each. */
    /* 0x0C */ s32 itemBaseData;     /* ItemDataRecord[], 0x20 bytes each. */
    /* 0x10 */ s32 statGrowth;       /* 0x200-byte growth tables; also ends itemBaseData. */
    /* 0x14 */ s32 levelStats;       /* AyaLevelStats[99]. */
    /* 0x18 */ s32 levelExp;         /* s32 cumulative EXP per level. */
    /* 0x1C */ s32 saveIcon;         /* Memory-card icon block (CLUT +0x14, image +0x40). */
    /* 0x20 */ s32 parasiteSpells;   /* ParasiteSpellEntry[]. */
    /* 0x24 */ s32 mapLocationIds;   /* u8 location index per map number. */
    /* 0x28 */ s32 glyphMetrics;     /* u8 advance width per glyph. */
    /* 0x2C */ s32 locationNames;    /* NUL-separated location names. */
} ResidentTableDirectory;

typedef union ResidentDataPack {
    ResidentTableDirectory directory;
    u8 bytes[0x8004];
} ResidentDataPack;

PE1_STATIC_ASSERT(sizeof(ResidentTableDirectory) == 0x30, resident_table_directory_size);

extern ResidentDataPack g_ResidentDataPack;

/* Table `slot` of the pack, viewed as an array of `type`. */
#define RESIDENT_TABLE(type, slot) \
    ((type *)&g_ResidentDataPack.bytes[g_ResidentDataPack.directory.slot])

void *Text_LookupMapLocationText(int map);
u8 *Text_GetMapLocationName(int map);
u8 *Text_GetSaveIconData(void);

#endif /* PE1_RESIDENT_TABLES_H */
