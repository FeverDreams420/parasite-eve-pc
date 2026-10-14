#include "akao_channel.h"
#include <stdlib.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
int main(void) {
    PeSpuBank *samples = calloc(1, sizeof(*samples)); CHECK(samples);
    samples->valid[0] = 1;
    for (unsigned i = 0; i < 12; ++i) samples->instruments[0].pitch[i] = 4096 + i * 10;
    uint16_t pitch;
    CHECK(pe_akao_pitch_period(samples, 0, 72, 0, &pitch) && pitch == 4096);
    CHECK(pe_akao_pitch_period(samples, 0, 73, 0, &pitch) && pitch == 4106);
    CHECK(pe_akao_pitch_period(samples, 0, 60, 0, &pitch) && pitch == 2048);
    CHECK(pe_akao_pitch_period(samples, 0, 84, 0, &pitch) && pitch == 8192);
    CHECK(pe_akao_pitch_period(samples, 0, 0, 0, &pitch) && pitch == 64);
    CHECK(pe_akao_pitch_period(samples, 0, 72, 1, &pitch) && pitch == 4128);
    CHECK(pe_akao_pitch_period(samples, 0, 72, 65535, &pitch) && pitch == 4064);
    CHECK(!pe_akao_pitch_period(samples, 1, 72, 0, &pitch));
    samples->instruments[0].pitch[0] = 0x80000000;
    CHECK(pe_akao_pitch_period(samples, 0, 72, 2, &pitch) && !pitch); /* wrapped product */
    samples->instruments[0].pitch[0] = 4096;
    PeAkaoControl control = {0};
    PeAkaoChannel channel = {0}; channel.control = &control; channel.voice_mask = 4;
    CHECK(pe_akao_channel_bind_samples(&channel, samples));
    channel.octave = 6; channel.last_note = 0;
    CHECK(pe_akao_channel_note_pitch(&channel));
    CHECK(channel.current_note == 72 && channel.pitch_base == 4096);
    CHECK(control.key_on_requests == 4 && control.allocated_voice_mask == 4);
    control.key_on_requests = 0;
    channel.last_note = 0x84;
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.pitch_base == 4096 && !control.key_on_requests);
    channel.last_note = 0x8f;
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.pitch_base == 4096);
    channel.last_note = 0; channel.transpose = 65535;
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.current_note == 72 && channel.pitch_base == 2103);
    channel.transpose = 0; channel.detune = 65535;
    CHECK(pe_akao_channel_note_pitch(&channel) && channel.pitch_base == 4064);
    channel.flags = 8;
    uint32_t previous = channel.pitch_base;
    CHECK(!pe_akao_channel_note_pitch(&channel) && channel.pitch_base == previous);
    CHECK(channel.error[0]);
    free(samples);
    puts("AKAO pitch: instrument rows, octave shifts, detune, transpose, ties/rests and explicit modes passed");
    return 0;
}
