#ifndef PE1_MAP_SELECTION_H
#define PE1_MAP_SELECTION_H

#include "common.h"

typedef struct {
    u8 filename_index[24];
} MapSelectionIndexRow;

typedef struct {
    char name[8];
} MapSelectionName;

typedef char MapSelectionIndexRow_size_must_be_24_bytes[
    sizeof(MapSelectionIndexRow) == 24 ? 1 : -1];
typedef char MapSelectionName_size_must_be_8_bytes[
    sizeof(MapSelectionName) == 8 ? 1 : -1];

extern MapSelectionIndexRow g_MapSelectIndexTable[];
extern MapSelectionName g_MapFilenameTable[];

/* Map id strings parsed by MapSelectionFlow: the debug overrides, the
 * fixed start maps and the per-entry id table (8 bytes per map). */
extern s8 g_MapNameOverrideA[];
extern s8 g_MapNameOverrideB[];
extern s8 g_MapIdTable[];

#endif
