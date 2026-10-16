#ifndef PE1_ROOM_M023_BEACON_H
#define PE1_ROOM_M023_BEACON_H

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
#include "pe1/room_m023_effects.h"
#include "pe1/scene_archive.h"

/* room_m023 joint beacon (RoomM023_JointBeacon): follows a model joint, swells
 * and pulses through four phases, tags the player on contact and draws a
 * glow sprite pair, two star fans and a loaded model at the joint. */
typedef struct RoomM023Beacon {
    /* 0x00 */ GteShortVector position;
    /* 0x08 */ GteShortVector target;
    /* 0x10 */ GteShortVector rotation;
    /* 0x18 */ int height;
    /* 0x1C */ int width;
    /* 0x20 */ int joint;
    /* 0x24 */ int armed;
    /* 0x28 */ s16 phase;
    /* 0x2A */ s16 timer;
    /* 0x2C */ s16 glow;
    /* 0x2E */ s16 shade;
} RoomM023Beacon;

typedef struct RoomM023BeaconNode {
    /* 0x00 */ u32 flags;
    /* 0x04 */ u8 reserved04[0x14];
    /* 0x18 */ u8 *state;
} RoomM023BeaconNode;

typedef struct RoomM023BeaconPool {
    /* 0x000 */ RoomM023BeaconNode *node;
    /* 0x004 */ u8 reserved004[PE1_OFFSETOF(FieldActor, render_object) + PE1_OFFSETOF(RenderObjectEntity, matrices) - sizeof(void *)];
    /* 0x238 */ GteMatrix *matrix;
} RoomM023BeaconPool;

typedef struct RoomM023BeaconChannel {
    s32 reserved[2];
    RoomM023BeaconPool *pool;
} RoomM023BeaconChannel;

typedef struct RoomM023BeaconActor {
    u8 reserved[PE1_OFFSETOF(FieldActorState, flags)];
    u32 flags;
} RoomM023BeaconActor;

typedef struct RoomM023BeaconEntity {
    RoomM023BeaconActor *actor;
} RoomM023BeaconEntity;

typedef struct RoomM023BeaconEvent {
    /* 0x00 */ u8 reserved[PE1_OFFSETOF(FieldFxTaskContext, flags)];
    /* 0x0D */ u8 active;
    /* 0x0E */ u8 reserved0E[4];
    /* 0x12 */ s16 variant;
} RoomM023BeaconEvent;

typedef struct RoomM023BeaconColor {
    u8 r, g, b, code;
} RoomM023BeaconColor;

#include "pe1/player_entity.h"
extern char *g_RoomM023BeaconModel;
extern u8 g_RoomM023BeaconColorTrack[];

int FieldFx_IsPlayerInSegmentBand(void *position, GteShortVector *target, int radius);
GteMatrix *MulMatrix0(GteMatrix *first, GteMatrix *second, GteMatrix *out);

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM023BeaconPool, matrix) ==
    PE1_OFFSETOF(FieldActor, render_object) + PE1_OFFSETOF(RenderObjectEntity, matrices), m023_native_matrices);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM023BeaconActor, flags) == PE1_OFFSETOF(FieldActorState, flags), m023_native_state_flags);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM023BeaconNode, state) == PE1_OFFSETOF(FieldActorState, action_state), m023_native_action_state);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM023BeaconEvent, active) == PE1_OFFSETOF(FieldFxTaskContext, flags), m023_native_event_active);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM023BeaconEvent, variant) == PE1_OFFSETOF(FieldFxTaskContext, variables), m023_native_event_variant);

#endif
