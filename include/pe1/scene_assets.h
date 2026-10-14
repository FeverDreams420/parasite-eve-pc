#ifndef PE1_SCENE_ASSETS_H
#define PE1_SCENE_ASSETS_H

#include "common.h"
#include "pe1/asset_table.h"
#include "pe1/game_state.h"
#include "pe1/pm.h"
#include "pe1/scene_room_load.h"

/* Scene startup and teardown callbacks; other handlers remain unidentified. */
typedef PmCommand SceneAssetHandler;

typedef struct SceneAssetRecord {
    u8 reserved[7];
    u8 handlerId;
    u8 trailing[4];
} SceneAssetRecord;

typedef struct SceneAssetDirectory {
    unsigned int reserved;
    /* Low 22 bits: byte offset from the blob; high 10 bits: record count. */
    unsigned int entries;
    unsigned int keyEntries; /* 0x08: word-keyed records */
    unsigned int bankRootEntries; /* 0x0C */
    unsigned int bankRowEntries;  /* 0x10 */
    unsigned int reserved14[5];
    unsigned int timEntries; /* 0x28: same offset/count encoding */
    unsigned int streamEntries; /* 0x2C: keyed CD stream records */
    unsigned int trackEntries; /* 0x30: same offset/count encoding */
} SceneAssetDirectory;

/* Stream records share the keyed offset prefix used by asset lookup. */
typedef struct SceneCdStreamRecord {
    u32 reserved;
    u32 offset; /* Low 24 bits: byte offset in the loaded blob. */
    u8 cdIndex;
    u8 reserved09;
    u16 key;
} SceneCdStreamRecord;

PE1_STATIC_ASSERT(sizeof(SceneCdStreamRecord) == 12, scene_cd_stream_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneCdStreamRecord, cdIndex) == 8,
                  scene_cd_stream_index_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneCdStreamRecord, key) == 10,
                  scene_cd_stream_key_offset);

typedef struct SceneTrackRecord {
    u32 size;   /* Low 24 bits: bytes copied to the selected bank workspace. */
    u32 offset; /* Low 24 bits: source offset in the loaded scene blob. */
    u16 bank;
    u16 key;
} SceneTrackRecord;

/* Sector offsets follow the archive's absolute base LBA in variable-size data. */
typedef struct SceneSectorDirectory {
    u32 base_lba;
    u16 offsets[0];
} SceneSectorDirectory;

typedef struct SceneAssetBlob {
    unsigned int reserved;
    unsigned int directoryOffset;
} SceneAssetBlob;

/* Byte view of a loaded scene container; records hold offsets from its base. */
typedef union SceneAssetView {
    SceneAssetBlob header;
    u8 bytes[1];
} SceneAssetView;

#define SCENE_ASSET_AT(view, offset) ((void *)&(view)->bytes[offset])

typedef SceneAssetBlob CdArchiveRoot;

typedef struct CdArchiveHeader {
    u8 reserved[0x24];
    u32 range_info;
} CdArchiveHeader;

typedef struct CdRange {
    u8 _00[3];
    u8 flags;
    u16 first;
    u16 last;
} CdRange;

/* Range table address as a pointer or a word: CdRom_FindNextDataSector adds
 * the scaled index to the table address as an integer, index first. */
typedef union CdRangeAddress {
    CdRange *range;
    u32 word;
} CdRangeAddress;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneSectorDirectory, offsets) == 4,
                  scene_sector_offsets_offset);
PE1_STATIC_ASSERT(sizeof(CdArchiveRoot) == 8, cd_archive_root_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CdArchiveRoot, directoryOffset) == 4,
                  cd_archive_root_directory_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CdArchiveHeader, range_info) == 0x24,
                  cd_archive_range_info_offset);


PE1_STATIC_ASSERT(sizeof(CdRange) == 8, cd_range_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CdRange, flags) == 3, cd_range_flags_offset);



typedef struct SceneBankAssetRecord {
    u32 reserved;
    union {
        u32 offsetAndId;
        struct { u8 offset[3]; u8 id; } bytes;
    } source;
    u32 trailing;
} SceneBankAssetRecord;

