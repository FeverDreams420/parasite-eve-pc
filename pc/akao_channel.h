#ifndef PE_PC_AKAO_CHANNEL_H
#define PE_PC_AKAO_CHANNEL_H

#include "akao_sequence.h"

typedef struct PeAkaoChannel {
    PeAkaoSequence sequence;
    uint32_t pc, flags, expression_value;
    uint16_t volume_base, panpot, note_length, default_note_length, fixed_note_length;
    uint16_t pan_duration, pan_slide_duration, expression_duration, drum_volume;
    uint8_t octave, stopped, last_note;
    uint8_t loop_index, key_off_requested;
    uint16_t repeats[4];
    uint32_t loop_points[4];
    uint64_t commands;
    char error[192];
} PeAkaoChannel;

typedef enum PeAkaoChannelResult {
    PE_AKAO_CHANNEL_ERROR = -1,
    PE_AKAO_CHANNEL_NOTE = 0,
    PE_AKAO_CHANNEL_STOP = 1
} PeAkaoChannelResult;

/* Initialize the command-state subset of a primary track. Octave is retained
 * across starts as in the original voice initializer; start with zero state.
 * This does not initialize SPU voice allocation or synthesize sound. */
int pe_akao_channel_start(PeAkaoChannel *channel, const PeAkaoSequence *sequence, unsigned voice);
/* Execute supported commands up to a raw note/tie/rest byte or stop.
 * Note timing/pitch, instrument changes, remaining flow/LFOs and mixing are pending. */
PeAkaoChannelResult pe_akao_channel_next(PeAkaoChannel *channel, unsigned limit);

#endif
