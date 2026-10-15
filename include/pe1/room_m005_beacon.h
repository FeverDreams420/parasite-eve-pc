#ifndef PE1_ROOM_M005_BEACON_H
#define PE1_ROOM_M005_BEACON_H

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/asset_find.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/field_transform_point.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_matrix_slot.h"
#include "pe1/render_prim.h"
#include "pe1/room_floor.h"
#include "pe1/room_m005_drifting_sprite.h"
#include "pe1/scene_archive.h"

/* room_m005 joint beacon (RoomM005_JointBeacon): follows joint 0x14 of the actor,
 * sheds drifting sprites, swells and shrinks, tags the player on contact and
 * draws four passes of a model, three glow sprites and a turning second
 * model while the effect starts. */
typedef struct RoomM005Beacon {
    /* 0x00 */ GteShortVector position;
    /* 0x08 */ GteShortVector target;
    /* 0x10 */ GteShortVector rotation;
    /* 0x18 */ int height;
    /* 0x1C */ int width;
    /* 0x20 */ s16 phase;
    /* 0x22 */ s16 timer;
} RoomM005Beacon;

typedef struct RoomM005BeaconSprite {
    /* 0x00 */ s16 x, y, z;
    /* 0x06 */ s16 pad06;
    /* 0x08 */ s16 vx, vy, vz;
} RoomM005BeaconSprite;

typedef struct RoomM005BeaconSpawnChannel {
    s32 reserved[2];
    void *pool;
} RoomM005BeaconSpawnChannel;

typedef struct RoomM005BeaconNode {
    /* 0x00 */ u32 flags;
    /* 0x04 */ u8 reserved04[0x14];
    /* 0x18 */ u8 *state;
} RoomM005BeaconNode;

typedef struct RoomM005BeaconPool {
    /* 0x000 */ RoomM005BeaconNode *node;
    /* 0x004 */ u8 reserved004[PE1_OFFSETOF(FieldActor, render_object) + PE1_OFFSETOF(RenderObjectEntity, matrices) - sizeof(void *)];
    /* 0x238 */ GteMatrix *matrix;
} RoomM005BeaconPool;

typedef struct RoomM005BeaconChannel {
    s32 reserved[2];
    RoomM005BeaconPool *pool;
} RoomM005BeaconChannel;

typedef struct RoomM005BeaconActor {
    u8 reserved[PE1_OFFSETOF(FieldActorState, flags)];
    u32 flags;
} RoomM005BeaconActor;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM005BeaconPool, matrix) ==
    PE1_OFFSETOF(FieldActor, render_object) + PE1_OFFSETOF(RenderObjectEntity, matrices), m005_pool_native_matrices);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM005BeaconActor, flags) == PE1_OFFSETOF(FieldActorState, flags), m005_native_state_flags);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM005BeaconNode, state) == PE1_OFFSETOF(FieldActorState, action_state), m005_native_action_state);

typedef struct RoomM005BeaconEntity {
    RoomM005BeaconActor *actor;
} RoomM005BeaconEntity;

typedef struct RoomM005BeaconEvent {
    /* 0x00 */ u8 reserved[PE1_OFFSETOF(FieldFxTaskContext, flags)];
    /* 0x0D */ u8 active;
    /* 0x0E */ u8 reserved0E[4];
    /* 0x12 */ s16 variant;
} RoomM005BeaconEvent;
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM005BeaconEvent, active) == PE1_OFFSETOF(FieldFxTaskContext, flags), m005_native_event_active);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM005BeaconEvent, variant) == PE1_OFFSETOF(FieldFxTaskContext, variables), m005_native_event_variant);

typedef union RoomM005BeaconOffset {
    s32 word[2];
    GteShortVector vector;
} RoomM005BeaconOffset;

int RoomM005_JointBeacon(int mode, void *state, intptr_t argument);

typedef struct RoomM005BeaconColor {
    u8 r, g, b, code;
} RoomM005BeaconColor;

#include "pe1/player_entity.h"
extern RoomM005BeaconOffset g_RoomM005JointBeaconAnchor;
extern u8 *g_RoomM005BeaconModel0;
extern u8 *g_RoomM005BeaconModel1;
extern GteShortVector g_RoomM005BeaconOffset;
extern GteShortVector g_RoomM005BeaconCenter;

void FieldFx_ModelShiftColors(u8 *data, int r, int g, int b);
int FieldFx_IsPlayerInSegmentBand(void *position, GteShortVector *target, int radius);
int rand(void);

#endif
