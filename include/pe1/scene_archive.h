#ifndef PE1_SCENE_ARCHIVE_H
#define PE1_SCENE_ARCHIVE_H

#include "common.h"

/* 0x800B0E64: the loaded scene archive. The map loader reads its directory;
 * room and scene effects look their models and sounds up in it. Readers
 * load element 0, a record read; the incomplete array keeps the absolute
 * address in -G8 units. */
extern struct SceneAssetBlob *g_LoadedSceneAssetBlock[];

#endif
