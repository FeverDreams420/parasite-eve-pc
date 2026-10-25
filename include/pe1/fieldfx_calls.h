#ifndef PE1_FIELDFX_CALLS_H
#define PE1_FIELDFX_CALLS_H

#include "pe1/field_collision.h"
#include "pe1/field_transform_point.h"

/* Field engine helpers called from the room and scene effects
 * (fieldfx/collision.c). The definitions take typed
 * records; each effect passes its own view, so these pointers are untyped. */

/* Sends asset message `message` at a halfword position while the object is
 * in status 3 or 4; returns the Asset_Find08w result, else 0. */
int FieldFx_FindAssetAtShortPosition(void *obj, int message, void *pos);
/* Nonzero when the player is within its hit radius plus radius of a
 * position on the XZ plane. */
int FieldFx_IsPlayerWithinRadius(void *position, int radius);
/* Status check on the object (FieldFx_GetStatus() == 3) that applies hit flags. */
int FieldFx_TriggerActorHit(void *obj);

void FieldFx_RandomScatterOffsetYZ(s16 *out);
void FieldFx_RandomScatterOffsetXY(s16 *out);

#endif
