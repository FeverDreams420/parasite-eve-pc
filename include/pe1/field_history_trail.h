#ifndef PE1_FIELD_HISTORY_TRAIL_H
#define PE1_FIELD_HISTORY_TRAIL_H

#include "common.h"
#include "pe1/field_tile.h"
#include "pe1/gte_types.h"
#include "pe1/render_object.h"
#include "pe1/render_prim.h"
#include "pe1/render_tint.h"

/* Field engine history trail (FieldFx_DrawHistoryTrail): a ribbon of gouraud quads
 * through the last `count` head/tail point pairs, fading out towards the
 * oldest pair. The `pad` halves of both points flag a recorded pair. */

typedef struct FieldTrailPair {
    /* 0x00 */ RenderHistoryPoint head;
    /* 0x08 */ RenderHistoryPoint tail;
} FieldTrailPair;

PE1_STATIC_ASSERT(sizeof(FieldTrailPair) == 0x10, field_trail_pair_size);

#endif /* PE1_FIELD_HISTORY_TRAIL_H */
