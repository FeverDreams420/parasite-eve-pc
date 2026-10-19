#ifndef PE1_EXT_M273
#define PE1_EXT_M273

#include "common.h"
#include "pe1/battle.h"
#include "pe1/battle_setup.h"
#include "pe1/field_actor.h"
#include "pe1/gte_types.h"
#include "pe1/room_fx.h"

#include "room_m273.h"
#include "room_m273_effects.h"

typedef GteShortVector Vector;

/* Threshold model triple callbacks and controller. */
typedef struct { short x,y,z,phase,scaleXZ,scaleY; unsigned short parameter12,unknown14; } ThresholdTripleEffect;

typedef struct { void *asset,*pool; } AssetState;

typedef struct { unsigned char r, g, b, unknown; } Color;

/* Partial storage views; the fourth halfword carries the emitter flag. */
typedef RoomM273PairPoolRecord Entry;

typedef struct { Entry entries[2]; unsigned char unknown[14]; unsigned char stopped; } EmitterState;

typedef struct {
    char pad_00[4];
    s32 *source;
    s16 scaleX;
    s16 scaleY;
    u16 value;
    u16 step;
} RoomM273PulseState;

typedef struct {
    u16 *source;
} RoomM273PhaseState;

extern FieldActor *g_PlayerEntity;
void RoomM273_GrabHoldStep(SceneGrabHead *state);
void RoomM273_GrabWaitRelease(SceneGrabHead *state);
extern u8 g_RoomM273ActionWeights[];
extern s32 Inv_ScrambleGrid(void);
extern GteRotation g_RoomM273FloorRotation;
extern unsigned char g_RoomM273PlayerPulseSpawned;
extern u8 g_RoomM273GrabEffectStopped;
extern RoomM273PulseSeed g_RoomM273PlayerStreakSeeds[2];
extern int FieldFx_UpdateCallbackList(void *);
extern short g_RoomM273GrabModelRingX[];
extern int g_RoomM273GrabModelRingZ[];
extern AssetState g_RoomM273GrabModel;
extern unsigned char g_RoomM273GrabEffectStopped;
extern void FieldFx_ModelSetTexture(int,int);
int RoomM273_GrabModelCallback(int mode, void *data, intptr_t argument);
extern short g_RoomM273PlayerTrailBurstsLeft, g_RoomM273PlayerTrailDelay;
int RoomM273_GrabSparkTriangleCallback(int mode, void *data, intptr_t argument);
extern s16 g_RoomM273HandTrailPhase, g_RoomM273HandTrailCooldown, g_RoomM273HandTrailPeriodA, g_RoomM273HandTrailPeriodB;
extern void *g_RoomM273HandTrailLandingFanPool;
int RoomM273_HandTrailLandingPulseCallback(int mode, void *data, intptr_t argument);
int RoomM273_HandTrailLandingFanCallback(int mode, void *data, intptr_t argument);
extern Color g_RoomM273HandTrailHitColor;
extern Vector g_RoomM273SweepDropVector;
extern int Math_IntSqrt(int);
extern short g_RoomM273SweepSplashScale[];
extern short g_RoomM273SweepRisingDelay;
extern unsigned char g_RoomM273SweepRisingColor[];
extern char g_RoomM273SwayShardTrailColor[];
extern unsigned char g_RoomM273SwayGroundColor[], g_RoomM273BlackColor[];
extern short g_RoomM273SwayPointerSpawned;
extern short g_RoomM273GrabColorIndex;
extern u8 g_RoomM273HoldFlareSpawned;
#endif /* PE1_EXT_M273 */
