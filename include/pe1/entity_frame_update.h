#ifndef PE1_ENTITY_FRAME_UPDATE_H
#define PE1_ENTITY_FRAME_UPDATE_H

/* Declarations used only by the per-frame entity driver Entity_FrameUpdate. */

#include "pe1/entity_animation.h"
#include "pe1/field_actor.h"
#include "pe1/game_state.h"
#include "pe1/player_entity.h"
#include "pe1/render_shadow.h"
#include "pe1/scene_room_load.h"
#include "pe1/scene_transition.h"

/* Global game-state flags (bit 4 = menu open, 2 = battle, 0x100 =
 * player-only animation) and the word after them. g_GameStateBlock is a
 * record view of g_GameStateFlags (a relative alias in
 * linkers/USA/undefined_syms_manual.txt). */
extern u32 g_FieldPadPressed;         /* pad trigger bits */
extern s16 g_MenuFrameCommand;         /* menu frame result */
extern BattleAction g_BattleMenuActionRecord;        /* menu frame argument */
extern u32 g_EntityRenderScratch[]; /* view matrix packet */
extern s16 g_CameraFocusHeight[];       /* camera height (integer part) */

void Battle_Update(void);
void Inv_OpenAyaItemList(unsigned int mode);
int Menu_RunFrameWithArg(void *arg);
void Entity_UpdateList(void);
void Entity_CopyParentPosition(void);
int Render_StepCameraTransition(void);
void Render_SetViewport(int *camera);
void Render_SetGteScreenOffset(void);
void Render_ResetGteScreenOffset(void);
void Render_DrawEntity(RenderObjectEntity *object, void *viewMatrix);
void Scene_UpdateEntityPositions(void);
void Script_GarbageCollectNodes(void);
void Entity_CollectGarbage(void);

#endif /* PE1_ENTITY_FRAME_UPDATE_H */
