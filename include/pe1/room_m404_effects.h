#ifndef PE1_ROOM_M404_EFFECTS_H
#define PE1_ROOM_M404_EFFECTS_H

#include "common.h"
#include "pe1/render_object.h"
#include "pe1/room_spark.h"

/* Room m404 burst effects: a debris burst emitted from a model joint and
 * the bouncing spark particle it spawns. */

typedef struct RoomM404BurstOrigin {
    s16 x, y, z;
} RoomM404BurstOrigin;

typedef struct RoomM404JointTemplate {
    s32 word[2];
} __attribute__((packed)) RoomM404JointTemplate;

extern const RoomM404JointTemplate g_RoomM404DebrisBurstAnchor;
extern const GteRotation g_RoomM404DebrisSparkRotation;
extern const RenderColor g_RoomM404DebrisSparkColor;
extern u8 g_RoomM404DebrisSparkColorTrack[];

extern int RoomM404_DebrisSparkCallback(int mode, void *data, intptr_t argument);

#endif
