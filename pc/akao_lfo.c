#include "akao_channel.h"

int pe_akao_channel_aux_lfo_step(PeAkaoChannel *channel) {
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
