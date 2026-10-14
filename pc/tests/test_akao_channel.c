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
    const uint8_t loop[] = {0xc8,0xa6,0xc9,3,0xa0};
    sequence.bytes = loop; sequence.size = sizeof(loop); channel.octave = 0;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_STOP);
    CHECK(channel.octave == 3 && !channel.loop_index && channel.repeats[1] == 3 && channel.commands == 8);
    const uint8_t loop256[] = {0xc8,0xa6,0xc9,0,0xa0};
    sequence.bytes = loop256; sequence.size = sizeof(loop256); channel.octave = 0;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 1024) == PE_AKAO_CHANNEL_STOP);
    CHECK(!channel.octave && channel.repeats[1] == 256 && channel.commands == 514);
    const uint8_t nested[] = {0xc8,0xc8,0xa6,0xc9,2,0xc9,3,0xa0};
    sequence.bytes = nested; sequence.size = sizeof(nested); channel.octave = 0;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_STOP);
    CHECK(channel.octave == 6 && !channel.loop_index && channel.commands == 20);
    for (unsigned pop = 0; pop < 2; ++pop) {
        uint8_t branch[] = {0xc8,0xa6,0xfc,8,3,2,0,0xc9,3,0xa0};
        branch[3] = (uint8_t)(8 + pop);
        sequence.bytes = branch; sequence.size = sizeof(branch); channel.octave = 0;
        CHECK(pe_akao_channel_start(&channel, &sequence, 0));
        CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_STOP);
        CHECK(channel.octave == 3 && channel.loop_index == (pop ? 0 : 1));
        CHECK(channel.repeats[1] == 2 && channel.commands == 10);
    }
    const uint8_t sustain[] = {0xc8,0x11,0xca};
    sequence.bytes = sustain; sequence.size = sizeof(sustain);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE && channel.repeats[1] == 1);
    channel.flags |= UINT32_C(1) << 21;
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_STOP && channel.key_off_requested);
    const uint8_t bad_jump[] = {0xc8,0xfc,9,1,0xf0,0xff,0xa0};
    sequence.bytes = bad_jump; sequence.size = sizeof(bad_jump);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_ERROR);
    CHECK(channel.pc == 1 && channel.loop_index == 1 && channel.commands == 1);
    const uint8_t ring[] = {0xc8,0xc8,0xc8,0xc8,0xc8,0xa0};
    sequence.bytes = ring; sequence.size = sizeof(ring);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_STOP && channel.loop_index == 1);
    const uint8_t incomplete_ext[] = {0xfc};
    sequence.bytes = incomplete_ext; sequence.size = sizeof(incomplete_ext);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_ERROR && !channel.pc);
    const uint8_t forward[] = {0xfc,6,2,0,0xa6,0xa6,0x11,0xa0};
    sequence.bytes = forward; sequence.size = sizeof(forward); channel.octave = 4;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.pc == 7 && channel.octave == 4 && channel.commands == 1);
    const uint8_t forever[] = {0xfc,6,0xfc,0xff}; /* -4 from the advanced PC */
    sequence.bytes = forever; sequence.size = sizeof(forever);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 7) == PE_AKAO_CHANNEL_ERROR);
    CHECK(channel.pc == 0 && channel.commands == 7 && strstr(channel.error, "limit"));
    puts("AKAO channel: state commands, finite/nested loops, signed jumps, key-off and explicit errors passed");
    return 0;
}
