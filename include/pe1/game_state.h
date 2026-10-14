#ifndef PE1_GAME_STATE_H
#define PE1_GAME_STATE_H

#include "pe1/game_state_types.h"

extern Pe1GameState g_GameState;
/* Field movement lock bits tested by the frame update, the actor list update
 * and the scene scripts (0x8009D2E8). */
extern int g_FieldMoveLock;

/* g_GameState.story_day_flags as its own byte symbol: Scene_SetStoryDay and
 * the battle update read and set it with absolute byte loads and stores. */
extern u8 g_DiscChangeFlags;
/* g_GameState.pending_story_day as an unsigned byte symbol. Matching debt:
 * Scene_SetStoryDay keeps the lbu and its separate sign extension only
 * through a volatile read; a plain one combines into lb (lev 28). */
extern volatile u8 g_PendingStoryDay;
/* Room and area bytes of g_GameState as their own symbols:
 * field_bg_cache_bank (the loaded texture page) and loaded_entity_bank
 * (the area type before the transition). */
extern s8 g_LoadedTexturePageId;
extern u8 g_SavedSceneAreaType;

/* g_GameState.flags as its own scalar symbol: the frame loop, the save
 * screens, the scene loader and the fx_common/menu_memcard overlays update
 * it with absolute loads and stores, where the record keeps its address in
 * a register. */
extern u32 g_GameStateFlagsWord;

/* g_GameState.scene_object.animation_source as its own scalar symbol:
 * Scene_UpdateBgDraw tests and sets it with absolute loads and stores. */
extern struct RenderObjectEntity *g_SceneObjectAnimationSource;

void Boot_Shutdown(void);

/* Records a started scene track in g_GameState.scene_audio. */
int CdRom_RegisterAudioSlot(int slot, int key, int bank, int volume);

int CdRom_FindAudioSlotIndex(int key);
int CdRom_GetAudioSlotByKey(int key);

#endif
