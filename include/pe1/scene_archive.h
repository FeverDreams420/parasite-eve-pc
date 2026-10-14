#ifndef PE1_SCENE_ARCHIVE_H
#define PE1_SCENE_ARCHIVE_H

#include "pe1/game_state.h"

/* Historical address aliases of the scene pointer slots. Keep both readers
 * and writers on the canonical state; these are not separate arrays. */
#define g_LoadedSceneAssetBlock (&g_GameState.loaded_scene_archive)
#define g_SceneLoadScratchBuffer (&g_GameState.scene_load_scratch)

#endif
