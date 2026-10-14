#ifndef PE1_RENDER_SHADOW_H
#define PE1_RENDER_SHADOW_H

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/gte_types.h"
#include "pe1/render_object.h"
#include "pe1/render_tint.h"

/* Ground shadow quad drawn under a field actor (POLY_FT4 layout). */
typedef struct RenderShadowQuad {
    RenderGpuTag tag;
    u8 r0, g0, b0, code;
    s32 xy0; u8 u0, v0; u16 clut;
    s32 xy1; u8 u1, v1; u16 tpage;
    s32 xy2; u8 u2, v2; u16 pad2;
    s32 xy3; u8 u3, v3; u16 pad3;
} RenderShadowQuad;

/* 16.16 world coordinate; the shadow uses the integer half. */
typedef struct RenderShadowCoord {
    int fraction : 16;
    int integer : 16;
} RenderShadowCoord;

/* The shadow pass uses the same native actor as movement and battle. */
typedef FieldActor RenderShadowActor;
PE1_STATIC_ASSERT(sizeof(RenderShadowQuad) == 40, render_shadow_quad_size);
PE1_STATIC_ASSERT(sizeof(((FieldActor *)0)->shadow_quads) ==
                  sizeof(((FieldActor *)0)->allocation_block), shadow_allocation_pointer_size);

int Render_DrawActorShadow(RenderShadowActor *actor);

#endif
