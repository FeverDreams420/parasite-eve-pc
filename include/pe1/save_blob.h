#ifndef PE1_SAVE_BLOB_H
#define PE1_SAVE_BLOB_H

#include "common.h"
#include "pe1/aya.h"

typedef unsigned char SaveU8;

typedef struct SaveBytes2 {
    SaveU8 b[2];
} SaveBytes2;

typedef struct SaveBytes4 {
    SaveU8 b[4];
} SaveBytes4;

typedef struct SaveBytes8 {
    SaveU8 b[8];
} SaveBytes8;

typedef struct SaveBytes18 {
    SaveU8 b[0x18];
} SaveBytes18;

typedef struct SaveBytes70 {
    SaveU8 b[0x70];
} SaveBytes70;

typedef struct SaveBytes800 {
    SaveU8 b[0x800];
} SaveBytes800;

typedef struct SaveMetadataWindow {
    SaveU8 text[0x10];
} SaveMetadataWindow;

/* Byte image of the runtime save state, as the card file stores it. */
typedef struct SaveBytes12E4 {
    SaveU8 b[0x12E4];
} SaveBytes12E4;

/* Runtime save state, copied whole into the card file after its 0x100-byte
 * header. The two leading 0x10-byte windows each start with an 8-character
 * name that the rename screen edits (see SaveMetadataWindow). */
typedef struct SaveRuntimeState {
    union {
        struct {
    union {
        SaveMetadataWindow metadata[2];
        struct {
/* 0x00 */ SaveU8 saveName[8];
/* 0x08 */ s32 playTimeMinutes;
/* 0x0C */ s32 gameTimeMinutes;
/* 0x10 */ SaveU8 ayaName[0xD];
/* 0x1D */ u8 menuLayout; /* bits 0-1 action submenu, bit 2 column layout */
/* 0x1E */ u8 reserved1E;
/* 0x1F */ s8 drawBaseY;
        } header;
    } windows;
/* 0x20 */ SaveU8 ayaStats[0x3C];
/* 0x5C */ s16 playTimeClass;
/* 0x5E */ s16 currentMapNumber;
/* 0x60 */ s16 menuClampValue;
/* 0x62 */ s16 reserved62;
/* 0x64 */ u32 drawBlendColor;
/* 0x68 */ SaveU8 inventory[0x1064];
/* 0x10CC */ u16 categoryCounts[6];
/* 0x10D8 */ union {
        SaveU8 remainder[0x20C];
        struct {
            SaveU8 reserved10D8[0x1EC];
/* 0x12C4 */ SaveMetadataWindow cursorMetadata[2];
        };
    };
        };
        struct {
            SaveU8 playerPrefix[0x20];
            PlayerInventoryState player;
        };
    };
} SaveRuntimeState;

extern SaveRuntimeState g_SaveRuntimeState;
/* Both retail labels name 0x800C20A4, inside the saved runtime image. */
#define g_CursorRenderMetadataWindows (g_SaveRuntimeState.cursorMetadata)
#define g_CursorRenderDataBlock ((SaveU8 *)&g_SaveRuntimeState.cursorMetadata)

#endif /* PE1_SAVE_BLOB_H */
