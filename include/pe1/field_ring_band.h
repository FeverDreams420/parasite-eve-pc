#ifndef PE1_FIELD_RING_BAND_H
#define PE1_FIELD_RING_BAND_H

#include "common.h"
#include "pe1/field_star_fan.h"
#include "pe1/field_tile.h"

/* Field engine ring band (FieldFx_DrawRingBand): an annulus of gouraud quads
 * between two radii, scaled, turned and placed at a projected anchor. */

/* TILE_OT_ADDPRIM through an ordering-table entry held as a plain tag
 * pointer: retail keeps that pointer in one register for every link. */
#define RING_OT_ADDPRIM(entry, packet, link) \
    ((packet)->tag.address = (entry)->address, \
     (link).tag = &(packet)->tag, \
     (entry)->address = (link).word)

#endif /* PE1_FIELD_RING_BAND_H */
