#include "akao_sequence.h"
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
static void word(uint8_t *p, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(value >> (i * 8));
}
int main(void) {
    uint8_t bytes[64] = {0};
    memcpy(bytes, "AKAO", 4);
    bytes[4] = 9; bytes[6] = 48; bytes[8] = 5;
    word(bytes + 16, 0xaa800001);
    word(bytes + 20, 0xbb800000);
    word(bytes + 24, 1);
    bytes[32] = 10; bytes[34] = 12;
    PeAkaoSequence sequence;
    char error[256];
    CHECK(pe_akao_sequence_open(bytes, sizeof(bytes), &sequence, error, sizeof(error)));
    CHECK(sequence.id == 9 && sequence.parameter == 5 && sequence.size == 64);
    CHECK(sequence.active == 0x800001 && sequence.pending == 0x800000 && sequence.yield == 1);
    CHECK(sequence.channels[0] == 44 && sequence.channels[23] == 48);
    CHECK(sequence.channels[1] == UINT32_MAX);
    for (size_t size = 0; size < sizeof(bytes); ++size)
        CHECK(!pe_akao_sequence_open(bytes, size, &sequence, error, sizeof(error)));
    bytes[34] = bytes[35] = 255;
    CHECK(!pe_akao_sequence_open(bytes, sizeof(bytes), &sequence, error, sizeof(error)));
    uint8_t room_bytes[128] = {0};
    PeRoom room = {0}; room.data.bytes = room_bytes; room.data.size = sizeof(room_bytes);
    room.data.directory[12] = (1u << 22) | 16;
    word(room_bytes + 16, 32); word(room_bytes + 20, 64);
    word(room_bytes + 24, (2u << 16) | 1);
    memcpy(room_bytes + 64, "AKAO", 4); room_bytes[70] = 11;
    PeRoomMusic music;
    CHECK(pe_room_music(&room, 0, &music, error, sizeof(error)));
    CHECK(music.unresolved_short_record && !music.sequence.bytes && !error[0]);
    CHECK(music.bank == 1 && music.key == 2 && music.payload_size == 32);
    room_bytes[64] = 'X';
    CHECK(!pe_room_music(&room, 0, &music, error, sizeof(error)));
    word(room_bytes + 20, 120);
    CHECK(!pe_room_music(&room, 0, &music, error, sizeof(error)));
    bytes[34] = 12; bytes[35] = 0;
    bytes[6] = 11;
    CHECK(!pe_akao_sequence_open(bytes, sizeof(bytes), &sequence, error, sizeof(error)));
    puts("AKAO sequences: sparse channels, post-word relative offsets, masks and truncations passed");
    return 0;
}
