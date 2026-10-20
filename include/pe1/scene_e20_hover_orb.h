#ifndef PE1_SCENE_E20_HOVER_ORB_H
#define PE1_SCENE_E20_HOVER_ORB_H

#include "pe1/asset_find.h"
#include "pe1/battle.h"
#include "pe1/field_actor.h"
#include "pe1/field_glow_line.h"
#include "pe1/fieldfx_task.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_matrix_slot.h"
#include "pe1/scene_archive.h"
#include "pe1/scene_e20_flare.h"

/* Scene e20 trail controller and its command/particle records.
 * Reserved members are observed offsets, not recovered field meanings. */
typedef RenderColor SceneE20Color;
typedef GteMatrix SceneE20Matrix;
typedef struct SceneEffectSlot {
    SceneE20Vec position, target;
    int duration, pending;
} SceneEffectSlot;
typedef struct SceneE20TrailEffect {
    SceneE20Vec position, rotation, endpoint, target, previousPosition;
    SceneE20Vec commandedEndpoint, previousEndpoint, trailHead, trailTail;
    s16 phase, timer, alpha, reserved4E, duration, reserved52;
    struct { SceneE20Vec head, tail; } history[7];
} SceneE20TrailEffect;
extern SceneE20Color g_SceneE20OrbBeamColor, g_SceneE20OrbEdgeColor, g_SceneE20OrbFlashColorOdd, g_SceneE20OrbFlashColorEven;
#include "pe1/player_entity.h"
extern void *g_SceneE20HoverOrbModel;
int SceneE20_HoverOrbController(int mode, void *memory, intptr_t argument);
intptr_t SceneE20_CommandEffectSlot(int command, intptr_t index, intptr_t value, intptr_t z);
extern FieldFxTaskProgram g_SceneE20HoverOrbProgram;
extern SceneEffectSlot g_SceneE20EffectSlots[4];
int rand(void);


extern s32 g_SceneE20HoverOrbHitCooldown;

PE1_STATIC_ASSERT(sizeof(SceneEffectSlot) == 0x18, scene_e20_slot_size);
PE1_STATIC_ASSERT(sizeof(SceneE20TrailEffect) == 0xC4, scene_e20_trail_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE20TrailEffect, history) == 0x54, scene_e20_history_offset);

#endif
