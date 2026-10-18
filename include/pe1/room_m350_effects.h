#ifndef PE1_ROOM_M350_EFFECTS_H
#define PE1_ROOM_M350_EFFECTS_H

#include "pe1/fieldfx_task.h"

#include "pe1/gte_types.h"

typedef struct RoomM350ParticleMotion {
    union {
        GteShortVector rotation;
        struct {
            s16 speed;
            s16 yaw;
            s32 phase;
        } spawn;
    } value;
} RoomM350ParticleMotion;

typedef struct RoomM350ParticleTail {
    union {
        struct {
            s16 speed;
            s16 reserved12;
            s16 brightness;
            s16 frame;
            u8 hit;
            u8 reserved19[3];
        } active;
        struct {
            s16 size;
            s16 duration;
            s16 reserved14;
            s16 frame;
            u8 state;
            u8 reserved19[3];
        } spawn;
    } value;
} RoomM350ParticleTail;

typedef struct RoomM350Particle {
    GteShortVector position;
    RoomM350ParticleMotion motion;
    RoomM350ParticleTail tail;
} RoomM350Particle;

typedef struct RoomM350PointerQueue {
    GteShortVector *positions[4];
    /* The position queue follows the four pointer slots in room_m350 data. */
    GteShortVector pendingPositions[8];
    struct FieldActor *anchorActor;
    s16 repeats;
    s16 count;
} RoomM350PointerQueue;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350PointerQueue, pendingPositions) == 4*sizeof(void *),
                  room_m350_pointer_queue_positions_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350PointerQueue, anchorActor) == 4*sizeof(void *)+64,
                  room_m350_pointer_queue_actor_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350PointerQueue, count) == 5*sizeof(void *)+66,
                  room_m350_pointer_queue_count_offset);

typedef struct RoomM350PositionQueue {
    GteShortVector positions[8];
    struct FieldActor *anchorActor;
    s16 repeats;
    s16 pointerCount;
    s16 count;
} RoomM350PositionQueue;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350PositionQueue, anchorActor) == 0x40,
                  room_m350_position_queue_actor_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350PositionQueue, count) == 64+sizeof(void *)+4,
                  room_m350_position_queue_count_offset);

typedef struct RoomM350FlareParticle {
    s16 y;
    s16 z;
    s16 shade;
    s16 size;
} RoomM350FlareParticle;

typedef struct RoomM350CloudParticleState {
    union {
        struct {
            s16 animatedSize;
            s16 firstShade;
            s16 secondShade;
            s16 reserved12;
        } active;
        s32 spawnReserved[2];
    } value;
} RoomM350CloudParticleState;

typedef struct RoomM350CloudParticle {
    s32 *anchor;
    s16 position[3];
    s16 size;
    RoomM350CloudParticleState state;
} RoomM350CloudParticle;

typedef struct RoomM350RadialSpriteState {
    GteRotation rotation;
    s16 count, brightness, fade, size;
} RoomM350RadialSpriteState;

PE1_STATIC_ASSERT(sizeof(RoomM350RadialSpriteState) == 0x10,
                  room_m350_radial_sprite_state_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350RadialSpriteState, count) == 8,
                  room_m350_radial_sprite_state_count_offset);

typedef struct RoomM350RadialSpriteParticle {
    s16 position[3];
    s16 size;
} RoomM350RadialSpriteParticle;

PE1_STATIC_ASSERT(sizeof(RoomM350RadialSpriteParticle) == 8,
                  room_m350_radial_sprite_particle_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350RadialSpriteParticle, size) == 6,
                  room_m350_radial_sprite_particle_size_offset);

typedef struct RoomM350FadingParticle {
    GteShortVector position;
    s16 velocity, acceleration, brightness, size, phase, counter;
} RoomM350FadingParticle;

PE1_STATIC_ASSERT(sizeof(RoomM350FadingParticle) == 0x14,
                  room_m350_fading_particle_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350FadingParticle, velocity) == 8,
                  room_m350_fading_particle_velocity_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350FadingParticle, size) == 0xE,
                  room_m350_fading_particle_size_offset);

typedef struct RoomM350SineParticle {
    GteShortVector position;
    GteRotation rotation;
    s16 brightness;
    s16 size;
    s16 amplitude;
    s16 reserved16;
} RoomM350SineParticle;

