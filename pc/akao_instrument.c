#include "akao_channel.h"
#include <string.h>

#define INSTRUMENT_NO_RELEASE ((UINT32_C(1) << 7) | (UINT32_C(1) << 16) | \
    (UINT32_C(1) << 8) | (UINT32_C(1) << 11) | (UINT32_C(1) << 12) | \
    (UINT32_C(1) << 9) | (UINT32_C(1) << 13) | (UINT32_C(1) << 15))
#define INSTRUMENT_RELEASE ((UINT32_C(1) << 10) | (UINT32_C(1) << 14))

static void select_instrument(PeAkaoChannel *channel, unsigned index, const PeInstrument *instrument) {
    channel->instrument = (uint16_t)index;
    channel->sample_start = instrument->start;
    channel->sample_loop = instrument->loop;
    uint8_t release_rate = channel->adsr[4], release_mode = channel->adsr[7];
    memcpy(channel->adsr, instrument->adsr, 8);
    channel->update_flags |= INSTRUMENT_NO_RELEASE;
    if (channel->flags & (UINT32_C(1) << 9)) {
        channel->adsr[4] = release_rate;
        channel->adsr[7] = release_mode;
    } else channel->update_flags |= INSTRUMENT_RELEASE;
}

int pe_akao_channel_bind_samples(PeAkaoChannel *channel, const PeSpuBank *samples) {
    if (!samples || !samples->valid[0]) {
        snprintf(channel->error, sizeof(channel->error), "Missing AKAO initial instrument");
        return 0;
    }
    channel->samples = samples;
    select_instrument(channel, 0, &samples->instruments[0]);
    return 1;
}

int pe_akao_channel_set_instrument(PeAkaoChannel *channel, unsigned encoded) {
    if (!channel->samples || !channel->control) {
        snprintf(channel->error, sizeof(channel->error), "Missing AKAO instrument context at 0x%x", channel->pc);
        return 0;
    }
    unsigned index = encoded;
    if ((channel->control->status_flags & 0x100) && index >= 0x20) index += 0x30;
    if (index >= 256 || !channel->samples->valid[index]) goto invalid;
    uint32_t pitch = channel->pitch_base;
    int rescale = channel->instrument != 255 &&
        !(channel->control->allocated_voice_mask & channel->voice_mask & channel->control->spu_active_mask);
    if (rescale) {
        if (channel->instrument >= 256 || !channel->samples->valid[channel->instrument]) goto invalid;
        uint32_t divisor = channel->samples->instruments[channel->instrument].pitch[0];
        if (!divisor) goto invalid;
        /* Original multiplication is a wrapped unsigned word before divide. */
        pitch = (pitch * channel->samples->instruments[index].pitch[0]) / divisor;
    }
    select_instrument(channel, index, &channel->samples->instruments[index]);
    if (rescale) { channel->pitch_base = pitch; channel->update_flags |= 0x10; }
    channel->flags &= ~(UINT32_C(1) << 12);
    return 1;
invalid:
    snprintf(channel->error, sizeof(channel->error), "Invalid AKAO instrument/pitch divisor at 0x%x", channel->pc);
    return 0;
}

int pe_akao_channel_select_sample(PeAkaoChannel *channel, unsigned encoded) {
    if (!channel->samples || !channel->control) return 0;
    unsigned index = encoded;
    if ((channel->control->status_flags & 0x100) && index >= 0x20) index += 0x30;
    if (index >= 256 || !channel->samples->valid[index]) return 0;
    /* Drum selection only changes the instrument parameters; A1's pitch
     * ratio and pending-pitch clear are not part of this operation. */
    if (channel->instrument != index)
        select_instrument(channel, index, &channel->samples->instruments[index]);
    return 1;
}

int pe_akao_channel_reset_adsr(PeAkaoChannel *channel, int release_only) {
    if (!channel->samples || channel->instrument >= 256 || !channel->samples->valid[channel->instrument]) {
        snprintf(channel->error, sizeof(channel->error), "Missing AKAO ADSR instrument at 0x%x", channel->pc);
        return 0;
    }
    const uint8_t *adsr = channel->samples->instruments[channel->instrument].adsr;
    if (release_only) {
        channel->flags &= ~(UINT32_C(1) << 9);
        channel->adsr[4] = adsr[4];
        channel->update_flags |= 0x4400;
    } else {
        /* B3 restores all fields even when release override remains set. */
        memcpy(channel->adsr, adsr, 8);
        channel->update_flags |= 0xff00;
    }
    return 1;
}

int pe_akao_channel_select_note_range(PeAkaoChannel *channel, unsigned note) {
    if (note == channel->current_note) return 1;
    uint32_t base = channel->note_range_target;
    size_t size = channel->sequence.size;
    if (!channel->control || !channel->samples || base >= size) return 0;
    const uint8_t *table = channel->sequence.bytes + base;
    size_t available = (size - base) / 8, end = 0;
    /* The high-bit entry terminates the scan and is also read as a neighbour
     * by the original hysteresis rules. Require its complete record. */
    while (end < available && table[end * 8] < 0x80) ++end;
    if (!end || end == available) return 0;
    size_t selected;
    const uint8_t *neighbour;
    if (channel->current_note < note) {
        selected = 0;
        while (selected < end && table[selected * 8 + 2] < note) ++selected;
        if (selected + 1 >= available) return 0;
        neighbour = table + (selected + 1) * 8;
    } else {
        selected = end;
        while (note < table[selected * 8 + 1]) {
            if (!selected) return 0;
            --selected;
        }
        if (!selected && base < 8) return 0;
        neighbour = channel->sequence.bytes + base + selected * 8 - 8;
    }
    unsigned index = table[selected * 8], other = neighbour[0];
    unsigned bank_offset = (channel->control->status_flags & 0x100) ? 0x30 : 0;
    if (index >= 0x20) index += bank_offset;
    if (other >= 0x20) other += bank_offset;
    if (channel->instrument == index || channel->instrument == other) return 1;
    if (index >= 256 || !channel->samples->valid[index]) return 0;
    select_instrument(channel, index, &channel->samples->instruments[index]);
    const uint8_t *range = table + selected * 8;
    channel->adsr[0] = range[3];
    channel->adsr[3] = range[4];
    channel->adsr[6] = range[5];
    channel->adsr[4] = range[6];
    channel->update_flags |= 0x4000;
    return 1;
}
