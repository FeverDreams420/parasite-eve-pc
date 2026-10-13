#ifndef PE1_ROOM_M005_H
#define PE1_ROOM_M005_H

#include "pe1/akao_send.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_m005_drifting_sprite.h"
#include "pe1/scene_archive.h"

/* Orbiter effect controller state and the event flag it watches. */
typedef struct RoomM005OrbiterState {
    u8 reserved[8];
    s16 armed;                    /* 0x08 */
    u16 frame;                    /* 0x0A */
} RoomM005OrbiterState;

typedef struct RoomM005OrbiterChild {
    s16 x, y, z;
    s16 flag;                     /* 0x06 */
    s16 seed;                     /* 0x08 */
    s16 radius;                   /* 0x0A */
    s16 state;                    /* 0x0C */
    s16 timer;                    /* 0x0E */
} RoomM005OrbiterChild;

typedef struct RoomM005Channel {
    s32 reserved[2];
    char *pool;
} RoomM005Channel;

typedef struct RoomM005EventState {
    u8 reserved[0x12];
    s16 triggered;                /* 0x12 */
} RoomM005EventState;

typedef struct RoomM005Seed8 {
    u8 bytes[8];
} RoomM005Seed8;

extern RoomM005Seed8 D_8018EFF4;
extern u8 D_80190B84[];
extern u8 D_80190B8C[];
extern int rand(void);
int RoomM005_Orbiter(int mode, RoomM005OrbiterState *state);

extern RenderColor D_8018F00C;
extern GteShortVector g_RoomM005BeaconOffset; /* drift offset, written by the joint beacon */
extern GteShortVector g_RoomM005BeaconCenter;

/* Cross flare (RoomM005_CrossFlare): two joint templates and the flash track. */
extern RoomM005Seed8 D_8018EFFC;
extern RoomM005Seed8 D_8018F004;
extern u8 g_RoomM005CrossFlareColorTrack[];

#endif
