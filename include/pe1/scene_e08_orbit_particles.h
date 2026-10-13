#ifndef PE1_SCENE_E08_ORBIT_PARTICLES_H
#define PE1_SCENE_E08_ORBIT_PARTICLES_H

#include "common.h"

/* Thirty-two orbiting sprites: draw and update walk byte and halfword
 * arrays with separate strides. Radius and size shrink independently. */
typedef struct SceneE08OrbitParticles {
    u8 active[32];                     /* 0x000 */
    u8 frame[32];                      /* 0x020 */
    s16 angle[32];                     /* 0x040 */
    s16 size[32];                      /* 0x080 */
    s16 radius[32];                    /* 0x0C0 */
    u8 radialSpeed[32];                /* 0x100 */
    u8 angularSpeed[32];               /* 0x120 */
} SceneE08OrbitParticles;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08OrbitParticles, angle) == 0x40, scene_e08_orbit_angle_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08OrbitParticles, radialSpeed) == 0x100, scene_e08_orbit_radial_speed_offset);
PE1_STATIC_ASSERT(sizeof(SceneE08OrbitParticles) == 0x140, scene_e08_orbit_state_size);

#endif
