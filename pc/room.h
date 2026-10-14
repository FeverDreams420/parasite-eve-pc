#ifndef PE_PC_ROOM_H
#define PE_PC_ROOM_H

#include "disc.h"

/* Table ends at 0x80094188, the documented PM handler array. The last
 * eight-byte slot is all zero, but is still included in the data audit. */
#define PE_USA_ROOM_SLOTS 450u
#define PE_USA_AUDIO_BANKS 250u
#define PE_AKAO_LFO_WORDS 708u

typedef struct PeAkaoGrammar {
    uint8_t lengths[96], extended_lengths[32];
    uint16_t note_lengths[11];
    uint16_t lfo_offsets[16], lfo_words[PE_AKAO_LFO_WORDS];
    uint8_t lfo_valid;
} PeAkaoGrammar;

/* Advance one original waveform sample, including its relative loop marker.
 * Cursor is a word offset; failed reads preserve cursor and output. */
int pe_akao_lfo_next(const PeAkaoGrammar *grammar, uint16_t *cursor, int16_t *sample);

typedef struct PeRoomRange {
    uint32_t start;
    uint16_t scratch_sectors, texture_sectors, room_sectors;
} PeRoomRange;

typedef struct PeRoomCatalog {
    PeDisc *disc; /* borrowed; must remain open */
    PeDiscFile archive;
    uint16_t sector_bounds[81]; /* resident/startup/UI data ranges */
    uint32_t audio_base;
    uint16_t audio_bounds[PE_USA_AUDIO_BANKS + 1];
    PeAkaoGrammar akao;
    PeRoomRange ranges[PE_USA_ROOM_SLOTS];
    char error[256];
} PeRoomCatalog;

typedef struct PeRoomBlob {
    uint8_t *bytes;
    size_t size;
    uint32_t directory[13]; /* decoded words, never cast to host pointers */
} PeRoomBlob;

typedef struct PeRoom {
    unsigned map;
    PeRoomBlob scratch, textures, data;
} PeRoom;

/* Read only data tables from the supplied USA executable inside the image.
 * No PS1 code runs, no extracted executable or linker addresses are needed. */
int pe_room_catalog_open(PeRoomCatalog *catalog, PeDisc *disc);
/* Returns an owned complete bank, without emulating CD queues/seeks. */
int pe_audio_bank_read(PeRoomCatalog *catalog, unsigned index, uint8_t **bytes, size_t *size);
/* Room starts zero-initialized. On failure the previous room is preserved. */
int pe_room_load(PeRoomCatalog *catalog, unsigned map, PeRoom *room);
/* Owned archive bank selected by the original resident sector boundaries.
 * Starts zeroed; failure preserves the old blob. */
int pe_sector_bank_load(PeRoomCatalog *catalog,unsigned index,PeRoomBlob *blob);
void pe_room_blob_close(PeRoomBlob *blob);
void pe_room_close(PeRoom *room);
/* Packed count/offset table, with explicit record stride and bounds checks. */
int pe_room_blob_table(const PeRoomBlob *blob, unsigned word, size_t stride,
                       const uint8_t **records, unsigned *count);

#endif
