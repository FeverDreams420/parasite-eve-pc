#ifndef PE1_ROOM_FLOOR_WALKER_H
#define PE1_ROOM_FLOOR_WALKER_H
#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/fieldfx_task.h"
#include "pe1/pm_types.h"
#include "pe1/gte_types.h"
struct RoomEnt;
/* Original handler G instance state; only pointer-bearing fields widen. */
typedef struct RoomFloorWalkerState {
    void (*callback)(struct RoomEnt *);
    int *signal;
    s16 active;
    s8 variant, frame;
#if INTPTR_MAX > INT32_MAX
    u8 argumentHigh[sizeof(intptr_t)-sizeof(s32)];
#endif
    s8 enabled;
    u8 flags, started, reserved;
    FieldActor *leader;
    s32 speed, hopVelocity, velocity, gravity;
    u16 angle;
    s16 rest;
    u16 restTime;
    s16 turning;
} RoomFloorWalkerState;
typedef struct RoomFloorWalkerObject {
    PmSlotHeader header;
    RoomFloorWalkerState state;
} RoomFloorWalkerObject;
/* One step's original scratchpad workspace, now ordinary local storage. */
typedef struct RoomFloorWalkerScratch {
    union {GteShortVector v;struct {s32 xy;s16 z;} words;} step;
    GteVector rotated,next;
    GteMatrix yaw;
    s32 area;
} RoomFloorWalkerScratch;
extern PmCommand g_RoomFloorWalkerHandlers;
extern FieldFxTaskProgram g_RoomTimedSpawnProgram;
extern u16 g_RoomTimedSpawnScript[16];
extern u8 RoomLib_TimedRenderTable[32];
int RoomLib_UpdateTimedRender(int,void *,intptr_t);
int RoomLib_ControlTimedSpawn(int,void *,intptr_t);
void RoomLib_HandlerGNop0(void);
void RoomLib_InitHandlerG(PmSlotHeader *);
int RoomLib_ConfigureHandlerG(PmSlotHeader *,int,int,intptr_t,intptr_t,intptr_t);
int RoomLib_HandlerGNop3(PmSlotHeader *);
int RoomLib_UpdateHandlerG(PmSlotHeader *);
void RoomLib_HandlerG(struct RoomEnt *);
void RoomLib_HandlerGFloorWalker(struct RoomEnt *);
int RoomLib_ReleaseHandlerG(PmSlotHeader *);
void RoomLib_HandlerGNop6(void);
#endif
