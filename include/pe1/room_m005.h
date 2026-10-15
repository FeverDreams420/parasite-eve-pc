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

typedef union RoomM005Seed8 {
    u8 bytes[8];
    GteShortVector vector;
} RoomM005Seed8;

extern RoomM005Seed8 g_RoomM005OrbiterJointAnchor;
/* One-field views of RoomM005_Dialog (g_RoomScriptRecord): the orbiter takes
 * the anchor pointer (+0x04) and the fade block (+0x0C) with their own
 * addresses, where a member access shares the record base. */
extern u8 *g_RoomM005ScriptAnchorView;
extern u8 *g_RoomM005ScriptFadeView;
extern int rand(void);
int RoomM005_Orbiter(int mode, void *state, intptr_t argument);
int RoomM005_CrossFlare(int mode, void *state, intptr_t argument);

extern RenderColor g_RoomM005BeaconTint;
extern GteShortVector g_RoomM005BeaconOffset; /* drift offset, written by the joint beacon */
extern GteShortVector g_RoomM005BeaconCenter;

/* Cross flare (RoomM005_CrossFlare): two joint templates and the flash track. */
extern RoomM005Seed8 g_RoomM005CrossFlareFlashAnchor;
extern RoomM005Seed8 g_RoomM005CrossFlareTwinAnchor;
extern u8 g_RoomM005CrossFlareColorTrack[80];

#endif
