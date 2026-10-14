#include "collision_math.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
int main(void) {
    const PeCollisionXZ triangle[3] = {{0,0},{10,0},{0,10}};
    CHECK(pe_collision_point_in_tri(triangle, 1, 1));
    CHECK(pe_collision_point_in_tri(triangle, 0, 0));
    CHECK(!pe_collision_point_in_tri(triangle, 10, 0));
    CHECK(!pe_collision_point_in_tri(triangle, 0, 10));
    CHECK(!pe_collision_point_in_tri(triangle, 5, 5));
    CHECK(!pe_collision_point_in_tri(triangle, -1, 1));
    const unsigned permutations[6][3] = {{0,1,2},{1,2,0},{2,0,1},{0,2,1},{2,1,0},{1,0,2}};
    for (unsigned p = 0; p < 6; ++p) {
        PeCollisionXZ reordered[3];
        for (unsigned i = 0; i < 3; ++i) reordered[i] = triangle[permutations[p][i]];
        for (int x = -1; x <= 11; ++x) for (int z = -1; z <= 11; ++z)
            CHECK(pe_collision_point_in_tri(reordered, x, z) == pe_collision_point_in_tri(triangle, x, z));
    }
    const PeCollisionXZ other[3] = {{10,0},{10,10},{0,10}};
    for (int x = 0; x < 10; ++x) for (int z = 0; z < 10; ++z)
        CHECK(pe_collision_point_in_tri(triangle, x, z) + pe_collision_point_in_tri(other, x, z) == 1);
    const PeCollisionXZ line[3] = {{0,0},{10,0},{20,0}};
    CHECK(!pe_collision_point_in_tri(line, 5, 0));
    const PeCollisionXZ extremes[3] = {{INT16_MIN,INT16_MIN},{INT16_MAX,INT16_MIN},{INT16_MIN,INT16_MAX}};
    CHECK(!pe_collision_point_in_tri(extremes, 0, 0));
    CHECK(pe_collision_point_in_tri(extremes, -1, -1));
    /* The original wrapped products exclude this otherwise-inside vertex. */
    CHECK(!pe_collision_point_in_tri(extremes, INT16_MIN, INT16_MIN));
    puts("Collision triangle: parity, winding, half-open shared edges and signed product wrap passed");
    return 0;
}
