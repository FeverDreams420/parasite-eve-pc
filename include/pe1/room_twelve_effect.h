#ifndef PE1_ROOM_TWELVE_EFFECT_H
#define PE1_ROOM_TWELVE_EFFECT_H

#include "pe1/room_fx.h"

typedef struct RoomLibTick12Rec {
    RoomSpriteMatrix matrix;
    unsigned short frameStep;     /* 0x20 */
    short timerLimit;             /* 0x22 */
    s16 initialPhase;
    unsigned short repeatCount;   /* 0x26 */
    short phaseLimit;             /* 0x28 */
    unsigned short phaseStep;     /* 0x2A */
    signed char state[12];        /* 0x2C */
    signed char timer[12];        /* 0x38 */
    unsigned short phase[12];     /* 0x44 */
    struct { s16 x, y; u16 z; s16 pad; } position[12]; /* 0x5C */
} RoomLibTick12Rec;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomLibTick12Rec, position) == 0x5C, room_twelve_position_offset);
PE1_STATIC_ASSERT(sizeof(RoomLibTick12Rec) == 0xBC, room_twelve_state_size);

#endif
