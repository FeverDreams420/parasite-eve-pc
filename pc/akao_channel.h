#ifndef PE_PC_AKAO_CHANNEL_H
#define PE_PC_AKAO_CHANNEL_H

#include "akao_sequence.h"
#include "spu_bank.h"

typedef struct PeAkaoControl {
    uint32_t tempo, tempo_delta, volume, volume_delta, accumulator;
    uint16_t tempo_duration, volume_duration, condition;
    uint16_t ticks_per_beat, beats_per_measure, tick, beat, measure;
    uint8_t volume_dirty;
    uint32_t status_flags, allocated_voice_mask, spu_active_mask;
    uint32_t reverb_mask, reserved_voice_mask;
    uint8_t voice_modes_dirty;
    uint64_t musical_ticks;
} PeAkaoControl;
/* Called around voice processing, in the original order: accumulate first,
 * then advance slides/meter after the due musical tick's channel commands. */
int pe_akao_control_due(PeAkaoControl *control, uint32_t scale, int forced);
void pe_akao_control_step(PeAkaoControl *control);

typedef struct PeAkaoChannel {
    PeAkaoSequence sequence; /* borrowed bytes must outlive this channel */
    PeAkaoControl *control; /* shared bank state; never a per-channel copy */
    const PeAkaoGrammar *grammar; /* decoded original skip/duration tables */
    const PeSpuBank *samples;
    uint32_t voice_mask, pitch_base, sample_start, sample_loop;
    uint16_t instrument, voice_index;
    uint8_t adsr[8];
    uint32_t pc, flags, expression_value, expression_delta, update_flags;
    uint16_t volume_base, panpot, note_length, default_note_length, fixed_note_length;
    uint16_t pan_duration, pan_slide_duration, expression_duration, drum_volume;
    uint16_t volume_duration, volume_delta, pan_delta; /* signed deltas stored as bits */
    uint8_t octave, stopped, last_note;
    uint8_t loop_index, key_off_requested;
    uint8_t tremolo_phase;
    uint16_t tremolo_duration;
    uint16_t repeats[4];
    uint32_t loop_points[4];
    uint64_t commands;
    uint64_t musical_ticks;
    char error[192];
} PeAkaoChannel;

typedef enum PeAkaoChannelResult {
    PE_AKAO_CHANNEL_ERROR = -1,
    PE_AKAO_CHANNEL_NOTE = 0,
    PE_AKAO_CHANNEL_STOP = 1,
    PE_AKAO_CHANNEL_WAIT = 2
} PeAkaoChannelResult;

/* Initialize the command-state subset of a primary track. Octave is retained
 * across starts as in the original voice initializer; start with zero state.
 * This does not initialize SPU voice allocation or synthesize sound. */
int pe_akao_channel_start(PeAkaoChannel *channel, const PeAkaoSequence *sequence, unsigned voice);
/* Execute supported commands up to a raw note/tie/rest byte or stop.
 * Note pitch, remaining flow/LFOs, SPU registers and mixing are pending. */
PeAkaoChannelResult pe_akao_channel_next(PeAkaoChannel *channel, unsigned limit);
/* Look ahead without executing channel commands or changing the real PC,
 * loop stack/counters, tempo or volume. Original tremolo clears are retained. */
int pe_akao_channel_peek(PeAkaoChannel *channel, unsigned limit, uint8_t *period);
/* Apply length/gate semantics after next() returns a raw note byte. */
int pe_akao_channel_begin_note(PeAkaoChannel *channel, unsigned lookahead_limit);
void pe_akao_channel_slides(PeAkaoChannel *channel);
int pe_akao_channel_bind_samples(PeAkaoChannel *channel, const PeSpuBank *samples);
int pe_akao_channel_set_instrument(PeAkaoChannel *channel, unsigned encoded);
/* One due musical tick, not a host frame. SPU application acknowledges the
 * latched key_off_requested flag; pitch/voice-register updates are pending. */
PeAkaoChannelResult pe_akao_channel_tick(PeAkaoChannel *channel, unsigned instruction_limit);

#endif
