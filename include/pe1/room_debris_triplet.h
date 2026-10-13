#ifndef PE1_ROOM_DEBRIS_TRIPLET_H
#define PE1_ROOM_DEBRIS_TRIPLET_H

#include "common.h"
#include "pe1/gte.h"

/* 16.16 fixed-point position; the integer halves are read as `>> 16`,
 * which the compiler narrows to halfword loads as retail does. */
typedef struct RoomDebrisPosition {
    s32 x, y, z;
    s32 reserved0C;
} RoomDebrisPosition;

typedef struct RoomDebrisVelocity {
    s16 x, y, z, reserved06;
} RoomDebrisVelocity;

/* Three pieces of debris that fall, bounce on the floor, and fade. */
typedef struct RoomDebrisTriplet {
    s16 remaining;                /* 0x00 */
    s16 speed;                    /* 0x02 */
    s16 acceleration;             /* 0x04 */
    s16 swingLimit;               /* 0x06 */
    s16 reserved08;
    u8 keepAlive;                 /* 0x0A */
    s8 jitter[3];                 /* 0x0B */
    u8 mode[4];                   /* 0x0E */
    u8 landed[4];                 /* 0x12 */
    s8 active[4];                 /* 0x16 */
    s8 countdown[4];              /* 0x1A */
    u8 size[4];                   /* 0x1E */
    u16 life[4];                  /* 0x22 */
    u16 swing[4];                 /* 0x2A */
    u16 phase[4];                 /* 0x32 */
    s16 reserved3A;
    RoomDebrisPosition position[3]; /* 0x3C */
    GteShortVector rotation[3];     /* 0x6C */
    u8 reserved84[0x18];
    RoomDebrisVelocity velocity[4]; /* 0x9C */
} RoomDebrisTriplet;


PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomDebrisTriplet, rotation) == 0x6C, room_debris_rotation_offset);

void RoomLib_InitThreeTransformParticles(void *owner, void *unused,
                                         RoomDebrisTriplet *state);
struct RoomFxControl;
void RoomLib_DrawThreeTransformParticles(void *unused, struct RoomFxControl *args,
                                         RoomDebrisTriplet *state);

#endif