PE1_STATIC_ASSERT(sizeof(SceneBankAssetRecord) == 12, scene_bank_asset_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneBankAssetRecord, source.bytes.id) == 7,
                  scene_bank_asset_id_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetDirectory, bankRootEntries) == 0xC,
                  scene_bank_root_entries_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetDirectory, bankRowEntries) == 0x10,
                  scene_bank_row_entries_offset);






PE1_STATIC_ASSERT(sizeof(SceneTrackRecord) == 12, scene_track_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneTrackRecord, bank) == 8, scene_track_bank_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneTrackRecord, key) == 10, scene_track_key_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetDirectory, keyEntries) == 0x08,
                  scene_asset_key_entries_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetDirectory, streamEntries) == 0x2C,
                  scene_asset_stream_entries_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetDirectory, trackEntries) == 0x30,
                  scene_track_entries_offset);









/* Range-table cursor used by CdRom_FindNextDataSector. */
extern int g_CdRangeIndex;

/* Scene containers encode byte offsets relative to their loaded base. */
static inline void *SceneAsset_ResolveOffset(void *base, unsigned int offset)
{
    return (u8 *)base + offset;
}

typedef union TimUploadSource {
    u32 offsetAndHeight;
    struct { u8 offset[3]; u8 height; } bytes;
} TimUploadSource;

/* Each source word stores a 24-bit byte offset and an 8-bit height.
 * Rectangles pack width (10 bits), x (11 bits), then y (11 bits). */
typedef struct TimUploadRecord {
    u32 reserved;
    TimUploadSource image_source;
    u32 image_rectangle;
    TimUploadSource secondary_source;
    u32 secondary_rectangle;
} TimUploadRecord;
typedef struct TimPackedImage {
    union {
        u32 value;
        struct { u8 reserved[3]; u8 type; } bytes;
    } key;
    TimUploadSource source;
    u32 geometry;
} TimPackedImage;

PE1_STATIC_ASSERT(sizeof(TimUploadRecord) == 20, scene_tim_upload_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(TimUploadRecord, image_source.bytes.height) == 7,
                  scene_tim_upload_image_height_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(TimUploadRecord, image_rectangle) == 8,
                  scene_tim_upload_image_rectangle_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(TimUploadRecord, secondary_source.bytes.height) == 15,
                  scene_tim_upload_secondary_height_offset);
PE1_STATIC_ASSERT(sizeof(TimPackedImage) == 12, scene_tim_packed_image_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(TimPackedImage, source.bytes.height) == 7,
                  scene_tim_image_height_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(TimPackedImage, geometry) == 8,
                  scene_tim_image_geometry_offset);


PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetHandler, init) == 0,
                  scene_asset_init_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetDirectory, timEntries) == 0x28,
                  scene_asset_tim_entries_offset);




PE1_STATIC_ASSERT(sizeof(SceneAssetRecord) == 12, scene_asset_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetRecord, handlerId) == 7,
                  scene_asset_handler_id_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetDirectory, entries) == 4,
                  scene_asset_directory_entries_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneAssetBlob, directoryOffset) == 4,
                  scene_asset_directory_offset);


/* Opaque handler pointers; the unload path reads the prefix declared above. */
extern void *g_FieldFxTaskPrograms[104];

int CdRom_ReadAssetSectors(unsigned int kind, unsigned int index, int channel,
                   void *buffer, u32 maximum, int blocking);
int CdRom_FindNextDataSector(void);

int Asset_UnloadTableEntries(void);
int Draw_LoadTimAsset(TimUploadRecord *record, void *base);
extern u16 g_PeImageSectorTable[];

/* Scene voice-bank entries: actor animation key and two voice ids; the
 * game-state initializer clears both ids. */
typedef struct AkaoVoiceBankEntry {
    u8 group;
    u8 set;
    u8 index;
    u8 frame;
    u16 ids[2];
} AkaoVoiceBankEntry;
extern AkaoVoiceBankEntry g_AkaoVoiceBankEntries[];
void *Asset_FindTable2CByU16Key(char *base, int key);
void *Asset_FindTable30ByU16Key(char *base, int key);
void Akao_LoadVoiceBankAlt(void);







PE1_STATIC_ASSERT(sizeof(AkaoVoiceBankEntry) == 8, akao_voice_bank_entry_size);

#endif
