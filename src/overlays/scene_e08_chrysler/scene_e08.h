#ifndef SCENE_E08_H
#define SCENE_E08_H

#include "pe1/akao_send.h"
#include "pe1/field_script_context.h"
#include "pe1/room_fx.h"
#include "pe1/scene_archive.h"

typedef struct {
    RoomFxEmitterParams emitters[2];
    char sourceData[2][0x100];
    s16 ringScale[2];       /* 0x230: shrinks by 30 per frame */
    char pad_234[0x20];
    s16 glowScale[4];       /* 0x254 */
    s16 glowAngle[4];       /* 0x25C */
    s16 glowDepth;          /* 0x264: fades by 2 per frame */
} Ovl178FadeState;

/* The scene object FieldFx_GetScriptContext returns while the e08 effects run: its
 * position, the sprite page mode the draws select, and the flag the
 * effects raise when they touch the player. */
typedef struct {
    char pad_00[0x18];
    s32 x;
    s32 y;
    s32 z;
    u8 spriteMode;      /* 0x24 */
    char pad_25[7];
    s16 playerHit;      /* 0x2C */
} Ovl178Position;


typedef struct {
    u8 pad_00[8];
    s16 extent;             /* 0x08: grows to 3000 */
    s16 intensity;          /* 0x0A: grows to 128 */
    s16 offsets[12];
    union {
        s16 variation[3];
        u8 flags[10];
    } tail;
} Ovl178RandomizedEffect;

/* Full-screen fade (SceneE08_*ScreenFade): the level climbs while the
 * phase is 1 and falls back to zero in phase 2. */
typedef struct {
    s16 reserved;
    s16 level;          /* 0x02 */
    s16 phase;          /* 0x04 */
} SceneE08ScreenFade;

/* Single glow sprite (SceneE08_*CentreGlow): a short position offset from
 * the scene centre and the sprite scale. */
typedef struct {
    s16 x, y, z;
    s16 pad06;
    s16 scale;          /* 0x08 */
    s16 unused0A;
    s16 unused0C;
} SceneE08CentreGlow;

/* Growing glow left of the scene object (SceneE08_*ExpandingGlow): the
 * scale grows by an accelerating step while the depth fades. */
typedef struct {
    s32 x, y, z;
    s32 pad0C;
    s16 scale;          /* 0x10 */
    s16 growth;         /* 0x12 */
    u16 depth;          /* 0x14 */
} SceneE08ExpandingGlow;

typedef struct SceneE08PositionSample {
    s32 x, y, z, extra;
} SceneE08PositionSample;

/* Three-sprite trail (SceneE08_*TrailGlow): the newest sample is
 * slot 0, the update shifts every sample one slot back each frame. */
typedef struct {
    SceneE08PositionSample trail[3];    /* 0x00 */
    GteShortVector rotation[3];         /* 0x30 */
    GteShortVector scale[3];            /* 0x48 */
    u8 pad60[0x28];
    s16 unused88;                       /* 0x88 */
    u16 speed;                          /* 0x8A */
    u16 acceleration;                   /* 0x8C */
} SceneE08TrailGlow;

/* One trail sprite's rotation and scale, viewed from a cursor that steps
 * eight bytes per sprite through the start of the work area. */
typedef struct {
    GteShortVector step;
    u8 pad08[0x28];
    GteShortVector rotation;            /* 0x30 */
    u8 pad38[0x10];
    GteShortVector scale;               /* 0x48 */
} SceneE08TrailLane;

/* Spinning glow (SceneE08_InitSpinningGlow init, SceneE08_DrawSpinningGlow draw, SceneE08_UpdateSpinningGlow
 * update): the sprite turns about z and shrinks by a growing step while
 * the frame byte selects the animation cell. */
typedef struct {
    int x;
    int y;
    int z;
    char pad_0C[4];
    GteShortVector rotation;    /* 0x10 */
    char pad_18[4];
    s16 scale;                  /* 0x1C */
    s16 shrink;                 /* 0x1E */
    s16 depth;                  /* 0x20 */
    u8 frame;                   /* 0x22 */
} Ovl178MotionState;

s32 *FieldFx_RegPtr(int index);
s32 *FieldFx_VarPtr(int index);
extern s16 g_SceneE08LimbBeamUnusedAngles[];
extern u16 g_SceneE08LimbBeamAngles[];
extern s32 g_SceneE08CentreX;
extern s32 g_SceneE08CentreZ;
void FieldFx_BuildRingGeometry(void *item);

#endif
