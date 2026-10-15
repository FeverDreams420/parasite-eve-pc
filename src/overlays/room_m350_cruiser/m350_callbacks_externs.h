#ifndef PE1_EXT_M350_CALLBACKS
#define PE1_EXT_M350_CALLBACKS

#include "common.h"
#include "pe1/battle.h"
#include "pe1/field_actor.h"
#include "pe1/field_color.h"
#include "pe1/gte_short_vector.h"
#include "pe1/gte_types.h"
#include "pe1/room_m350_effects.h"

typedef struct { unsigned char frame,slot,value,pad; } Event;

typedef struct { Event entries[8]; } Events;

extern short g_RoomM350AreaRepeats;
extern unsigned char g_RoomM350AreaPlayerInside;
extern FieldActor *g_PlayerEntity;
extern short g_RoomM350DropHitCooldown;
extern int Math_IntSqrt(int);
extern unsigned char g_RoomM350DropEmitterTrigger;
extern short g_RoomM350DropLungeTimer;
extern int Inv_ScrambleGrid(void);
extern RenderColor g_RoomM350DropHitColor;
extern unsigned char g_RoomM350DropHitTrigger;
extern short g_RoomM350DropHitPosition[3];
extern short g_RoomM350DropTrailCount;
s32 RoomM350_DropTrailCallback(s32 mode, void *ctx);
extern int g_RoomM350DropBurstColor[];
extern unsigned char g_RoomM350DropBurstCount;
int RoomM350_DropBurstCallback(int event, short *object);
extern Events g_RoomM350DropEvents;
extern int g_RoomM350DropBurstState;
extern short g_RoomM350DropTrailCount,g_RoomM350DropRepeats,g_RoomM350DropLungeTimer,g_RoomM350DropHitCooldown;
extern short g_RcossinTblCos[];
extern GteMatrix g_RoomM350RushMatrix;
extern short g_RoomM350RushCenter[];
extern int g_RoomM350RushFlareColors[];
extern unsigned char g_RoomM350RushPlayerHit;
extern volatile short g_RoomM350RushEndpointX,g_RoomM350RushOppositeX;
extern GteShortVector g_RoomM350RushPreviousEndpoint,g_RoomM350RushPreviousOpposite;
extern volatile short g_RoomM350RushEndpointZ,g_RoomM350RushEndpointValid,g_RoomM350RushOppositeZ;
extern RenderColor g_RoomM350GlowSpriteColor;
extern unsigned char g_RoomM350BodyDone;
extern int Asset_Find08w(int,int,int,int,int);
int RoomM350_BodyDebrisCallback(int event, RoomM350FadingParticle *object);
extern int rand(void),Inv_ScrambleGrid(void),rcos(int),rsin(int);
extern RenderColor g_RoomM350BodyRingColor0, g_RoomM350BodyRingColor1;
extern int rand(void);
extern void g_RoomM350HandSparkColor;
extern unsigned char g_RoomM350AreaTrackPlayer;
#include "pe1/player_entity.h"
extern void RoomM350_AreaPlayerPulseCallback(void);
int RoomM350_DropFlareCallback(int event, int **position);
extern char g_RoomM350DropSplashTrigger;
extern u16 g_RoomM350DropSplashPosX;
extern u16 g_RoomM350DropSplashPosZ;
int RoomM350_DropSplashCallback(int event, void *context);
extern void g_RoomM350DropTrailColor;
extern int g_RoomM350DropFlareColors[];
extern int g_RoomM350DropSplashColor[];
#endif /* PE1_EXT_M350_CALLBACKS */