typedef struct RoomM350Model {
    u8 reserved[2];
    u8 transformCount;
} RoomM350Model;

/* Runtime actor/state/task records use their canonical native layouts. */
typedef FieldActorState RoomM350EffectOwner;
typedef FieldActor RoomM350EffectInstance;
typedef FieldFxObjectPrefix RoomM350EffectActor;
typedef FieldFxTaskSlot RoomM350EffectEmitter;

PE1_STATIC_ASSERT(sizeof(RoomM350ParticleMotion) == 8,
                  room_m350_particle_motion_size);
PE1_STATIC_ASSERT(sizeof(RoomM350ParticleTail) == 12,
                  room_m350_particle_tail_size);
PE1_STATIC_ASSERT(sizeof(RoomM350Particle) == 28,
                  room_m350_particle_size);
PE1_STATIC_ASSERT(sizeof(RoomM350FlareParticle) == 8,
                  room_m350_flare_particle_size);
PE1_STATIC_ASSERT(sizeof(RoomM350CloudParticleState) == 8,
                  room_m350_cloud_particle_state_size);
PE1_STATIC_ASSERT(sizeof(RoomM350CloudParticle) == sizeof(void *)+16,
                  room_m350_cloud_particle_size);
PE1_STATIC_ASSERT(sizeof(RoomM350SineParticle) == 24,
                  room_m350_sine_particle_size);
PE1_STATIC_ASSERT(sizeof(RoomM350Model) == 3, room_m350_model_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350SineParticle, rotation) == 8,
                  room_m350_sine_particle_rotation_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350SineParticle, brightness) == 0x10,
                  room_m350_sine_particle_brightness_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350Model, transformCount) == 2,
                  room_m350_model_transform_count_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350CloudParticle, position) == sizeof(void *),
                  room_m350_cloud_particle_position_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350CloudParticle, size) == sizeof(void *)+6,
                  room_m350_cloud_particle_size_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350CloudParticle, state) == sizeof(void *)+8,
                  room_m350_cloud_particle_state_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350Particle, motion) == 8,
                  room_m350_particle_motion_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350Particle, tail) == 16,
                  room_m350_particle_tail_offset);


