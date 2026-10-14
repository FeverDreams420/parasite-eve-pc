#ifndef PE1_SCENE_ROOM_LOAD_H
#define PE1_SCENE_ROOM_LOAD_H

#include "common.h"
#include "pe1/cdrom.h"
#include "pe1/field_actor.h"
#include "pe1/player_entity.h"
#include "pe1/render_lighting.h"
#include "pe1/render_setup.h"
#include "pe1/scene_assets.h"

/* Room load steps shared by the boot, scene and render code. */
/* Returns -2 when a serialized room descriptor has no native binding. */
int Scene_LoadRoom(unsigned int roomId);
int Render_InitRoomPrimState(void *object);
/* 14-state loader for the area entity texture bank. */
int Scene_LoadEntityTextures(void);
/* Steps the room TIM texture loader. */
int Asset_LoadTimTextures(int force);

/* Model allocation per actor type, given to actors of types below 10. */
extern uintptr_t g_SceneMapPrimBaseTable[];

/* Startup resource blob Scene_LoadSceneData copies out of the scratch buffer. */
extern unsigned char g_StartupResourceBlob[];

/*
 * Scene_LoadEntityTextures: 14-state loader for the area entity bank.
 * States 1..3 stream and upload the per-day TIM set, 4..6 stream the
 * entity bank rows, 7 reads the selected CD stream, 11..13 rebuild the
 * scene render object from the loaded bank.
 */

extern u8 g_SceneAreaType;
extern s8 g_CurrentStoryDay[];
extern u32 g_PeImageBaseLba;
extern u16 g_PeImageSectorTable[], g_PeImageSectorBoundsNext[];
extern u32 g_EntityRenderScratch[];
/* Aya action sequencer step (battle_runtime.h); a battle-start texture
 * load waits until it reaches 2. */
extern u8 g_AyaActionStep;

int Scene_UpdateBgDraw(void);
void Render_DrawWithAnim(RenderObjectEntity *object, void *animation, s16 frame,
                         union RenderLightingMatrix *light);

#endif /* PE1_SCENE_ROOM_LOAD_H */
