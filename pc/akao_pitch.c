#include "akao_channel.h"

static int signed16(uint16_t value) {
    return value & 32768 ? (int)value - 65536 : value;
}

int pe_akao_pitch_period(const PeSpuBank *samples, unsigned instrument, uint8_t note,
                          uint16_t detune_bits, uint16_t *pitch) {
    if (!samples || instrument >= 256 || !samples->valid[instrument]) return 0;
    unsigned octave = note / 12;
    uint32_t period = samples->instruments[instrument].pitch[note % 12];
    if (detune_bits) {
        uint32_t product = period * (uint32_t)signed16(detune_bits);
        period += product >> 7; /* Original unsigned shift of wrapped product. */
    }
    if (octave >= 7) period <<= octave - 6;
    else if (octave < 6) period >>= 6 - octave;
    *pitch = (uint16_t)period;
    return 1;
}

int pe_akao_channel_note_pitch(PeAkaoChannel *channel) {
    if (channel->last_note >= 0xa0) goto invalid;
    if (channel->last_note >= 0x8f) return 1; /* rest preserves current pitch */
    if ((channel->flags & (7u | 8u | (UINT32_C(1) << 12))) ||
        channel->tremolo_duration || channel->pitch_slide_steps) {
        snprintf(channel->error, sizeof(channel->error), "Unimplemented AKAO pitch mode at 0x%x", channel->pc);
        return 0;
    }
    if (channel->last_note < 0x84) {
        unsigned note = channel->last_note / 11 + (unsigned)channel->octave * 12;
        uint8_t transposed = (uint8_t)((int)note + signed16(channel->transpose));
        uint16_t pitch;
        if (!channel->control || !pe_akao_pitch_period(channel->samples, channel->instrument,
                                                      transposed, channel->detune, &pitch)) goto invalid;
        channel->current_note = (uint16_t)note;
        channel->pitch_base = pitch;
        if (!(channel->tremolo_phase & 2)) {
            channel->control->key_on_requests |= channel->voice_mask;
            channel->pitch_slide_steps = 0;
        }
        channel->control->allocated_voice_mask |= channel->voice_mask;
        channel->update_flags |= 0x13;
    }
    /* Ties retain pitch/key-on but advance the same phrase bookkeeping. */
    channel->tremolo_phase = (channel->tremolo_phase & (uint8_t)~2u) | ((channel->tremolo_phase & 1u) << 1);
    channel->tremolo_counter = channel->current_note;
    channel->tremolo_delta = channel->transpose;
    return 1;
invalid:
    snprintf(channel->error, sizeof(channel->error), "Invalid AKAO note pitch context at 0x%x", channel->pc);
    return 0;
}
