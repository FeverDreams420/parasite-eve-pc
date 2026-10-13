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

extern short D_800966EE[];
extern int Inv_ScrambleGrid(void);
extern int g_RoomM350HandPulseColors[];
extern GteShortVector g_RoomM350HandPoints[];
extern FieldActor *g_PlayerEntity;
extern RenderColor D_8019A464;
extern unsigned char g_RoomM350HandSweepDone,g_RoomM350HandSweepHit;
extern short D_8019A77E[], g_RoomM350HandSweepRepeats;
extern int D_8019A3CC[];
extern int D_8019A4DC[], D_8019A4E0[];
extern GteRotation D_8019A3C0;
extern RenderColor D_8019A4E4,D_8019A3C8;
extern int Math_IntSqrt(int);
extern GteShortVector g_RoomM350OrbSpawnPoint;
extern RenderColor D_8019A4E8;
extern Entry g_RoomM350OrbTableFlashEntries[][2];
extern unsigned char D_8019A4EC[];
extern RenderColor D_8019A3D0;
extern int D_8019A56C[];
extern char D_8019A570[];
extern unsigned char g_RoomM350AreaTrackPlayer, g_RoomM350AreaPlayerInside;
extern Matrix g_RoomM350AreaMatrix;
extern int D_8019A81C;
extern Vector g_RoomM350AreaCorners[];
extern char D_8019A594[];
extern void FieldFx_ModelSetTexture(int,int);
extern int D_8019A598[];
#endif /* PE1_EXT_M350 */
