#include "assets.h"

#include <string.h>

static int source_error(PeAssets *assets, unsigned source) {
    snprintf(assets->error, sizeof(assets->error), "Disc %u: %s",
             source + 1, assets->discs[source].error);
    return 0;
}

static int check_source(PeAssets *assets, unsigned source) {
    if (source < 2 && assets->discs[source].stream) return 1;
    snprintf(assets->error, sizeof(assets->error), "Asset source %u is not mounted", source);
    return 0;
}

void pe_assets_close(PeAssets *assets) {
    for (unsigned source = 0; source < 2; ++source)
        pe_disc_close(&assets->discs[source]);
}

int pe_assets_open(PeAssets *assets, const char *disc1, const char *disc2) {
    memset(assets, 0, sizeof(*assets));
    const char *paths[2] = {disc1, disc2};
    for (unsigned source = 0; source < 2; ++source) {
        if (!paths[source] || !*paths[source]) {
            snprintf(assets->error, sizeof(assets->error), "Disc %u is required", source + 1);
            pe_assets_close(assets);
            return 0;
        }
        if (!pe_disc_open(&assets->discs[source], paths[source])) {
            source_error(assets, source);
            pe_assets_close(assets);
            return 0;
        }
    }
    return 1;
}

int pe_assets_find(PeAssets *assets, unsigned source, const char *path,
                   PeAssetFile *file) {
    if (!check_source(assets, source)) return 0;
    PeDiscFile extent;
    if (!pe_disc_find(&assets->discs[source], path, &extent))
        return source_error(assets, source);
    file->source = source;
    file->extent = extent;
    return 1;
}

int pe_assets_read(PeAssets *assets, const PeAssetFile *file, uint64_t offset,
                   void *data, size_t size) {
    if (!check_source(assets, file->source)) return 0;
    if (!pe_disc_read(&assets->discs[file->source], &file->extent, offset, data, size))
        return source_error(assets, file->source);
    return 1;
}

int pe_assets_read_raw(PeAssets *assets, const PeAssetFile *file,
                       uint32_t sector, void *data) {
    if (!check_source(assets, file->source)) return 0;
    if (!pe_disc_read_raw(&assets->discs[file->source], &file->extent, sector, data))
        return source_error(assets, file->source);
    return 1;
}
