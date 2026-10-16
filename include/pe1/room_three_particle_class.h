#ifndef PE1_ROOM_THREE_PARTICLE_CLASS_H
#define PE1_ROOM_THREE_PARTICLE_CLASS_H

#include "pe1/room_module.h"

/* The three transform particles' module class
 * (src/overlays/room_lib/three_particles.c): the plant, spawn,
 * registration, start and close methods of the room module class, in
 * class-table slot order, and the script and lists they hand the field
 * engine, named in each room's symbol file. The class's no-ops open the
 * particles' run in the same unit. */
extern int g_RoomThreeParticleScript[];
extern FieldFxHandler g_RoomThreeParticleDrawList[];
extern FieldFxHandler g_RoomThreeParticleUpdateList[];
extern FieldFxHandler g_RoomThreeParticleInitList[];
extern u16 g_RoomThreeParticleSpawnLayout[4];
extern PmCommand g_RoomThreeParticleHandlers;

void RoomLib_ThreeParticleNop0(void);
void RoomLib_ThreeParticleNop1(void);
void RoomLib_ThreeParticlePlantScript(PmSlotHeader *object);
int RoomLib_ThreeParticleSpawn6(PmSlotHeader *slot, int b, int c, intptr_t d, intptr_t e, intptr_t f);
int RoomLib_ThreeParticleRegister(PmSlotHeader *object);
int RoomLib_ThreeParticleStart(PmSlotHeader *object);
int RoomLib_ThreeParticleClose(PmSlotHeader *object);

#endif
