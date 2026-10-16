#ifndef PE1_ROOM_EMBER_BURST_H
#define PE1_ROOM_EMBER_BURST_H

#include "common.h"
#include <stdio.h>
#include "pe1/field_actor.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/fieldfx_task.h"
#include "pe1/gte_types.h"
#include "pe1/player_entity.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"
#include "pe1/room_orbit_trail.h"

/* Ember burst controller (room_m318): attaches to the actor named by its
 * parameters, glows up, bursts orbiting sparks, embers and smoke, then
 * fades a halo and ring. */

typedef struct RoomEmberBurst {
    s16 x, y, z;                  /* 0x00 */
    s16 reserved06;
    s16 state;                    /* 0x08 */
    s16 timer;                    /* 0x0A */
    s16 glow;                     /* 0x0C */
} RoomEmberBurst;

typedef struct RoomEmberBurstParams {
    s32 subId;                    /* 0x00 */
    s32 typeId;                   /* 0x04 */
} RoomEmberBurstParams;

typedef RoomOrbitTrailPoolChannel RoomEmberBurstChannel;

extern char g_RoomM318GetMayaPosErrorFormat[];
extern RenderColor g_RoomM318EmberHaloColor;
extern char g_RoomM318GetMayaPosErrorFormat[];
extern RenderColor g_RoomM318EmberRingColor;
extern RenderColor g_RoomM318EmberHaloColor;

extern int rand(void);
extern int RoomM318_EmberBurstParticleCallback(int mode, RoomOrbitTrailParticle *p);

/* Ember fountain controller (RoomM318_EmberFountain): follows its actor for 141
 * frames while it sprays embers, smoke and orbiting sparks. */
typedef struct RoomEmberFountain {
    s16 x, y, z;                  /* 0x00 */
    s16 reserved06;
    s16 reserved08;
    s16 timer;                    /* 0x0A */
    s16 glow;                     /* 0x0C */
    s16 ringScale;                /* 0x0E */
} RoomEmberFountain;

typedef struct RoomEmberFountainChannel {
    s32 reserved[2];
    void *pool;                   /* 0x08 */
} RoomEmberFountainChannel;

extern GteShortVector g_RoomM318EmberFountainAnchor;
extern int RoomM318_EmberFountainParticleCallback(int mode, RoomOrbitTrailParticle *p);

/* Ember fountain particle (RoomM318_EmberFountainParticleCallback). */
extern GteRotation g_RoomM318ZeroSpin;
extern RenderColor g_RoomM318EmberFountainParticleColor;
extern u8 g_RoomM318EmberFountainParticleColorTrack[];

/* Twisting ember particle (RoomM318_EmberSpiralParticleCallback): its glow track and the bent
 * trail renderer it draws the orbiting sparks with. */
extern u8 g_RoomM318EmberSpiralColorTrack[];
extern void FieldFx_OffsetPointByAngles(void *, void *, int, void *, int, int);

/* Fan sweep controller (RoomM318_FanSweep): attaches to the actor named by its
 * parameters, fans out sparks on a turning angle into its own pool and
 * publishes that pool and its anchor for the pool's callbacks. */
typedef struct RoomFanSweep {
    s16 x, y, z;                  /* 0x00 */
    s16 reserved06;
    s32 angle;                    /* 0x08 */
    void *pool;                   /* 0x0C */
} RoomFanSweep;

/* Fan sweep spark: circles the published anchor on its angle at its
 * radius, sheds trail points, then swells, holds and fades. */
typedef struct RoomFanSweepSpark {
    s16 state;                    /* 0x00 */
    u16 angle;                    /* 0x02 */
    u16 timer;                    /* 0x04 */
    s16 x, y, z;                  /* 0x06 */
    s16 radius;                   /* 0x0C */
    u16 reserved0E;               /* 0x0E */
} RoomFanSweepSpark;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomFanSweepSpark, x) == 0x06,
                  room_fan_sweep_spark_position_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomFanSweepSpark, radius) == 0x0C,
                  room_fan_sweep_spark_radius_offset);
PE1_STATIC_ASSERT(sizeof(RoomFanSweepSpark) == 0x10,
                  room_fan_sweep_spark_size);

