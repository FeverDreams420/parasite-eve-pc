#ifndef PE1_PROJECTION_SLOT_H
#define PE1_PROJECTION_SLOT_H

#include "common.h"

/* g_GeomScroll.state.position.projectionDistance as a one-field record:
 * it points at the projection distance (GTE H) of the current view.
 * fx_common points it at its own scratch while it projects; its two units
 * take cc1 -G8, so cc1 sees this small word as a cheap address and loads
 * and stores it absolutely, as retail does, instead of keeping its address
 * in a register. */
typedef struct FieldProjectionSlot {
    s32 *distance;
} FieldProjectionSlot;

/* Include geom_state.h at access sites, after renderer types are complete. */
#define g_ProjectionSlot (*(FieldProjectionSlot *)(void *)&g_GeomScroll.state.position.projectionDistance)

#endif /* PE1_PROJECTION_SLOT_H */
