#include "music.h"
#include <stdlib.h>
#include <string.h>

void pe_music_close(PeMusic *music) {
    if (!music) return;
    free(music->samples); free(music->bytes); free(music);
}

int pe_music_open(PeRoomCatalog *catalog, const PeRoom *room, uint16_t key,
                  const PeSpuBank *resident, const PeAkaoControl *initial,
                  PeMusic **music, char *error, size_t error_size) {
    const uint8_t *records; unsigned count;
    if (!resident || !initial || !pe_room_blob_table(&room->data, 12, 12, &records, &count)) {
        snprintf(error, error_size, "Missing music source/context"); return 0;
    }
    PeRoomMusic record = {0}; unsigned index;
    for (index = 0; index < count; ++index) {
        if ((unsigned)(records[index * 12 + 10] | (unsigned)records[index * 12 + 11] << 8) != key) continue;
        if (!pe_room_music(room, index, &record, error, error_size)) return 0;
        break;
    }
    if (index == count || record.unresolved_short_record) {
        snprintf(error, error_size, "Map %u: missing or unresolved music key %u", room->map, key); return 0;
    }
    PeMusic *next = calloc(1, sizeof(*next));
    uint8_t *bank_bytes = NULL; size_t bank_size = 0;
    if (!next) { snprintf(error, error_size, "Cannot allocate music owner"); return 0; }
    next->samples = malloc(sizeof(*next->samples));
    next->bytes = malloc(record.sequence.size);
    if (!next->samples || !next->bytes) { snprintf(error, error_size, "Cannot allocate music data"); goto failed; }
    memcpy(next->samples, resident, sizeof(*resident));
    memcpy(next->bytes, record.sequence.bytes, record.sequence.size);
    if (!pe_audio_bank_read(catalog, record.bank, &bank_bytes, &bank_size)) {
        snprintf(error, error_size, "%s", catalog->error); goto failed;
    }
    if (!pe_spu_bank_upload_rebased(next->samples, bank_bytes, bank_size, 0x8000, 0x20,
                                    &next->sample_stats, error, error_size)) goto failed;
    record.sequence.bytes = next->bytes;
    if (!pe_akao_player_start(&next->player, &record.sequence, &catalog->akao, initial) ||
        !pe_akao_player_bind_samples(&next->player, next->samples)) {
        snprintf(error, error_size, "Cannot initialize music player"); goto failed;
    }
    next->key = key; next->bank = record.bank;
    free(bank_bytes); pe_music_close(*music); *music = next;
    if (error_size) error[0] = 0;
    return 1;
failed:
    free(bank_bytes); pe_music_close(next); return 0;
}
