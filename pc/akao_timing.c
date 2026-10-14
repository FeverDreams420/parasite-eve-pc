#include "akao_channel.h"

static int returned(PeAkaoChannel *channel, uint8_t phase, uint16_t duration,
                     uint8_t value, uint8_t *period) {
    channel->tremolo_phase = phase;
    channel->tremolo_duration = duration;
    *period = value;
    return 1;
}

int pe_akao_channel_peek(PeAkaoChannel *channel, unsigned limit, uint8_t *period) {
    if (channel->error[0]) return 0;
    if (!channel->grammar || channel->loop_index > 3) {
        snprintf(channel->error, sizeof(channel->error), "Missing AKAO grammar or invalid loop state");
        return 0;
    }
    uint32_t cursor = channel->pc;
    unsigned repeat = channel->loop_index;
    uint8_t phase = channel->tremolo_phase;
    uint16_t duration = channel->tremolo_duration;
    size_t size = channel->sequence.size;
    const uint8_t *bytes = channel->sequence.bytes;
    for (unsigned step = 0; step < limit; ++step) {
        if (cursor >= size) goto invalid;
        unsigned code = bytes[cursor];
        if (code < 0x9a) {
            if (code >= 0x8f) { duration = 0; phase &= (uint8_t)~5u; }
            return returned(channel, phase, duration, (uint8_t)code, period);
        }
        if (code < 0xa0) return returned(channel, phase, duration, 0xa0, period);
        unsigned skip = channel->grammar->lengths[code - 0xa0];
        if (skip) {
            if (skip > size - cursor) goto invalid;
            cursor += skip;
            continue;
        }
        switch (code) {
        case 0xfc: {
            if (size - cursor < 2) goto invalid;
            ++cursor;
            unsigned extended = bytes[cursor];
            if (extended >= 32) goto invalid;
            skip = channel->grammar->extended_lengths[extended];
            if (skip) {
                if (skip > size - cursor) goto invalid;
                cursor += skip;
                break;
            }
            if (extended < 6 || extended >= 10) {
                /* The original zero-length path re-dispatches the small
                 * subopcode as a plain byte; preserve that lookup behavior. */
                return returned(channel, phase, duration, (uint8_t)extended, period);
            }
            unsigned count = extended == 6 ? 2 : 3;
            if (size - cursor - 1 < count) goto invalid;
            const uint8_t *args = bytes + cursor + 1;
            uint32_t next = cursor + 1 + count;
            int take;
            if (extended == 7) {
                if (!channel->control) goto invalid;
                take = channel->control->condition >= args[0];
            } else take = extended == 6 || args[0] == (unsigned)channel->repeats[repeat] + 1;
            if (take) {
                unsigned index = extended == 6 ? 0 : 1;
                unsigned word = args[index] | (unsigned)args[index + 1] << 8;
                int displacement = word & 32768 ? (int)word - 65536 : (int)word;
                int64_t target = (int64_t)next + displacement;
                if (target < 0 || (uint64_t)target >= size) goto invalid;
                cursor = (uint32_t)target;
                /* Unlike execution, lookahead pops for both FC/08 and /09. */
                if (extended >= 8) repeat = (repeat + 3) & 3;
            } else cursor = next;
            break;
        }
        case 0xc9:
            if (size - cursor < 2) goto invalid;
            /* Lookahead compares the raw byte: zero is not normalized to
             * 256 here, unlike SeqOp_LoopCounter during execution. */
            if (bytes[cursor + 1] == (unsigned)channel->repeats[repeat] + 1) {
                cursor += 2;
                repeat = (repeat + 3) & 3;
            } else cursor = channel->loop_points[repeat];
            break;
        case 0xcb: case 0xcd: case 0xd1: case 0xdb:
            ++cursor;
            duration = 0; phase &= (uint8_t)~5u;
            break;
        case 0xcc: case 0xd0:
            phase &= (uint8_t)~5u;
            return returned(channel, phase, duration, 0xa0, period);
        case 0xca:
            if (!(channel->flags & (UINT32_C(1) << 21))) {
                cursor = channel->loop_points[repeat];
                break;
            }
            /* Deferred key-off takes the same stop lookup path. */
            duration = 0; phase &= (uint8_t)~5u;
            return returned(channel, phase, duration, 0xa0, period);
        default:
            duration = 0; phase &= (uint8_t)~5u;
            return returned(channel, phase, duration, 0xa0, period);
        }
    }
    snprintf(channel->error, sizeof(channel->error), "AKAO lookahead limit %u reached", limit);
    return 0;
invalid:
    snprintf(channel->error, sizeof(channel->error), "Invalid AKAO lookahead at 0x%x", cursor);
    return 0;
}

