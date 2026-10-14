#include "collision_math.h"

static int64_t signed_word(uint32_t word) {
    return word & UINT32_C(0x80000000) ? (int64_t)word - INT64_C(0x100000000) : word;
}

int pe_collision_point_in_tri(const PeCollisionXZ vertices[3], int16_t x, int16_t z) {
    PeCollisionXZ next = vertices[2];
    unsigned inside = 0;
    for (unsigned edge = 0; edge < 3; ++edge) {
        PeCollisionXZ previous = next; next = vertices[edge];
        if ((z >= next.z && z < previous.z) || (z < next.z && z >= previous.z)) {
            if (x < next.x && x < previous.x) inside ^= 1;
            else if (x < next.x || x < previous.x) {
                int delta_z = (int)previous.z - next.z;
                uint32_t side = (uint32_t)((int)previous.x - next.x) * (uint32_t)((int)z - next.z);
                uint32_t cross = (uint32_t)delta_z * (uint32_t)((int)x - next.x);
                if (delta_z < 0 ? signed_word(side) < signed_word(cross) :
                                  signed_word(cross) < signed_word(side)) inside ^= 1;
            }
        }
    }
    return (int)inside;
}
