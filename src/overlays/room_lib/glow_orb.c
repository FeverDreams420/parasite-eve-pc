/*
 * The glow orb: a module whose spawner drops a glow at the owner, which
 * drifts until it leaves the walkable area, leaves a fading flash every
 * third frame and is picked up when the player touches it.
 *
 * room_m034, room_m174 and room_m383 link the same fourteen functions in
 * this order, from the class's two no-ops after RoomLib_CloseTarget to the
 * flash update, with the orb's spin and the flash scale as their read-only
 * data at 0xC, which splat had merged into the room header's rodata. The
 * module's lists, script, matrices and sprite blocks are each room's own
 * data (pe1/room_glow_orb.h, pe1/room_module.h).
 */

#include "pe1/field_collision.h"
#include "pe1/gte.h"
#include "pe1/room_glow_orb.h"
#include <string.h>
#include "pe1/player_entity.h"

static const GteShortVector s_GlowOrbSpin = { 0xC00, 0, 0, 0 };
static const GteVector s_GlowOrbFlashScale = { 0x448, 0x448, 0x448, 0 };

/* Class slots 6 and 0. */
void RoomEffect_GlowOrbNop6(void) {
}

void RoomEffect_GlowOrbNop0(void) {
}

/* Controller init: captures the owner's placement matrix, and the matrix
 * the orb starts from (the player's own when the owner is the player). */
int RoomEffect_GlowOrbCaptureOwner(void *data, void *slot, void *state) {
    RoomGlowOrbObject *object = data;
    g_RoomGlowOrbOwner = object->prefix.actor;
    memcpy(&g_RoomGlowOrbAxes, &g_RoomGlowOrbOwner->render_object.matrices[0], sizeof(g_RoomGlowOrbAxes));
    /* The player overrides matrix49 in retail; avoid its unobserved read
     * beyond the player's native model matrix allocation. */
    memcpy(&g_RoomGlowOrbOrigin,
           &g_RoomGlowOrbOwner->render_object.matrices[g_RoomGlowOrbOwner == g_PlayerEntity ? 0 : 0x31],
           sizeof(g_RoomGlowOrbOrigin));
    return 0;
}

int RoomEffect_GlowOrbControllerDraw(void *object, void *slot, void *data) {
    return 0;
}

int RoomEffect_GlowOrbControllerUpdate(void *object, void *slot, void *data) {
    RoomFallingParticleControl *control = slot;
    control->state = 2;
    return 0;
}

int RoomEffect_GlowOrbSpawnerInit(void *object, void *control, void *data) {
    RoomGlowOrbSpawner *spawner = data;
    spawner->pad00 = 0;
    spawner->yaw = *FieldFx_RegPtr(4);
    return 0;
}

int RoomEffect_GlowOrbSpawnerDraw(void *object, void *slot, void *data) {
    return 0;
}

/* Restores the recorded yaw and drops the orb. */
int RoomEffect_GlowOrbSpawnerUpdate(void *object, void *slot, void *data) {
    RoomFallingParticleControl *control = slot;
    RoomGlowOrbSpawner *spawner = data;
    int *yaw;

    yaw = FieldFx_RegPtr(4);
    *yaw = spawner->yaw;
    FieldFx_StartSlot(object, 2, (unsigned short *)g_RoomSpawnLayout, g_RoomInitList);
    control->state = 2;
    return 0;
}

