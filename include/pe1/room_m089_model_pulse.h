#ifndef PE1_ROOM_M089_MODEL_PULSE_H
#define PE1_ROOM_M089_MODEL_PULSE_H

#include "common.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/room_m089_spin_model.h"
#include "pe1/room_swirl_rise.h"

/* room_m089 pulsing model burst (RoomM089_ModelPulse): three turning layers of
 * one model and a fourth model over a glow strip, while swirl/rise sprites
 * spawn around the anchor and the player is tagged on contact. */
typedef struct RoomM089ModelPulse {
    /* 0x00 */ GteShortVector position;
    /* 0x08 */ s16 phase;
    /* 0x0A */ s16 timer;
} RoomM089ModelPulse;

#include "pe1/player_entity.h"
extern u8 *g_RoomM089PulseModel;
extern u8 g_RoomM089PulseColorTrack[];

#include "pe1/fieldfx_task.h"
extern short g_RoomM089Anchor[3];
extern u16 g_RoomM089Script[22];
extern FieldFxTaskProgram g_RoomM089Program;
int RoomM089_SpiralTrailCallback(int mode,void *data,intptr_t argument);
int RoomM089_SpiralTrail(int mode,void *data,intptr_t argument);
int RoomM089_SpinModel(int mode,void *data,intptr_t argument);
int RoomM089_SwirlRiseSprite(int mode,void *data,intptr_t argument);
int RoomM089_ModelPulse(int mode,void *data,intptr_t argument);
intptr_t RoomM089_SetAnchor(int mode,intptr_t x,intptr_t y,intptr_t z);
int rand(void);

#endif