extern RenderColor g_RoomM318FanSweepSparkColor;
extern void *g_RoomM318FanSweepSparkPool;
extern RenderColor g_RoomM318FanSweepSparkColor;
extern GteShortVector g_RoomM318FanSweepAnchor;
extern int RoomM318_FanSweepSpark(int mode, void *data, intptr_t argument);
/* Main executable copy of the fan sweep spark (FieldFx_FanSweepSpark). */
extern void *g_FanSweepSparkTrailPool;
extern RenderColor g_FieldFxFanSweepSparkTint;
extern GteShortVector g_FanSweepSparkAnchor;
extern int FieldFx_FanSweepSpark(int mode, void *data, intptr_t argument);
extern int FieldFx_EmberSpark(int mode, void *data, intptr_t argument);

/* Main executable fan sweep controller (FieldFx_FanSweep) and the glowing
 * anchor drawn under it (FieldFx_FanSweepGlow). */
typedef struct FieldFanSweep {
    s32 angle;                    /* 0x00 */
    void *pool;                   /* 0x04 */
} FieldFanSweep;

typedef struct FieldFanSweepGlow {
    GteShortVector position;      /* 0x00 */
    s16 state;                    /* 0x08 */
    s16 timer;                    /* 0x0A */
} FieldFanSweepGlow;

typedef struct FieldFanSweepEvent {
    u8 reserved[PE1_OFFSETOF(struct FieldFxTaskContext, variables) + 6 * sizeof(s16)];
    s16 stage;                    /* 0x1E */
} FieldFanSweepEvent;

extern RenderColor g_FieldFxFanSweepGlowTint;
extern int RoomM318_FanSweepTrailCallback(int mode, s16 *position);
extern int FieldFx_UpdateCallbackList(void *pool);

/* Spiral drop controller (RoomM318_SpiralDrop): attaches to the actor named by
 * its parameters and drops falling sparks from a point that circles it on
 * a turning angle at a pulsing radius. */
typedef struct RoomSpiralDrop {
    s16 x, y, z;                  /* 0x00 */
    s16 reserved06;
    s32 angle;                    /* 0x08 */
    s32 radius;                   /* 0x0C */
} RoomSpiralDrop;

typedef struct RoomSpiralDropSpark {
    s16 x, y, z;                  /* 0x00 */
    s16 speed;                    /* 0x06 */
} RoomSpiralDropSpark;

typedef struct RoomSpiralDropSparkCallbackView {
    u16 x, y, z, speed;
} RoomSpiralDropSparkCallbackView;

typedef union RoomSpiralDropSparkRecord {
    RoomSpiralDropSpark emitter;
    RoomSpiralDropSparkCallbackView callback;
} RoomSpiralDropSparkRecord;

PE1_STATIC_ASSERT(sizeof(RoomSpiralDropSparkRecord) == 8,
                  room_spiral_drop_spark_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomSpiralDropSparkRecord, emitter.speed) ==
                      PE1_OFFSETOF(RoomSpiralDropSparkRecord, callback.speed),
                  room_spiral_drop_spark_speed_views_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomSpiralDropSparkRecord, callback.speed) == 6,
                  room_spiral_drop_spark_speed_offset);

extern u8 g_RoomM318SpiralDropColorTrack[];
extern int RoomM318_SpiralDropSparkCallback(int mode, volatile RoomSpiralDropSparkRecord *spark);

/* Attaches the effect to the first live actor with the given ids, or
 * reports the missing actor. */
static inline void RoomEffect_AttachToActor(int subId, int typeId, void *anchor) {
    FieldActor *actor;

    for (actor = g_FieldActorListHead; actor != 0; actor = actor->next) {
        if (actor != g_PlayerEntity && actor->state != 0 &&
            actor->state->control10.command_value > 0 &&
            actor->sub_id == subId && actor->type_id == typeId) {
            FieldFx_GetActorPosition((char *)actor, 0, (s16 *)anchor);
            return;
        }
    }
    printf(g_RoomM318GetMayaPosErrorFormat, subId, typeId);
}

#endif
