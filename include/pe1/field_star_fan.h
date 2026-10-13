#ifndef PE1_FIELD_STAR_FAN_H
#define PE1_FIELD_STAR_FAN_H

#include "common.h"
#include "pe1/field_rotated_triangle.h"

/* Field engine star fan (FieldFx_DrawStarFan): a ring of gouraud triangles from
 * a common centre whose rim alternates between two radii, scaled, turned
 * and placed at a projected anchor. */

/* Unit scale used before the caller's X/Y scale is filled in. */
extern GteVector g_FieldFxUnitScale;

#endif /* PE1_FIELD_STAR_FAN_H */
