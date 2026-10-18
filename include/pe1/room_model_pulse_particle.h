#ifndef PE1_ROOM_MODEL_PULSE_PARTICLE_H
#define PE1_ROOM_MODEL_PULSE_PARTICLE_H

#include "common.h"
#include "pe1/gte_short_vector.h"

/* room_m256 pulsing model: one ring particle. RoomM256_ModelPulseParticleCallback advances and
 * draws it; the model pulse controller (RoomM256_ModelPulse) seeds sixteen. */
typedef struct RoomModelPulseParticle {
    u16 frame;                    /* 0x00 */
    u16 offset;                   /* 0x02 */
    s16 scale;                    /* 0x04 */
} RoomModelPulseParticle;

typedef struct RoomModelPulseAnchor {
    s16 x, y, z, pad;
} RoomModelPulseAnchor;

/* Shared with the controller: the ring spin it writes (slot-2 output) and
 * the ring centre the particles orbit. */
extern GteShortVector g_RoomM256PulseAngles;
extern RoomModelPulseAnchor g_RoomM256PulseCenter;

int RoomM256_ModelPulseParticleCallback(int, void *, intptr_t);

#endif /* PE1_ROOM_MODEL_PULSE_PARTICLE_H */
