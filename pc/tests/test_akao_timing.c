#include "akao_channel.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
int main(void) {
    PeAkaoGrammar grammar = {0};
    const uint16_t lengths[] = {192,96,48,24,12,6,3,32,16,8,4};
    memcpy(grammar.note_lengths, lengths, sizeof(lengths));
    grammar.lengths[1] = 2; /* instrument command can be skipped in lookahead */
    PeAkaoChannel channel = {0}; channel.grammar = &grammar;
    const uint8_t tie[] = {0xa1,5,0x84};
    channel.sequence.bytes = tie; channel.sequence.size = sizeof(tie);
    uint8_t period;
    CHECK(pe_akao_channel_peek(&channel, 100, &period) && period == 0x84);
    CHECK(!channel.pc && !channel.commands && !channel.octave);
    CHECK(pe_akao_channel_begin_note(&channel, 100));
    CHECK(channel.note_length == 192 && channel.pan_duration == 192);
    const uint8_t rest[] = {0x8f};
    channel.sequence.bytes = rest; channel.sequence.size = sizeof(rest);
    channel.last_note = 1; channel.note_length = 0;
    channel.tremolo_phase = 5; channel.tremolo_duration = 19;
    CHECK(pe_akao_channel_begin_note(&channel, 100));
    CHECK(channel.note_length == 96 && channel.pan_duration == 94 && (channel.flags & 0x40));
    CHECK(!channel.tremolo_phase && !channel.tremolo_duration);
    const uint8_t key[] = {0x11};
    channel.sequence.bytes = key; channel.sequence.size = sizeof(key);
    channel.note_length = channel.pan_duration = 17;
    CHECK(pe_akao_channel_begin_note(&channel, 100));
    CHECK(channel.note_length == 17 && channel.pan_duration == 15 && !(channel.flags & 0x40));
    channel.fixed_note_length = 1;
    CHECK(pe_akao_channel_begin_note(&channel, 100));
    CHECK(channel.note_length == 1 && channel.pan_duration == 65535); /* original u16 wrap */
    channel.fixed_note_length = 0;
    const uint8_t zero_loop[] = {0xc9,0,0xa0,0x11};
    channel.sequence.bytes = zero_loop; channel.sequence.size = sizeof(zero_loop);
    channel.loop_index = 1; channel.repeats[1] = 255; channel.loop_points[1] = 3;
    CHECK(pe_akao_channel_peek(&channel, 100, &period) && period == 0x11);
    CHECK(channel.loop_index == 1 && channel.repeats[1] == 255 && !channel.pc);
    const uint8_t virtual_pop[] = {0xfc,8,1,0,0,0xc9,3,0xa0,0x84};
    channel.sequence.bytes = virtual_pop; channel.sequence.size = sizeof(virtual_pop);
    channel.repeats[1] = 0; channel.repeats[0] = 2; channel.loop_points[1] = 8;
    CHECK(pe_akao_channel_peek(&channel, 100, &period) && period == 0xa0);
    CHECK(channel.loop_index == 1 && channel.repeats[0] == 2);
    const uint8_t pending[] = {0xca};
    channel.sequence.bytes = pending; channel.sequence.size = sizeof(pending);
    channel.flags |= UINT32_C(1) << 21;
    channel.tremolo_phase = 5; channel.tremolo_duration = 12;
    CHECK(pe_akao_channel_peek(&channel, 100, &period) && period == 0xa0);
    CHECK(!channel.tremolo_phase && !channel.tremolo_duration);
    const uint8_t forever[] = {0xfc,6,0xfc,0xff};
    channel.sequence.bytes = forever; channel.sequence.size = sizeof(forever);
    CHECK(!pe_akao_channel_peek(&channel, 7, &period) && strstr(channel.error, "limit"));
    channel.error[0] = 0;
    const uint8_t truncated[] = {0xa1};
    channel.sequence.bytes = truncated; channel.sequence.size = sizeof(truncated);
    CHECK(!pe_akao_channel_peek(&channel, 100, &period));
    CHECK(!channel.pc);
    channel.error[0] = 0;
    const uint8_t timed[] = {0x04,0xa0}; /* 12 musical ticks, gate 10 */
    PeAkaoSequence sequence = {0}; sequence.bytes = timed; sequence.size = sizeof(timed);
    for (unsigned i = 0; i < 24; ++i) sequence.channels[i] = UINT32_MAX;
    sequence.channels[0] = 0;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    for (unsigned i = 0; i < 3; ++i) CHECK(pe_akao_channel_tick(&channel, 100) == PE_AKAO_CHANNEL_WAIT);
    CHECK(pe_akao_channel_tick(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.musical_ticks == 4 && channel.note_length == 12 && channel.pan_duration == 10);
    channel.key_off_requested = 0;
    for (unsigned i = 0; i < 9; ++i) CHECK(pe_akao_channel_tick(&channel, 100) == PE_AKAO_CHANNEL_WAIT);
    CHECK(!channel.key_off_requested);
    CHECK(pe_akao_channel_tick(&channel, 100) == PE_AKAO_CHANNEL_WAIT && channel.key_off_requested);
    CHECK(pe_akao_channel_tick(&channel, 100) == PE_AKAO_CHANNEL_WAIT);
    CHECK(pe_akao_channel_tick(&channel, 100) == PE_AKAO_CHANNEL_STOP);
    CHECK(channel.musical_ticks == 16);
    puts("AKAO timing: lookahead skips, ties/rests, gate lengths, virtual loops and bounds passed");
    return 0;
}
