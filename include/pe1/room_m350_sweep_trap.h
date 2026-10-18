#ifndef PE1_ROOM_M350_SWEEP_TRAP_H
#define PE1_ROOM_M350_SWEEP_TRAP_H

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_sine_table.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"

/* room_m350 sweeping beam trap (RoomM350_HandSweepTrapCallback): a column of sprites swept
 * down from the actor's head, a flare and two fan blades towards the
 * anchor, and a quad test that tags the player once. */
typedef struct RoomM350SweepTrap {
    /* 0x00 */ GteShortVector *anchor;
    /* 0x04 */ s16 done;
} RoomM350SweepTrap;

typedef FieldActorState RoomM350TrapNode;
typedef FieldActor RoomM350TrapActor;
typedef FieldFxObjectPrefix RoomM350TrapChannel;
typedef FieldActor RoomM350Player;

/* rcossin table pairs; the bitfields make each read an SImode extract,
 * which keeps the lh + sra form of retail's shifted reads. */
typedef struct RoomM350Trig {
    int sin : 16;
    int cos : 16;
} RoomM350Trig;

extern struct FieldActor *g_PlayerEntity;
extern RenderColor g_RoomM350HandSweepTrapPulseColors[];
extern RenderColor g_RoomM350BlackColor;
extern RenderColor g_RoomM350HandSweepTrapTriangleColor;
extern RenderColor g_RoomM350HandSweepTrapRingColor;
extern GteShortVector g_RoomM350HandSweepTrapCorners[];
extern u8 g_RoomM350HandSweepHit;
int Math_IntSqrt(int value);

#endif
