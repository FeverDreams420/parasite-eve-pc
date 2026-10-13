#ifndef PE1_ROOM_ORBITING_PAIR_H
#define PE1_ROOM_ORBITING_PAIR_H

#include "common.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_types.h"
#include "pe1/render_object.h"

typedef struct RoomOrbitPairTemplate8 {
    s32 word[2];
} __attribute__((packed)) RoomOrbitPairTemplate8;

typedef struct RoomOrbitPairChannel {
    s32 reserved[2];
    char *pool;
} RoomOrbitPairChannel;

typedef struct RoomOrbitPairNode {
    u8 reserved[0x18];
    u8 *state;
} RoomOrbitPairNode;

typedef struct RoomOrbitPairEventState {
    u8 reserved[0xD];
    u8 active;
} RoomOrbitPairEventState;

typedef struct RoomOrbitPairParticle {
    s16 x, y, z, pad6;
    s16 vx, vy, vz, padE;
    s16 state, timer;
} RoomOrbitPairParticle;

/* room_m256 spark: the pair particle layout with a parity flag at +6 and
 * a velocity computed from a randomized look angle. */
typedef struct RoomOrbitSpark {
    s16 x, y, z, parity;
    s16 vx, vy, vz, padE;
    s16 state, timer;
} RoomOrbitSpark;

PE1_STATIC_ASSERT(sizeof(RoomOrbitSpark) == 0x14, room_orbit_spark_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomOrbitPairChannel, pool) == 8,
                  room_orbit_pair_pool_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomOrbitPairNode, state) == 0x18,
                  room_orbit_pair_node_state_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomOrbitPairEventState, active) == 0xD,
                  room_orbit_pair_event_active_offset);
PE1_STATIC_ASSERT(sizeof(RoomOrbitPairParticle) == 0x14,
                  room_orbit_pair_particle_size);

extern int rand(void);

extern const RoomOrbitPairTemplate8 D_8018F208;
int RoomM256_ScatterDropEmitter(int mode, u16 *counter);

#endif
