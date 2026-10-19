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
        int index;
        u8 page;                  /* texture page the draws select */
    } asset;
} RoomOrbitSetState;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomOrbitSetState, asset) == sizeof(void *) + 0x20,
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
extern FieldFxHandler g_RoomOrbitSetDrawList[];
extern FieldFxHandler g_RoomOrbitSetUpdateList[];
extern FieldFxHandler g_RoomOrbitSetInitList[];
extern u16 g_RoomOrbitSetSpawnLayout[];

extern PmCommand g_RoomOrbitSetHandlers;
void RoomEffect_OrbitSetNop0(void);
void RoomEffect_OrbitSetPlantScript(PmSlotHeader *object);
int RoomEffect_OrbitSetSpawn6(PmSlotHeader *, int, int, intptr_t, intptr_t, intptr_t);
int RoomEffect_OrbitSetRegister(PmSlotHeader *object);
int RoomEffect_OrbitSetStart(PmSlotHeader *object);
int RoomEffect_OrbitSetClose(PmSlotHeader *object);
void RoomEffect_OrbitSetNop6(void);

int RoomEffect_OrbitSetInit(void *, void *, void *);
int RoomEffect_OrbitSetDraw(void *, void *, void *);
int RoomEffect_OrbitSetUpdate(void *, void *, void *);
int RoomEffect_InitOrbitParticles(void *, void *, void *);
int RoomEffect_DrawOrbitParticles(void *, void *, void *);
int RoomEffect_UpdateOrbitParticles(void *, void *, void *);
int RoomEffect_InitOrbitBurst(void *, void *, void *);
int RoomEffect_DrawOrbitBurst(void *, void *, void *);
int RoomEffect_UpdateOrbitBurst(void *, void *, void *);
int RoomEffect_InitOrbitSprite(void *, void *, void *);
int RoomEffect_DrawOrbitSprite(void *, void *, void *);
int RoomEffect_UpdateOrbitSprite(void *, void *, void *);

/* Current room floor height is declared by room_floor.h. */

void FieldFx_SetScriptContext(void *state);
int *FieldFx_RegPtr(int index);
int FieldFx_PlaySoundAtMatrixPosition(void *object, int message, void *state);

#endif
