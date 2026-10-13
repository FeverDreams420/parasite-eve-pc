#ifndef ROOM_M273_BOSS_H
#define ROOM_M273_BOSS_H

#include "common.h"
#include "pe1/asset_find.h"
#include "pe1/field_actor.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_texture_tables.h"
#include "pe1/field_vector_math.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_sine_table.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_libc.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"

#include "room_m273_boss_state.h"
#include "room_m273_effects.h"
#include "room_m273_sway.h"
#include "room_m273_sweep.h"

/* Boss instance fields read by room_m273's animation controllers. */
typedef struct RoomM273BossOwner {
    u32 flags;
    u8 reserved_04[4];
    s32 sound;
    u8 reserved_0C[0xC];
    u8 *status;
} RoomM273BossOwner;

typedef struct RoomM273BossInstance {
    RoomM273BossOwner *owner;
    u8 reserved_04[0x0A];
    u8 animation;
    u8 frame_count;
    u8 reserved_10[4];
    union {
        s32 fixed;
        struct {
            u16 fraction;
            u16 frame;
        } parts;
        struct {
            unsigned int fraction : 16;
            unsigned int frame : 16;
        } bits;
    } time;
    union {
        s32 fixed;
        struct {
            u16 fraction;
            u16 frame;
        } parts;
        struct {
            unsigned int fraction : 16;
            unsigned int frame : 16;
        } bits;
    } previous;
    u8 reserved_1C[0x0C];
    s32 x;
    s32 height;
    s32 z;
    u8 reserved_34[0x06];
    s16 yaw;
    u8 reserved_3C[0x1C0];
    s32 position[3];
    u8 reserved_208[0x30];
    GteMatrix *transforms;
    u8 reserved_23C[0x1A];
    s16 attack_angle;
    u8 reserved_258[2];
    u8 attack_kind;
    u8 attack_power;
} RoomM273BossInstance;

typedef struct RoomM273BossActor {
    u8 reserved_00[8];
    RoomM273BossInstance *instance;
} RoomM273BossActor;

typedef struct RoomM273BossPlayerActor {
    u8 reserved_00[0x4C];
    u32 flags;
} RoomM273BossPlayerActor;

/* 16.16 fixed-point coordinate. */
typedef union RoomM273Fixed {
    s32 value;
    struct {
        signed int fraction : 16;
        signed int integer : 16;
    } parts;
} RoomM273Fixed;

typedef struct RoomM273BossPlayer {
    RoomM273BossPlayerActor *actor;
    u8 reserved_04[0x0A];
    u8 mode;
    u8 reserved_0F[0x19];
    RoomM273Fixed location[3];
    u8 reserved_34[0x1C8];
    s32 position[3];
    u8 reserved_208[0x30];
    GteMatrix *transforms;
} RoomM273BossPlayer;

extern struct FieldActor *g_PlayerEntity;
/* Packed sine/cosine table: the low half is the sine, the high half the cosine. */
typedef struct RoomM273BossTrig {
    signed int sine : 16;
    signed int cosine : 16;
} RoomM273BossTrig;

/* Falling drop: position (the pad halfword counts frames after landing),
 * ring position, velocity and the player-contact flag. */
typedef struct RoomM273Drop {
    GteShortVector position;
    GteShortVector ring;
    s16 vx;
    s16 vy;
    s16 vz;
    s16 touched;
} RoomM273Drop;
PE1_STATIC_ASSERT(sizeof(RoomM273Drop) == 0x18, room_m273_drop_size);

/* Sweep attack frame windows at 0x8019AD80, one per side. */
typedef struct RoomM273SweepStep {
    s16 start;        /* animation frame set on entry */
    s16 loop_start;   /* frame the loop rewinds to; sweep window start */
    s16 loop_end;     /* sweep window end */
    s16 end;          /* animation done */
    s16 scale;        /* sweep angle per step (times the step) */
    s16 offset;       /* sweep angle offset */
} RoomM273SweepStep;

extern RoomM273SweepStep g_RoomM273SweepSteps[];

/* Ground ring (RoomM273_SwayGroundRingCallback): model draw helpers and the ring colours. */
extern RenderColor g_RoomM273SwayGroundRingColors[2];
int Math_IntSqrt(int value);
extern GteRotation g_RoomM273FloorRotation;
extern RenderColor g_RoomM273SweepDropColorA;
extern RenderColor g_RoomM273SweepDropColorB;
extern RenderColor g_RoomM273SweepDropColorC;
/* Shard spawned from the sway points toward the player. */
typedef struct RoomM273SwayShard {
    GteShortVector position;
    GteShortVector velocity;
    s8 side;
    u8 frame;
    u8 state;
} RoomM273SwayShard;

typedef RoomM273PoolContext RoomM273EffectPools;

extern void *g_RoomM273SwayShardTrailPool;
extern GteShortVector g_RoomM273SwayShardVelocity;
extern RenderColor g_RoomM273SwayShardRingColor;
extern RenderColor g_RoomM273SwayShardStarColor;
extern RenderColor g_RoomM273SwayShardInnerStarColor;
extern u8 g_RoomM273BlackColor[];

int FieldFx_UpdateCallbackList(void *list);
int RoomM273_SwayShardCallback(int mode, RoomM273SwayShard *shard);
int RoomM273_SwayShardTrailCallback(int mode, GteShortVector *position);

/* Falling trail (RoomM273_HandTrailCallback): 100-byte pool record that spins down to
 * the floor and keeps an eight-entry ring of two-point trail samples. */
typedef struct RoomM273FallingTrail {
    GteShortVector position;
    s16 pitch;
    s16 yaw;
    s16 spin;
    s16 landed;
    s16 trail_x[16];
    s16 trail_z[16];
    s16 trail_y[8];
    s16 head;
    s16 count;
} RoomM273FallingTrail;

extern GteShortVector g_RoomM273HandTrailDropOffset; /* drop offset */
extern GteShortVector g_RoomM273HandTrailSampleOffsetA; /* trail sample offsets */
extern GteShortVector g_RoomM273HandTrailSampleOffsetB;
extern RenderColor g_RoomM273HandTrailColor;

#endif
