#include "akao_channel.h"

static int32_t shift_word(uint32_t word, unsigned bits) {
    int64_t value = word & UINT32_C(0x80000000) ? (int64_t)word - INT64_C(0x100000000) : word;
    int64_t divisor = INT64_C(1) << bits;
    return (int32_t)(value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor));
}

int pe_akao_channel_pitch_lfo_step(PeAkaoChannel *channel, int key_on) {
    if (!key_on && channel->pitch_lfo_counter) --channel->pitch_lfo_counter;
    if (!key_on && channel->pitch_lfo_slide_duration) {
        channel->pitch_lfo_target = (uint16_t)(channel->pitch_lfo_target + channel->pitch_lfo_delta);
        --channel->pitch_lfo_slide_duration;
        pe_akao_channel_pitch_lfo_depth(channel, 0);
        if (!channel->pitch_lfo_counter && channel->pitch_lfo_phase != 1) {
            uint16_t cursor = channel->pitch_lfo_cursor; int16_t sample;
            if (!pe_akao_lfo_next(channel->grammar, &cursor, &sample)) {
                snprintf(channel->error, sizeof(channel->error), "Invalid AKAO pitch LFO slide at 0x%x", channel->pc);
                return 0;
            }
            int value = shift_word((uint32_t)((int)channel->pitch_lfo_depth * sample), 16);
            if ((uint16_t)value != channel->pitch_lfo_value) {
                channel->pitch_lfo_value = (uint16_t)(value >= 0 ? value * 2 : value);
                channel->update_flags |= 0x10;
            }
        }
    }
    if (!(channel->flags & 1) || (!key_on && channel->pitch_lfo_counter)) return 1;
    uint16_t phase = (uint16_t)(channel->pitch_lfo_phase - 1);
    if (phase) { channel->pitch_lfo_phase = phase; return 1; }
    uint16_t cursor = channel->pitch_lfo_cursor; int16_t sample;
    if (!channel->pitch_lfo_duration || !pe_akao_lfo_next(channel->grammar, &cursor, &sample)) {
        snprintf(channel->error, sizeof(channel->error), "Invalid AKAO pitch LFO state at 0x%x", channel->pc);
        return 0;
    }
    int value = shift_word((uint32_t)((int)channel->pitch_lfo_depth * sample), 16);
    /* Retail compares the undoubled value, then doubles nonnegative output. */
    if ((uint16_t)value != channel->pitch_lfo_value) {
        channel->pitch_lfo_value = (uint16_t)(value >= 0 ? value * 2 : value);
        channel->update_flags |= 0x10;
    }
    channel->pitch_lfo_cursor = cursor;
    channel->pitch_lfo_phase = channel->pitch_lfo_duration;
    return 1;
}

static void volume_sample(PeAkaoChannel *channel, int16_t sample) {
    unsigned word = channel->expression_value >> 16;
    int expression = word & 32768 ? (int)word - 65536 : (int)word;
    int32_t base = shift_word((uint32_t)(expression * (channel->volume_base >> 8)), 7);
    uint32_t product = (uint32_t)base * (channel->volume_lfo_target >> 8);
    int32_t value = shift_word(product << 9, 16);
    value = shift_word((uint32_t)(value * sample), 15);
    if ((uint16_t)value != channel->volume_lfo_value) {
        channel->volume_lfo_value = (uint16_t)value;
        channel->update_flags |= 3;
    }
}

int pe_akao_channel_volume_lfo_step(PeAkaoChannel *channel, int key_on) {
    if (!key_on && channel->volume_lfo_counter) --channel->volume_lfo_counter;
    if (!key_on && channel->volume_lfo_slide_duration) {
        channel->volume_lfo_target = (uint16_t)(channel->volume_lfo_target + channel->volume_lfo_delta);
        --channel->volume_lfo_slide_duration;
        if (!channel->volume_lfo_counter && channel->volume_lfo_phase != 1) {
            uint16_t cursor = channel->volume_lfo_cursor; int16_t sample;
            if (!pe_akao_lfo_next(channel->grammar, &cursor, &sample)) {
                snprintf(channel->error, sizeof(channel->error), "Invalid AKAO volume LFO slide at 0x%x", channel->pc);
                return 0;
            }
            volume_sample(channel, sample);
        }
    }
    if (!(channel->flags & 2) || (!key_on && channel->volume_lfo_counter)) return 1;
    uint16_t phase = (uint16_t)(channel->volume_lfo_phase - 1);
    if (phase) { channel->volume_lfo_phase = phase; return 1; }
    uint16_t cursor = channel->volume_lfo_cursor; int16_t sample;
    if (!channel->volume_lfo_duration || !pe_akao_lfo_next(channel->grammar, &cursor, &sample)) {
        snprintf(channel->error, sizeof(channel->error), "Invalid AKAO volume LFO state at 0x%x", channel->pc);
        return 0;
    }
    volume_sample(channel, sample);
    channel->volume_lfo_cursor = cursor;
    channel->volume_lfo_phase = channel->volume_lfo_duration;
    return 1;
}

int pe_akao_channel_aux_lfo_step(PeAkaoChannel *channel) {
    if (channel->aux_lfo_slide_duration) {
        channel->aux_lfo_target = (uint16_t)(channel->aux_lfo_target + channel->aux_lfo_delta);
        --channel->aux_lfo_slide_duration;
        if (channel->aux_lfo_phase != 1) {
            uint16_t cursor = channel->aux_lfo_cursor; int16_t sample;
            if (!pe_akao_lfo_next(channel->grammar, &cursor, &sample)) {
                snprintf(channel->error, sizeof(channel->error), "Invalid AKAO auxiliary LFO slide at 0x%x", channel->pc);
                return 0;
            }
            int value = shift_word((uint32_t)((channel->aux_lfo_target >> 8) * (int)sample), 15);
            if ((uint16_t)value != channel->aux_lfo_value) {
                channel->aux_lfo_value = (uint16_t)value;
                channel->update_flags |= 3;
            }
        }
    }
    if (!(channel->flags & 4)) return 1;
    uint16_t phase = (uint16_t)(channel->aux_lfo_phase - 1);
    if (phase) { channel->aux_lfo_phase = phase; return 1; }
    uint16_t cursor = channel->aux_lfo_cursor;
    int16_t sample;
    if (!channel->aux_lfo_duration || !pe_akao_lfo_next(channel->grammar, &cursor, &sample)) {
        snprintf(channel->error, sizeof(channel->error), "Invalid AKAO auxiliary LFO state at 0x%x", channel->pc);
        return 0;
    }
    int product = (channel->aux_lfo_target >> 8) * (int)sample;
    /* Explicit arithmetic right shift: round negative products down. */
    int value = product >= 0 ? product / 32768 : -((-product + 32767) / 32768);
    if ((uint16_t)value != channel->aux_lfo_value) {
        channel->aux_lfo_value = (uint16_t)value;
        channel->update_flags |= 3;
    }
    channel->aux_lfo_cursor = cursor;
    channel->aux_lfo_phase = channel->aux_lfo_duration;
    return 1;
}
