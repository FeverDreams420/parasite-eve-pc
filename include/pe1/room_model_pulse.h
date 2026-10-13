#ifndef PE1_ROOM_MODEL_PULSE_H
#define PE1_ROOM_MODEL_PULSE_H

#include "pe1/field_texture_tables.h"
#include "pe1/room_model_pulse_particle.h"
#include "pe1/room_spark.h"

/* room_m256 pulsing model effect: one controller (RoomM256_ModelPulse) that seeds
 * sixteen ring particles on frame 1, draws a ring sprite at the slot-2
 * position and, from frame 4 on, a model scaled by cos and brightened by sin.
 * The ring particles (RoomM256_ModelPulseParticleCallback) read the shared state below. */

typedef struct RoomModelPulseMatrix {
    s16 m[3][3];
    s16 reserved12;
    s32 t[3];                     /* 0x14 */
} RoomModelPulseMatrix;

typedef struct RoomModelPulseScale {
    s32 x, y, z, reserved0C;
} RoomModelPulseScale;

extern GteRotation g_RoomM256PulseAnchor;          /* ring placement rotation seed */
extern u8 g_RoomM256PulseColorTrack[];                 /* colour ramp table */
extern s16 g_RoomM256PulseBrightness;                  /* ring brightness */
extern void *g_RoomM256PulseModel;                /* model asset */

int RoomM256_ModelPulse(int mode, s16 *state);

#endif /* PE1_ROOM_MODEL_PULSE_H */
