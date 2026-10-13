#ifndef PE1_ROOM_M404_H
#define PE1_ROOM_M404_H

#include "common.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/field_transform_point.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"

typedef struct { s32 word[2]; } __attribute__((packed)) RoomTemplate8;
typedef struct RoomM404Node {
    u8 reserved[0x18];
    u8 *state;
} RoomM404Node;
typedef struct RoomM404Channel {
    s32 reserved[2];
    char *pool;
} RoomM404Channel;
typedef struct RoomM404EventState {
    u8 reserved[0xD];
    u8 active;
} RoomM404EventState;
typedef struct RoomM404Particle {
    s16 x, y, z, unused06;
    s16 vx, vy, vz, unused0E;
    s16 state, timer;
} RoomM404Particle;
extern const RoomTemplate8 D_8018F208;

#endif
