#ifndef PE_PC_ASSETS_H
#define PE_PC_ASSETS_H

#include "disc.h"

/* Both sources stay mounted for the lifetime of the game. A source number is
 * asset provenance, not an emulated drive or a request to swap discs. */
typedef struct PeAssets {
    PeDisc discs[2];
    char error[256];
} PeAssets;

typedef struct PeAssetFile {
    unsigned source; /* 0 = Disc 1, 1 = Disc 2 */
    PeDiscFile extent;
} PeAssetFile;

int pe_assets_open(PeAssets *assets, const char *disc1, const char *disc2);
void pe_assets_close(PeAssets *assets);
int pe_assets_find(PeAssets *assets, unsigned source, const char *path,
                   PeAssetFile *file);
int pe_assets_read(PeAssets *assets, const PeAssetFile *file, uint64_t offset,
                   void *data, size_t size);
int pe_assets_read_raw(PeAssets *assets, const PeAssetFile *file,
                       uint32_t sector, void *data);

#endif
