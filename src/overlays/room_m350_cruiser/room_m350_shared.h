#ifndef ROOM_M350_SHARED_H
#define ROOM_M350_SHARED_H

#include "pe1/gte_short_vector.h"

#include "pe1/field_actor.h"
#include "pe1/fieldfx_task_pool.h"
typedef FieldFxTaskSlot RoomM350Emitter;
typedef GteShortVector RoomM350Vector;
typedef RenderMatrix RoomM350TransformMatrix;
typedef FieldActor RoomM350TransformOwner;
typedef FieldFxObjectPrefix RoomM350Actor;
typedef FieldActorState RoomM350SoundOwner;

typedef struct RoomM350Action {
    unsigned char state;
} RoomM350Action;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350Action, state) == 0,
                  room_m350_action_state_offset);
PE1_STATIC_ASSERT(sizeof(RoomM350Action) == 1, room_m350_action_size);

typedef struct RoomM350SignedHalfPair {
    signed int unused : 16;
    signed int value : 16;
} RoomM350SignedHalfPair;

PE1_STATIC_ASSERT(sizeof(RoomM350SignedHalfPair) == 4,
                  room_m350_signed_half_pair_size);

typedef struct RoomM350TableShadeEntry {
    signed int size : 16;
    signed int shade : 16;
} RoomM350TableShadeEntry;

PE1_STATIC_ASSERT(sizeof(RoomM350TableShadeEntry) == 4,
                  room_m350_table_shade_entry_size);

#endif /* ROOM_M350_SHARED_H */
