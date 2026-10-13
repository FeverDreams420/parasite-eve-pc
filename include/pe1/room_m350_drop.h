#ifndef PE1_ROOM_M350_DROP_H
#define PE1_ROOM_M350_DROP_H

#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"

/* Falling drop moved by room_m350's splash callback. */
typedef struct RoomM350Drop {
    GteShortVector velocity;
    s16 x;
    s16 y;
    s16 z;
    u8 landed;
} RoomM350Drop;

/* Trail state at 0x8019A834: four queued positions, the splash flags,
 * the queued position count and the splash/contact positions. */
typedef struct RoomM350TrailQueue {
    GteShortVector points[4];
    u8 reserved20[2];
    u8 splashed;
    u8 touched;
    s32 reserved24;
    s16 count;
    s16 reserved2A;
    s16 splash_x;
    s16 splash_z;
    s16 hit_x;
    s16 hit_y;
    s16 hit_z;
} RoomM350TrailQueue;

extern RoomM350TrailQueue g_RoomEffectTrailPositions;
extern GteRotation g_RoomM350StarFanTilt;
extern RenderColor g_RoomM350DropColors[3];

int RoomM350_DropCallback(int mode, RoomM350Drop *drop);
int RoomM350_TestDropPlayerHit(short *point, int radius);

#endif
