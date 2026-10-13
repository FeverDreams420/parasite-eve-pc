#ifndef PE1_FLOOR_POLYGON_H
#define PE1_FLOOR_POLYGON_H

#include "common.h"

/* Little-endian 16.16 X/Z pair. The containment test reads only the
 * signed integer halves, at offsets 2 and 6 of each eight-byte vertex. */
typedef struct {
    unsigned short xFraction;
    short x;
    unsigned short zFraction;
    short z;
} PolygonVertex;

/* The current room's walkable floor polygon and its vertex count, set by
 * scene command 0x2E. Room effects that read the packed y/z word view the
 * vertices through FLOOR_POLYGON_AS. */
extern PolygonVertex *g_FloorPolygonVertices;
extern u16 g_FloorPolygonVertexCount;
#define FLOOR_POLYGON_AS(type) ((type *)g_FloorPolygonVertices)

#endif