int pe_akao_channel_begin_note(PeAkaoChannel *channel, unsigned lookahead_limit) {
    if (channel->last_note >= 0xa0) {
        snprintf(channel->error, sizeof(channel->error), "Invalid AKAO note byte");
        return 0;
    }
    uint8_t period;
    if (!pe_akao_channel_peek(channel, lookahead_limit, &period)) return 0;
    uint16_t length = channel->note_length, gate = channel->pan_duration;
    if (channel->fixed_note_length) length = gate = channel->fixed_note_length;
    if (length) {
        if (period >= 0x8f || (period < 0x84 && !(channel->tremolo_phase & 5)))
            gate = (uint16_t)(gate - 2);
    } else {
        length = channel->grammar->note_lengths[channel->last_note % 11];
        gate = length;
        if ((uint32_t)period - 0x84 >= 11 && !(channel->tremolo_phase & 5))
            gate = (uint16_t)(gate - 2);
    }
    channel->note_length = channel->default_note_length = length;
    channel->update_flags |= 0x4000;
    channel->pan_duration = gate;
    if (period < 0x8f) channel->flags &= ~UINT32_C(0x40);
    else channel->flags |= 0x40;
    if (channel->last_note >= 0x8f) {
        channel->tremolo_duration = 0;
        channel->tremolo_phase &= (uint8_t)~2u;
    }
    return 1;
}

void pe_akao_channel_slides(PeAkaoChannel *channel) {
    if (channel->expression_duration) {
        uint32_t old = channel->expression_value;
        channel->expression_value += channel->expression_delta;
        --channel->expression_duration;
        if ((old & UINT32_C(0xffe00000)) != (channel->expression_value & UINT32_C(0xffe00000)))
            channel->update_flags |= 3;
    }
    if (channel->volume_duration) {
        uint16_t old = channel->volume_base;
        channel->volume_base = (uint16_t)(old + channel->volume_delta);
        --channel->volume_duration;
        if ((old & 0x7f00) != (channel->volume_base & 0x7f00)) channel->update_flags |= 3;
    }
    if (channel->pan_slide_duration) {
        uint16_t old = channel->panpot;
        channel->panpot = (uint16_t)(old + channel->pan_delta);
        --channel->pan_slide_duration;
        if ((old & 0xff00) != (channel->panpot & 0xff00)) channel->update_flags |= 3;
    }
}

PeAkaoChannelResult pe_akao_channel_tick(PeAkaoChannel *channel, unsigned instruction_limit) {
    if (channel->error[0]) return PE_AKAO_CHANNEL_ERROR;
    if (channel->stopped) return PE_AKAO_CHANNEL_STOP;
    ++channel->musical_ticks;
    --channel->note_length;
    --channel->pan_duration;
    if (!channel->note_length) {
        PeAkaoChannelResult result = pe_akao_channel_next(channel, instruction_limit);
        if (result == PE_AKAO_CHANNEL_ERROR) return result;
        if (result == PE_AKAO_CHANNEL_NOTE && !pe_akao_channel_begin_note(channel, instruction_limit))
            return PE_AKAO_CHANNEL_ERROR;
        if (result == PE_AKAO_CHANNEL_NOTE && channel->samples && !pe_akao_channel_note_pitch(channel))
            return PE_AKAO_CHANNEL_ERROR;
        pe_akao_channel_slides(channel);
        return result;
    }
    if (!channel->pan_duration) channel->key_off_requested = 1;
    pe_akao_channel_slides(channel);
    return PE_AKAO_CHANNEL_WAIT;
}