int RoomEffect_GlowOrbInit(void *object, void *slot, void *data) {
    RoomGlowOrb *orb = data;
    GteShortVector tilt = {0};
    GteShortVector spin = s_GlowOrbSpin;
    GteMatrix local = {0};
    GteShortVector turn = {0};
    GteMatrix base = {0};
    int *yaw;

    turn.x = 0;
    yaw = FieldFx_RegPtr(4);
    turn.y = (s16)((u32)*yaw + (u32)*FieldFx_VarPtr(2));
    turn.z = 0;
    RotMatrixYXZ(&turn, &base);
    tilt.x = 0;
    tilt.y = 0;
    tilt.z = (s16)(0u - (u32)*FieldFx_VarPtr(0));
    ApplyMatrixSV(&g_RoomGlowOrbAxes, &tilt, &orb->offset);
    ApplyMatrixSV(&base, &orb->offset, &orb->offset);
    orb->matrix = g_RoomGlowOrbAxes;
    orb->x = g_RoomGlowOrbOrigin.t[0];
    orb->y = g_RoomFloorY->y - 0x100;
    orb->z = g_RoomGlowOrbOrigin.t[2];
    orb->size = 0x80;
    orb->h10 = 0;
    orb->h12 = 0;
    orb->h14 = 0;
    orb->depth = 0;
    orb->flag2 = 0;
    orb->state = 1;
    orb->slot = *FieldFx_RegPtr(3);
    RotMatrixYXZ(&spin, &local);
    gte_CompMatrix(&orb->matrix, &local, &orb->matrix);
    gte_CompMatrix(&base, &orb->matrix, &orb->matrix);
    return 0;
}

/* Draw the glow orb: two scaled halo layers at the orb, a touch test
 * against the player that flags the pickup, then three trailing sparks
 * stepping back along the orb's offset. The GNU constructor assignments
 * clear a temporary with memset and copy it, and the second scale lives in
 * a nested block so it takes the first temporary's freed stack slot. */
int RoomEffect_GlowOrbDraw(void *data, void *control, void *state) {
    RoomGlowOrbObject *object = data;
    u8 *slot = control;
    RoomGlowOrb *orb = state;
    GteMatrix matrix;
    GteShortVector point = {0};

    FieldFx_SetSpriteTexturePreset(0);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteCellSize(0x40, 0x20);
    FieldFx_SetSpriteBlendMode(2);
    if (orb->state == 1) {
        GteVector scale;

        g_RoomGlowOrbSprite.r = 0x10;
        g_RoomGlowOrbSprite.g = 0x10;
        g_RoomGlowOrbSprite.b = 0x20;
        g_RoomGlowOrbSprite.depth = orb->depth;
        matrix = orb->matrix;
        /* Retail subtracts offset.x on all three axes for the inner halo. */
        matrix.t[0] = orb->x - orb->offset.x;
        matrix.t[1] = orb->y - orb->offset.x;
        matrix.t[2] = orb->z - orb->offset.x;
        scale = (GteVector){ orb->size >> 1, 0x251, 0x251 };
        ScaleMatrix(&matrix, &scale);
        FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomGlowOrbSprite, &matrix, 0);

        g_RoomGlowOrbSprite.r = 0x20;
        g_RoomGlowOrbSprite.g = 0x20;
        g_RoomGlowOrbSprite.b = 0x40;
        matrix = orb->matrix;
        matrix.t[0] = orb->x;
        matrix.t[1] = orb->y;
        matrix.t[2] = orb->z;
        {
            GteVector size;

            size = (GteVector){ orb->size, 0x448, 0x448 };
            ScaleMatrix(&matrix, &size);
            FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomGlowOrbSprite, &matrix, 0);
        }

        point.x = (s16)(g_PlayerEntity->pos_x >> 16);
        point.y = (s16)(g_PlayerEntity->pos_y >> 16);
        point.z = (s16)(g_PlayerEntity->pos_z >> 16);
        if (FieldFx_IsPointInSpriteQuad(&point, &matrix) != 0 && FieldFx_GetStatus(object) == 3) {
            g_PlayerEntity->state->flags |= 0x4000;
            if (object->prefix.actor != 0) {
                object->prefix.actor->state->core_flags |= 0x80000000;
            }
            slot[1] = 2;
        }

        g_RoomGlowOrbSprite.r = 0x78;
        g_RoomGlowOrbSprite.g = 0xF0;
        g_RoomGlowOrbSprite.b = 0x78;
        matrix.t[1] = orb->y - 0x100;
        FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomGlowOrbSprite, &matrix, 0);

        matrix.t[0] = orb->x - orb->offset.x;
        matrix.t[1] = orb->y - orb->offset.y;
        matrix.t[2] = orb->z - orb->offset.z;
        g_RoomGlowOrbSprite.r = 0x3C;
        g_RoomGlowOrbSprite.g = 0x78;
        g_RoomGlowOrbSprite.b = 0x3C;
        matrix.t[1] = orb->y - 0x100;
        FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomGlowOrbSprite, &matrix, 0);

        matrix.t[0] = orb->x - orb->offset.x * 2;
        matrix.t[1] = orb->y - orb->offset.y * 2;
        matrix.t[2] = orb->z - orb->offset.z * 2;
        g_RoomGlowOrbSprite.r = 0x1E;
        g_RoomGlowOrbSprite.g = 0x3C;
        g_RoomGlowOrbSprite.b = 0x1E;
        matrix.t[1] = orb->y - 0x100;
        FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomGlowOrbSprite, &matrix, 0);
    }
    return 0;
}

