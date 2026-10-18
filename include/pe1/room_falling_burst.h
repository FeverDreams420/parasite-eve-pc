#ifndef PE1_ROOM_FALLING_BURST_H
#define PE1_ROOM_FALLING_BURST_H

#include "common.h"
#include "pe1/field_glow_sprite.h"
#include "pe1/field_script_context.h"
#include "pe1/field_sprite_state.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/pe_image.h"
#include "pe1/psyq_libc.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"
#include "pe1/room_module.h"

/* The falling sprite burst linked by ten hospital and Chrysler rooms
 * (src/overlays/room_lib/falling_sprite_burst.c): a sprite and its
 * floor shadow fall until they reach the floor, then spawn an impact
 * shimmer, a ground pulse and six particles. */

/* Effect control pair: state byte 1 is set to 2 when the effect is done. */
typedef struct RoomFallingBurstControl {
    u8 pad0;
    u8 state;                     /* 0x01 */
    s16 endTick;                  /* 0x02 */
} RoomFallingBurstControl;

/* The script object the burst belongs to and its owner actor. */
typedef struct RoomFallingBurstEntity {
    u8 pad00[0x8];
    struct FieldActor *actor;     /* 0x08 */
} RoomFallingBurstEntity;

/* Shared scene clock returned by FieldFx_GetScriptContext: the owner's actor, the
 * re-arm reload and timer, the hit request, the clock step and tick, and
 * the sprite asset whose low byte selects the texture page. */
typedef struct RoomFallingBurstClock {
    struct FieldActor *actor;     /* 0x00 */
    s16 reload;                   /* 0x04 */
    s16 cooldown;                 /* 0x06 */
    s16 hit;                      /* 0x08 */
    s16 step;                     /* 0x0A */
    s16 tick;                     /* 0x0C */
    u8 padE[0x2];
    union {
        int index;
        u8 page;
    } asset;                      /* 0x10 */
} RoomFallingBurstClock;

/* Parameter block of the floor decals: the ground pulse uses the scale and
 * depth at 0x08 (as RoomUniformSpriteFxParams), the impact shimmer the
 * ones at 0x10 and its alpha (as RoomSpriteFxParams). */
typedef struct RoomFallingBurstDecal {
    s16 x, y, z;                  /* 0x00 */
    s16 pad6;
    s16 pulseScale;               /* 0x08 */
    s16 pulseDepth;               /* 0x0A */
    s16 hC;                       /* 0x0C */
    u8 padE[0x2];
    s16 shimmerScale;             /* 0x10 */
    s16 shimmerDepth;             /* 0x12 */
    u8 pad14;
    u8 shimmerAlpha;              /* 0x15 */
    u8 shimmerPhase;              /* 0x16 */
} RoomFallingBurstDecal;

typedef struct RoomFallingBurstVec {
    u16 x, y, z;
    s16 pad6;
} RoomFallingBurstVec;

/* Sprite position and velocity use wrapping halfword storage. The vertical
 * step at 0x14 is integrated as a signed 8.8 value by the update pass. */
typedef struct RoomFallingSpriteState {
    u16 x, y, z;
    s16 pad6;
    u16 velocityX, velocityY, velocityZ;
    s16 padE;
    u16 scale;
    u16 depth;
    s16 verticalStep;             /* 0x14: signed 8.8 displacement per tick */
    u8 active;
} RoomFallingSpriteState;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomFallingSpriteState, verticalStep) == 0x14,
                  room_falling_sprite_vertical_step);

/* The six impact particles. */
typedef struct RoomFallingBurstParticles {
    RoomSpriteMatrix matrix;          /* 0x00 */
    RoomFallingBurstVec position[6];  /* 0x20 */
    RoomFallingBurstVec velocity[6];  /* 0x50 */
    u8 pad80[0xC];
    u16 depth[6];                     /* 0x8C */
    s16 active[6];                    /* 0x98 */
    s16 liveCount;                    /* 0xA4 */
} RoomFallingBurstParticles;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomFallingBurstParticles, liveCount) == 0xA4,
                  room_falling_burst_particles_live);

/* Per-room data: the five sprite records. When it lands the falling sprite
 * spawns from the room module's init list and spawn layout
 * (pe1/room_module.h). */
extern RoomFxSpritePacket g_RoomFallingBurstShadow;
extern RoomFxSpritePacket g_RoomFallingBurstShimmer;
extern RoomFxSpritePacket g_RoomFallingBurstSprite;
extern RoomFxSpritePacket g_RoomFallingBurstPulse;
extern RoomFxSpritePacket g_RoomFallingBurstParticle;

int *FieldFx_RegPtr(int index);
intptr_t *FieldFx_VarPtr(int index);

extern PmCommand g_RoomFallingBurstHandlers;
extern FieldFxHandler g_RoomInitList__falling[6],g_RoomDrawList__falling[6],g_RoomUpdateList__falling[6];
extern int g_RoomScript__falling[19],g_RoomSpawnLayout__falling[3];

#endif
