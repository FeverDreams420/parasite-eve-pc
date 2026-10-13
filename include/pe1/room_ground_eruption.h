#ifndef PE1_ROOM_GROUND_ERUPTION_H
#define PE1_ROOM_GROUND_ERUPTION_H

#include "common.h"
#include "pe1/psyq_libc.h"
#include "pe1/room_fx.h"
#include "pe1/room_module.h"

/* The ground eruption (src/overlays/room_lib/ground_eruption.c). */

/* The eruption's owner state: the owner's actor link, its view matrix (the
 * translation is where the eruption stands), the sprite asset whose low
 * byte selects the texture page, and the hit timer and radius. */
typedef struct RoomEruptionActor {
    void *entity;                 /* 0x00 */
    GteMatrix matrix;             /* 0x04 */
    union {
        int handle;
        unsigned char page;
    } asset;                      /* 0x24 */
    s16 reload;                   /* 0x28 */
    s16 cooldown;                 /* 0x2A */
    s16 collisionState;           /* 0x2C */
    s16 collisionRadius;          /* 0x2E */
} RoomEruptionActor;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomEruptionActor, asset) == 0x24,
                  room_eruption_actor_asset_offset);

/* The rising column: the emitter's image source, the emitter, the
 * column's scale and depth, and its rise speed. */
typedef struct RoomEruptionColumn {
    u8 header[8];
    u8 sourceData[0x100];         /* 0x008 */
    FieldRingGeometry emitter;    /* 0x108 */
    union {
        u16 unsignedValue;
        s16 signedValue;
    } scale;                      /* 0x120 */
    s16 depth;                    /* 0x122 */
    union {
        u16 unsignedValue;
        s16 signedValue;
    } speed;                      /* 0x124 */
} RoomEruptionColumn;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomEruptionColumn, scale) == 0x120,
                  room_eruption_column_scale_offset);

/* The flash at the owner's feet: its position and scale. */
typedef struct RoomEruptionGlow {
    s16 x;
    s16 y;
    s16 z;
    s16 pad06;
    s16 scale;                    /* 0x08 */
    s16 phase;                    /* 0x0A */
} RoomEruptionGlow;

/* The floor shadow: sixteen points under the owner, its scale and depth
 * (the depth is what RoomFxGroundSpriteParams reads at 0x82). */
typedef struct RoomEruptionShadow {
    GteShortVector point[16];
    s16 scale;                    /* 0x80 */
    s16 depth;                    /* 0x82 */
} RoomEruptionShadow;

/* Six eight-byte point/velocity records followed by gravity increments
 * and trail visibility flags, shared by debris setup and drawing. */
typedef struct RoomEruptionDebrisState {
    GteShortVector position[6];
    GteShortVector velocity[6];
    s32 gravityStep[6];
    s16 visible[6];
    s16 scale;
    s16 depth;
    s16 speed;
} RoomEruptionDebrisState;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomEruptionDebrisState, velocity) == 0x30,
                  room_eruption_debris_velocity_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomEruptionDebrisState, gravityStep) == 0x60,
                  room_eruption_debris_gravity_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomEruptionDebrisState, visible) == 0x78,
                  room_eruption_debris_visible_offset);

typedef struct RoomEruptionRingDirection {
    s16 x;
    s16 pad02;
    s16 z;
    s16 pad06;
} RoomEruptionRingDirection;

/* The shock ring: sixteen points around the owner, each moving outwards
 * along its own direction. */
typedef struct RoomEruptionRing {
    GteShortVector point[16];
    RoomEruptionRingDirection direction[16];
    s16 radius;
    s16 depth;
    u8 frame;
    u8 phase;
    s16 speed;
    s16 acceleration;
} RoomEruptionRing;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomEruptionRing, direction) == 0x80,
                  room_eruption_ring_direction_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomEruptionRing, depth) == 0x102,
                  room_eruption_ring_depth_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomEruptionRing, frame) == 0x104,
                  room_eruption_ring_frame_offset);

/* Per-room data: the four sprite packets the eruption draws with, named in
 * each overlay's symbol file. The glow is the flash at the owner's feet,
 * the debris the six flung particles, the shadow the floor sprite under
 * the rising column and the ring the sixteen-point shock ring. */
extern RoomFxSpritePacket g_RoomEruptionGlowPacket;
extern RoomFxSpritePacket g_RoomEruptionDebrisPacket;
extern RoomFxSpritePacket g_RoomEruptionShadowPacket;
extern RoomFxSpritePacket g_RoomEruptionRingPacket;
/* The debris packet's depth as its own symbol: the debris draw stores the
 * floor shadow's depth through a separate address, and a store through the
 * packet would let the compiler keep the packet's address in a * across the loop. */
extern s16 g_RoomEruptionDebrisDepth;

/* The eruption's module class (ground_eruption.c), in
 * class-table slot order, and the script and lists it hands the field
 * engine, named in each overlay's symbol file. */
extern int g_RoomEruptionScript[];
extern RoomModuleHandler g_RoomEruptionDrawList[];
extern RoomModuleHandler g_RoomEruptionUpdateList[];
extern RoomModuleHandler g_RoomEruptionInitList[];
extern int g_RoomEruptionSpawnLayout[];

int RoomEffect_GroundEruptionNop0(void);
int RoomEffect_GroundEruptionPlantScript(void *object);
int RoomEffect_GroundEruptionSpawn6(int a, int b, int c, int d, int e, int f);
int RoomEffect_GroundEruptionRegister(void *o);
int RoomEffect_GroundEruptionStart(void *o);
int RoomEffect_GroundEruptionRelease(struct RoomEnt *o);
int RoomEffect_GroundEruptionNop6(void);

int *FieldFx_VarPtr(int index);
int FieldFx_PlaySoundAtMatrixPosition(void *object, int message, void *state);
void FieldFx_BuildRingGeometry(void *params);

#endif
