#ifndef ROOM_M350_DATA_H
#define ROOM_M350_DATA_H

/* room_m350 data shared by several effect units, declared once. */

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/field_color.h"
#include "pe1/gte_short_vector.h"
#include "pe1/gte_types.h"
#include "pe1/room_m350_drop.h"
#include "pe1/room_m350_effects.h"
#include "pe1/fieldfx_task.h"

/* Colour seeds handed to the trail and glow drawers. */
extern RenderColor g_RoomM350BlackColor;
extern RenderColor g_RoomM350GlowSpriteColor;

/* Actor found by RoomM350_HandConfigure (type and sub id lookup), and the pair of
 * anchor positions the paired effects attach to. */
extern FieldActor *g_RoomM350HandActor;
extern GteShortVector g_RoomM350HandPoints[4];
extern s16 g_RoomM350HandSweepRepeats;
extern u8 g_RoomM350HandSweepDone,g_RoomM350HandSweepHit;

/* Actor found by RoomM350_OrbConfigure, and the joint position it returns. */



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


/* The original adjacent data stays adjacent after pointer widening. */
typedef struct RoomM350OrbStorage {
    GteShortVector spawn;
    GteShortVector *pointers[4];
    GteShortVector positions[8];
    FieldActor *actor;
    RoomM350TrailSequence state;
} RoomM350OrbStorage;
extern RoomM350OrbStorage g_RoomM350OrbData;
#define g_RoomM350OrbSpawnPoint (g_RoomM350OrbData.spawn)
#define g_RoomM350OrbActor (g_RoomM350OrbData.actor)
#define g_RoomM350OrbState (g_RoomM350OrbData.state)
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350OrbStorage,state.pointerCount)-PE1_OFFSETOF(RoomM350PointerQueue,count)==PE1_OFFSETOF(RoomM350OrbStorage,pointers),room_m350_native_pointer_queue_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350OrbStorage,state.positionCount)-PE1_OFFSETOF(RoomM350PositionQueue,count)==PE1_OFFSETOF(RoomM350OrbStorage,positions),room_m350_native_position_queue_alias);

typedef struct RoomM350AreaStorage {
    GteMatrix matrix;
    s16 repeats,yaw[2];
    u8 done,inside,entered,track;
} RoomM350AreaStorage;
extern RoomM350AreaStorage g_RoomM350AreaData;
#define g_RoomM350AreaMatrix (g_RoomM350AreaData.matrix)
#define g_RoomM350AreaMatrixTxView (g_RoomM350AreaData.matrix.t[0])
#define g_RoomM350AreaRepeats (g_RoomM350AreaData.repeats)
#define g_RoomM350AreaConfigYaw (g_RoomM350AreaData.yaw)
#define g_RoomM350AreaDone (g_RoomM350AreaData.done)
#define g_RoomM350AreaPlayerInside (g_RoomM350AreaData.inside)
#define g_RoomM350AreaEntered (g_RoomM350AreaData.entered)
#define g_RoomM350AreaTrackPlayer (g_RoomM350AreaData.track)
#define g_RoomM350DropEventSlots (g_RoomEffectTrailPositions.event_slots[0])
#define g_RoomM350DropEmitterTrigger (g_RoomEffectTrailPositions.event_slots[1])
#define g_RoomM350DropSplashTrigger (g_RoomEffectTrailPositions.event_slots[2])
#define g_RoomM350DropHitTrigger (g_RoomEffectTrailPositions.event_slots[3])
#define g_RoomM350DropBurstState (g_RoomEffectTrailPositions.reserved24)
#define g_RoomM350DropBurstCount (((u8 *)&g_RoomEffectTrailPositions.reserved24)[1])
#define g_RoomM350DropTrailCount (g_RoomEffectTrailPositions.count)
#define g_RoomM350DropRepeats (g_RoomEffectTrailPositions.reserved2A)
#define g_RoomM350DropSplashPosX (g_RoomEffectTrailPositions.splash_x)
#define g_RoomM350DropSplashPosZ (g_RoomEffectTrailPositions.splash_z)
#define g_RoomM350DropHitPosition (g_RoomEffectTrailPositions.hit_values)
#define g_RoomM350DropLungeTimer (g_RoomEffectTrailPositions.lunge_timer)
#define g_RoomM350DropHitCooldown (g_RoomEffectTrailPositions.hit_cooldown)
#define g_RoomM350DropDone (g_RoomEffectTrailPositions.done)

typedef struct RoomM350RushStorage {
    GteMatrix matrix;
    union {GteShortVector center;s16 center_values[4];};
    GteShortVector endpoint,opposite,previous_endpoint,previous_opposite;
    RoomM350CloudTracking state;
    u8 player_hit,reserved[3],body_done;
} RoomM350RushStorage;
extern RoomM350RushStorage g_RoomM350RushData;
#define g_RoomM350RushMatrix (g_RoomM350RushData.matrix)
#define g_RoomM350RushCenter (g_RoomM350RushData.center_values)
#define g_RoomM350RushEndpointX (g_RoomM350RushData.endpoint.x)
#define g_RoomM350RushEndpointZ (g_RoomM350RushData.endpoint.z)
#define g_RoomM350RushEndpointValid (g_RoomM350RushData.endpoint.pad)
#define g_RoomM350RushOppositeX (g_RoomM350RushData.opposite.x)
#define g_RoomM350RushOppositeZ (g_RoomM350RushData.opposite.z)
#define g_RoomM350RushPreviousEndpoint (g_RoomM350RushData.previous_endpoint)
#define g_RoomM350RushPreviousOpposite (g_RoomM350RushData.previous_opposite)
#define g_RoomM350RushState (g_RoomM350RushData.state)
#define g_RoomM350RushPlayerHit (g_RoomM350RushData.player_hit)
#define g_RoomM350BodyDone (g_RoomM350RushData.body_done)
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350AreaStorage,track)==0x29,room_m350_area_argument_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350TrailQueue,reserved2A)==0x2a,room_m350_drop_argument_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM350RushStorage,state)==0x48,room_m350_rush_argument_alias);

/* Draw callback for position-only particles emitted by the pulsed pair. */
int RoomM350_HandSparkCallback(int mode, void *state, intptr_t argument);

#endif /* ROOM_M350_DATA_H */
