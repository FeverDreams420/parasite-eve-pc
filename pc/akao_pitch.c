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
    if ((channel->flags & 7u) ||
        channel->tremolo_duration || channel->pitch_slide_steps) {
        snprintf(channel->error, sizeof(channel->error), "Unimplemented AKAO pitch mode at 0x%x", channel->pc);
        return 0;
    }
    if (channel->last_note < 0x84) {
        if (channel->flags & 8) {
            uint64_t offset = (uint64_t)channel->drum_target + (channel->last_note / 11 % 12) * 6;
            if (!channel->control || !channel->samples || offset > channel->sequence.size ||
                channel->sequence.size - offset < 6) goto invalid;
            const uint8_t *sample = channel->sequence.bytes + (size_t)offset;
            unsigned instrument = sample[0];
            if ((channel->control->status_flags & 0x100) && instrument >= 0x20) instrument += 0x30;
            uint16_t pitch;
            if (!pe_akao_pitch_period(channel->samples, instrument, sample[1], channel->detune, &pitch) ||
                !pe_akao_channel_select_sample(channel, sample[0])) goto invalid;
            uint32_t weight = sample[2] | (uint32_t)sample[3] << 8;
            channel->expression_value = ((uint32_t)channel->drum_volume * weight) << 2;
            channel->panpot = (uint16_t)(((sample[4] + 0x40u) & 255) << 8);
            if (sample[5]) channel->control->reverb_mask |= channel->voice_mask;
            else channel->control->reverb_mask &= ~channel->voice_mask;
            channel->control->voice_modes_dirty = 1;
            channel->control->key_on_requests |= channel->voice_mask;
            channel->pitch_base = pitch;
        } else {
            unsigned note = channel->last_note / 11 + (unsigned)channel->octave * 12;
            if (!(channel->tremolo_phase & 2) && (channel->flags & (UINT32_C(1) << 12)) &&
                !pe_akao_channel_select_note_range(channel, note)) goto invalid;
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
