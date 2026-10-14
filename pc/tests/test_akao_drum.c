#include "akao_channel.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
int main(void) {
    uint8_t bytes[96] = {0xfc,4,8,0,0,0xfc,5,0,0xa0};
    bytes[12] = 32; bytes[13] = 72; bytes[15] = 1; bytes[16] = 0xc0; bytes[17] = 1;
    PeSpuBank *samples = calloc(1, sizeof(*samples)); CHECK(samples);
    samples->valid[0] = samples->valid[32] = samples->valid[80] = 1;
    samples->instruments[32].start = 0x8000; samples->instruments[32].loop = 0x8010;
    samples->instruments[32].pitch[0] = 4096;
    samples->instruments[80].pitch[0] = 8192;
    /* Instrument zero's divisor is zero: drum selection must not invoke A1's ratio. */
    PeAkaoControl control = {0};
    PeAkaoSequence sequence = {0}; sequence.bytes = bytes; sequence.size = sizeof(bytes);
    for (unsigned i = 0; i < 24; ++i) sequence.channels[i] = UINT32_MAX;
    sequence.channels[0] = 0;
    PeAkaoChannel channel = {0}; channel.control = &control; channel.samples = samples;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK((channel.flags & 8) && channel.drum_target == 12 && channel.drum_volume == 0x3fff);
    channel.current_note = 123;
    CHECK(pe_akao_channel_note_pitch(&channel));
    CHECK(channel.instrument == 32 && channel.pitch_base == 4096 && channel.sample_start == 0x8000);
    CHECK(channel.expression_value == 0xfffc00 && !channel.panpot);
    CHECK(control.reverb_mask == 1 && control.key_on_requests == 1);
    CHECK(channel.current_note == 123 && channel.tremolo_counter == 123);
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE && !(channel.flags & 8));
    channel.octave = 6;
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.current_note == 72);
    control.status_flags = 0x100;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.instrument == 80 && channel.pitch_base == 8192);
    bytes[17] = 0;
    CHECK(pe_akao_channel_note_pitch(&channel) && !control.reverb_mask);
    channel.drum_target = 94;
    uint32_t old_pitch = channel.pitch_base;
    CHECK(!pe_akao_channel_note_pitch(&channel) && channel.pitch_base == old_pitch);
    channel.error[0] = 0;
    bytes[2] = 0xff; bytes[3] = 0x7f;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_ERROR);
    CHECK(!channel.pc && !(channel.flags & 8));
    free(samples);
    puts("AKAO drums: table offsets, sample selection, gain/pan/reverb, alternate bank and bounds passed");
    return 0;
}
