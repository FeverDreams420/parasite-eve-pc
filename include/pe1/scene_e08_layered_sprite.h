#ifndef PE1_SCENE_E08_LAYERED_SPRITE_H
#define PE1_SCENE_E08_LAYERED_SPRITE_H

#include "pe1/fixed_point.h"
#include "pe1/room_fx.h"

typedef struct SceneLayerPosition {
    s16 x, y, z, pad;
} SceneLayerPosition;

/* Layered sprite state shared by init, update and draw. The update also
 * keeps an eight-entry history of the oldest layer at 0x86. */
typedef struct SceneLayeredSpriteState {
    Pe1Fixed16_16 center[3];     /* 0x00 */
    u32 reserved0C;
    s32 velocity[3];            /* 0x10 */
    u32 reserved1C;
    RoomFxSeed8 initialSeed;    /* 0x20 */
    u8 direction;              /* 0x28 */
    u8 reserved29;
    u16 drift;                 /* 0x2A */
    s32 phase;                 /* 0x2C */
    SceneLayerPosition layers[5]; /* 0x30 */
    RoomFxSeed8 layerSeeds[5];  /* 0x58 */
    u8 reserved80[6];
    RoomFxSeed8 history[8];     /* 0x86 */
} SceneLayeredSpriteState;

/* The update shifts the history back one entry per pass through a cursor
 * stepping eight bytes back from layers[1]; seen from that cursor, the
 * entry it reads and the one it writes. */
typedef struct SceneLayeredHistoryLane {
    u8 reserved00[0x7E];
    RoomFxSeed8 entry;          /* 0x7E */
    RoomFxSeed8 olderEntry;     /* 0x86 */
} SceneLayeredHistoryLane;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneLayeredSpriteState, velocity) == 0x10, scene_layer_velocity_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneLayeredSpriteState, initialSeed) == 0x20, scene_layer_seed_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneLayeredSpriteState, layers) == 0x30, scene_layer_position_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneLayeredSpriteState, layerSeeds) == 0x58, scene_layer_history_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneLayeredSpriteState, history) == 0x86, scene_layer_trail_offset);

#endif
