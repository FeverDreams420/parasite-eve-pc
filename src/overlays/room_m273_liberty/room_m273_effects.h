#ifndef ROOM_M273_EFFECTS_H
#define ROOM_M273_EFFECTS_H

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_types.h"
#include "pe1/s32_vector3.h"

#include "room_m273_particles.h"

/* The third word of the shared allocator context points at its pool. */
typedef struct RoomM273PoolContext {
    s32 reserved[2];
    void *pool;
} RoomM273PoolContext;

/* Common prefix read by the threshold-effect controller family. */
typedef FieldActor RoomM273EffectModeState;

typedef struct RoomM273PulseSeed {
    u8 reserved_00[2];
    u16 angle;
    u8 reserved_06[4];
} RoomM273PulseSeed;

/* Emitter interpretation of the shared 12-byte pool record. */
typedef struct RoomM273PulseEmitterView {
    u16 x;
    u16 y;
    u16 z;
    u16 size;
    u16 phase;
    u16 unknown10;
} RoomM273PulseEmitterView;

typedef struct RoomM273PairEmitterFields {
    u16 x;
    u16 y;
    u16 z;
    u16 flags;
} RoomM273PairEmitterFields;

typedef RoomM273PairEmitterFields RoomM273TemplatePoint;

/* The same eight-byte pool entry is also copied as four opaque halfwords. */
typedef union RoomM273PairPoolRecord {
    RoomM273PairEmitterFields emitter;
    u16 values[4];
} RoomM273PairPoolRecord;

/* Controller work buffers use parallel coordinate arrays and a shared count. */
typedef struct RoomM273PairedPulseBatch {
    u16 x[8];
    u16 y[8];
    u16 z[8];
    s16 count;
    u8 reserved_32[0x0E];
    u8 stopped;
} RoomM273PairedPulseBatch;

typedef struct RoomM273RisingBatchBuffer {
    u16 x[12];
    u16 y[12];
    u16 z[12];
    s16 count;
    u8 reserved_4A[0x13];
    u8 stopped;
} RoomM273RisingBatchBuffer;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PairedPulseBatch, count) == 0x30,
                  room_m273_paired_pulse_count_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PairedPulseBatch, stopped) == 0x40,
                  room_m273_paired_pulse_stopped_offset);
PE1_STATIC_ASSERT(sizeof(RoomM273PairedPulseBatch) == 0x42,
                  room_m273_paired_pulse_batch_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273RisingBatchBuffer, count) == 0x48,
                  room_m273_rising_batch_count_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273RisingBatchBuffer, stopped) == 0x5D,
                  room_m273_rising_batch_stopped_offset);
PE1_STATIC_ASSERT(sizeof(RoomM273RisingBatchBuffer) == 0x5E,
                  room_m273_rising_batch_buffer_size);

/* Position payload shared by the player-following effect callbacks. */
typedef Pe1S32Vector3 RoomM273WorldPosition;

typedef struct RoomM273PlayerTransform {
    u8 reserved_00[0x14];
    RoomM273WorldPosition position;
} RoomM273PlayerTransform;

/* The player record exposes the same trailing pointer as a transform or a
 * byte-addressed object, depending on the effect callback. */
typedef union RoomM273PlayerActorTail {
    RoomM273PlayerTransform *transform;
    u8 *object;
} RoomM273PlayerActorTail;

typedef FieldActor RoomM273PlayerActorView;

/* Several sprite pools store only a pointer to their position payload. */
typedef struct RoomM273PointerPoolEffect {
    void *position;
} RoomM273PointerPoolEffect;

typedef RoomM273PointerPoolEffect RoomM273SpritePoolEffect;

typedef struct RoomM273ThresholdTransform {
    u8 reserved_00[0x594];
    s32 x;
    s32 y;
    s32 z;
} RoomM273ThresholdTransform;

typedef FieldActor RoomM273ThresholdEntityState;

typedef struct RoomM273DirectedRingOwner {
    u8 reserved_00[8];
    void *value;
} RoomM273DirectedRingOwner;

typedef struct RoomM273DirectedRingState {
    RoomM273DirectedRingOwner *owner;
} RoomM273DirectedRingState;

typedef FieldActor RoomM273ScaledLayerOwner;

/* g_FieldFxOwner points at either view as each controller is active. */
typedef union RoomM273EffectStatePointer {
    RoomM273EffectModeState *mode;
    RoomM273ThresholdEntityState *threshold;
    RoomM273DirectedRingState *directed_rings;
    RoomM273ScaledLayerOwner *scaled_layer;
    RoomM273PlayerActorView *player;
} RoomM273EffectStatePointer;

typedef struct RoomM273EffectStateContext {
    u8 reserved_00[8];
    RoomM273EffectStatePointer state;
} RoomM273EffectStateContext;


PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PoolContext, pool) == 8,
                  room_m273_pool_context_pool_offset);
PE1_STATIC_ASSERT(sizeof(RoomM273PulseSeed) == 8, room_m273_pulse_seed_size);
PE1_STATIC_ASSERT(sizeof(RoomM273PulseEmitterView) == 0x0C,
                  room_m273_pulse_emitter_view_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PairEmitterFields, flags) == 6,
                  room_m273_pair_emitter_flags_offset);
PE1_STATIC_ASSERT(sizeof(RoomM273PairPoolRecord) == 8,
                  room_m273_pair_pool_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PulseEmitterView, x) == 0,
                  room_m273_pulse_emitter_x_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PulseEmitterView, y) == 2,
                  room_m273_pulse_emitter_y_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PulseEmitterView, z) == 4,
                  room_m273_pulse_emitter_z_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PulseEmitterView, size) == 6,
                  room_m273_pulse_emitter_size_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PulseEmitterView, phase) == 8,
                  room_m273_pulse_emitter_phase_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PulseEmitterView, unknown10) == 0x0A,
                  room_m273_pulse_emitter_unknown_offset);
PE1_STATIC_ASSERT(sizeof(RoomM273WorldPosition) == 0x0C,
                  room_m273_world_position_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PlayerTransform, position) == 0x14,
                  room_m273_player_transform_position_offset);
PE1_STATIC_ASSERT(sizeof(RoomM273PlayerTransform) == 0x20,
                  room_m273_player_transform_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273ThresholdTransform, x) == 0x594,
                  room_m273_threshold_transform_x_offset);
PE1_STATIC_ASSERT(sizeof(RoomM273ThresholdTransform) == 0x5A0,
                  room_m273_threshold_transform_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273DirectedRingOwner, value) == 8,
                  room_m273_directed_ring_owner_value_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273EffectStateContext, state) == 8,
                  room_m273_effect_context_state_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273EffectStatePointer, mode) == 0,
                  room_m273_effect_state_mode_pointer_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273EffectStatePointer, threshold) == 0,
                  room_m273_effect_state_threshold_pointer_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273EffectStatePointer, player) == 0,
                  room_m273_effect_state_player_pointer_offset);

#endif
