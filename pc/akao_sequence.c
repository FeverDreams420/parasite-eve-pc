#include "akao_sequence.h"
#include <string.h>

static uint16_t read16(const uint8_t *p) {
    return (uint16_t)p[0] | (uint16_t)p[1] << 8;
}
static uint32_t read32(const uint8_t *p) {
    return (uint32_t)read16(p) | (uint32_t)read16(p + 2) << 16;
}

int pe_akao_sequence_open(const uint8_t *bytes, size_t size, PeAkaoSequence *sequence,
                           char *error, size_t error_size) {
    if (size < 32 || memcmp(bytes, "AKAO", 4)) goto malformed;
    PeAkaoSequence next = {0};
    next.bytes = bytes;
    next.size = (size_t)read16(bytes + 6) + 16;
    if (next.size < 32 || next.size > size) goto malformed;
    next.id = read16(bytes + 4);
    next.parameter = read16(bytes + 8);
    next.active = read32(bytes + 16) & UINT32_C(0xffffff);
    next.pending = read32(bytes + 20) & UINT32_C(0xffffff);
    next.yield = read32(bytes + 24) & UINT32_C(0xffffff);
    size_t cursor = 32;
    for (unsigned voice = 0; voice < 24; ++voice) {
        next.channels[voice] = UINT32_MAX;
        if (!(next.active & (UINT32_C(1) << voice))) continue;
        if (cursor > next.size || next.size - cursor < 2) goto malformed;
        /* Akao_StepSequencerVoice reads an unsigned displacement, advances
         * the offset cursor by two, then adds it to that advanced cursor. */
        size_t target = cursor + 2 + read16(bytes + cursor);
        cursor += 2;
        if (target >= next.size) goto malformed;
        next.channels[voice] = (uint32_t)target;
    }
    *sequence = next;
    if (error_size) error[0] = 0;
    return 1;
malformed:
    snprintf(error, error_size, "Malformed AKAO sequence header or channel offset");
    return 0;
}

int pe_room_music(const PeRoom *room, unsigned index, PeRoomMusic *music,
                    char *error, size_t error_size) {
    const uint8_t *records;
    unsigned count;
    if (!pe_room_blob_table(&room->data, 12, 12, &records, &count) || index >= count) {
        snprintf(error, error_size, "Map %u: missing music record %u", room->map, index);
        return 0;
    }
    const uint8_t *record = records + index * 12;
    size_t size = read32(record) & UINT32_C(0xffffff);
    size_t offset = read32(record + 4) & UINT32_C(0xffffff);
    if (offset > room->data.size || size > room->data.size - offset) {
        snprintf(error, error_size, "Map %u: music payload exceeds room", room->map);
        return 0;
    }
    PeRoomMusic next = {0};
    next.bank = read16(record + 8);
    next.key = read16(record + 10);
    next.flags = record[3];
    next.payload = room->data.bytes + offset;
    next.payload_size = size;
    /* Some room records are ID-zero compact data, too short to contain the
     * four-word channel header. Preserve them explicitly for further porting,
     * rather than manufacture a channel mask or silently discard the record. */
    if (size >= 16 && !memcmp(next.payload, "AKAO", 4) && !read16(next.payload + 4) &&
        (size_t)read16(next.payload + 6) + 16 < 32 &&
        (size_t)read16(next.payload + 6) + 16 <= size) {
        next.unresolved_short_record = 1;
        *music = next;
        if (error_size) error[0] = 0;
        return 1;
    }
    if (!pe_akao_sequence_open(room->data.bytes + offset, size, &next.sequence, error, error_size)) return 0;
    *music = next;
    return 1;
}
