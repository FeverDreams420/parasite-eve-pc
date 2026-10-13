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
extern RenderColor D_8018F1D0;
extern RenderColor D_8018F1D4;
extern GteShortVector D_801994EC;
extern int rand(void);
extern int SceneE22_RingBurstParticleCallback(int mode, RoomOrbitTrailParticle *p);

/* Orbit ring trail particle (SceneE22_RingBurstParticleCallback): its colour seed and trail
 * track, and the trail strip renderer. */
extern RenderColor D_8018F1CC;
extern u8 D_80199168[];
extern void FieldFx_DrawGlowLine(void *, void *, void *, void *, int, int, int);

/* Damped glow particle (SceneE22_LinkBeamSparkCallback): its glow colour track. */
extern u8 D_80199190[];

/* Twisting trail particle (SceneE22_TwistTrailCallback): spin template, colour track and
 * the bent trail renderer. */
extern GteRotation D_8018F1F4;
extern u8 D_80199434[];
extern u8 D_8019944C[];
extern void FieldFx_OffsetPointByAngles(void *, void *, int, void *, int, int);

/* Twist beam particle (SceneE22_TwistBeamCallback): its colours, trail track, the beam
 * release flag and the beam sprite colour row. */
extern RenderColor D_8018F21C;
extern RenderColor D_8018F220;
extern u8 D_8019946C[];
extern s16 D_80199504;

#endif
