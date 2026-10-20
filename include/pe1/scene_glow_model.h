#ifndef PE1_SCENE_GLOW_MODEL_H
#define PE1_SCENE_GLOW_MODEL_H

#include "pe1/field_glow_sprite.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_script_context.h"
#include "pe1/field_sprite_state.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_libc.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"

/* Scene e08 glowing model: the effect model drawn at the scene anchor,
 * followed by a glow sprite that grows once the timer passes 10. */
typedef struct SceneGlowModelOwner {
    /* 0x00 */ u8 pad00[sizeof(void *) + 0x20];
    /* 0x24 */ int mode;
} SceneGlowModelOwner;

typedef struct SceneGlowModelTimer {
    /* 0x00 */ u8 pad00[2];
    /* 0x02 */ s16 ticks;
} SceneGlowModelTimer;

typedef struct SceneGlowModelState {
    /* 0x00 */ u8 pad00[2];
    /* 0x02 */ s16 height;
    /* 0x04 */ u8 pad04[4];
    /* 0x08 */ RoomFxSeed8 seed;
    /* 0x10 */ s16 scaleX;
    /* 0x12 */ s16 scaleY;
    /* 0x14 */ s16 scaleZ;
    /* 0x16 */ u8 pad16[2];
    /* 0x18 */ u16 depth;
} SceneGlowModelState;

typedef struct SceneGlowModelColor {
    u8 bytes[8];
} SceneGlowModelColor;

extern char *g_SceneE08GlowModel;
extern int g_SceneE08CentreX;
extern int g_SceneE08CentreZ;
extern u8 g_SceneE08GlowModelRamp[];
extern FieldGlowSprite g_SceneE08GlowModelHaloSprite;

#endif
