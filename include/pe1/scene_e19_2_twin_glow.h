#ifndef PE1_SCENE_E19_2_TWIN_GLOW_H
#define PE1_SCENE_E19_2_TWIN_GLOW_H

#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"
#include "pe1/room_orbit_trail.h"

/* Scene e19_2 twin glow controller: glows on two actor joints that pulse,
 * burst into a ring of sparks at frame 15 and then fade while a ring and a
 * floor glow spread from the first joint. */
typedef struct SceneTwinGlow {
    /* 0x00 */ s16 state;
    /* 0x02 */ s16 timer;
    /* 0x04 */ s16 jointA;
    /* 0x06 */ s16 jointB;
    /* 0x08 */ GteShortVector position;
} SceneTwinGlow;

typedef struct SceneTwinGlowChannel {
    s32 reserved[2];
    char *pool; /* 0x08 */
} SceneTwinGlowChannel;

extern GteRotation g_SceneE19FlatTilt;
extern GteShortVector g_SceneE19TwinGlowJointOrigin;
extern RenderColor g_SceneE19TwinGlowColor;

int rand(void);

#endif
