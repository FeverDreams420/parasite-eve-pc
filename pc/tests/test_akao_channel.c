#include "akao_channel.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
int main(void) {
    const uint8_t bytes[] = {0xa5,15,0xa6,0xa7,0xa2,17,0xa3,90,0xa8,255,0xaa,200,0x43,0xa0};
    PeAkaoSequence sequence = {0}; sequence.bytes = bytes; sequence.size = sizeof(bytes);
    for (unsigned i = 0; i < 24; ++i) sequence.channels[i] = UINT32_MAX;
    sequence.channels[0] = 0;
    PeAkaoChannel channel = {0}; channel.octave = 4;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(channel.octave == 4 && channel.volume_base == 0x7f00 && channel.panpot == 0x8000);
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.last_note == 0x43 && channel.pc == 13 && channel.commands == 7);
    CHECK(channel.octave == 15 && channel.note_length == 17 && channel.pan_duration == 17);
    CHECK(channel.volume_base == 90 * 256 && channel.expression_value == 0xff800000);
    CHECK(channel.panpot == 8 * 256);
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_STOP);
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_STOP && channel.commands == 8);
    CHECK(!pe_akao_channel_start(&channel, &sequence, 1));
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    channel.flags = 8;
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.drum_volume == 255 * 128 && channel.expression_value == 0x3fff0000);
    const uint8_t missing[] = {0xa1,32};
    sequence.bytes = missing; sequence.size = sizeof(missing);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_ERROR);
    CHECK(!channel.pc && !channel.commands && strstr(channel.error, "Unimplemented"));
    const uint8_t truncated[] = {0xa3};
    sequence.bytes = truncated; sequence.size = sizeof(truncated);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_ERROR && !channel.pc);
    sequence.bytes = bytes; sequence.size = sizeof(bytes);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 1) == PE_AKAO_CHANNEL_ERROR);
    CHECK(channel.pc == 2 && channel.commands == 1 && strstr(channel.error, "limit"));
    puts("AKAO channel commands: length, octave, volume/expression, pan, stop and explicit errors passed");
    return 0;
}
