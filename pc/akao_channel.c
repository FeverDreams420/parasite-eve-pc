#include "akao_channel.h"
#include <string.h>

int pe_akao_control_due(PeAkaoControl *control, uint32_t scale, int forced) {
    uint32_t tempo = control->tempo >> 16;
    if (scale) {
        if (scale < 128) tempo += (tempo * scale) >> 7;
        else tempo = (tempo * scale) >> 8;
    }
    control->accumulator += tempo;
    if ((control->accumulator & UINT32_C(0xffff0000)) || forced) {
        control->accumulator &= 65535;
        return 1;
    }
    return 0;
}

void pe_akao_control_step(PeAkaoControl *control) {
    ++control->musical_ticks;
    if (control->tempo_duration) {
        --control->tempo_duration;
        control->tempo += control->tempo_delta;
    }
    if (control->volume_duration) {
        --control->volume_duration;
        control->volume += control->volume_delta;
        control->volume_dirty = 1;
    }
    if (control->ticks_per_beat && ++control->tick == control->ticks_per_beat) {
        control->tick = 0;
        if (++control->beat == control->beats_per_measure) {
            control->beat = 0;
            ++control->measure;
        }
    }
}

static uint32_t slide_delta(uint32_t target, uint32_t current, unsigned duration) {
    uint32_t difference = target - current;
    int64_t signed_difference = difference & UINT32_C(0x80000000) ?
        (int64_t)difference - INT64_C(4294967296) : difference;
    return (uint32_t)(signed_difference / duration);
}

static void stop_channel(PeAkaoChannel *channel) {
    if (channel->control) {
        channel->control->reverb_mask &= ~channel->voice_mask;
        if (channel->flags & (UINT32_C(1) << 11))
            channel->control->reserved_voice_mask &= ~(UINT32_C(1) << (channel->voice_index & 31));
        channel->control->voice_modes_dirty = 1;
    }
    channel->stopped = 1;
    channel->flags = 0;
}

int pe_akao_channel_start(PeAkaoChannel *channel, const PeAkaoSequence *sequence, unsigned voice) {
    if (voice >= 24 || sequence->channels[voice] == UINT32_MAX ||
        sequence->channels[voice] >= sequence->size) return 0;
    uint8_t octave = channel->octave;
    PeAkaoControl *control = channel->control;
    const PeAkaoGrammar *grammar = channel->grammar;
    const PeSpuBank *samples = channel->samples;
    if (samples && !samples->valid[0]) return 0;
    memset(channel, 0, sizeof(*channel));
    channel->octave = octave;
    channel->control = control;
    channel->grammar = grammar;
    channel->voice_mask = UINT32_C(1) << voice;
    channel->sequence = *sequence;
    channel->pc = sequence->channels[voice];
    channel->volume_base = 0x7f00;
    channel->expression_value = UINT32_C(0x3fff0000);
    channel->panpot = 0x8000;
    channel->note_length = 4;
    channel->pan_duration = 2;
    channel->drum_target = UINT32_MAX;
    for (unsigned i = 0; i < 4; ++i) channel->loop_points[i] = UINT32_MAX;
    if (samples) pe_akao_channel_bind_samples(channel, samples);
    return 1;
}

