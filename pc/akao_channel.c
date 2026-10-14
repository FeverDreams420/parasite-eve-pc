#include "akao_channel.h"
#include <string.h>

int pe_akao_channel_start(PeAkaoChannel *channel, const PeAkaoSequence *sequence, unsigned voice) {
    if (voice >= 24 || sequence->channels[voice] == UINT32_MAX ||
        sequence->channels[voice] >= sequence->size) return 0;
    uint8_t octave = channel->octave;
    memset(channel, 0, sizeof(*channel));
    channel->octave = octave;
    channel->sequence = *sequence;
    channel->pc = sequence->channels[voice];
    channel->volume_base = 0x7f00;
    channel->expression_value = UINT32_C(0x3fff0000);
    channel->panpot = 0x8000;
    channel->note_length = 4;
    channel->pan_duration = 2;
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
        unsigned operands;
        switch (opcode) {
        case 0xa0: case 0xa6: case 0xa7: operands = 0; break;
        case 0xa2: case 0xa3: case 0xa5: case 0xa8: case 0xaa: operands = 1; break;
        default:
            snprintf(channel->error, sizeof(channel->error), "Unimplemented AKAO opcode 0x%02x at 0x%x", opcode, pc);
            return PE_AKAO_CHANNEL_ERROR;
        }
        if (channel->sequence.size - pc - 1 < operands) {
            snprintf(channel->error, sizeof(channel->error), "Truncated AKAO operand at 0x%x", pc);
            return PE_AKAO_CHANNEL_ERROR;
        }
        unsigned value = operands ? channel->sequence.bytes[pc + 1] : 0;
        switch (opcode) {
        case 0xa0: channel->stopped = 1; break;
        case 0xa2:
            channel->fixed_note_length = 0;
            channel->note_length = channel->pan_duration = channel->default_note_length = (uint16_t)value;
            break;
        case 0xa3: channel->volume_base = (uint16_t)(value << 8); break;
        case 0xa5: channel->octave = (uint8_t)value; break;
        case 0xa6: channel->octave = (channel->octave + 1) & 15; break;
        case 0xa7: channel->octave = ((unsigned)channel->octave + 15u) & 15; break;
        case 0xa8:
            if (channel->flags & 8) channel->drum_volume = (uint16_t)(value << 7);
            else {
                channel->expression_duration = 0;
                uint32_t signed_byte = value & 128 ? UINT32_C(0xffffff00) | value : value;
                channel->expression_value = signed_byte << 23;
            }
            break;
        case 0xaa:
            channel->pan_slide_duration = 0;
            channel->panpot = (uint16_t)(((value + 0x40) & 255) << 8);
            break;
        }
        channel->pc = pc + 1 + operands;
        ++channel->commands;
        if (channel->stopped) return PE_AKAO_CHANNEL_STOP;
    }
    snprintf(channel->error, sizeof(channel->error), "AKAO instruction limit %u reached at 0x%x", limit, channel->pc);
    return PE_AKAO_CHANNEL_ERROR;
}