/* Native task and particle callbacks keep the original entry points. */
int RoomM350_AreaController(int mode, void *state, intptr_t argument);
int RoomM350_AreaDropletCallback(int mode, void *state, intptr_t argument);
int RoomM350_AreaDropletEmitter(int mode, void *state, intptr_t argument);
int RoomM350_AreaPlayerPulseCallback(int mode, void *state, intptr_t argument);
int RoomM350_AreaPlayerPulseEmitter(int mode, void *state, intptr_t argument);
int RoomM350_AreaSparkCallback(int mode, void *state, intptr_t argument);
int RoomM350_AreaSparkEmitter(int mode, void *state, intptr_t argument);
int RoomM350_AreaZoneEmitter(int mode, void *state, intptr_t argument);
int RoomM350_BodyDebrisCallback(int mode, void *state, intptr_t argument);
int RoomM350_BodyDebrisEmitter(int mode, void *state, intptr_t argument);
int RoomM350_BodyRingCallback(int mode, void *state, intptr_t argument);
int RoomM350_BodyRingEmitter(int mode, void *state, intptr_t argument);
int RoomM350_DropBurstCallback(int mode, void *state, intptr_t argument);
int RoomM350_DropBurstEmitter(int mode, void *state, intptr_t argument);
int RoomM350_DropCallback(int mode, void *state, intptr_t argument);
int RoomM350_DropController(int mode, void *state, intptr_t argument);
int RoomM350_DropEmitter(int mode, void *state, intptr_t argument);
int RoomM350_DropFlareCallback(int mode, void *state, intptr_t argument);
int RoomM350_DropFlareEmitter(int mode, void *state, intptr_t argument);
int RoomM350_DropHitCallback(int mode, void *state, intptr_t argument);
int RoomM350_DropHitEmitter(int mode, void *state, intptr_t argument);
int RoomM350_DropSplashCallback(int mode, void *state, intptr_t argument);
int RoomM350_DropSplashEmitter(int mode, void *state, intptr_t argument);
int RoomM350_DropTrailCallback(int mode, void *state, intptr_t argument);
int RoomM350_DropTrailEmitter(int mode, void *state, intptr_t argument);
int RoomM350_HandFlashCallback(int mode, void *state, intptr_t argument);
int RoomM350_HandFlashEmitter(int mode, void *state, intptr_t argument);
int RoomM350_HandGlowCallback(int mode, void *state, intptr_t argument);
int RoomM350_HandGlowEmitter(int mode, void *state, intptr_t argument);
int RoomM350_HandPulseCallback(int mode, void *state, intptr_t argument);
int RoomM350_HandPulseEmitter(int mode, void *state, intptr_t argument);
int RoomM350_HandSparkCallback(int mode, void *state, intptr_t argument);
int RoomM350_HandSparkEmitter(int mode, void *state, intptr_t argument);
int RoomM350_HandSweepController(int mode, void *state, intptr_t argument);
int RoomM350_HandSweepHitCallback(int mode, void *state, intptr_t argument);
int RoomM350_HandSweepHitEmitter(int mode, void *state, intptr_t argument);
int RoomM350_HandSweepTrapCallback(int mode, void *state, intptr_t argument);
int RoomM350_HandSweepTrapEmitter(int mode, void *state, intptr_t argument);
int RoomM350_OrbBurstCallback(int mode, void *state, intptr_t argument);
int RoomM350_OrbBurstEmitter(int mode, void *state, intptr_t argument);
int RoomM350_OrbCallback(int mode, void *state, intptr_t argument);
int RoomM350_OrbController(int mode, void *state, intptr_t argument);
int RoomM350_OrbEmitter(int mode, void *state, intptr_t argument);
int RoomM350_OrbHandSparkCallback(int mode, void *state, intptr_t argument);
int RoomM350_OrbHandSparkEmitter(int mode, void *state, intptr_t argument);
int RoomM350_OrbImpactCallback(int mode, void *state, intptr_t argument);
int RoomM350_OrbImpactEmitter(int mode, void *state, intptr_t argument);
int RoomM350_OrbTableFlashCallback(int mode, void *state, intptr_t argument);
int RoomM350_OrbTableFlashEmitter(int mode, void *state, intptr_t argument);
int RoomM350_OrbTrailCallback(int mode, void *state, intptr_t argument);
int RoomM350_OrbTrailEmitter(int mode, void *state, intptr_t argument);
int RoomM350_RushCloudCallback(int mode, void *state, intptr_t argument);
int RoomM350_RushCloudEmitter(int mode, void *state, intptr_t argument);
int RoomM350_RushController(int mode, void *state, intptr_t argument);
int RoomM350_RushFlareCallback(int mode, void *state, intptr_t argument);
int RoomM350_RushFlareEmitter(int mode, void *state, intptr_t argument);
intptr_t RoomM350_HandConfigure(int mode, intptr_t first, intptr_t second, intptr_t third);
intptr_t RoomM350_OrbConfigure(int mode, intptr_t first, intptr_t second, intptr_t third);
intptr_t RoomM350_AreaConfigure(int mode, intptr_t first, intptr_t second, intptr_t third);
intptr_t RoomM350_DropConfigure(int mode, intptr_t first, intptr_t second, intptr_t third);
intptr_t RoomM350_RushConfigure(int mode, intptr_t first, intptr_t second, intptr_t third);
typedef struct RoomM350TableFlashEntry {u8 index,alternate;} RoomM350TableFlashEntry;
extern RoomM350TableFlashEntry g_RoomM350OrbTableFlashEntries[13][2];

extern FieldFxTaskProgram g_RoomM350HandEffects;
extern u16 g_RoomM350HandScript[];
extern FieldFxTaskProgram g_RoomM350OrbEffects;
extern u16 g_RoomM350OrbScript[];
extern FieldFxTaskProgram g_RoomM350AreaEffects;
extern u16 g_RoomM350AreaScript[];
extern FieldFxTaskProgram g_RoomM350DropEffects;
extern u16 g_RoomM350DropScript[];
extern FieldFxTaskProgram g_RoomM350RushEffects;
extern u16 g_RoomM350RushScript[];
extern FieldFxTaskProgram g_RoomM350BodyEffects;
extern u16 g_RoomM350BodyScript[];
#endif
