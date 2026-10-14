#ifndef PE_PC_ANIMATION_H
#define PE_PC_ANIMATION_H
#include <stddef.h>
#include <stdint.h>
#include "matrix.h"
#define PE_ANIMATION_BONES 256u
#define PE_ANIMATION_CHANNELS (3u+PE_ANIMATION_BONES*3u)
typedef struct PeAnimation {
    const uint8_t *bytes;size_t size;
    uint8_t encoding,frames;
    unsigned bones;
    uint16_t body[4];
    uint32_t samples[PE_ANIMATION_CHANNELS];
    uint8_t constant[PE_ANIMATION_CHANNELS];
} PeAnimation;
typedef struct PeAnimationFrame {
    int32_t translation[3];uint16_t angles[PE_ANIMATION_BONES][3],body[4];unsigned bones;
} PeAnimationFrame;
/* Borrowed byte/halfword keyframe channels. Failure preserves output.
 * Frame-zero decoding remains possible for a zero frame-count header, as
 * its original channel stride still stores one sample. Timing is separate. */
int pe_animation_open(const uint8_t *bytes,size_t size,PeAnimation *animation,char *error,size_t error_size);
int pe_animation_frame(const PeAnimation *animation,unsigned frame,PeAnimationFrame *output);
typedef struct PeRotationOverride { unsigned matrix_index;uint8_t flags;uint16_t angles[3]; } PeRotationOverride;
/* Two ordered overrides; index zero is ignored as in the original.
 * Preserves non-root translations and all output on failure. */
int pe_animation_matrices(const PeAnimationFrame *frame,const PeRotationOverride overrides[2],
                          PeMatrix *matrices,unsigned capacity);
typedef struct PeAnimationClock { uint32_t frame,previous;uint16_t last,stop; } PeAnimationClock;
/* Original local 16.16 advance. Parent/action synchronization and voice-bank
 * updates are handled by the actor layer; inherited mode fails explicitly. */
int pe_animation_advance(PeAnimationClock *clock,uint32_t step,uint32_t *flags);
#endif
