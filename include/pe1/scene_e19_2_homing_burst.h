#ifndef PE1_SCENE_E19_2_HOMING_BURST_H
#define PE1_SCENE_E19_2_HOMING_BURST_H

#include "pe1/field_sprite_draw.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"

/* Scene e19_2 homing burst controller: anchored on the room actor, it
 * releases twenty homing particles (SceneE19_HomingParticleCallback) on odd frames, then
 * draws a fading flash, a screen tint and a shrinking set of glows. */
typedef struct SceneHomingBurst {
    /* 0x00 */ GteShortVector position;
    /* 0x08 */ s16 timer;
    /* 0x0A */ s16 released;
} SceneHomingBurst;

/* Child particle fields the controller seeds. */
typedef struct SceneHomingBurstChild {
    /* 0x00 */ s16 x, y, z;
    /* 0x06 */ s16 index;
    /* 0x08 */ u8 pad08[0x14];
    /* 0x1C */ s16 state;
    /* 0x1E */ s16 timer;
} SceneHomingBurstChild;

typedef struct SceneHomingBurstObject {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ u8 *status;
} SceneHomingBurstObject;

typedef struct SceneHomingBurstPool {
    /* 0x00 */ SceneHomingBurstObject *object;
} SceneHomingBurstPool;

typedef struct SceneHomingBurstChannel {
    s32 reserved[2];
    SceneHomingBurstPool *pool; /* 0x08 */
} SceneHomingBurstChannel;

typedef struct SceneHomingBurstEvent {
    /* 0x00 */ u8 pad00[0xD];
    /* 0x0D */ u8 active;
} SceneHomingBurstEvent;

extern GteRotation D_8018F210;
extern GteRotation D_8018F1CC;
extern RenderColor D_8018F21C;
extern RenderColor D_8018F220;


#endif
