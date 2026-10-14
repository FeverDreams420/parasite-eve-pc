#include "disc.h"

#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
#define pe_seek _fseeki64
#define pe_tell _ftelli64
#else
#define pe_seek fseek
#define pe_tell ftell
#endif

static int fail(PeDisc *disc, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(disc->error, sizeof(disc->error), format, args);
    va_end(args);
    return 0;
}

static uint32_t le32(const unsigned char *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int read_image(PeDisc *disc, uint64_t offset, void *data, size_t size) {
    if (offset > disc->image_size || size > disc->image_size - offset)
        return fail(disc, "Read exceeds the disc image");
    if (pe_seek(disc->stream, (int64_t)offset, SEEK_SET) != 0 ||
        fread(data, 1, size, disc->stream) != size)
        return fail(disc, "Cannot read the disc image");
    return 1;
}

static int sector_payload(PeDisc *disc, uint32_t sector, void *data) {
    return read_image(disc, (uint64_t)sector * disc->sector_size +
                      disc->payload_offset, data, 2048);
}

static int decode_record(PeDisc *disc, const unsigned char *record,
                         size_t available, PeDiscFile *file) {
    unsigned size = record[0];
    if (size < 34 || size > available || record[32] > size - 33)
        return fail(disc, "Malformed ISO9660 directory record");
    if (record[25] & 0x80)
        return fail(disc, "Multi-extent ISO9660 files are not supported");
    file->sector = le32(record + 2);
    file->size = le32(record + 10);
    file->directory = (record[25] & 2) != 0;
    uint64_t end = (uint64_t)file->sector + ((uint64_t)file->size + 2047) / 2048;
    if (end > disc->image_size / disc->sector_size)
        return fail(disc, "ISO9660 file extent exceeds the disc image");
    return 1;
}

int pe_disc_open(PeDisc *disc, const char *path) {
    unsigned char sector[2048];
    memset(disc, 0, sizeof(*disc));
    disc->stream = fopen(path, "rb");
    if (!disc->stream) return fail(disc, "Cannot open disc image: %s", path);
    if (pe_seek(disc->stream, 0, SEEK_END) != 0 || pe_tell(disc->stream) < 0) {
        pe_disc_close(disc);
        return fail(disc, "Cannot measure disc image");
    }
    disc->image_size = (uint64_t)pe_tell(disc->stream);
    const uint32_t sizes[] = {2352, 2048};
    for (unsigned kind = 0; kind < 2; ++kind) {
        disc->sector_size = sizes[kind];
        disc->payload_offset = kind == 0 ? 24 : 0;
        if (!sector_payload(disc, 16, sector)) continue;
        if (sector[0] != 1 || memcmp(sector + 1, "CD001", 5) != 0 || sector[6] != 1)
            continue;
        if (sector[128] != 0 || sector[129] != 8) break;
        if (!decode_record(disc, sector + 156, sizeof(sector) - 156, &disc->root))
            break;
        if (!disc->root.directory) break;
        disc->error[0] = 0;
        return 1;
    }
    pe_disc_close(disc);
    return fail(disc, "Expected a MODE2/2352 BIN or 2048-byte ISO9660 image");
}

void pe_disc_close(PeDisc *disc) {
    if (disc->stream) fclose(disc->stream);
    disc->stream = NULL;
}

static int same_name(const unsigned char *name, size_t length,
                     const char *wanted, size_t wanted_length) {
    size_t stem = 0;
    while (stem < length && name[stem] != ';') ++stem;
    if (stem != wanted_length) return 0;
    for (size_t i = 0; i < stem; ++i)
        if (toupper(name[i]) != toupper((unsigned char)wanted[i])) return 0;
    return 1;
}

static int find_child(PeDisc *disc, const PeDiscFile *directory,
                      const char *name, size_t name_length, PeDiscFile *file) {
    unsigned char sector[2048];
    for (uint64_t offset = 0; offset < directory->size; offset += 2048) {
        if (!sector_payload(disc, directory->sector + (uint32_t)(offset / 2048), sector))
            return 0;
        size_t available = directory->size - offset < 2048 ?
                           (size_t)(directory->size - offset) : 2048;
        size_t cursor = 0;
        while (cursor < available && sector[cursor]) {
            const unsigned char *entry = sector + cursor;
            PeDiscFile candidate;
            if (!decode_record(disc, entry, available - cursor, &candidate)) return 0;
            if (same_name(entry + 33, entry[32], name, name_length)) {
                *file = candidate;
                return 1;
            }
            cursor += entry[0];
        }
    }
    return fail(disc, "Disc file not found: %.*s", (int)name_length, name);
}

int pe_disc_find(PeDisc *disc, const char *path, PeDiscFile *file) {
    PeDiscFile current = disc->root;
    while (*path == '/' || *path == '\\') ++path;
    while (*path) {
        const char *end = path;
        while (*end && *end != '/' && *end != '\\') ++end;
        const char *version = memchr(path, ';', (size_t)(end - path));
        size_t length = (size_t)((version ? version : end) - path);
        if (!current.directory) return fail(disc, "Path crosses a regular file");
        if (!length || !find_child(disc, &current, path, length, &current)) return 0;
        path = end;
        while (*path == '/' || *path == '\\') ++path;
    }
    *file = current;
    return 1;
}

int pe_disc_read(PeDisc *disc, const PeDiscFile *file, uint64_t offset,
                 void *data, size_t size) {
    unsigned char sector[2048];
    unsigned char *output = data;
    if (offset > file->size || size > file->size - offset)
        return fail(disc, "Read exceeds the file extent");
    while (size) {
        size_t start = (size_t)(offset % 2048);
        size_t count = size < 2048 - start ? size : 2048 - start;
        if (!sector_payload(disc, file->sector + (uint32_t)(offset / 2048), sector))
            return 0;
        memcpy(output, sector + start, count);
        offset += count;
        output += count;
        size -= count;
    }
    return 1;
}

int pe_disc_read_raw(PeDisc *disc, const PeDiscFile *file, uint32_t sector,
                     void *data) {
    if (disc->sector_size != 2352)
        return fail(disc, "Movie audio requires a raw MODE2/2352 disc image");
    if (sector >= ((uint64_t)file->size + 2047) / 2048)
        return fail(disc, "Raw sector exceeds the file extent");
    return read_image(disc, ((uint64_t)file->sector + sector) * 2352, data, 2352);
}
