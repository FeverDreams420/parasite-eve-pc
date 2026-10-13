#ifndef PE1_SCENE_E18_EFFECTS_H
#define PE1_SCENE_E18_EFFECTS_H

#include "common.h"
#include "pe1/asset_find.h"
#include "pe1/field_actor.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_libc.h"
#include "pe1/render_object.h"
#include "pe1/scene_archive.h"
#include "pe1/scene_e18_pulse.h"

/* Particle emitters of the scene_e18 script: one rises from random model
 * joints and fades out, the other drops wave rings from random joints. */

typedef struct SceneE18Owner {
    u32 flags;                    /* 0x00 */
    s32 reserved04;
    s32 asset;                    /* 0x08 */
    u8 reserved0C[0xC];
    u8 *stage;                    /* 0x18 */
    u8 reserved1C[0x30];
    u32 status;                   /* 0x4C */
} SceneE18Owner;

typedef struct SceneE18Instance {
    SceneE18Owner *owner;         /* 0x00 */
    u8 reserved04[0xA];
    u8 animationId;               /* 0x0E */
    u8 frameLimit;                /* 0x0F */
    u8 reserved10[4];
    union {
        s32 word;                 /* 0x14 */
        struct {
            u16 step;
            u16 frame;            /* 0x16 */
        } part;
    } animation;
    u8 reserved18[0x10];
    s32 x, y, z;                  /* 0x28: 16.16 position */
    u8 reserved34[0x1B4];
    GteMatrix transform;          /* 0x1E8 */
    u8 reserved208[0x30];
    GteMatrix *transforms;        /* 0x238 */
} SceneE18Instance;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE18Instance, transforms) == 0x238,
                  scene_e18_instance_transforms);

typedef struct SceneE18Actor {
    s32 reserved[2];
    SceneE18Instance *instance;   /* 0x08 */
} SceneE18Actor;
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE18Actor, instance) == 8,
                  scene_e18_actor_instance_offset);

typedef struct SceneE18Emitter {
    s32 reserved[2];
    void *pool;                   /* 0x08 */
} SceneE18Emitter;
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE18Emitter, pool) == 8,
                  scene_e18_emitter_pool_offset);

typedef struct SceneE18EmitterState {
    s16 count;
    s16 delay;
} SceneE18EmitterState;

typedef struct SceneE18RisingParticle {
    GteShortVector position;
    s16 size;                     /* 0x08 */
} SceneE18RisingParticle;

typedef struct SceneE18WaveParticle {
    GteShortVector position;
    s16 controls[4];              /* 0x08 */
    s16 brightness;               /* 0x10 */
    s16 size;                     /* 0x12 */
    s16 amplitude;                /* 0x14 */
} SceneE18WaveParticle;

extern u8 g_SceneE18RisingStop;

extern int Inv_ScrambleGrid(void);

/* Flank model emitter (SceneE18_FlankEmitter): four model particles that follow
 * the room model's translation and alternate sides, the model asset it
 * looks up and the frame countdown its particles run down. */
typedef struct SceneE18FlankParticle {
    s32 *position;                /* 0x00: the room model's translation */
    s16 angle;                    /* 0x04 */
    s16 turn;                     /* 0x06 */
    s16 width;                    /* 0x08 */
    s16 height;                   /* 0x0A */
    s16 widthGrowth;              /* 0x0C */
    s16 heightGrowth;             /* 0x0E */
    s16 brightness;               /* 0x10 */
    s16 leader;                   /* 0x12 */
} SceneE18FlankParticle;

extern void *g_SceneE18FlankModel;
extern s16 g_SceneE18FlankRemaining;
extern int SceneE18_FlankParticleCallback(int mode, SceneE18FlankParticle *particle);

/* Flank model particle (SceneE18_FlankParticleCallback): the player instance whose distance
 * to the model releases the actor, and the model draw helpers. */
extern struct FieldActor *g_PlayerEntity; /* the player actor (0x8009D254) */
extern int Math_IntSqrt(int value);

/* Pulse ring particle (SceneE18_PulseRingCallback): shrinking light rings around the
 * room model; the last one releases the actor once the player is near. */
extern RenderColor g_SceneE18PulseRingColors[2];

/* Spinning model controller (SceneE18_SpinShell): spins the scene model's
 * glow shell around the room model while it grows and flares, and sets
 * the model's stage byte once its seventh animation ends. */
typedef struct SceneE18SpinShell {
    s16 width;                    /* 0x00 */
    s16 height;                   /* 0x02 */
    s16 widthGrowth;              /* 0x04 */
    s16 heightGrowth;             /* 0x06 */
    s16 angle;                    /* 0x08 */
    s16 turn;                     /* 0x0A */
    s16 brightness;               /* 0x0C */
} SceneE18SpinShell;

extern void *g_SceneE18ShellModel;

#endif
