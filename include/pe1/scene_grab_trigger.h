#ifndef PE1_SCENE_GRAB_TRIGGER_H
#define PE1_SCENE_GRAB_TRIGGER_H

#include "common.h"
#include "pe1/battle_setup.h"
#include "pe1/field_actor.h"
#include "pe1/field_vector_math.h"
#include "pe1/player_entity.h"

/* Scene script object that pulls the player into a scripted grab animation
 * when the player walks up to the owning actor during its reach window. */
typedef struct SceneGrabControl {
    /* 0x0C */ void (*callback)();
    /* 0x10 */ int *signal;
    /* 0x14 */ u8 pad_14[6];
    /* 0x1A */ u8 started;
    /* 0x1B */ u8 pad_1B;
    /* 0x1C */ RenderMatrix player_matrix;
    /* 0x3C */ int saved_x;
    /* 0x40 */ int saved_z;
    /* 0x44 */ u8 grabbed;
} SceneGrabControl;

typedef struct SceneGrabPoint {
    s32 x;
    s32 y;
    s32 z;
} SceneGrabPoint;

typedef struct SceneGrabTrigger {
    /* 0x00 */ u8 status;      /* 4 = closed */
    /* 0x01 */ u8 pad_01[2];
    /* 0x03 */ u8 flag3;       /* RoomEnt flag3, 0xFF while armed */
    /* 0x04 */ u8 pad_04[4];
    /* 0x08 */ FieldActor *actor;
    /* 0x0C */ SceneGrabControl control;
} SceneGrabTrigger;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneGrabTrigger, control.signal) == 0x10,
                  scene_grab_signal_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneGrabTrigger, control.player_matrix) == 0x1C,
                  scene_grab_matrix_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneGrabTrigger, control.grabbed) == 0x44,
                  scene_grab_flag_offset);

s32 SceneE19_ResetGrabTarget(char *obj);
void SceneE19_GrabSequence();
void SceneE19_RunAfterFrame37();

#endif
