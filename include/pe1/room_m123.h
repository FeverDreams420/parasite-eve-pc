#ifndef PE1_ROOM_M123_H
#define PE1_ROOM_M123_H

#include "common.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_glow_line.h"
#include "pe1/field_orbit_point.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_m123_burst_particle.h"
#include "pe1/room_m123_joint_triangle.h"

/* Particle drawn by the room_m123 pulsing sprite callback: it scales with a
 * cosine of its frame counter, then fades into a second blended sprite. */
typedef RoomM123BurstParticle RoomM123PulsingParticle;

extern u8 g_RoomM123GlowColorTrack[40];
extern GteShortVector g_RoomM123PulseRingTilt,g_RoomM123WaveSpinBase;
extern s16 g_RoomM123SwirlSpeed,g_RoomM123SwirlBrightness;
extern char g_RoomM123PulseColorTrack0[32],g_RoomM123PulseColorTrack1[32],g_RoomM123WaveColorTrack[40];
extern void *g_RoomM123SelectedColorTrack;
extern u8 g_RoomM123SwirlAngles[8];
extern u16 g_RoomM123SwirlOrigin[3];
extern u16 g_RoomM123JointScript[24],g_RoomM123PulseRingScript[16],g_RoomM123WaveScript[12];

void LoadAverageShort12(void *, void *, int, int, void *);

/* Joint triangle: three boss joint points joined by lines that pull toward a
 * fourth joint, then a spark that runs around the triangle edges. */
extern GteShortVector g_RoomM123JointOffset;
extern GteShortVector g_RoomM123JointOrigin;
extern RenderColor g_RoomM123TriangleColor;

#include "pe1/fieldfx_task.h"
int RoomM123_JointTriangleCallback(int mode, void *data, intptr_t argument);
int RoomM123_JointTriangleEmitter(int mode, void *data, intptr_t argument);
int RoomM123_GlowBurstParticleCallback(int mode, void *data, intptr_t argument);
int RoomM123_GlowBurst(int mode, void *data, intptr_t argument);
int RoomM123_JointBeacon(int mode, void *data, intptr_t argument);
int RoomM123_JointSwirlParticleCallback(int mode, void *data, intptr_t argument);
int RoomM123_JointSwirl(int mode, void *data, intptr_t argument);
int RoomM123_PulseRingCallback(int mode, void *data, intptr_t argument);
int RoomM123_PulseRing(int mode, void *data, intptr_t argument);
int RoomM123_WaveCallback(int mode, void *data, intptr_t argument);
int RoomM123_WaveBurst(int mode, void *data, intptr_t argument);
extern FieldFxTaskProgram g_RoomM123JointEffects,g_RoomM123PulseRingEffects,g_RoomM123WaveBurstEffects;
intptr_t RoomM123_SelectSlotPointer(int, intptr_t, intptr_t, intptr_t);

#endif