PeAkaoChannelResult pe_akao_channel_next(PeAkaoChannel *channel, unsigned limit) {
    if (channel->error[0]) return PE_AKAO_CHANNEL_ERROR;
    if (channel->stopped) return PE_AKAO_CHANNEL_STOP;
    for (unsigned i = 0; i < limit; ++i) {
        uint32_t pc = channel->pc;
        if (pc >= channel->sequence.size) {
            snprintf(channel->error, sizeof(channel->error), "Truncated AKAO opcode at 0x%x", pc);
            return PE_AKAO_CHANNEL_ERROR;
        }
        unsigned opcode = channel->sequence.bytes[pc];
        if (opcode < 0xa0) {
            channel->pc = pc + 1;
            channel->last_note = (uint8_t)opcode;
            return PE_AKAO_CHANNEL_NOTE;
        }
        unsigned operands, prefix = 1, extended = 0;
        switch (opcode) {
        case 0xa0: case 0xa6: case 0xa7: case 0xb3: operands = 0; break;
        case 0xa1: case 0xa2: case 0xa3: case 0xa5: case 0xa8: case 0xaa:
        case 0xc0: case 0xc1: case 0xd8: case 0xd9:
        case 0xad: case 0xae: case 0xaf: case 0xb1: case 0xb2:
        case 0xb7: case 0xbb: case 0xbf: operands = 1; break;
        case 0xa9: case 0xab: case 0xb0: operands = 2; break;
        case 0xc2: case 0xc3: case 0xc8: case 0xca: operands = 0; break;
        case 0xc9: operands = 1; break;
        case 0xfc:
            if (channel->sequence.size - pc < 2) goto truncated;
            extended = channel->sequence.bytes[pc + 1];
            prefix = 2;
            if (extended > 5 && extended != 6 && extended != 7 && extended != 8 && extended != 9 &&
                extended != 0x10 && extended != 0x11 && extended != 0x12 && extended != 0x15 && extended != 0x16) {
                snprintf(channel->error, sizeof(channel->error), "Unimplemented AKAO opcode 0xfc/0x%02x at 0x%x", extended, pc);
                return PE_AKAO_CHANNEL_ERROR;
            }
            operands = extended == 5 || extended == 0x11 ? 0 : extended == 0x10 ? 1 :
                       extended == 0 || extended == 2 || extended == 4 || extended == 6 || extended == 0x12 || extended == 0x15 || extended == 0x16 ? 2 : 3;
            if ((extended <= 3 || extended == 7 || extended == 0x15 || extended == 0x16) && !channel->control) {
                snprintf(channel->error, sizeof(channel->error), "Missing shared AKAO control at 0x%x", pc);
                return PE_AKAO_CHANNEL_ERROR;
            }
            break;
        default:
            snprintf(channel->error, sizeof(channel->error), "Unimplemented AKAO opcode 0x%02x at 0x%x", opcode, pc);
            return PE_AKAO_CHANNEL_ERROR;
        }
        if (channel->sequence.size - pc < prefix + operands) {
truncated:
            snprintf(channel->error, sizeof(channel->error), "Truncated AKAO operand at 0x%x", pc);
            return PE_AKAO_CHANNEL_ERROR;
        }
        unsigned value = operands ? channel->sequence.bytes[pc + prefix] : 0;
        uint32_t next = pc + prefix + operands;
        if (channel->loop_index > 3) goto invalid;
        switch (opcode) {
        case 0xa0: stop_channel(channel); break;
        case 0xa1:
            if (!pe_akao_channel_set_instrument(channel, value)) return PE_AKAO_CHANNEL_ERROR;
            break;
        case 0xa2:
            channel->fixed_note_length = 0;
            channel->note_length = channel->pan_duration = channel->default_note_length = (uint16_t)value;
            break;
        case 0xa3: channel->volume_base = (uint16_t)(value << 8); channel->update_flags |= 3; break;
        case 0xa5: channel->octave = (uint8_t)value; break;
        case 0xa6: channel->octave = (channel->octave + 1) & 15; break;
        case 0xa7: channel->octave = ((unsigned)channel->octave + 15u) & 15; break;
        case 0xa8:
            if (channel->flags & 8) channel->drum_volume = (uint16_t)(value << 7);
            else {
                channel->expression_duration = 0;
                uint32_t signed_byte = value & 128 ? UINT32_C(0xffffff00) | value : value;
                channel->expression_value = signed_byte << 23;
                channel->update_flags |= 3;
            }
            break;
        case 0xa9: {
            unsigned target_byte = channel->sequence.bytes[pc + prefix + 1];
            uint32_t signed_byte = target_byte & 128 ? UINT32_C(0xffffff00) | target_byte : target_byte;
            channel->expression_duration = (uint16_t)(value ? value : 256);
            channel->expression_value &= UINT32_C(0xffff0000);
            channel->expression_delta = slide_delta(signed_byte << 23, channel->expression_value,
                                                     channel->expression_duration);
            break;
        }
        case 0xaa:
            channel->pan_slide_duration = 0;
            channel->panpot = (uint16_t)(((value + 0x40) & 255) << 8);
            channel->update_flags |= 3;
            break;
        case 0xab: {
            unsigned target_byte = channel->sequence.bytes[pc + prefix + 1];
            channel->pan_slide_duration = (uint16_t)(value ? value : 256);
            channel->panpot &= 0xff00;
            int target = ((target_byte + 0x40) & 255) << 8;
            channel->pan_delta = (uint16_t)((target - (int)channel->panpot) / channel->pan_slide_duration);
            break;
        }
        case 0xc2: case 0xc3:
            if (!channel->control) {
                snprintf(channel->error, sizeof(channel->error), "Missing AKAO reverb bank context at 0x%x", pc);
                return PE_AKAO_CHANNEL_ERROR;
            }
            if (opcode == 0xc2) channel->control->reverb_mask |= channel->voice_mask;
            else {
                channel->control->reverb_mask &= ~channel->voice_mask;
                if (channel->flags & (UINT32_C(1) << 11))
                    channel->control->reserved_voice_mask &= ~(UINT32_C(1) << (channel->voice_index & 31));
            }
            channel->control->voice_modes_dirty = 1;
            break;
        case 0xc0: case 0xc1: case 0xd8: case 0xd9: {
            int signed_byte = value & 128 ? (int)value - 256 : (int)value;
            uint16_t *field = opcode < 0xd0 ? &channel->transpose : &channel->detune;
            if (opcode == 0xc0 || opcode == 0xd8) *field = (uint16_t)signed_byte;
            else *field = (uint16_t)((unsigned)*field + (uint32_t)signed_byte);
            break;
        }
        case 0xad: channel->adsr[0] = (uint8_t)value; channel->update_flags |= 0x0900; break;
        case 0xae: channel->adsr[1] = (uint8_t)value; channel->update_flags |= 0x1000; break;
        case 0xaf: channel->adsr[2] = (uint8_t)value; channel->update_flags |= 0x8000; break;
        case 0xb0:
            channel->adsr[1] = (uint8_t)value;
            channel->adsr[2] = channel->sequence.bytes[pc + prefix + 1];
            channel->update_flags |= 0x9000;
            break;
        case 0xb1: channel->adsr[3] = (uint8_t)value; channel->update_flags |= 0x2200; break;
        case 0xb2: channel->adsr[4] = (uint8_t)value; channel->update_flags |= 0x4400; break;
        case 0xb3:
            if (!pe_akao_channel_reset_adsr(channel, 0)) return PE_AKAO_CHANNEL_ERROR;
            break;
        case 0xb7: channel->adsr[5] = (uint8_t)value; channel->update_flags |= 0x0100; break;
        case 0xbb: channel->adsr[6] = (uint8_t)value; channel->update_flags |= 0x0200; break;
        case 0xbf: channel->adsr[7] = (uint8_t)value; channel->update_flags |= 0x0400; break;
        case 0xc8:
            if (next >= channel->sequence.size) goto invalid;
            channel->loop_index = (channel->loop_index + 1) & 3;
            channel->loop_points[channel->loop_index] = next;
            channel->repeats[channel->loop_index] = 0;
            break;
        case 0xc9: {
            unsigned count = value ? value : 256;
            uint16_t repeat = (uint16_t)(channel->repeats[channel->loop_index] + 1);
            if (repeat != count) {
                next = channel->loop_points[channel->loop_index];
                if (next >= channel->sequence.size) goto invalid;
            }
            channel->repeats[channel->loop_index] = repeat;
            if (repeat == count) channel->loop_index = (channel->loop_index + 3) & 3;
            break;
        }
        case 0xca:
            if (channel->flags & (UINT32_C(1) << 21)) {
                channel->key_off_requested = 1;
                stop_channel(channel);
            } else {
                next = channel->loop_points[channel->loop_index];
                if (next >= channel->sequence.size) goto invalid;
                ++channel->repeats[channel->loop_index];
            }
            break;
        case 0xfc: {
            const uint8_t *args = channel->sequence.bytes + pc + prefix;
            if (extended == 0x16) {
                /* +64 in the bank alias is measure, not a voice loop count. */
                channel->control->measure = (uint16_t)(args[0] | (unsigned)args[1] << 8);
                break;
            }
            if (extended == 0x15) {
                /* SeqOp_StreamPair's track pointer aliases the bank:
                 * +60 = ticks_per_beat, +62 = tick, +5E = beat,
                 * +5C = beats_per_measure. No voice allocation occurs. */
                channel->control->ticks_per_beat = (uint16_t)value;
                channel->control->beats_per_measure = args[1];
                channel->control->tick = channel->control->beat = 0;
                break;
            }
            if (extended == 0x10) {
                channel->flags |= UINT32_C(1) << 9;
                channel->adsr[4] = (uint8_t)value;
                channel->update_flags |= 0x4400;
                break;
            }
            if (extended == 0x11) {
                if (!pe_akao_channel_reset_adsr(channel, 1)) return PE_AKAO_CHANNEL_ERROR;
                break;
            }
            if (extended == 4) {
                unsigned word = args[0] | (unsigned)args[1] << 8;
                int displacement = word & 32768 ? (int)word - 65536 : (int)word;
                int64_t target = (int64_t)next + displacement;
                if (target < 0 || (uint64_t)target >= channel->sequence.size) goto invalid;
                channel->drum_volume = (uint16_t)(channel->expression_value >> 16);
                channel->drum_target = (uint32_t)target;
                channel->flags |= 8;
                break;
            }
            if (extended == 5) { channel->flags &= ~UINT32_C(8); break; }
            if (extended == 0x12) {
                channel->volume_duration = (uint16_t)(value ? value : 256);
                channel->volume_base &= 0x7f00;
                int target = args[1] << 8;
                channel->volume_delta = (uint16_t)((target - (int)channel->volume_base) / channel->volume_duration);
                break;
            }
            if (extended <= 3) {
                int volume = extended >= 2;
                int slide = extended & 1;
                unsigned word_index = slide ? 1 : 0;
                uint32_t target = (uint32_t)(args[word_index] | (unsigned)args[word_index + 1] << 8) << 16;
                PeAkaoControl *control = channel->control;
                uint32_t *current = volume ? &control->volume : &control->tempo;
                uint16_t *duration = volume ? &control->volume_duration : &control->tempo_duration;
                uint32_t *delta = volume ? &control->volume_delta : &control->tempo_delta;
                if (slide) {
                    *duration = (uint16_t)(value ? value : 256);
                    *current &= UINT32_C(0xffff0000);
                    *delta = slide_delta(target, *current, *duration);
                } else {
                    *duration = 0;
                    *current = target;
                    if (volume) control->volume_dirty = 1;
                }
                break;
            }
            unsigned count = value ? value : 256;
            int taken = extended == 6 || (extended == 7 ? channel->control->condition >= value :
                        (unsigned)channel->repeats[channel->loop_index] + 1 == count);
            if (taken) {
                unsigned displacement_index = extended == 6 ? 0 : 1;
                unsigned word = args[displacement_index] | (unsigned)args[displacement_index + 1] << 8;
                int displacement = word & 32768 ? (int)word - 65536 : (int)word;
                int64_t target = (int64_t)next + displacement;
                if (target < 0 || (uint64_t)target >= channel->sequence.size) goto invalid;
                next = (uint32_t)target;
                if (extended == 9) channel->loop_index = (channel->loop_index + 3) & 3;
            }
            break;
        }
        }
        channel->pc = next;
        ++channel->commands;
        if (channel->stopped) return PE_AKAO_CHANNEL_STOP;
    }
    snprintf(channel->error, sizeof(channel->error), "AKAO instruction limit %u reached at 0x%x", limit, channel->pc);
    return PE_AKAO_CHANNEL_ERROR;
invalid:
    snprintf(channel->error, sizeof(channel->error), "Invalid AKAO loop state/target at 0x%x", channel->pc);
    return PE_AKAO_CHANNEL_ERROR;
}
