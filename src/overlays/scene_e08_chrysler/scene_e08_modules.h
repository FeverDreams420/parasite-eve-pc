#ifndef SCENE_E08_MODULES_H
#define SCENE_E08_MODULES_H

#include "pe1/room_module.h"

/* scene_e08 links four effect modules, each behind a class of seven
 * methods in class-table slot order (pe1/room_module.h). The first module
 * (fade layers and particle cluster) uses the shared class units and the
 * room's g_Room* lists, with its own registration and start methods
 * (SceneE08_ModuleStart.c), which act whatever the field engine's status.
 * The other three modules each have a class unit of their own with the
 * same methods over their own script and lists, named in the symbol file. */
extern u16 g_SceneE08FadeSpawnLayout[];

int SceneE08_RegisterDrawList(PmSlotHeader *o);
int SceneE08_StartModule(PmSlotHeader *o);

/* Second module: emitter and fade slots, the player orb and its ring
 * sprites (SceneE08_OrbClass.c). */
extern int g_SceneE08OrbScript[];
extern FieldFxHandler g_SceneE08OrbDrawList[];
extern FieldFxHandler g_SceneE08OrbUpdateList[];
extern FieldFxHandler g_SceneE08OrbInitList[];
extern u16 g_SceneE08OrbSpawnLayout[];
void SceneE08_OrbNop0(void);
void SceneE08_OrbPlantScript(PmSlotHeader *object);
int SceneE08_OrbSpawn6(PmSlotHeader *slot, int b, int c, intptr_t d, intptr_t e, intptr_t f);
int SceneE08_OrbRegister(PmSlotHeader *o);
int SceneE08_OrbStart(PmSlotHeader *o);
int SceneE08_OrbClose(PmSlotHeader *o);
void SceneE08_OrbNop6(void);

/* Third module: moving and particle slots, the glow model and the limb
 * beams (SceneE08_SlotsClass.c). */
extern int g_SceneE08SlotsScript[];
extern FieldFxHandler g_SceneE08SlotsDrawList[];
extern FieldFxHandler g_SceneE08SlotsUpdateList[];
extern FieldFxHandler g_SceneE08SlotsInitList[];
extern u16 g_SceneE08SlotsSpawnLayout[];
void SceneE08_SlotsNop0(void);
void SceneE08_SlotsPlantScript(PmSlotHeader *object);
int SceneE08_SlotsSpawn6(PmSlotHeader *slot, int b, int c, intptr_t d, intptr_t e, intptr_t f);
int SceneE08_SlotsRegister(PmSlotHeader *o);
int SceneE08_SlotsStart(PmSlotHeader *o);
int SceneE08_SlotsClose(PmSlotHeader *o);
void SceneE08_SlotsNop6(void);

/* Fourth module: effect palettes, the layered sprite and the arm glow
 * (SceneE08_LayeredSpriteClass.c). */
extern int g_SceneE08LayeredSpriteScript[];
extern FieldFxHandler g_SceneE08LayeredSpriteDrawList[];
extern FieldFxHandler g_SceneE08LayeredSpriteUpdateList[];
extern FieldFxHandler g_SceneE08LayeredSpriteInitList[];
extern u16 g_SceneE08LayeredSpriteSpawnLayout[];
void SceneE08_LayeredSpriteNop0(void);
void SceneE08_LayeredSpritePlantScript(PmSlotHeader *object);
int SceneE08_LayeredSpriteSpawn6(PmSlotHeader *slot, int b, int c, intptr_t d, intptr_t e, intptr_t f);
int SceneE08_LayeredSpriteRegister(PmSlotHeader *o);
int SceneE08_LayeredSpriteStart(PmSlotHeader *o);
int SceneE08_LayeredSpriteClose(PmSlotHeader *o);
void SceneE08_LayeredSpriteNop6(void);

extern PmCommand g_SceneE08Classes[4];
void SceneE08_ResetModule(void);

#endif
