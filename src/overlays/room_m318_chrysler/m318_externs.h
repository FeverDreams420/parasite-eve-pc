#ifndef PE1_EXT_M318
#define PE1_EXT_M318

#include "common.h"
#include "pe1/battle.h"
#include "pe1/field_actor.h"
#include "pe1/field_color.h"

#include "pe1/player_entity.h"
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

extern int g_RoomM318ScriptArg2;
extern void *g_RoomM318EmberSpiralModel;

/* Native task descriptors and original script entry points. */
#include "pe1/fieldfx_task.h"
int RoomM318_PlayerGate(int, void *, intptr_t);
intptr_t RoomM318_SetScriptArg2(int, intptr_t, intptr_t, intptr_t);
intptr_t RoomM318_GetActorParams(int, intptr_t, intptr_t, intptr_t);
extern FieldFxTaskProgram g_RoomM318FanSweepEffects;
extern FieldFxTaskProgram g_RoomM318ShakeSweepEffects;
extern FieldFxTaskProgram g_RoomM318ShakeBurstEffects;
extern FieldFxTaskProgram g_RoomM318EmberBurstEffects;
extern FieldFxTaskProgram g_RoomM318RisingColumnEffects;
extern FieldFxTaskProgram g_RoomM318FlashEffects;
extern FieldFxTaskProgram g_RoomM318TwinRibbonEffects;
extern FieldFxTaskProgram g_RoomM318EmberFountainEffects;
extern char g_RoomM318GetMayaPosErrorFormat[36];
extern char g_RoomM318FanSweepTrailColorTrack[40];
extern char g_RoomM318SpiralDropSparkColorTrack[48];
extern u8 g_RoomM318SpiralDropColorTrack[48];
extern char g_RoomM318StarFlareColorTrackA[32];
extern char g_RoomM318StarFlareColorTrackB[32];
extern char g_RoomM318ShakeBurstParticleColorTrack[40];
extern u8 g_RoomM318EmberBurstParticleColorTrack[40];
extern u8 g_RoomM318RisingColumnParticleColorTrack[32];
extern u8 g_RoomM318FlashBurstColorTrack[40];
extern u8 g_RoomM318CrossFlashParticleColorTrack[40];
extern u8 g_RoomM318EmberFountainParticleColorTrack[40];
extern u8 g_RoomM318EmberSpiralColorTrack[40];
extern u16 g_RoomM318FanSweepScript[16];
extern u16 g_RoomM318ShakeSweepScript[18];
extern u16 g_RoomM318ShakeBurstScript[28];
extern u16 g_RoomM318EmberBurstScript[12];
extern u16 g_RoomM318RisingColumnScript[12];
extern u16 g_RoomM318FlashScript[22];
extern u16 g_RoomM318TwinRibbonScript[10];
extern u16 g_RoomM318EmberFountainScript[18];
int RoomM318_FanSweepTrailCallback(int, void *, intptr_t);
int RoomM318_FanSweep(int, void *, intptr_t);
int RoomM318_SpiralDropSparkCallback(int, void *, intptr_t);
int RoomM318_SpiralDrop(int, void *, intptr_t);
int RoomM318_ShakeSweepParticleCallback(int, void *, intptr_t);
int RoomM318_ShakeSweep(int, void *, intptr_t);
int RoomM318_ArcingSparkCallback(int, void *, intptr_t);
int RoomM318_ArcingSparks(int, void *, intptr_t);
int RoomM318_StarFlare(int, void *, intptr_t);
int RoomM318_ShakeBurstParticleCallback(int, void *, intptr_t);
int RoomM318_ShakeBurst(int, void *, intptr_t);
int RoomM318_EmberBurstParticleCallback(int, void *, intptr_t);
int RoomM318_EmberBurst(int, void *, intptr_t);
int RoomM318_RisingColumnParticleCallback(int, void *, intptr_t);
int RoomM318_RisingColumn(int, void *, intptr_t);
int RoomM318_FlashBurstParticleCallback(int, void *, intptr_t);
int RoomM318_FlashBurst(int, void *, intptr_t);
int RoomM318_CrossFlashParticleCallback(int, void *, intptr_t);
int RoomM318_CrossFlash(int, void *, intptr_t);
int RoomM318_TwinRibbonPulseCallback(int, void *, intptr_t);
int RoomM318_TwinRibbon(int, void *, intptr_t);
int RoomM318_EmberFountainParticleCallback(int, void *, intptr_t);
int RoomM318_EmberFountain(int, void *, intptr_t);
int RoomM318_EmberSpiralParticleCallback(int, void *, intptr_t);
int RoomM318_EmberSpiral(int, void *, intptr_t);
intptr_t RoomM318_SetActorParams(int, intptr_t, intptr_t, intptr_t);
intptr_t RoomM318_SetEmberBurstParams(int, intptr_t, intptr_t, intptr_t);
intptr_t RoomM318_SetRisingColumnParams(int, intptr_t, intptr_t, intptr_t);
#endif /* PE1_EXT_M318 */
