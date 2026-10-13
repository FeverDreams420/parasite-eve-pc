#ifndef PE1_EXT_M318
#define PE1_EXT_M318

#include "common.h"
#include "pe1/battle.h"
#include "pe1/field_actor.h"
#include "pe1/field_color.h"

extern struct BattleEntity *D_8009D254;
extern char g_RoomM318GetMayaPosErrorFormat[];
extern char g_RoomM318FanSweepTrailColorTrack[];
extern s32 rand(void);
extern char g_RoomM318SpiralDropSparkColorTrack[];
extern int g_RoomM318StarFlarePhase;
extern char g_RoomM318StarFlareColorTrackA[], g_RoomM318StarFlareColorTrackB[];
extern char g_RoomM318ShakeBurstParticleColorTrack[];
extern int rand(void);
extern RenderColor g_RoomM318EmberParticleColor;
extern u8 g_RoomM318EmberBurstParticleColorTrack[];
extern int g_RoomM318ActorParams[2];
extern int g_RoomM318EmberBurstParams[2];
extern int g_RoomM318RisingColumnParams[4];
#endif /* PE1_EXT_M318 */
