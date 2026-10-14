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
    PeAkaoControl control = {0};
    channel.control = &control;
    const uint8_t slides[] = {0xfc,0,0,0x20, 0xfc,1,4,0,0x60,
                              0xfc,2,0,0x50, 0xfc,3,0,0,0x10, 0x11};
    sequence.bytes = slides; sequence.size = sizeof(slides);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(control.tempo == 0x20000000 && control.tempo_duration == 4 && control.tempo_delta == 0x10000000);
    CHECK(control.volume == 0x50000000 && control.volume_duration == 256 && control.volume_delta == 0xffc00000);
    for (unsigned i = 0; i < 4; ++i) pe_akao_control_step(&control);
    CHECK(control.tempo == 0x60000000 && !control.tempo_duration && control.volume == 0x4f000000);
    for (unsigned i = 0; i < 252; ++i) pe_akao_control_step(&control);
    CHECK(control.volume == 0x10000000 && !control.volume_duration && control.musical_ticks == 256);
    const uint8_t quantized[] = {0xfc,1,3,5,0,0x11};
    control.tempo = 0x10001;
    sequence.bytes = quantized; sequence.size = sizeof(quantized);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    for (unsigned i = 0; i < 3; ++i) pe_akao_control_step(&control);
    CHECK(control.tempo == 0x4ffff); /* preserve truncation; do not snap to target */
    const uint8_t conditional[] = {0xfc,7,4,2,0,0xa6,0xa6,0x11};
    for (unsigned taken = 0; taken < 2; ++taken) {
        sequence.bytes = conditional; sequence.size = sizeof(conditional);
        control.condition = taken ? 4 : 3; channel.octave = 4;
        CHECK(pe_akao_channel_start(&channel, &sequence, 0));
        CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
        CHECK(channel.octave == (taken ? 4 : 6));
    }
    PeAkaoChannel second = {0}; second.control = &control;
    CHECK(pe_akao_channel_start(&second, &sequence, 0));
    CHECK(second.control == channel.control); /* same bank across channels */
    memset(&control, 0, sizeof(control));
    control.tempo = 0x80000000;
    CHECK(!pe_akao_control_due(&control, 0, 0) && control.accumulator == 32768);
    CHECK(pe_akao_control_due(&control, 0, 0) && !control.accumulator);
    CHECK(pe_akao_control_due(&control, 128, 0) == 0 && control.accumulator == 16384);
    control.accumulator = 0;
    CHECK(pe_akao_control_due(&control, 64, 0) == 0 && control.accumulator == 49152);
    CHECK(pe_akao_control_due(&control, 0, 1) && control.accumulator == 16384);
    control.ticks_per_beat = control.beats_per_measure = 2;
    for (unsigned i = 0; i < 4; ++i) pe_akao_control_step(&control);
    CHECK(control.measure == 1 && !control.tick && !control.beat);
    channel.control = NULL;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_ERROR && !channel.pc);
    CHECK(strstr(channel.error, "Missing shared AKAO control"));
    const uint8_t channel_slides[] = {0xa9,4,0, 0xab,4,0, 0xfc,0x12,4,0, 0x04,0xa0};
    sequence.bytes = channel_slides; sequence.size = sizeof(channel_slides);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.expression_duration == 4 && channel.volume_duration == 4 && channel.pan_slide_duration == 4);
    CHECK(channel.volume_delta == 0xe040 && channel.pan_delta == 0xf000);
    for (unsigned i = 0; i < 4; ++i) pe_akao_channel_slides(&channel);
    CHECK(!channel.expression_value && !channel.volume_base && channel.panpot == 0x4000);
    CHECK(!channel.expression_duration && !channel.volume_duration && !channel.pan_slide_duration);
    CHECK(channel.update_flags == 3);
    const uint8_t long_pan[] = {0xab,0,0xc0,0x11};
    sequence.bytes = long_pan; sequence.size = sizeof(long_pan);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.pan_slide_duration == 256 && channel.pan_delta == 0xff80);
    for (unsigned i = 0; i < 256; ++i) pe_akao_channel_slides(&channel);
    CHECK(!channel.panpot && !channel.pan_slide_duration);
    const uint8_t override[] = {0xfc,0x12,4,0,0xa3,20,0x11};
    sequence.bytes = override; sequence.size = sizeof(override);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.volume_base == 0x1400 && channel.volume_duration == 4 && channel.volume_delta == 0xe040);
    pe_akao_channel_slides(&channel);
    CHECK(channel.volume_base == 0xf440); /* A3 does not cancel the old slide */
    memset(&channel, 0, sizeof(channel));
    channel.volume_duration = 2; channel.volume_delta = 1;
    pe_akao_channel_slides(&channel);
    CHECK(channel.volume_base == 1 && !channel.update_flags);
    channel.volume_duration = 1; channel.volume_base = 0; channel.volume_delta = 0x8000;
    pe_akao_channel_slides(&channel);
    CHECK(channel.volume_base == 0x8000 && !channel.update_flags); /* original 7F00 mask */
    channel.pan_slide_duration = 1; channel.panpot = 0; channel.pan_delta = 0x8000;
    pe_akao_channel_slides(&channel);
    CHECK(channel.panpot == 0x8000 && channel.update_flags == 3); /* FF00 mask */
    puts("AKAO channel: state commands, finite/nested loops, signed jumps, key-off and explicit errors passed");
    return 0;
}
