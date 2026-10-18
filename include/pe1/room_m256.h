#ifndef PE1_ROOM_M256_H
#define PE1_ROOM_M256_H

#include "pe1/asset_find.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/room_m256_trail.h"
#include "pe1/room_model_pulse_particle.h"
#include "pe1/scene_archive.h"

/* ABI declarations used by the room_m256 effect dispatcher. */
extern void *g_RoomM256TrailModel;
int RoomM256_TrailCallback(int, void *, intptr_t);

int RoomM256_TrailEmitter(int, void *, intptr_t);

extern char g_RoomM256PulseParticleColorTrack[];
extern s16 g_RoomM256PulseBrightness;

#include "pe1/field_orbit_point.h"

#include "pe1/pm.h"
extern PmCommand g_RoomM256SwingClass;
extern FieldFxTaskProgram g_RoomM256ScatterEffects,g_RoomM256TrailEffects;
extern u16 g_RoomM256ScatterScript[18],g_RoomM256TrailScript[34];
extern s32 g_RoomM256ScatterConfig,g_RoomM256TrailConfig,g_RoomM256TrailTurnScale;
intptr_t RoomM256_ScatterConfigure(int,intptr_t,intptr_t,intptr_t);
intptr_t RoomM256_SetTrailTurnScale(int,intptr_t,intptr_t,intptr_t);
int RoomM256_ScatterDropCallback(int,void *,intptr_t);
int RoomM256_ScatterDropEmitter(int,void *,intptr_t);
int RoomM256_SwirlSparkEmitter(int,void *,intptr_t);
#endif
