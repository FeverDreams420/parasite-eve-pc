#ifndef PE1_ROOM_ORBIT_SET_H
#define PE1_ROOM_ORBIT_SET_H

#include "common.h"
#include "pe1/field_glow_sprite.h"
#include "pe1/field_script_context.h"
#include "pe1/field_sprite_state.h"
#include "pe1/gte_types.h"
#include "pe1/pe_image.h"
#include "pe1/psyq_libc.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"
#include "pe1/room_module.h"

/* The orbit effect set linked by room_m186, m187, m385, m388 and m389
 * (src/overlays/room_lib/orbit_set.c): eight orbiting
 * particles, a burst of eight falling sprites with floor shadows and one
 * drifting sprite, sharing four sprite records the set's init fills. */

/* Owner matrix copied by the module init and passed to its spawn handler. */
typedef struct RoomOrbitSetState {
    struct FieldActor *owner;
    RenderMatrix matrix;
    union {
        void *allocation;
        u8 page;                  /* texture page the draws select */
    } asset;
} RoomOrbitSetState;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomOrbitSetState, asset) == 0x24,
                  room_orbit_set_allocation_offset);

/* Per-room data: the four sprite records, at different addresses and with
 * different gaps in each room. */
extern RoomFxSpritePacket g_RoomOrbitSpritePacket;
extern RoomFxSpritePacket g_RoomOrbitParticlePacket;
extern RoomFxSpritePacket g_RoomOrbitBurstPacket;
extern RoomFxSpritePacket g_RoomOrbitShadowPacket;

/* The set's module class (orbit_set.c), in class-table slot
 * order, and the script and lists it hands the field engine, named in each
 * room's symbol file. */
extern int g_RoomOrbitSetScript[];
extern RoomModuleHandler g_RoomOrbitSetDrawList[];
extern RoomModuleHandler g_RoomOrbitSetUpdateList[];
extern RoomModuleHandler g_RoomOrbitSetInitList[];
extern int g_RoomOrbitSetSpawnLayout[];

int RoomEffect_OrbitSetNop0(void);
int RoomEffect_OrbitSetPlantScript(void *object);
int RoomEffect_OrbitSetSpawn6(intptr_t a, int b, int c, intptr_t d, int e, int f);
int RoomEffect_OrbitSetRegister(void *o);
int RoomEffect_OrbitSetStart(void *o);
int RoomEffect_OrbitSetClose(struct RoomEnt *o);
int RoomEffect_OrbitSetNop6(void);

/* The frame counter at 0x800942EC, read as the floor height. */

void FieldFx_SetScriptContext(void *state);
int *FieldFx_RegPtr(int index);
int FieldFx_PlaySoundAtMatrixPosition(void *object, int message, void *state);

#endif
