#ifndef PE1_SCENE_E22_QUAKE_BLAST_H
#define PE1_SCENE_E22_QUAKE_BLAST_H

#include "common.h"
#include "pe1/akao_send.h"
#include "pe1/asset_find.h"
#include "pe1/field_actor.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_sprite_draw.h"
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
#include "pe1/scene_archive.h"

/* Quake blast controller (scene_e22 SceneE22_QuakeBlast): a blast at the target
 * that lights the actor once it stands in reach, sprays sparks for six
 * frames, then draws a floor glow, a halo, a ring and three expanding
 * shock models. */
typedef struct SceneE22QuakeBlast {
    GteShortVector position;      /* 0x00 */
    u8 reserved08[8];
    s16 state;                    /* 0x10 */
    s16 timer;                    /* 0x12 */
} SceneE22QuakeBlast;

typedef struct SceneE22QuakeTarget {
    s16 x, y, z, reserved06;      /* 0x00 */
    int radius;                   /* 0x08 */
} SceneE22QuakeTarget;

/* The block SceneE22_ConfigureBlastTarget fills (0x801994F4) and the script hands to the quake blast. */
extern SceneE22QuakeTarget g_SceneE22BlastTarget;

/* A colour written whole; the byte array keeps the union in memory
 * (BLKmode) so it gets its stack slot in declaration order. */
typedef struct SceneE22QuakeBytes {
    u8 rgb[3];
    u8 code;
} SceneE22QuakeBytes;

typedef union SceneE22QuakeColor {
    RenderColor color;
    SceneE22QuakeBytes bytes;
    u32 word;
} SceneE22QuakeColor;

typedef struct SceneE22QuakeMatrix {
    s16 m[3][3];
    s16 reserved12;
    s32 t[3];                     /* 0x14 */
} SceneE22QuakeMatrix;

typedef struct SceneE22QuakeScale {
    s32 x, y, z, reserved0C;
} SceneE22QuakeScale;

typedef struct SceneE22QuakeObject {
    u32 flags;                    /* 0x00 */
} SceneE22QuakeObject;

typedef struct SceneE22QuakeSlot {
    SceneE22QuakeObject *object;
} SceneE22QuakeSlot;

typedef struct SceneE22QuakeObjectChannel {
    s32 reserved[2];
    SceneE22QuakeSlot *pool;      /* 0x08 */
} SceneE22QuakeObjectChannel;

typedef struct SceneE22QuakeChannel {
    s32 reserved[2];
    void *pool;                   /* 0x08 */
} SceneE22QuakeChannel;

typedef struct SceneE22QuakeEvent {
    u8 reserved[0xD];
    u8 active;                    /* 0x0D */
} SceneE22QuakeEvent;

typedef struct SceneE22QuakeActor {
    u8 reserved[0x4C];
    u32 flags;                    /* 0x4C */
} SceneE22QuakeActor;

extern struct FieldActor *g_PlayerEntity; /* the player actor (0x8009D254) */
/* Texture page indices: [0] the shared effect page, [8] this scene's. */
extern void *g_SceneE22QuakeModel0;
extern void *g_SceneE22QuakeModel1;
extern u8 g_SceneE22QuakeColorTrack[];
extern GteRotation g_SceneE22FlatTilt;
extern RenderColor g_SceneE22QuakeFlashColor;

extern int rand(void);

#endif
