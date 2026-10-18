/*
 * Seeds of the homing projectile (homing_projectile.c): the bounce
 * spark's rotation and colour, the report fields and the ring's rotation and
 * colours. room_m141, room_m146, room_m153, room_m154, room_m328 and scene_e02
 * carry these 0x28 bytes in their data.
 */

#include "pe1/room_homing_model.h"

GteRotation g_RoomHomingSparkRotation = { 0x400, 0, 0, 1 };
RenderColor g_RoomHomingSparkColor = { 0x50, 0x20, 0, 0 };
s16 g_RoomHomingReportFields[5] = { 0, 1, 2, 5, 6 };
GteRotation g_RoomHomingRingRotation = { 0x400, 0, 0, 1 };
RenderColor g_RoomHomingRingInner = { 0x20, 0x10, 8, 0 };
RenderColor g_RoomHomingRingOuter = { 0, 0, 0, 0 };

RoomHomingLaunch g_RoomHomingLaunch;
u16 g_RoomHomingScript[12]={5,0,0,2,7,0,2,0,0,0,0,0};
FieldFxTaskProgram g_RoomHomingProgram={
    {{RoomEffect_HomingProjectile,NULL,NULL,NULL,NULL,NULL,NULL,RoomEffect_HomingProjectileThrow},
     {0,0,0,0,0,0,0,sizeof(RoomHomingModel)}},
    RoomEffect_HomingProjectileConfigure,g_RoomHomingScript};
PE1_STATIC_ASSERT(sizeof(void *)<=5*sizeof(((FieldFxTaskContext *)0)->variables[0]),homing_context_pointer_fits_unused_variables);
