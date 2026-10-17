#ifndef PE1_FIELD_ORBIT_POINT_H
#define PE1_FIELD_ORBIT_POINT_H

#include "pe1/gte_types.h"

/* Identity rotation with zero translation used as the roll matrix seed. */
extern GteMatrix g_FieldFxIdentityMatrix;

GteMatrix *MulRotMatrix(GteMatrix *matrix);
void FieldFx_OffsetPointByAngles(GteShortVector *origin, GteShortVector *out, int radius,
                               GteShortVector *angles, int arm, int roll);

#endif /* PE1_FIELD_ORBIT_POINT_H */
