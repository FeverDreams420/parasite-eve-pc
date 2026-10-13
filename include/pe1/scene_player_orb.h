#ifndef PE1_SCENE_PLAYER_ORB_H
#define PE1_SCENE_PLAYER_ORB_H

#include "common.h"
#include "pe1/akao_send.h"
#include "pe1/field_actor.h"
#include "pe1/field_glow_sprite.h"
#include "pe1/field_script_context.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_state.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/gte_types.h"
#include "pe1/player_entity.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_libc.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"
#include "pe1/scene_archive.h"

/* Scene e08 player orb: an effect spawned at a random spot around the
 * player, swung around the scene anchor by the anchor's facing, then drawn
 * as two colour-faded trails plus an eight-point sprite chain. */
typedef struct ScenePlayerOrbAnchor {
    /* 0x00 */ FieldActor *actor;
    /* 0x04 */ u8 pad04[0x14];
    /* 0x18 */ int x;
    /* 0x1C */ int y;
    /* 0x20 */ int z;
    /* 0x24 */ u8 mode;
    /* 0x25 */ u8 pad25[7];
    /* 0x2C */ s16 orbHit;
} ScenePlayerOrbAnchor;

/* The same scene object as its init (SceneE08_InitLayeredSpriteContext) fills it before
 * making it the script context: the source actor, a copy of that actor's
 * root bone matrix whose translation is the anchor position above, the
 * sprite asset handle whose low byte is the mode above, and the re-arming
 * hit timer (pe1/room_rearm_timer.h) the effects request through. */
typedef struct SceneE08Context {
    /* 0x00 */ FieldActor *source;
    /* 0x04 */ RenderMatrix rootMatrix;
    /* 0x24 */ s32 spriteAsset;
    /* 0x28 */ s16 hitReload;
    /* 0x2A */ s16 hitTimer;
    /* 0x2C */ s16 hitRequest;
} SceneE08Context;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08Context, spriteAsset) == 0x24,
                  scene_e08_context_sprite_asset_offset);

typedef struct ScenePlayerOrbTimer {
    /* 0x00 */ u8 pad00;
    /* 0x01 */ u8 state;
} ScenePlayerOrbTimer;

typedef struct ScenePlayerOrb {
    /* 0x00 */ GteShortVector position;
    /* 0x08 */ GteShortVector start;
    /* 0x10 */ GteShortVector anchor;
    /* 0x18 */ u8 pad18[0x10];
    /* 0x28 */ GteShortVector points[8];
    /* 0x68 */ GteShortVector trailA[3];
    /* 0x80 */ GteShortVector trailB[3];
    /* 0x98 */ RenderColor head;
    /* 0x9C */ RenderColor tail;
    /* 0xA0 */ int alpha;
    /* 0xA4 */ int fade;
    /* 0xA8 */ u8 delay;
    /* 0xA9 */ u8 active;
    /* 0xAA */ u8 count;
} ScenePlayerOrb;

extern GteRotation g_SceneE08PlayerOrbRotationSeed;
extern RenderColor g_SceneE08PlayerOrbTrailBColor;
extern GteVector g_SceneE08PlayerOrbPointScale;
extern u8 g_SceneE08PlayerOrbRamp[];
extern FieldGlowSprite g_SceneE08PlayerOrbPointSprite;

int *FieldFx_RegPtr(int index);

/* Scene e08 limb beams: ten ribbons fanned from the player's chest bone to
 * hand and foot bones, each with its own swing and bend offsets. */
typedef struct SceneLimbBeamShape {
    u16 rise;
    u16 bend;
    u16 reach;
    u16 swing;
} SceneLimbBeamShape;

typedef struct SceneLimbBeams {
    /* 0x00 */ u8 pad00[0xC];
    /* 0x0C */ s16 fade[10];
    /* 0x20 */ u8 pad20[4];
    /* 0x24 */ u8 mode[10];
} SceneLimbBeams;

extern SceneLimbBeamShape g_SceneE08LimbBeamOffsets[10];
extern u16 g_SceneE08LimbBeamAngles[10];
extern u8 g_SceneE08LimbBeamsHeadRamp[];
extern u8 g_SceneE08LimbBeamsTailRamp[];

/* Scene e08 arm glow: a ribbon from the player's hand bone (matrix 32) to
 * the elbow bone (matrix 22) with a scaled glow sprite on each end. */
typedef struct SceneArmGlow {
    /* 0x00 */ u8 pad00[8];
    /* 0x08 */ s16 scale;
    /* 0x0A */ u16 frame;
    /* 0x0C */ u16 offsets[5][3];
} SceneArmGlow;


extern FieldGlowSprite g_SceneE08ArmGlowSprite;
extern RenderColor g_SceneE08ArmGlowHeadColor;
extern RenderColor g_SceneE08ArmGlowTailColor;

#endif
