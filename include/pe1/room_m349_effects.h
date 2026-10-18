#ifndef PE1_ROOM_M349_EFFECTS_H
#define PE1_ROOM_M349_EFFECTS_H

#include "common.h"
#include "pe1/field_texture_tables.h"
#include "pe1/render_object.h"
#include "pe1/room_spark.h"
#include "pe1/fieldfx_task.h"

/* Room m349 flare spark: jitters its velocity (states 0 and 1) or its
 * position (state 2) while it falls and bounces off the floor height. */
typedef struct RoomM349FlareSpark {
    s16 x, y, z;                  /* 0x00 */
    u16 angle;                    /* 0x06 */
    s16 vx, vy, vz;               /* 0x08 */
    s16 size;                     /* 0x0E */
    s16 state;                    /* 0x10 */
    u16 timer;                    /* 0x12 */
} RoomM349FlareSpark;

PE1_STATIC_ASSERT(sizeof(RoomM349FlareSpark) == 0x14, room_m349_flare_spark_size);

/* Controller target: the burst origin plus the looping sound handle. */
typedef struct RoomM349FlareTarget {
    GteShortVector position;
    s16 sound;                    /* 0x08: -1 when no sound plays */
} RoomM349FlareTarget;

typedef struct RoomM349FlareParams {
    s16 x, y, z, pad;
    s32 radius;                   /* 0x08 */
    s32 period;                   /* 0x0C: frames between ring sparks */
    s32 lastFrame;                /* 0x10: last frame that spawns ring sparks */
} RoomM349FlareParams;

extern GteRotation g_RoomM349SparkRotationBase;
extern GteShortVector g_RoomM349JointGlowAnchorNegY;
extern GteShortVector g_RoomM349JointGlowAnchorPosY;
extern void Akao_Cmd_21(int handle, int arg);
int RoomM349_FlareSparkCallback(int mode, void *data, intptr_t argument);
int RoomM349_FlareRing(int mode, void *data, intptr_t argument);
intptr_t RoomM349_SetFlareParams(int command, intptr_t x, intptr_t y, intptr_t z);
extern RenderColor g_RoomM349FlareColor;
extern RoomM349FlareParams g_RoomM349FlareParams;
extern u16 g_RoomM349FlareScript[18];
extern FieldFxTaskProgram g_RoomM349FlareEffects;

#endif
