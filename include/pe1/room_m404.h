#ifndef PE1_ROOM_M404_H
#define PE1_ROOM_M404_H

#include "common.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/field_transform_point.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"

typedef struct { s32 word[2]; } __attribute__((packed)) RoomTemplate8;
extern const RoomTemplate8 g_RoomM404SinkingDropAnchor;

#include "pe1/fieldfx_task.h"
int RoomM404_SinkingDropCallback(int, void *, intptr_t);
int RoomM404_SinkingDrop(int, void *, intptr_t);
int RoomM404_DebrisSparkCallback(int, void *, intptr_t);
int RoomM404_DebrisBurst(int, void *, intptr_t);
extern FieldFxTaskProgram g_RoomM404SinkingDropEffects, g_RoomM404DebrisBurstEffects;
extern u16 g_RoomM404SinkingDropScript[16],g_RoomM404DebrisBurstScript[10];
#endif
