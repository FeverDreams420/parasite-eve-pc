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
extern unsigned char D_8019A855;
extern short g_RoomM350DropLungeTimer;
extern int Inv_ScrambleGrid(void);
extern RenderColor D_8019A62C;
extern unsigned char D_8019A857;
extern short D_8019A864[3];
extern short D_8019A85C;
s32 RoomM350_DropTrailCallback(s32 mode, void *ctx);
extern int D_8019A634[];
extern unsigned char g_RoomM350DropBurstCount;
int RoomM350_DropBurstCallback(int event, short *object);
extern Events g_RoomM350DropEvents;
extern int D_8019A858;
extern short D_8019A85C,g_RoomM350DropRepeats,g_RoomM350DropLungeTimer,g_RoomM350DropHitCooldown;
extern short D_800966EE[];
extern GteMatrix g_RoomM350RushMatrix;
extern short g_RoomM350RushCenter[];
extern int D_8019A690[];
extern unsigned char g_RoomM350RushPlayerHit;
extern volatile short D_8019A898,D_8019A8A0;
extern GteShortVector D_8019A8A8,D_8019A8B0;
extern volatile short D_8019A89C,D_8019A89E,D_8019A8A4;
extern RenderColor D_8019A3D0;
extern unsigned char g_RoomM350BodyDone;
extern int Asset_Find08w(int,int,int,int,int);
int RoomM350_BodyDebrisCallback(int event, RoomM350FadingParticle *object);
extern int rand(void),Inv_ScrambleGrid(void),rcos(int),rsin(int);
extern RenderColor D_8019A738, D_8019A734;
extern int rand(void);
extern void D_8019A410;
extern short D_8019A82C;
extern unsigned char g_RoomM350AreaTrackPlayer;
extern struct BattleEntity *D_8009D254;
extern void RoomM350_AreaPlayerPulseCallback(void);
int RoomM350_DropFlareCallback(int event, int **position);
extern char D_8019A856;
extern u16 D_8019A860;
extern u16 D_8019A862;
int RoomM350_DropSplashCallback(int event, void *context);
extern void D_8019A630;
extern int D_8019A614[];
extern int D_8019A628[];
#endif /* PE1_EXT_M350_CALLBACKS */
