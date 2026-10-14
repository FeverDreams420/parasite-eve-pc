#ifndef PE1_FLOOR_EDGE_POINT_H
#define PE1_FLOOR_EDGE_POINT_H

#include "common.h"

/* Endpoints of the edge the player last crossed onto a ramp, as screen
 * style XY pairs: the GTE normal clip reads each point as one packed word. */
typedef union FloorEdgePoint {
    struct {
        s16 x, z;
    } point;
    u32 packed;
} FloorEdgePoint;
extern FloorEdgePoint g_FloorBlockedEdges[2];
#define g_FloorBlockedEdgePointA (g_FloorBlockedEdges[0])
#define g_FloorBlockedEdgePrev (g_FloorBlockedEdges[1])
/* Byte view of the z halves, which Entity_ResetStateGlobals clears by
 * byte offset together with the x halves. */
#define g_FloorBlockedEdgeZHalves ((char *)g_FloorBlockedEdges + 2)

#endif /* PE1_FLOOR_EDGE_POINT_H */
