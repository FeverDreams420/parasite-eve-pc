#ifndef PE1_ROOM_RING_PAIR_H
#define PE1_ROOM_RING_PAIR_H

#include "common.h"

/* Ring-pair work area shared by init, draw and update in m174/m348/m383.
 * secondaryScale advances by secondaryScaleStep each frame. */
typedef struct RoomRingPairState {
    s16 x, y, z;
    u8 reserved06[0xA];
    s16 scale;                         /* 0x10 */
    u16 secondaryScale;                /* 0x12 */
    u16 depth;                         /* 0x14 */
    u16 secondaryScaleStep;            /* 0x16 */
} RoomRingPairState;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomRingPairState, scale) == 0x10, room_ring_pair_scale_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomRingPairState, secondaryScaleStep) == 0x16, room_ring_pair_step_offset);

#endif
