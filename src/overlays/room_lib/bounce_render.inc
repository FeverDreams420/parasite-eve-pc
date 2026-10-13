/*
 * Bounce particle: state 0 bounces along the floor and draws a fading
 * sprite, state 1 rises to the floor height and draws a coloured tile.
 * Instantiated by the line burst unit (line_burst.c) and by the
 * burst orb rooms, which differ only in gravity, CLUT column and frame base.
 *
 * The includer names the function (ROOMLIB_UPDATE_BOUNCE_RENDER_NAME) and
 * the tile colour constant (ROOMLIB_BOUNCE_RENDER_BLOB), which it defines
 * before the include.
 */
#include "pe1/room_line_burst.h"
#include "pe1/field_tile.h"
#include "pe1/gte.h"

#ifndef ROOMLIB_BOUNCE_GRAVITY
#define ROOMLIB_BOUNCE_GRAVITY 1
#endif
#ifndef ROOMLIB_BOUNCE_CLUT_X
#define ROOMLIB_BOUNCE_CLUT_X 0x10
#endif
#ifndef ROOMLIB_BOUNCE_FRAME_BASE
#define ROOMLIB_BOUNCE_FRAME_BASE 0xC8
#endif

int ROOMLIB_UPDATE_BOUNCE_RENDER_NAME(int mode, RoomLineBurstParticle *fx) {
    RoomLineBurstColor blob;
    int saved0;
    int saved2;

    blob = ROOMLIB_BOUNCE_RENDER_BLOB;
    switch (mode) {
    case 1:

    if (fx->state != 0) {
        if (fx->state != 1) {
            return 0;
        }
    } else
    {
        int vx;
        int vz;
        int rawVy;
        int nextVy;
        register int y;

        fx->timer++;
        fx->x += fx->vx;
        fx->y += fx->vy;
        fx->z += fx->vz;
        vx = fx->vx * 511;
        if (vx < 0) {
            vx += 511;
        }
        fx->vx = vx >> 9;
        vz = fx->vz * 511;
        if (vz < 0) {
            vz += 511;
        }
        rawVy = (unsigned short)fx->vy;
        fx->vz = vz >> 9;
        y = fx->y;
        nextVy = rawVy - ROOMLIB_BOUNCE_GRAVITY;
        fx->vy = nextVy;
        if (y >= g_RoomFloorY->y) {
            int bounce;

            bounce = (nextVy << 16) >> 16;
            bounce = -bounce;
            fx->vy = bounce;
        }
        if (fx->timer < 24) {
            goto ret0;
        }
        return 1;
    }
    {
        fx->timer++;
        fx->y += fx->vy;
        fx->vy += fx->ay;
        if (fx->y >= g_RoomFloorY->y) {
            fx->y = g_RoomFloorY->y;
        }
        if (fx->timer < 32) {
            goto ret0;
        }
        return 1;
    }

    case 2:
    if (fx->state != 0) {
        if (fx->state != 1) {
            return 0;
        }
    } else
    {
        int sinValue;
        int scale;
        /* Matching debt: the angle still needs retail's argument register. */
        register int angle asm("$4");
        int cosine;
        int texture;

        sinValue = rsin((fx->timer << 10) / 24);
        angle = (fx->timer * 1204) / 24;
        PE1_COMPILER_USE(angle);
        scale = ((unsigned int)sinValue >> 31) + sinValue;
        scale >>= 1;
        saved2 = scale + 0x1000;
        cosine = rcos(angle);
        if (cosine < 0) {
            cosine += 0x7F;
        }
        saved0 = (cosine >> 7) + 0x28;
        texture = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
        if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
            texture += 4;
        }
        texture = GetClut(ROOMLIB_BOUNCE_CLUT_X, texture);
        FieldFx_DrawShapeQuads((GteShortVector *)fx, 0, saved2, saved2,
                      (s16)g_FieldFxSpriteParams.parameter02 * (fx->timer / 6) + ROOMLIB_BOUNCE_FRAME_BASE,
                      (unsigned short)texture, 1, saved0, 0);
        return 0;
    }
    {
        int angle;
        RenderMatrixSlot *matrixSlot;

        angle = rcos(fx->timer << 5);
        if (angle < 0) {
            angle += 0x1F;
        }
        saved0 = angle >> 5;
        matrixSlot = &g_ViewMatrixSlot;
        gte_ldrotmatrix(matrixSlot->value);
        gte_ldtransmatrix(matrixSlot->value);
        FieldFx_DrawPointTile((GteShortVector *)fx, (u8 *)&blob, saved0, 1);
    }
    default:
        break;
    }
ret0:
    return 0;
}
