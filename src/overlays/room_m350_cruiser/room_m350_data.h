#ifndef ROOM_M350_DATA_H
#define ROOM_M350_DATA_H

/* room_m350 data shared by several effect units, declared once. */

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/field_color.h"
#include "pe1/gte_short_vector.h"
#include "pe1/gte_types.h"
#include "pe1/room_m350_drop.h"

/* Colour seeds handed to the trail and glow drawers. */
extern RenderColor g_RoomM350BlackColor;
extern RenderColor g_RoomM350GlowSpriteColor;

/* Actor found by RoomM350_HandConfigure (type and sub id lookup), and the pair of
 * anchor positions the paired effects attach to. */
extern FieldActor *g_RoomM350HandActor;
extern GteShortVector g_RoomM350HandPoints[];

/* Actor found by RoomM350_OrbConfigure, and the joint position it returns. */
extern FieldActor *g_RoomM350OrbActor;
extern GteShortVector g_RoomM350OrbSpawnPoint;

/* First animation event slot; the event controller clears the four slots
 * as one word and sets them byte by byte from here. */
extern unsigned char g_RoomM350DropEventSlots;

/* Rotation matrix the cloud controller builds and the swept flare uses. */
extern GteMatrix g_RoomM350RushMatrix;

/* Repeat budget and player distance used by the cloud controller. */
typedef struct RoomM350CloudTracking {
    short repeats;
    short playerDistance;
    short playerAngle;
    unsigned char complete;
} RoomM350CloudTracking;
extern RoomM350CloudTracking g_RoomM350RushState;

/* Shared counters for the homing trail producer and its two queue consumers. */
typedef struct RoomM350TrailSequence {
    short repeats;
    short pointerCount;
    short positionCount;
    short activeCount;
    unsigned char completionFlags[3];
} RoomM350TrailSequence;
extern RoomM350TrailSequence g_RoomM350OrbState;

/* Draw callback for position-only particles emitted by the pulsed pair. */
s32 RoomM350_HandSparkCallback(s32 mode, GteShortVector *position);

#endif /* ROOM_M350_DATA_H */
