#ifndef PE1_EXT_M123
#define PE1_EXT_M123

#include "common.h"
#include "pe1/battle.h"
#include "pe1/gte_short_vector.h"

#include "room_m123_effects.h"

extern s16 g_RoomM123SwirlSpeed, g_RoomM123SwirlBrightness;
extern u16 g_RoomM123SwirlOrigin[3];
extern u8 g_RoomM123SwirlAngles[];
extern GteShortVector g_RoomM123JointAnchor;
int RoomM123_JointSwirlParticleCallback(int mode, void *data, intptr_t argument);
extern int rand(void);
extern GteShortVector g_RoomM123PulseRingTilt;
#include "pe1/player_entity.h"
int RoomM123_PulseRingCallback(int mode, void *data, intptr_t argument);
extern GteShortVector g_RoomM123JointOffset;
extern GteShortVector g_RoomM123WaveSpinBase;
extern char g_RoomM123WaveColorTrack[];
#endif /* PE1_EXT_M123 */
