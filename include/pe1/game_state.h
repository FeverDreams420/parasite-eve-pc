#ifndef PE1_GAME_STATE_H
#define PE1_GAME_STATE_H

#include "pe1/game_state_types.h"

extern Pe1GameState g_GameState;
/* Field movement lock bits tested by the frame update, the actor list update
 * and the scene scripts (0x8009D2E8). */
extern int g_FieldMoveLock;
#define D_8009D2E8 g_FieldMoveLock

/* Original scalar symbols share the canonical native game-state fields. */
#define g_DiscChangeFlags (g_GameState.story_day_flags)
#define g_PendingStoryDay (g_GameState.pending_story_day)
#define g_CurrentStoryDay (&g_GameState.current_story_day)
#define g_LoadedTexturePageId (g_GameState.field_bg_cache_bank)
#define g_SceneAreaType (g_GameState.requested_entity_bank)
#define g_SavedSceneAreaType (g_GameState.loaded_entity_bank)
#define g_GameStateFlagsWord (g_GameState.flags)
/* Screen offsets at original game-state bytes 0xD8/0xD9. */
#define g_DrawBaseX ((signed char *)&g_GameState.scene_audio.tracks.reserved)
#define g_DrawBaseY (g_DrawBaseX[1])

/* g_GameState.scene_object.animation_source as its own scalar symbol:
 * Scene_UpdateBgDraw tests and sets it with absolute loads and stores. */

void Boot_Shutdown(void);

/* Records a started scene track in g_GameState.scene_audio. */
int CdRom_RegisterAudioSlot(int slot, int key, int bank, int volume);

int CdRom_FindAudioSlotIndex(int key);
int CdRom_GetAudioSlotByKey(int key);

#endif
