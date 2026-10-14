#ifndef PE_PC_AKAO_SEQUENCE_H
#define PE_PC_AKAO_SEQUENCE_H

#include "room.h"

typedef struct PeAkaoSequence {
    const uint8_t *bytes;
    size_t size;
    uint16_t id, parameter;
    uint32_t active, pending, yield;
    uint32_t channels[24]; /* byte offsets, UINT32_MAX for inactive voices */
} PeAkaoSequence;

typedef struct PeRoomMusic {
    uint16_t bank, key;
    unsigned flags;
    const uint8_t *payload;
    size_t payload_size;
    int unresolved_short_record; /* not playable by the sequence parser */
    PeAkaoSequence sequence;
} PeRoomMusic;

int pe_akao_sequence_open(const uint8_t *bytes, size_t size, PeAkaoSequence *sequence,
                           char *error, size_t error_size);
int pe_room_music(const PeRoom *room, unsigned index, PeRoomMusic *music,
                    char *error, size_t error_size);

#endif
