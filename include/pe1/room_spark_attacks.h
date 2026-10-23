#ifndef PE1_ROOM_SPARK_ATTACKS_H
#define PE1_ROOM_SPARK_ATTACKS_H

#include "common.h"
#include "pe1/gte_types.h"
#include "pe1/render_object.h"
#include "pe1/room_interface.h"
#include "pe1/room_spark.h"
#include "pe1/fieldfx_task.h"

/* The spark attacks linked by room_m156, room_m291 and room_m380
 * (src/overlays/room_lib/spark_attacks.c): burst orbs launched
 * from the actor's hand, a spray of damped sparks, a beam with a bouncing
 * spark shower and a ring of jittering sparks. */

/* Eight-byte joint offset copied to the stack unaligned, as retail does. */
typedef struct RoomSparkWords8 {
    s32 word[2];
} __attribute__((packed)) RoomSparkWords8;

/* Per-room data: the heading the burst orbs fly along, set by the orb
 * controller from the actor's rotation, and the beam's model asset. */
extern s16 g_RoomBurstOrbHeading;
extern void *g_RoomBeamSparkAsset;

/* The capture block the same rooms fill through
 * RoomLib_CaptureFrameSample (src/overlays/room_lib/capture_frame_sample.c). */
extern RoomSparkRingParams g_RoomSparkCapture;
#define g_RoomCaptureX (g_RoomSparkCapture.x)
#define g_RoomCaptureY (g_RoomSparkCapture.y)
#define g_RoomCaptureZ (g_RoomSparkCapture.z)
#define g_RoomCaptureFrameWord (g_RoomSparkCapture.radius)

intptr_t RoomLib_CaptureFrameSample(int mode, intptr_t x, intptr_t y, intptr_t z);
int RoomEffect_BurstOrbParticle(int mode, void *state, intptr_t argument);
int RoomEffect_BurstOrbController(int mode, void *state, intptr_t argument);
int RoomEffect_DampedSparkParticle(int mode, void *state, intptr_t argument);
int RoomEffect_DampedSparkController(int mode, void *state, intptr_t argument);
int RoomEffect_BeamBounceParticle(int mode, void *state, intptr_t argument);
int RoomEffect_BeamSparkController(int mode, void *state, intptr_t argument);
int RoomEffect_JitterSparkParticle(int mode, void *state, intptr_t argument);
int RoomEffect_JitterSparkController(int mode, void *state, intptr_t argument);

extern u16 g_RoomBurstOrbScript[22], g_RoomBeamSparkScript[18], g_RoomJitterSparkScript[18];
extern FieldFxTaskProgram g_RoomBurstOrbProgram, g_RoomBeamSparkProgram, g_RoomJitterSparkProgram;

/* Complete USA M413/M416/M417/M437 joint-spray unit (original slot135). */
int RoomEffect_SeekerSparkCallback(int mode, void *data, intptr_t argument);
int RoomEffect_JointSprayController(int mode, void *data, intptr_t argument);
intptr_t RoomEffect_SetJointSprayArgs(int mode, intptr_t speed, intptr_t spread, intptr_t unused);
extern int g_RoomJointSprayArgs[2];
extern FieldFxTaskProgram g_RoomJointSprayProgram;

#endif
