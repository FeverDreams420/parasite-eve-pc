#include "akao_channel.h"
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
int main(void) {
    PeAkaoGrammar grammar = {0}; grammar.lfo_valid = 1;
    grammar.lfo_words[0] = 32767; grammar.lfo_words[1] = 32769;
    grammar.lfo_words[4] = 65534; /* marker at 2 jumps back two words */
    uint16_t cursor = 0; int16_t sample = 0;
    for (unsigned i = 0; i < 1000; ++i) {
        CHECK(pe_akao_lfo_next(&grammar, &cursor, &sample));
        CHECK(sample == (i & 1 ? -32767 : 32767));
    }
    cursor = 2; sample = 99; grammar.lfo_words[4] = 65533;
    CHECK(!pe_akao_lfo_next(&grammar, &cursor, &sample) && cursor == 2 && sample == 99);
    grammar.lfo_words[4] = PE_AKAO_LFO_WORDS;
    CHECK(!pe_akao_lfo_next(&grammar, &cursor, &sample) && cursor == 2 && sample == 99);
    cursor = PE_AKAO_LFO_WORDS - 2;
    CHECK(!pe_akao_lfo_next(&grammar, &cursor, &sample) && sample == 99);
    grammar.lfo_valid = 0; cursor = 0;
    CHECK(!pe_akao_lfo_next(&grammar, &cursor, &sample) && !cursor && sample == 99);
    grammar.lfo_valid = 1; grammar.lfo_words[4] = 65534;
    PeAkaoChannel channel = {0}; channel.grammar = &grammar;
    const uint8_t bytes[] = {0xbc,2,0,0xbd,128,0};
    PeAkaoSequence sequence = {0}; sequence.bytes = bytes; sequence.size = sizeof(bytes);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 20) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.aux_lfo_duration == 2 && channel.aux_lfo_phase == 1 && (channel.flags & 4));
    channel.update_flags = 0;
    CHECK(pe_akao_channel_aux_lfo_step(&channel) && channel.aux_lfo_value == 63 && channel.update_flags == 3);
    channel.update_flags = 0;
    CHECK(pe_akao_channel_aux_lfo_step(&channel) && channel.aux_lfo_value == 63 && !channel.update_flags);
    CHECK(pe_akao_channel_aux_lfo_step(&channel) && channel.aux_lfo_value == (uint16_t)-64);
    channel.aux_lfo_phase = 1; channel.aux_lfo_cursor = PE_AKAO_LFO_WORDS - 1;
    CHECK(!pe_akao_channel_aux_lfo_step(&channel) && channel.aux_lfo_phase == 1);
    CHECK(channel.aux_lfo_cursor == PE_AKAO_LFO_WORDS - 1 && channel.aux_lfo_value == (uint16_t)-64);
    PeSpuBank *samples = calloc(1, sizeof(*samples)); CHECK(samples);
    samples->valid[0] = 1; samples->instruments[0].pitch[0] = 4096;
    PeAkaoControl control = {0}; channel.control = &control; channel.samples = samples;
    channel.error[0] = 0; channel.last_note = 0; channel.octave = 6; channel.pitch_slide_value = 0x12345678;
    CHECK(pe_akao_channel_note_pitch(&channel));
    CHECK(!channel.aux_lfo_cursor && channel.aux_lfo_phase == 1 && !channel.pitch_slide_value);
    channel.last_note = 0x84; channel.aux_lfo_cursor = 1; channel.aux_lfo_phase = 2;
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.aux_lfo_cursor == 1 && channel.aux_lfo_phase == 2);
    const uint8_t volume[] = {0xb8,2,2,0,0xb9,128,0,0xba,0};
    sequence.bytes = volume; sequence.size = sizeof(volume);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 20) == PE_AKAO_CHANNEL_NOTE);
    channel.expression_value = 0x10000000; channel.volume_base = 0x8000; channel.update_flags = 0;
    CHECK(pe_akao_channel_volume_lfo_step(&channel, 0) && channel.volume_lfo_counter == 1);
    CHECK(!channel.volume_lfo_value && channel.volume_lfo_phase == 1);
    CHECK(pe_akao_channel_volume_lfo_step(&channel, 0) && channel.volume_lfo_value == 4095);
    CHECK(channel.volume_lfo_phase == 2 && channel.update_flags == 3);
    CHECK(pe_akao_channel_volume_lfo_step(&channel, 0) && channel.volume_lfo_value == 4095);
    CHECK(pe_akao_channel_volume_lfo_step(&channel, 0) && channel.volume_lfo_value == (uint16_t)-4096);
    CHECK(pe_akao_channel_next(&channel, 20) == PE_AKAO_CHANNEL_NOTE);
    CHECK(!(channel.flags & 2) && !channel.volume_lfo_value && channel.volume_lfo_target == 0x8000);
    channel.flags |= 2; channel.volume_lfo_phase = 1; channel.volume_lfo_cursor = 0; channel.volume_lfo_counter = 9;
    CHECK(pe_akao_channel_volume_lfo_step(&channel, 1) && channel.volume_lfo_value == 4095);
    CHECK(channel.volume_lfo_counter == 9);
    const uint8_t pitch[] = {0xb4,2,0,0,0xb5,192,0,0xb6,0};
    sequence.bytes = pitch; sequence.size = sizeof(pitch);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0)); channel.pitch_base = 4096;
    CHECK(pe_akao_channel_next(&channel, 20) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.pitch_lfo_duration == 256 && channel.pitch_lfo_depth == 2048);
    channel.update_flags = 0;
    CHECK(pe_akao_channel_pitch_lfo_step(&channel, 0) && channel.pitch_lfo_counter == 1 && !channel.pitch_lfo_value);
    CHECK(pe_akao_channel_pitch_lfo_step(&channel, 0) && channel.pitch_lfo_value == 2046 && channel.update_flags == 0x10);
    channel.pitch_lfo_phase = 1;
    CHECK(pe_akao_channel_pitch_lfo_step(&channel, 0) && channel.pitch_lfo_value == (uint16_t)-1024);
    CHECK(pe_akao_channel_next(&channel, 20) == PE_AKAO_CHANNEL_NOTE);
    CHECK(!(channel.flags & 1) && !channel.pitch_lfo_value && channel.pitch_lfo_target == 0xc000);
    channel.pitch_lfo_target = 0x4000; pe_akao_channel_pitch_lfo_depth(&channel, 0);
    CHECK(channel.pitch_lfo_depth == 120);
    free(samples);
    puts("AKAO LFO data: signed samples, relative loops, auxiliary phase/modulation and bounds passed");
    return 0;
}
