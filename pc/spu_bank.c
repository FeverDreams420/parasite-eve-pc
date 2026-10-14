#include "spu_bank.h"
#include <stdlib.h>
#include <string.h>

static uint32_t read32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static void write32(uint8_t *p, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(value >> (i * 8));
}

int pe_spu_bank_upload_rebased(PeSpuBank *bank, const uint8_t *bytes, size_t size,
                                uint32_t address, unsigned first, PeSpuBankStats *stats,
                                char *error, size_t error_size) {
    if (size < 128 || memcmp(bytes, "AKAO", 4)) goto malformed;
    unsigned start = read32(bytes + 24), end = read32(bytes + 28);
    if (!end) end = 256;
    if (start >= end || end > 256 || first >= 256 || end - start > 256 - first ||
        (size_t)(end - start) * 64 > size - 64) goto malformed;
    unsigned count = end - start;
    uint32_t payload = read32(bytes + 20);
    size_t required = 64 + (size_t)count * 64;
    if (address > PE_SPU_RAM_SIZE || (address & 7) || payload > PE_SPU_RAM_SIZE - address ||
        payload > size - required) goto malformed;
    required += payload;
    int64_t delta = (int64_t)address - read32(bytes + 64);
    /* Validate every relocated address before allocating or changing RAM. */
    for (unsigned i = 0; i < count; ++i) {
        for (unsigned field = 0; field < 2; ++field) {
            int64_t relocated = (int64_t)read32(bytes + 64 + i * 64 + field * 4) + delta;
            if (relocated < 0 || relocated >= PE_SPU_RAM_SIZE) goto malformed;
        }
    }
    uint8_t *copy = malloc(required);
    if (!copy) {
        snprintf(error, error_size, "Cannot allocate relocated sample bank");
        return 0;
    }
    memcpy(copy, bytes, required);
    write32(copy + 16, address);
    write32(copy + 24, first);
    write32(copy + 28, first + count);
    for (unsigned i = 0; i < count; ++i)
        for (unsigned field = 0; field < 2; ++field)
            write32(copy + 64 + i * 64 + field * 4,
                    (uint32_t)((int64_t)read32(bytes + 64 + i * 64 + field * 4) + delta));
    int ok = pe_spu_bank_upload(bank, copy, required, stats, error, error_size);
    free(copy);
    return ok;
malformed:
    snprintf(error, error_size, "Invalid relocated sample bank");
    return 0;
}

int pe_spu_bank_upload(PeSpuBank *bank, const uint8_t *bytes, size_t size,
                        PeSpuBankStats *stats, char *error, size_t error_size) {
    if (size < 64 || memcmp(bytes, "AKAO", 4)) goto malformed;
    PeSpuBankStats next;
    next.address = read32(bytes + 16);
    next.bytes = read32(bytes + 20);
    next.first = read32(bytes + 24);
    next.end = read32(bytes + 28);
    if (!next.end) next.end = 256;
    if (next.first > next.end || next.end > 256 || next.address > PE_SPU_RAM_SIZE ||
        next.bytes > PE_SPU_RAM_SIZE - next.address || (next.address & 7)) goto malformed;
    size_t records = (next.end - next.first) * 64u;
    if (records > size - 64 || next.bytes > size - 64 - records) goto malformed;
    for (unsigned i = next.first; i < next.end; ++i) {
        const uint8_t *record = bytes + 64 + (i - next.first) * 64;
        if (read32(record) >= PE_SPU_RAM_SIZE || read32(record + 4) >= PE_SPU_RAM_SIZE)
            goto malformed;
    }
    /* No allocation or fallible operation after validation. */
    memcpy(bank->ram + next.address, bytes + 64 + records, next.bytes);
    for (unsigned i = next.first; i < next.end; ++i) {
        const uint8_t *record = bytes + 64 + (i - next.first) * 64;
        PeInstrument *instrument = &bank->instruments[i];
        instrument->start = read32(record);
        instrument->loop = read32(record + 4);
        memcpy(instrument->adsr, record + 8, 8);
        for (unsigned note = 0; note < 12; ++note)
            instrument->pitch[note] = read32(record + 16 + note * 4);
        bank->valid[i] = 1;
    }
    if (stats) *stats = next;
    if (error_size) error[0] = 0;
    return 1;
malformed:
    snprintf(error, error_size, "Malformed AKAO sample bank or SPU address");
    return 0;
}

int pe_spu_bank_load_startup(PeRoomCatalog *catalog, PeSpuBank *bank,
                             PeSpuBankStats *stats) {
    unsigned start = catalog->sector_bounds[6], end = catalog->sector_bounds[7];
    if (end <= start) {
        snprintf(catalog->error, sizeof(catalog->error), "Invalid startup sample sector range");
        return 0;
    }
    size_t size = (size_t)(end - start) * 2048;
    uint8_t *bytes = malloc(size);
    if (!bytes) {
        snprintf(catalog->error, sizeof(catalog->error), "Cannot allocate startup sample bank");
        return 0;
    }
    int ok = pe_disc_read(catalog->disc, &catalog->archive, (uint64_t)start * 2048,
                          bytes, size);
    if (ok) ok = pe_spu_bank_upload(bank, bytes, size, stats, catalog->error, sizeof(catalog->error));
    else snprintf(catalog->error, sizeof(catalog->error), "%s", catalog->disc->error);
    free(bytes);
    return ok;
}
