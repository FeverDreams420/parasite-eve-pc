#include "akao_channel.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
int main(void) {
    uint8_t bytes[64] = {0xfc,0x14,12,0,0};
    const uint8_t ranges[] = {1,0,60,11,12,13,14,0, 32,50,127,21,22,23,24,0, 255,255,255,0,0,0,0,0};
    memcpy(bytes + 16, ranges, sizeof(ranges));
    PeSpuBank *samples = calloc(1, sizeof(*samples)); CHECK(samples);
    samples->valid[0] = samples->valid[1] = samples->valid[32] = samples->valid[80] = 1;
    samples->instruments[1].pitch[0] = 4096;
    samples->instruments[32].pitch[0] = 8192;
    samples->instruments[80].pitch[0] = 12288;
    PeAkaoSequence sequence = {0}; sequence.bytes = bytes; sequence.size = sizeof(bytes);
    PeAkaoControl control = {0}; PeAkaoChannel channel = {0};
    channel.samples = samples; channel.control = &control;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 10) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.note_range_target == 16 && channel.instrument == 255 && !channel.current_note);
    channel.octave = 4;
    CHECK(pe_akao_channel_note_pitch(&channel));
    CHECK(channel.instrument == 1 && channel.current_note == 48 && channel.pitch_base == 1024);
    CHECK(channel.adsr[0] == 11 && channel.adsr[3] == 12 && channel.adsr[6] == 13 && channel.adsr[4] == 14);
    channel.octave = 5; channel.last_note = 11; /* ascending 61 switches */
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.instrument == 32);
    channel.last_note = 0; channel.octave = 4; /* descending below 50 switches back */
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.instrument == 1);
    channel.current_note = 70; channel.instrument = 32;
    CHECK(pe_akao_channel_select_note_range(&channel, 55) && channel.instrument == 32);
    channel.current_note = 40; channel.instrument = 32; /* neighbouring instrument retained */
    CHECK(pe_akao_channel_select_note_range(&channel, 55) && channel.instrument == 32);
    channel.instrument = 255; channel.current_note = 0; control.status_flags = 0x100;
    CHECK(pe_akao_channel_select_note_range(&channel, 72) && channel.instrument == 80);
    CHECK(channel.adsr[4] == 24);
    channel.current_note = 0; channel.instrument = 255;
    sequence.size = 32; channel.sequence = sequence; /* no complete sentinel */
    CHECK(!pe_akao_channel_select_note_range(&channel, 72) && channel.instrument == 255);
    sequence.size = sizeof(bytes); channel.sequence = sequence;
    channel.current_note = 72;
    CHECK(pe_akao_channel_select_note_range(&channel, 72) && channel.instrument == 255);
    channel.instrument = 32; channel.current_note = 60; channel.last_note = 0;
    channel.octave = 6; channel.tremolo_phase = 2; control.key_on_requests = 0;
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.instrument == 32 && !control.key_on_requests);
    const uint8_t backward[] = {1,0,127,1,2,3,4,0,255,255,255,0,0,0,0,0,0xfc,0x14,0xec,0xff,0};
    sequence.bytes = backward; sequence.size = sizeof(backward); sequence.channels[0] = 16;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 10) == PE_AKAO_CHANNEL_NOTE && !channel.note_range_target);
    const uint8_t invalid[] = {0xfc,0x14,0,0x80};
    sequence.bytes = invalid; sequence.size = sizeof(invalid); sequence.channels[0] = 0;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 10) == PE_AKAO_CHANNEL_ERROR && !channel.pc && channel.instrument == 0);
    free(samples);
    puts("AKAO note ranges: relative table, ascending/descending hysteresis, ADSR and bank offsets passed");
    return 0;
}
