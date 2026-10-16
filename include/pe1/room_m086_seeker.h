#ifndef PE1_ROOM_M086_SEEKER_H
#define PE1_ROOM_M086_SEEKER_H

#include "common.h"
#include "pe1/asset_find.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/field_transform_point.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_matrix_slot.h"
#include "pe1/render_prim.h"
#include "pe1/room_floor.h"
#include "pe1/scene_archive.h"

/* room_m086 falling seeker (RoomM086_SeekerCallback): a turning model that flies
 * along its heading until it lands, bursts on the player (flagging the
 * battle actor), then rises and fades or bounces as a flaring spark. */
typedef struct RoomM086Seeker {
    /* 0x00 */ GteShortVector position;
    /* 0x08 */ GteShortVector heading;
    /* 0x10 */ s16 timer;
    /* 0x12 */ s16 state;
} RoomM086Seeker;

typedef struct RoomM086SeekerParams {
    /* 0x00 */ u32 size;
    /* 0x04 */ s16 distance;
} RoomM086SeekerParams;

#include "pe1/player_entity.h"
extern u8 *g_RoomM086SeekerModelB;

/* room_m086 seeker controller (RoomM086_SeekerController): follows joint 7 of the
 * actor, drifts and falls, then bursts into three seeker rings. */
typedef struct RoomM086SeekerController {
    /* 0x00 */ GteShortVector position;
    /* 0x08 */ GteShortVector velocity;
    /* 0x10 */ GteShortVector rotation;
    /* 0x18 */ s16 state;
    /* 0x1A */ s16 timer;
} RoomM086SeekerController;

typedef struct RoomM086ControllerParams {
    /* 0x00 */ u32 size;
    /* 0x04 */ s32 reserved04;
    /* 0x08 */ s32 spacing;
    /* 0x0C */ u32 count;
} RoomM086ControllerParams;

typedef struct RoomM086Offset {
    s16 x, y, z, pad;
} RoomM086Offset;

extern RoomM086Offset g_RoomM086JointAnchor,g_RoomM086LaunchVelocity;
extern u8 *g_RoomM086SeekerModelA;
#include "pe1/fieldfx_task.h"
extern int g_RoomM086ControllerParams[4],g_RoomM086BounceGlintParams[3];
extern FieldFxTaskProgram g_RoomM086SeekerProgram,g_RoomM086GlintProgram;
extern u16 g_RoomM086SeekerScript[16],g_RoomM086GlintScript[12];
int RoomM086_SeekerCallback(int mode, void *data, intptr_t argument);
int RoomM086_SeekerController(int mode, void *data, intptr_t argument);
int RoomM086_BounceGlintCallback(int mode, void *data, intptr_t argument);
int RoomM086_BounceGlint(int mode, void *data, intptr_t argument);
intptr_t RoomM086_SetControllerParams(int mode, intptr_t a, intptr_t b, intptr_t c);
intptr_t RoomM086_SetBounceGlintParams(int mode, intptr_t a, intptr_t b, intptr_t c);
#endif
