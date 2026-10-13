#ifndef PE1_FIELD_TRANSFORM_POINT_H
#define PE1_FIELD_TRANSFORM_POINT_H

#include "common.h"
#include "pe1/gte_short_vector.h"
#include "pe1/gte_types.h"

/* Field engine helpers that place a point through one of an owner's
 * transform records (RoomFxTransformOwner in pe1/room_fx.h). */
struct RoomFxTransformOwner;

void FieldFx_GetTransformLookAngles(struct RoomFxTransformOwner *owner, int index,
                   GteShortVector *out);
void FieldFx_TransformMatrixPoint(struct RoomFxTransformOwner *owner, int index,
                                  const GteShortVector *input,
                                  GteShortVector *output);

void FieldFx_RotateVector(const GteMatrixWords *matrix,
                           const GteShortVector *input, GteShortVector *output);
void FieldFx_CalculateLookAngles(GteShortVector *from, GteShortVector *to,
                                 GteShortVector *out);

#endif /* PE1_FIELD_TRANSFORM_POINT_H */
