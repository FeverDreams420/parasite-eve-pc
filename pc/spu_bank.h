#ifndef PE_PC_SPU_BANK_H
#define PE_PC_SPU_BANK_H

#include "room.h"

#define PE_SPU_RAM_SIZE (512u * 1024u)

typedef struct PeInstrument {
    uint32_t start, loop;
    uint8_t adsr[8]; /* attack/decay/sustain/release rates and mode bytes */
    uint32_t pitch[12];
} PeInstrument;

typedef struct PeSpuBank {
    uint8_t ram[PE_SPU_RAM_SIZE];
    PeInstrument instruments[256];
    uint8_t valid[256];
} PeSpuBank;

typedef struct PeSpuBankStats {
    uint32_t address, bytes;
    unsigned first, end;
} PeSpuBankStats;

/* Original Spu_UploadSampleBlock format. Validation precedes mutation;
 * existing RAM and instruments outside the uploaded range are preserved. */
int pe_spu_bank_upload(PeSpuBank *bank, const uint8_t *bytes, size_t size,
                        PeSpuBankStats *stats, char *error, size_t error_size);
int pe_spu_bank_load_startup(PeRoomCatalog *catalog, PeSpuBank *bank,
                             PeSpuBankStats *stats);

#endif
