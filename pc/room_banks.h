#ifndef PE_PC_ROOM_BANKS_H
#define PE_PC_ROOM_BANKS_H
#include "room.h"
typedef struct PeRoomSlice {
    const uint8_t *bytes;
    size_t size;
    uint32_t offset;
    uint8_t present;
} PeRoomSlice;
typedef struct PeRoomBanks {
    PeRoomSlice roots[10], rows[10][48];
    unsigned root_records, row_records;
} PeRoomBanks;
/* Borrow the owned room bytes. Preserve original record order/last override;
 * failure preserves the previous view. External entity banks and model/
 * animation decoding are not supplied by these room overrides. */
int pe_room_banks(const PeRoom *room, PeRoomBanks *banks, char *error, size_t error_size);
/* External entity layout: optional first root installs slot zero; area
 * overrides retain the primary root. Row source IDs index group zero.
 * Applies transactionally over existing bank views. */
int pe_entity_banks(const PeRoomBlob *blob,int install_root,PeRoomBanks *banks,char *error,size_t error_size);
#endif
