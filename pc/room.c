#include "room.h"

#include <stdlib.h>
#include <string.h>

/* USA SLUS_006.62 data layout, documented in naming_review_main2.csv.
 * These are file-data coordinates translated through the EXE header, not
 * native addresses, function pointers or requirements on the host linker. */
#define ROOM_TABLE_VA UINT32_C(0x80093378)
#define SECTOR_TABLE_VA UINT32_C(0x800930d8)
#define AUDIO_TABLE_VA UINT32_C(0x8009317c)

static uint32_t read32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int disc_error(PeRoomCatalog *catalog) {
    snprintf(catalog->error, sizeof(catalog->error), "%s", catalog->disc->error);
    return 0;
}

int pe_room_catalog_open(PeRoomCatalog *catalog, PeDisc *disc) {
    memset(catalog, 0, sizeof(*catalog));
    catalog->disc = disc;
    PeDiscFile executable;
    uint8_t header[0x800], table[PE_USA_ROOM_SLOTS * 8], bounds[162];
    uint8_t audio[4 + (PE_USA_AUDIO_BANKS + 1) * 2];
    if (!pe_disc_find(disc, "PE.IMG", &catalog->archive) ||
        !pe_disc_find(disc, "SLUS_006.62", &executable) ||
        !pe_disc_read(disc, &executable, 0, header, sizeof(header)))
        return disc_error(catalog);
    uint32_t base = read32(header + 0x18), size = read32(header + 0x1c);
    if (memcmp(header, "PS-X EXE", 8) || base > SECTOR_TABLE_VA ||
        size < sizeof(table) || ROOM_TABLE_VA - base > size - sizeof(table) ||
        (uint64_t)size + 0x800 > executable.size) {
        snprintf(catalog->error, sizeof(catalog->error), "Unsupported USA room table layout");
        return 0;
    }
    if (!pe_disc_read(disc, &executable, (uint64_t)ROOM_TABLE_VA - base + 0x800,
                      table, sizeof(table))) return disc_error(catalog);
    if (!pe_disc_read(disc, &executable, (uint64_t)SECTOR_TABLE_VA - base + 0x800,
                      bounds, sizeof(bounds))) return disc_error(catalog);
    for (unsigned i = 0; i < 81; ++i)
        catalog->sector_bounds[i] = (uint16_t)bounds[i * 2] |
                                    (uint16_t)bounds[i * 2 + 1] << 8;
    if (!pe_disc_read(disc, &executable, (uint64_t)AUDIO_TABLE_VA - base + 0x800,
                      audio, sizeof(audio))) return disc_error(catalog);
    catalog->audio_base = read32(audio);
    for (unsigned i = 0; i <= PE_USA_AUDIO_BANKS; ++i) {
        catalog->audio_bounds[i] = (uint16_t)audio[4 + i * 2] |
                                  (uint16_t)audio[5 + i * 2] << 8;
        if ((i && catalog->audio_bounds[i] < catalog->audio_bounds[i - 1]) ||
            ((uint64_t)catalog->audio_base + catalog->audio_bounds[i]) * 2048 > catalog->archive.size) {
            snprintf(catalog->error, sizeof(catalog->error), "Invalid audio bank boundary %u", i);
            return 0;
        }
    }
    for (unsigned i = 0; i < PE_USA_ROOM_SLOTS; ++i) {
        PeRoomRange *range = &catalog->ranges[i];
        range->start = read32(table + i * 8);
        uint32_t counts = read32(table + i * 8 + 4);
        range->scratch_sectors = (uint16_t)(counts & 255);
        range->texture_sectors = (uint16_t)((counts >> 8) & 4095);
        range->room_sectors = (uint16_t)(counts >> 20);
        uint64_t end = (uint64_t)range->start + range->scratch_sectors +
                       range->texture_sectors + range->room_sectors;
        if (end * 2048 > catalog->archive.size) {
            snprintf(catalog->error, sizeof(catalog->error), "Map %u exceeds PE.IMG", i + 1);
            return 0;
        }
    }
    return 1;
}

