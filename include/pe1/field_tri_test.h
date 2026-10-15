#ifndef PE1_FIELD_TRI_TEST_H
#define PE1_FIELD_TRI_TEST_H

#include "pe1/gte_types.h"

/* Scratchpad work area for the floor-triangle containment test: three edge
 * vectors, the point relative to each corner, and the OP cross products. */
typedef struct FieldTriScratch {
    GteVector edge[3];  /* 0x00 */
    GteVector rel[3];   /* 0x30 */
    GteVector cross[3]; /* 0x60 */
} FieldTriScratch;

#define FIELD_TRI_SCRATCH (g_FieldFxTriScratch)

extern FieldTriScratch *g_FieldFxTriScratch;

#include "pe1/player_entity.h"

int FieldFx_IsPointInQuadXZ(void *point, GteShortVector *quad);
int FieldFx_IsPointInTriangleXZ(GteShortVector *point, GteShortVector *tri);
int FieldFx_IsPlayerInTriangleXZ(GteShortVector *tri);
/* Nonzero when the player stands inside the XZ quad of four corners. */
int FieldFx_IsPlayerInQuadXZ(GteShortVector *quad);

#endif /* PE1_FIELD_TRI_TEST_H */
