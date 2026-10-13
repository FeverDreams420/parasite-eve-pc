#ifndef PE1_EXT_M350
#define PE1_EXT_M350

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/field_color.h"
#include "pe1/gte_short_vector.h"
#include "pe1/gte_types.h"

#include "room_m350_shared.h"

typedef RoomM350Vector Vector;

typedef GteMatrix Matrix;

typedef struct { unsigned char index, alternate; } Entry;

extern short g_RcossinTblCos[];
extern int Inv_ScrambleGrid(void);
extern int g_RoomM350HandPulseColors[];
extern GteShortVector g_RoomM350HandPoints[];
extern FieldActor *g_PlayerEntity;
extern RenderColor g_RoomM350HandSweepHitColor;
extern unsigned char g_RoomM350HandSweepDone,g_RoomM350HandSweepHit;
extern short g_RoomM350HandPointPadView[], g_RoomM350HandSweepRepeats;
extern int g_RoomM350OrbHandSparkColor[];
extern int g_RoomM350OrbBurstRingColorA[], g_RoomM350OrbBurstRingColorB[];
extern GteRotation g_RoomM350StarFanTilt;
extern RenderColor g_RoomM350OrbGroundFanColor,g_RoomM350BlackColor;
extern int Math_IntSqrt(int);
extern GteShortVector g_RoomM350OrbSpawnPoint;
extern RenderColor g_RoomM350OrbImpactColor;
extern Entry g_RoomM350OrbTableFlashEntries[][2];
extern unsigned char g_RoomM350OrbTableFlashSizes[];
extern RenderColor g_RoomM350GlowSpriteColor;
extern int g_RoomM350AreaSparkColor[];
extern char g_RoomM350AreaSparkEmitterColor[];
extern unsigned char g_RoomM350AreaTrackPlayer, g_RoomM350AreaPlayerInside;
extern Matrix g_RoomM350AreaMatrix;
extern int g_RoomM350AreaMatrixTxView;
extern Vector g_RoomM350AreaCorners[];
extern char g_RoomM350AreaZoneTriangleColor[];
extern void FieldFx_ModelSetTexture(int,int);
extern int g_RoomM350AreaDropletColor[];
#endif /* PE1_EXT_M350 */