int pe_audio_bank_read(PeRoomCatalog *catalog, unsigned index, uint8_t **bytes, size_t *size) {
    if (!catalog->disc || !catalog->disc->stream) {
        snprintf(catalog->error, sizeof(catalog->error), "Audio source is not mounted");
        return 0;
    }
    if (index >= PE_USA_AUDIO_BANKS || catalog->audio_bounds[index + 1] <= catalog->audio_bounds[index]) {
        snprintf(catalog->error, sizeof(catalog->error), "Audio bank %u is missing", index);
        return 0;
    }
    size_t length = (size_t)(catalog->audio_bounds[index + 1] - catalog->audio_bounds[index]) * 2048;
    uint8_t *next = malloc(length);
    if (!next) {
        snprintf(catalog->error, sizeof(catalog->error), "Cannot allocate audio bank");
        return 0;
    }
    uint64_t offset = ((uint64_t)catalog->audio_base + catalog->audio_bounds[index]) * 2048;
    if (!pe_disc_read(catalog->disc, &catalog->archive, offset, next, length)) {
        free(next);
        return disc_error(catalog);
    }
    *bytes = next;
    *size = length;
    catalog->error[0] = 0;
    return 1;
}

void pe_room_close(PeRoom *room) {
    free(room->scratch.bytes);
    free(room->textures.bytes);
    free(room->data.bytes);
    memset(room, 0, sizeof(*room));
}

int pe_room_blob_table(const PeRoomBlob *blob, unsigned word, size_t stride,
                       const uint8_t **records, unsigned *count) {
    if (word >= 13 || !stride || !blob->bytes) return 0;
    uint32_t packed = blob->directory[word];
    size_t offset = packed & UINT32_C(0x3fffff);
    unsigned length = packed >> 22;
    if (offset > blob->size || length > (blob->size - offset) / stride) return 0;
    *records = blob->bytes + offset;
    *count = length;
    return 1;
}

static int load_blob(PeRoomCatalog *catalog, uint32_t sector, unsigned sectors,
                      PeRoomBlob *blob) {
    size_t size = (size_t)sectors * 2048;
    if (!sectors) return 0;
    blob->bytes = malloc(size);
    if (!blob->bytes) {
        snprintf(catalog->error, sizeof(catalog->error), "Cannot allocate room assets");
        return 0;
    }
    if (!pe_disc_read(catalog->disc, &catalog->archive, (uint64_t)sector * 2048,
                      blob->bytes, size)) return disc_error(catalog);
    uint32_t declared = read32(blob->bytes), directory = read32(blob->bytes + 4);
    if (declared < 8 || declared > size || directory > declared ||
        declared - directory < sizeof(blob->directory)) goto malformed;
    blob->size = declared;
    for (unsigned i = 0; i < 13; ++i)
        blob->directory[i] = read32(blob->bytes + directory + i * 4);
    /* Task roots/data/aux/slots and streams use eight-byte records; TIM
     * uploads twenty-byte records; scripts/banks/audio twelve-byte records.
     * The old task-root casts to SceneRoomRecord only read its +4 payload. */
    const unsigned words[] = {1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    for (unsigned i = 0; i < sizeof(words) / sizeof(words[0]); ++i) {
        const uint8_t *records;
        unsigned count, word = words[i];
        size_t stride = word == 10 ? 20 : word >= 5 && word <= 9 ? 8 : 12;
        if (!pe_room_blob_table(blob, word, stride,
                                 &records, &count)) goto malformed;
    }
    return 1;
malformed:
    snprintf(catalog->error, sizeof(catalog->error), "Malformed room directory at sector %u", sector);
    return 0;
}

int pe_room_load(PeRoomCatalog *catalog, unsigned map, PeRoom *room) {
    if (!map || map > PE_USA_ROOM_SLOTS || !catalog->disc || !catalog->disc->stream) {
        snprintf(catalog->error, sizeof(catalog->error), "Invalid room source or map %u", map);
        return 0;
    }
    const PeRoomRange *range = &catalog->ranges[map - 1];
    if (!range->scratch_sectors || !range->texture_sectors || !range->room_sectors) {
        snprintf(catalog->error, sizeof(catalog->error), "Map %u has no room assets", map);
        return 0;
    }
    PeRoom next = {0};
    next.map = map;
    uint32_t sector = range->start;
    if (!load_blob(catalog, sector, range->scratch_sectors, &next.scratch)) goto failed;
    sector += range->scratch_sectors;
    if (!load_blob(catalog, sector, range->texture_sectors, &next.textures)) goto failed;
    sector += range->texture_sectors;
    if (!load_blob(catalog, sector, range->room_sectors, &next.data)) goto failed;
    pe_room_close(room);
    *room = next;
    catalog->error[0] = 0;
    return 1;
failed:
    pe_room_close(&next);
    return 0;
}
