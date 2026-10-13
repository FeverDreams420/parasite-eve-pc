#ifndef PE1_FIELD_FADE_PARTICLE_H
#define PE1_FIELD_FADE_PARTICLE_H

#include "common.h"
#include "pe1/field_effect_pool.h"
#include "pe1/gte_types.h"
#include "pe1/player_entity.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"

/* Field engine fade particle (FieldFx_FadeParticle), spawned by the fading
 * emitter FieldFx_FadeEmitter: a glow pinned to one actor matrix point that
 * sheds drifting and falling sparks into the same pool. */

int rand(void);

#endif /* PE1_FIELD_FADE_PARTICLE_H */
