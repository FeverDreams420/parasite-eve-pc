#include "pe1/scene_assets.h"
#include "pe1/asset_key_record.h"

/* Keyed record tables: SceneAssetDirectory word = 22-bit byte offset of the
 * records from the blob plus a 10-bit record count; each record holds the
 * 24-bit byte offset of its payload. Retail reserves an 8-byte frame for an
 * unused local in each lookup. */
#define DEFINE_ASSET_SEARCHER(name, base_type, key_type, table_member, record_type, offset_member) \
void *name(base_type base, key_type key) { \
    SceneAssetDirectory *directory; \
    record_type *records; \
    u32 descriptor; \
    u32 count; \
    u32 i; \
    void *found; \
    char unused[1]; \
 \
    directory = SceneAsset_ResolveOffset(base, ((SceneAssetBlob *)base)->directoryOffset); \
    descriptor = directory->table_member; \
    records = SceneAsset_ResolveOffset(base, descriptor & 0x3FFFFF); \
    count = descriptor >> 22; \
    found = 0; \
    for (i = 0; i < count; i++) { \
        if (records[i].key == key) { \
            found = SceneAsset_ResolveOffset(base, records[i].offset_member & 0xFFFFFF); \
            return found; \
        } \
    } \
    return found; \
}

DEFINE_ASSET_SEARCHER(Asset_FindTable08ByU32Key, void *, int, keyEntries, AssetU32KeyRecord, packedOffset)
DEFINE_ASSET_SEARCHER(Asset_FindTable2CByU16Key, char *, int, streamEntries, SceneCdStreamRecord, offset)
DEFINE_ASSET_SEARCHER(Asset_FindTable30ByU16Key, char *, int, trackEntries, SceneTrackRecord, offset)

