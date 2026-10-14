#ifndef PE_PC_MUSIC_H
#define PE_PC_MUSIC_H
#include "akao_player.h"
typedef struct PeMusic {
    PeAkaoPlayer player;
    PeSpuBank *samples;
    uint8_t *bytes;
    uint16_t key, bank;
    PeSpuBankStats sample_stats;
} PeMusic;
/* Owned primary sequence and samples, independent of room/disc lifetime.
 * On failure the previous music remains intact. Initial control is explicit;
 * nested banks, script command dispatch and synthesis are separate work. */
int pe_music_open(PeRoomCatalog *catalog, const PeRoom *room, uint16_t key,
                  const PeSpuBank *resident, const PeAkaoControl *initial,
                  PeMusic **music, char *error, size_t error_size);
void pe_music_close(PeMusic *music);
#endif
