#ifndef PE1_SCENE_E19_2_SPIN_RAY_H
#define PE1_SCENE_E19_2_SPIN_RAY_H

#include "pe1/field_owner_actor.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"

/* Scene e19_2 spin ray: a spinning ray anchored at the scene origin that
 * grows, sweeps out as a textured blade and closes as a glow sprite. The
 * spin angles double as the draw rotation. */
typedef struct SceneSpinRay {
    /* 0x00 */ union {
        GteRotation rotation;
        GteShortVector angles;
    } spin;
    /* 0x08 */ s16 state;
    /* 0x0A */ u16 timer;
    /* 0x0C */ s16 reach;
    /* 0x0E */ s16 spinY;
} SceneSpinRay;

/* The controller: its frame timer and the number of blades left to cast. */
typedef struct SceneSpinRayBurst {
    /* 0x00 */ u16 timer;
    /* 0x02 */ s16 blades;
} SceneSpinRayBurst;

/* Particle pools reached through the two effect channels. */
typedef struct SceneSpinRayChannel {
    s32 reserved[2];
    char *pool; /* 0x08 */
} SceneSpinRayChannel;

extern GteRotation g_SceneE19RollTilt;
extern RenderColor g_SceneE19SpinRayColor;
extern RenderColor g_SceneE19SpinRayBladeHeadColor;
extern RenderColor g_SceneE19SpinRayBladeTailColor;
extern GteRotation g_SceneE19RollTilt;
extern RenderColor g_SceneE19SpinRayBurstRingColor;
extern u8 g_SceneE19SpinRayColorTrack[];
extern u8 g_SceneE19SpinRayBurstColorTrack[];
extern GteShortVector g_SceneE19SpinRayOrigin;

int rand(void);
int SceneE19_SpinRayCallback(int mode, void *memory, intptr_t argument);

#endif
