#ifndef PE1_ROOM_FOUR_SPRITE_H
#define PE1_ROOM_FOUR_SPRITE_H

#include "pe1/room_fx.h"

typedef struct RoomFourSpriteColor {
    u8 r, g, b, pad;
} RoomFourSpriteColor;

typedef struct RoomFourSpritePosition {
    s16 x, y, z, pad;
} RoomFourSpritePosition;

/* Per-room sprite set: the shared rotation matrix followed by the four
 * sprites' colors, positions, scales and packet depths. */
typedef struct RoomFourSpriteState {
    RoomSpriteMatrix rotation;            /* 0x00 */
    RoomFourSpriteColor color[4];         /* 0x20 */
    RoomFourSpritePosition position[4];   /* 0x30 */
    RoomFourSpritePosition velocity[4];   /* 0x50 */
    s16 scale[4];                         /* 0x70 */
    u16 depth[4];                         /* 0x78 */
} RoomFourSpriteState;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomFourSpriteState, velocity) == 0x50, room_four_sprite_velocity_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomFourSpriteState, depth) == 0x78, room_four_sprite_depth_offset);

#endif
