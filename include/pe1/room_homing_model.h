#ifndef PE1_ROOM_HOMING_MODEL_H
#define PE1_ROOM_HOMING_MODEL_H

#include "common.h"
#include "pe1/asset_find.h"
#include "pe1/battle_context.h"
#include "pe1/field_texture_tables.h"
#include "pe1/field_vector_math.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/floor_polygon.h"
#include "pe1/gte_types.h"
#include "pe1/pm.h"
#include "pe1/fieldfx_task.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"

/* Room projectile that flies along its heading, bounces off the walkable
 * polygon's edges, drops onto the floor and finally hands its position to
 * the scene script. */

typedef union RoomHomingModelCoord {
    s32 word;                     /* 16.16 */
    struct {
        int fraction : 16;
        int integer : 16;
    } part;
} RoomHomingModelCoord;

typedef struct RoomHomingModel {
    RoomHomingModelCoord position[3]; /* 0x00 */
    s32 floor;                    /* 0x0C */
    s32 speed;                    /* 0x10: 16.16 */
    s32 scriptArg0;               /* 0x14: reported with the resting position */
    s32 scriptArg1;               /* 0x18 */
    s32 drag;                     /* 0x1C */
    s32 lift;                     /* 0x20 */
    s32 gravity;                  /* 0x24 */
    s32 soundHandle;              /* 0x28 */
    s16 angle;                    /* 0x2C */
    s16 state;                    /* 0x2E */
    s16 finished;                 /* 0x30 */
    s16 matrixIndex;              /* 0x32 */
    s16 escaped;                  /* 0x34 */
    s16 bounce;                   /* 0x36 */
    s16 damping;                  /* 0x38 */
    u8 released;                  /* 0x3A: the throw ended or was interrupted */
    u8 armed;                     /* 0x3B: the throw animation took the cue */
} RoomHomingModel;

PE1_STATIC_ASSERT(sizeof(RoomHomingModel) == 0x3C, room_homing_model_size);

#include "pe1/player_entity.h"

extern void Akao_Cmd_21(int handle, int arg);

/* The projectile's constant seeds: the spark sprite's rotation and colour,
 * the model words reported to the scene script, and the floor ring's
 * rotation and colours. They are the data of
 * src/overlays/room_lib/homing_projectile_seeds.c; scene_e04 and
 * scene_e05 keep them past the end of their extracted image and name them
 * in their symbol files. */
extern GteRotation g_RoomHomingSparkRotation;
extern RenderColor g_RoomHomingSparkColor;
extern s16 g_RoomHomingReportFields[5];
extern GteRotation g_RoomHomingRingRotation;
extern RenderColor g_RoomHomingRingInner;
extern RenderColor g_RoomHomingRingOuter;

/* Launch parameters set by the scene script through the configure entry
 * and copied into the model when the throw starts. Each room keeps them in
 * its own data and names them in its symbol file. */
typedef struct RoomHomingLaunch {
    s32 speed;                    /* 0x00 */
    s32 drag;                     /* 0x04 */
    s32 lift;                     /* 0x08 */
    s32 gravity;                  /* 0x0C */
    s32 scriptArg0;               /* 0x10 */
    s32 scriptArg1;               /* 0x14 */
    s32 bounce;                   /* 0x18 */
    s32 damping;                  /* 0x1C */
} RoomHomingLaunch;

extern RoomHomingLaunch g_RoomHomingLaunch;

extern FieldFxTaskProgram g_RoomHomingProgram;
extern u16 g_RoomHomingScript[12];
int RoomEffect_HomingProjectileSpark(int,void *,intptr_t);
int RoomEffect_HomingProjectile(int,void *,intptr_t);
int RoomEffect_HomingProjectileThrow(int,void *,intptr_t);
intptr_t RoomEffect_HomingProjectileConfigure(int,intptr_t,intptr_t,intptr_t);

#endif
