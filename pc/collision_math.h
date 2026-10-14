#ifndef PE_PC_COLLISION_MATH_H
#define PE_PC_COLLISION_MATH_H
#include <stdint.h>
typedef struct PeCollisionXZ { int16_t x, z; } PeCollisionXZ;
/* Original Geo_PointInTri parity and half-open edges, including wrapped
 * signed 32-bit products. The caller decodes flat/sloped vertex records. */
int pe_collision_point_in_tri(const PeCollisionXZ vertices[3], int16_t x, int16_t z);
#endif
