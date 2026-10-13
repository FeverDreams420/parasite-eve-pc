#ifndef PE1_EXT_M123
#define PE1_EXT_M123

#include "common.h"
#include "pe1/battle.h"
#include "pe1/gte_short_vector.h"

#include "room_m123_effects.h"

extern s16 g_RoomM123SwirlSpeed, g_RoomM123SwirlBrightness;
extern u16 g_RoomM123SwirlOrigin[3];
extern u8 g_RoomM123SwirlAngles[];
extern void FieldFx_OffsetPointByAngles(void *, void *, int, void *, int, int);
extern GteShortVector g_RoomM123JointAnchor;
int RoomM123_JointSwirlParticleCallback(int mode, RoomM123Particle *particle);
extern int rand(void);
extern GteShortVector g_RoomM123PulseRingTilt;
extern struct BattleEntity *D_8009D254;
int RoomM123_PulseRingCallback(int mode, RoomPulseParticle *particle, int *reference);
extern GteShortVector g_RoomM123JointOffset;
extern GteShortVector g_RoomM123WaveSpinBase;
extern GteShortVector g_RoomM123JointOffset;
extern char g_RoomM123WaveColorTrack[];
#endif /* PE1_EXT_M123 */
