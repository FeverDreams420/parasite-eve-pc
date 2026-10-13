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
extern int D_80195EF8;
extern int RoomM256_TrailCallback(int mode, void *state);

int RoomM256_TrailEmitter(int mode, u16 *out);

extern char g_RoomM256PulseParticleColorTrack[];
extern s16 g_RoomM256PulseBrightness;

void FieldFx_OffsetPointByAngles(void *, void *, int, void *, int, int);

#endif