/* Drifts the orb by its offset and brightens it; every third frame over the
 * walkable area it leaves a flash on the floor, and it ends once it leaves
 * the area. */
int RoomEffect_GlowOrbUpdate(void *owner, void *slot, void *data) {
    RoomFallingParticleControl *control = slot;
    RoomFallingParticleState *state = data;
    u16 *spawn;

    state->x += state->velocityX;
    state->y += state->velocityY;
    state->z += state->velocityZ;
    state->phase += 30;
    if (state->intensity < 129) {
        state->intensity += 10;
    }

    state->frame++;
    if (Geo_PointInPoly((s32)((u32)(s16)state->x << 16), (s32)((u32)(s16)state->z << 16), g_FloorPolygonVertices, g_FloorPolygonVertexCount)) {
        if ((s8)state->frame % 3 == 0) {
            spawn = FieldFx_StartSlot(owner, 3, (unsigned short *)g_RoomSpawnLayout, g_RoomInitList);
            if (spawn != 0) {
                spawn[4] = state->x;
                spawn[5] = g_RoomFloorY->y;
                spawn[6] = state->z;
            }
        }
    }

    if (!Geo_PointInPoly((s32)((u32)(s16)state->x << 16), (s32)((u32)(s16)state->z << 16), g_FloorPolygonVertices, g_FloorPolygonVertexCount)) {
        control->state = 2;
        state->active = 0;
    }
    return 0;
}

int RoomEffect_GlowOrbFlashInit(void *object, void *control, void *data) {
    RoomGlowOrbFlash *flash = data;
    flash->timer = 0;
    flash->depth = 0x80;
    return 0;
}

int RoomEffect_GlowOrbFlashDraw(void *object, void *control, void *data) {
    RoomGlowOrbFlash *flash = data;
    GteMatrix matrix = {0};
    GteVector scale;
    s16 *depth;

    FieldFx_SetSpriteTexturePreset(0);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteBlendMode(2);

    matrix.m[2][2] = 0x1000;
    matrix.m[1][1] = 0x1000;
    matrix.m[0][0] = 0x1000;
    matrix.t[2] = 0;
    matrix.t[1] = 0;
    matrix.t[0] = 0;
    matrix.m[2][1] = 0;
    matrix.m[2][0] = 0;
    matrix.m[1][2] = 0;
    matrix.m[1][0] = 0;
    matrix.m[0][2] = 0;
    matrix.m[0][1] = 0;

    depth = &g_RoomGlowOrbFlashSprite.depth;
    *depth = flash->depth;
    matrix.t[0] = flash->x;
    matrix.t[1] = flash->y;
    matrix.t[2] = flash->z;
    scale = s_GlowOrbFlashScale;
    ScaleMatrix(&matrix, &scale);
    FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomGlowOrbFlashSprite, &matrix, 1);
    return 0;
}

int RoomEffect_GlowOrbFlashUpdate(void *object, void *slot, void *data) {
    RoomFallingParticleControl *control = slot;
    RoomGlowOrbFlash *flash = data;
    flash->timer++;

    if (flash->timer >= 0x15) {
        control->state = 2;
    }

    if (flash->depth >= 9) {
        flash->depth -= 8;
    }
    return 0;
}
