#ifndef PE1_SCENE_E22_RING_BURST_H
#define PE1_SCENE_E22_RING_BURST_H

#include "common.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"
#include "pe1/room_orbit_trail.h"

/* Ring burst controller (scene_e22): scene event gate, particle channel
 * and the helpers the burst and its orbit trail particles call. */

typedef struct SceneE22BurstEvent {
    u8 reserved[0x16];
    s16 running;                  /* 0x16 */
} SceneE22BurstEvent;

typedef struct SceneE22BurstChannel {
    s32 reserved[2];
    void *pool;                   /* 0x08 */
} SceneE22BurstChannel;

extern struct FieldActor *g_PlayerEntity; /* the player actor (0x8009D254) */
extern RenderColor g_SceneE22RingBurstRingColor;
extern RenderColor g_SceneE22RingBurstHaloColor;
extern RenderColor g_SceneE22RingBurstRingColor;
extern RenderColor g_SceneE22RingBurstHaloColor;
extern GteShortVector g_SceneE22BurstAnchor;
extern int rand(void);
extern int SceneE22_RingBurstParticleCallback(int mode, RoomOrbitTrailParticle *p);

/* Orbit ring trail particle (SceneE22_RingBurstParticleCallback): its colour seed and trail
 * track, and the trail strip renderer. */
extern RenderColor g_SceneE22RingBurstTriangleColor;
extern u8 g_SceneE22RingBurstParticleColorTrack[];
extern void FieldFx_DrawGlowLine(void *, void *, void *, void *, int, int, int);

/* Damped glow particle (SceneE22_LinkBeamSparkCallback): its glow colour track. */
extern u8 g_SceneE22LinkBeamSparkColorTrack[];

/* Twisting trail particle (SceneE22_TwistTrailCallback): spin template, colour track and
 * the bent trail renderer. */
extern GteRotation g_SceneE22ZeroSpin;
extern u8 g_SceneE22SwirlRingParticleColorTrack[];
extern u8 g_SceneE22TwistTrailColorTrack[];
extern void FieldFx_OffsetPointByAngles(void *, void *, int, void *, int, int);

/* Twist beam particle (SceneE22_TwistBeamCallback): its colours, trail track, the beam
 * release flag and the beam sprite colour row. */
extern RenderColor g_SceneE22TwistBeamColorA;
extern RenderColor g_SceneE22TwistBeamColorB;
extern RenderColor g_SceneE22TwistBeamColorA;
extern RenderColor g_SceneE22TwistBeamColorB;
extern u8 g_SceneE22TwistBeamColorTrack[];
extern s16 g_SceneE22TwistBeamStop;

#endif
