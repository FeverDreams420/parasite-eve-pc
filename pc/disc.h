#ifndef PE_PC_DISC_H
#define PE_PC_DISC_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct PeDiscFile {
    uint32_t sector;
    uint32_t size;
    int directory;
} PeDiscFile;

typedef struct PeDisc {
    FILE *stream;
    uint64_t image_size;
    uint64_t track_offset;
    uint32_t sector_size;
    uint32_t payload_offset;
    PeDiscFile root;
    char error[192];
} PeDisc;

/* Read-only ISO9660 access to ISO, MODE2/2352 BIN or a single-track CUE.
 * CUE FILE paths are relative to the descriptor, INDEX 01 sets sector zero. */
int pe_disc_open(PeDisc *disc, const char *path);
void pe_disc_close(PeDisc *disc);
int pe_disc_find(PeDisc *disc, const char *path, PeDiscFile *file);
int pe_disc_read(PeDisc *disc, const PeDiscFile *file, uint64_t offset,
                 void *data, size_t size);
/* Full raw sectors preserve XA audio and STR headers for the movie decoder. */
int pe_disc_read_raw(PeDisc *disc, const PeDiscFile *file, uint32_t sector,
                     void *data);

#endif
