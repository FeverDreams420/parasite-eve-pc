#include "pe1/akao/tick.h"
#include "pe1/akao/file_header.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); return 1; } } while (0)

int main(void)
{
    AkaoTickBank banks[2] = {0};
    AkaoTickTrack tracks[48] = {0};
    unsigned char scripts[48][8] = {{0}};
    for (unsigned i = 0; i < 2; ++i) {
        banks[i].bank.sequence_base = scripts[i];
        banks[i].bank.tempo = 0x12345678u + i;
        banks[i].bank.pitch_current = 0x10203040 + i;
        banks[i].bank.volume = 0x55667788u + i;
        banks[i].bank.volume_slide_duration = 19 + i;
        banks[i].bank.measure = 61 + i;
        CHECK(banks[i].timing.sequence_base == scripts[i]);
        CHECK(banks[i].timing.tempo.value == 0x12345678u + i);
        CHECK(banks[i].timing.pitch_integer == 0x1020);
        CHECK(banks[i].timing.volume == 0x55667788u + i);
        CHECK(banks[i].timing.volume_slide_duration == 19 + i);
        CHECK(banks[i].timing.measure == 61 + i);
        banks[i].timing.volume += 23;
        CHECK(banks[i].bank.volume == 0x55667788u + i + 23);
    }
    for (unsigned i = 0; i < 48; ++i) {
        tracks[i].track.pc = scripts[i];
        tracks[i].track.call_stack[3] = scripts[i] + 7;
        tracks[i].track.key_on_mask = 1u << (i % 24);
        tracks[i].track.field_50_duration = 0x1234;
        tracks[i].track.field_52_duration = 0x5678;
        tracks[i].track.parent_track_id = i;
        tracks[i].track.note_length = 73;
        tracks[i].track.pan_duration = 71;
        CHECK(tracks[i].timing.key_on_mask == 1u << (i % 24));
        CHECK(tracks[i].timing.tick_count == 0x56781234u);
        CHECK(tracks[i].timing.parent_track_id == i);
        --tracks[i].timing.note_length;
        --tracks[i].timing.gate_length;
        CHECK(tracks[i].track.note_length == 72);
        CHECK(tracks[i].track.pan_duration == 70);
        CHECK(tracks[i].track.pc == scripts[i]);
        CHECK(tracks[i].track.call_stack[3] == scripts[i] + 7);
    }
    CHECK(sizeof(AkaoInstrument) == 64);
    CHECK(sizeof(AkaoFilePrefix) == 4);
    puts("original AKAO: native bank/track timing views preserve pointers and shared state");
    return 0;
}
