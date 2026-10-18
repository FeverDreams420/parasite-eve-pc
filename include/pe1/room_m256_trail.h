#ifndef PE1_ROOM_M256_TRAIL_H
#define PE1_ROOM_M256_TRAIL_H

#include "pe1/field_actor.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task.h"
#include "pe1/gte_types.h"
#include "pe1/player_entity.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"

/* Shared 24-byte pool record: steering trail, damped sparks, model burst,
 * expanding sprite and floor impact. Modes 1 and 2 update and draw it. */
typedef struct RoomM256TrailEffect {
    s16 x, y, z, index;               /* 0x00 */
    s16 motionX, motionY, motionZ;     /* 0x08: angles or particle velocity */
    s16 ending;                       /* 0x0E */
    s16 speed, angle, kind, frame;     /* 0x10 */
} RoomM256TrailEffect;
PE1_STATIC_ASSERT(sizeof(RoomM256TrailEffect) == 0x18, room_m256_trail_effect_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM256TrailEffect, kind) == 0x14,
                  room_m256_trail_effect_kind);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM256TrailEffect, frame) == 0x16,
                  room_m256_trail_effect_frame);

extern GteRotation g_RoomM256SpriteTilt;
extern const RenderColor g_RoomM256TrailColor, g_RoomM256TrailDarkColor;
extern const GteRotation g_RoomM256TrailAnchor;
extern GteRotation g_RoomM256SpriteTilt;
/* Sixteen stored steps plus the current position, each an eight-byte vector. */
typedef struct RoomM256TrailHistory {
    GteShortVector point[17];
} RoomM256TrailHistory;
PE1_STATIC_ASSERT(sizeof(RoomM256TrailHistory) == 0x88, room_m256_trail_history_size);
extern RoomM256TrailHistory g_RoomM256TrailHistory[3];
/* Original overlapping views within the contiguous three-history array. */
#define g_RoomM256TrailPoint4X ((char *)g_RoomM256TrailHistory + PE1_OFFSETOF(RoomM256TrailHistory,point[4].x))
#define g_RoomM256TrailPoint4Y ((char *)g_RoomM256TrailHistory + PE1_OFFSETOF(RoomM256TrailHistory,point[4].y))
#define g_RoomM256TrailPoint4Z ((char *)g_RoomM256TrailHistory + PE1_OFFSETOF(RoomM256TrailHistory,point[4].z))
extern s32 g_RoomM256TrailTurnScale;
extern void *g_RoomM256TrailModel;
typedef struct RoomM256TrailPalettes { u16 first, second; } RoomM256TrailPalettes;
int rand(void);
void FieldFx_ModelSetTexture(int,int);

int RoomM256_TrailCallback(int mode, void *effect, intptr_t argument);

#endif
