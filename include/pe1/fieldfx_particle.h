#ifndef PE1_FIELDFX_PARTICLE_H
#define PE1_FIELDFX_PARTICLE_H

#include "common.h"
#include "pe1/gte_types.h"

/* Moving glow particle shared by the four emitter setups and the bounce
 * updater. Positions and velocities wrap at sixteen bits; Y is tested as
 * signed for the bounce. Script records reserve 0x18 bytes per particle. */
typedef struct FieldFxParticlePosition {
    u16 x, y, z, reserved06;
} FieldFxParticlePosition;

typedef struct FieldFxMovingParticle {
    u8 reserved00;
    u8 age;
    u8 lifetime;
    u8 reserved03;
    u16 brightness;
    s16 scale;
    FieldFxParticlePosition position;
    GteShortVector velocity;
} FieldFxMovingParticle;

PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxMovingParticle, position) == 8,
                  fieldfx_moving_particle_position);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxMovingParticle, velocity) == 0x10,
                  fieldfx_moving_particle_velocity);
PE1_STATIC_ASSERT(sizeof(FieldFxMovingParticle) == 0x18,
                  fieldfx_moving_particle_size);

/* Two position/displacement slots updated for eight frames. */
typedef struct FieldFxTwoPointMotion {
    u8 reserved00[3];
    u8 age;
    u8 reserved04[12];
    FieldFxParticlePosition position[2];
    FieldFxParticlePosition velocity[2];
} FieldFxTwoPointMotion;
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxTwoPointMotion, position) == 0x10,
                  fieldfx_two_point_position);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxTwoPointMotion, velocity) == 0x20,
                  fieldfx_two_point_velocity);
PE1_STATIC_ASSERT(sizeof(FieldFxTwoPointMotion) == 0x30,
                  fieldfx_two_point_motion_size);

/* Spark setup, draw and fade callbacks share this 0x10-byte prefix.
 * Position and displacement arithmetic wraps at sixteen bits. */
typedef struct FieldFxSparkPoint {
    u8 reserved00;
    u8 brightness;
    u8 reserved02;
    u8 scaleStep;
    struct { u16 x, y, z; } position;
    struct { u16 x, y, z; } displacement;
} FieldFxSparkPoint;
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxSparkPoint, position) == 4,
                  fieldfx_spark_position);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxSparkPoint, displacement) == 0xA,
                  fieldfx_spark_displacement);
PE1_STATIC_ASSERT(sizeof(FieldFxSparkPoint) == 0x10,
                  fieldfx_spark_point_size);

/* Eight sparks stored by coordinate, followed by matching displacements. */
typedef struct FieldFxSparkCloud {
    FieldFxSparkPoint center;
    u16 position[3][8];
    u16 velocity[3][8];
} FieldFxSparkCloud;
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxSparkCloud, position) == 0x10,
                  fieldfx_spark_cloud_position);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxSparkCloud, velocity) == 0x40,
                  fieldfx_spark_cloud_velocity);
PE1_STATIC_ASSERT(sizeof(FieldFxSparkCloud) == 0x70,
                  fieldfx_spark_cloud_size);

/* Six-frame spark: randomized position, fixed brightness and age-driven draw. */
typedef struct FieldFxTimedSpark {
    u8 reserved00;
    u8 brightness;
    u8 reserved02;
    u8 age;
    u16 reserved04;
    struct { u16 x, y, z; } position;
} FieldFxTimedSpark;
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxTimedSpark, position) == 6,
                  fieldfx_timed_spark_position);
PE1_STATIC_ASSERT(sizeof(FieldFxTimedSpark) == 0xC,
                  fieldfx_timed_spark_size);

/* Single glow point used by transformed and fixed-position emitters. */
typedef struct FieldFxGlowPoint {
    u8 reserved00[4];
    u16 brightness;
    s16 scale;
    FieldFxParticlePosition position;
} FieldFxGlowPoint;
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxGlowPoint, position) == 8,
                  fieldfx_glow_point_position);
PE1_STATIC_ASSERT(sizeof(FieldFxGlowPoint) == 0x10,
                  fieldfx_glow_point_size);

/* Two independently transformed points rendered with the same glow scale. */
typedef struct FieldFxTwinGlow {
    u8 reserved00[4];
    u16 brightness;
    s16 scale;
    FieldFxParticlePosition position[2];
} FieldFxTwinGlow;
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxTwinGlow, position) == 8,
                  fieldfx_twin_glow_position);
PE1_STATIC_ASSERT(sizeof(FieldFxTwinGlow) == 0x18,
                  fieldfx_twin_glow_size);

/* Glow point with an emitter matrix copied at setup and rescaled at draw. */
typedef struct FieldFxMatrixGlow {
    u8 reserved00;
    u8 age;
    u8 reserved02[2];
    u16 brightness;
    s16 scale;
    FieldFxParticlePosition position;
    GteMatrixStorage transform;
} FieldFxMatrixGlow;
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxMatrixGlow, transform) == 0x10,
                  fieldfx_matrix_glow_transform);
PE1_STATIC_ASSERT(sizeof(FieldFxMatrixGlow) == 0x30,
                  fieldfx_matrix_glow_size);

extern FieldFxParticlePosition g_BurstEffectBTargetPosition, g_BurstEffectCTargetPosition;

extern FieldFxParticlePosition g_GlowEffectPosition, g_GlowEffectBPosition, g_GlowEffectCPosition, g_FourLayerGlowPosition;

#endif
