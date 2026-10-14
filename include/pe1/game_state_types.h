#ifndef PE1_GAME_STATE_TYPES_H
#define PE1_GAME_STATE_TYPES_H

#include "pe1/render_object.h"

/* Original global mode flags and their shared record view. */
typedef struct GameStateBlock {
    u32 flags;
    u32 reserved;
    u32 reserved_tail[2];
} GameStateBlock;
extern GameStateBlock g_GameStateBlock;
#define g_GameStateFlags (g_GameStateBlock.flags)

typedef signed char Pe1S8;
typedef unsigned char Pe1U8;
typedef unsigned int Pe1U32;

/*
 * Shared scene, field, and AKAO state at 0x800B0CD8 (g_GameState).
 * Only fields corroborated by multiple main-executable users are named;
 * field comments document original offsets. Runtime pointers and embedded
 * objects use host layout; this record is not serialized to a save or disc.
 */
struct SceneAssetBlob;
struct GeomState;
struct TimFile;
struct PmSlotBanks;
struct RenderObjectEntity;
union SceneAssetView;

typedef union Pe1SceneAudioState {
    Pe1U8 bytes[0x10];
    struct {
        Pe1U8 reserved[2];
        Pe1S8 banks[2];
        Pe1S8 keys[2][2];
        Pe1S8 pending_key, pending_bank; /* 0xE0, 0xE1 in game state */
        /* Movie playback state (menu_memcard video player). */
        Pe1U8 movie_active;      /* 0xE2: frame countdown while playing */
        Pe1S8 movie_wide;        /* 0xE3: wide-screen movie flag */
        short movie_frame;       /* 0xE4 */
        Pe1U8 movie_volume;      /* 0xE6: fade scale, reset to 0x98 */
        Pe1S8 movie_index;       /* 0xE7 */
    } tracks;
} Pe1SceneAudioState;

/* Native PsyZ primitive tag; these packets are runtime state, not disc data. */
typedef struct Pe1PrimTag {
    uintptr_t addr;
    uintptr_t len;
} Pe1PrimTag;

/* Full-screen fade primitive pair drawn by the end-of-frame pass. */
typedef struct Pe1FadeTile {
    Pe1PrimTag tag;
    Pe1U8 r, g, b, code;
    short x, y;
    unsigned short w, h;
} Pe1FadeTile;

typedef struct Pe1FadeTpage {
    Pe1PrimTag tag;
    uintptr_t mode;
} Pe1FadeTpage;

typedef struct Pe1GameState {
    Pe1U32 flags;                    /* 0x000 */
    unsigned short saved_state_004;  /* 0x004: kept in the save tail, reset to 10 */
    short field_bg_cache_id;         /* 0x006: reset to -1 after a background load */
    Pe1U8 room_type;                /* 0x008: room directory byte 3 */
    Pe1S8 field_bg_cache_bank;       /* 0x009: reset to -1 after a background load */
    Pe1U8 requested_entity_bank;    /* 0x00A */
    Pe1U8 loaded_entity_bank;       /* 0x00B */
    Pe1S8 current_story_day;         /* 0x00C */
    Pe1S8 pending_story_day;         /* 0x00D */
    Pe1U8 story_day_flags;           /* 0x00E */
    Pe1U8 unk_00f;
    Pe1U8 cd_range_read_mode;      /* 0x010: CdRom_FindNextDataSector */
    Pe1U8 bank_state_11, bank_state_12;
    Pe1U8 scene_area_type_backup;   /* 0x013: area type kept across a disc swap */
    RenderObjectEntity scene_object; /* 0x014 */
    Pe1U8 reserved_scene_object_tail[8]; /* 0x0D0: outside the render object */
    Pe1SceneAudioState scene_audio; /* 0x0D8: cached track banks and keys */
    short pending_sample_bank;      /* 0x0E8: -1 means no sample upload */
    Pe1U8 pending_stream_banks[2];  /* 0x0EA: zero means no stream upload */
    Pe1U8 entity_texture_phase;     /* 0x0EC */
    Pe1U8 scene_init_phase;      /* 0x0ED */
    Pe1U8 scene_init_subphase;   /* 0x0EE */
    Pe1U8 tim_load_state;            /* 0x0EF: 0, 0x34, 0x35, 0x36 */
    Pe1U8 cd_read_phase;            /* 0x0F0 */
    Pe1U8 cd_track_phase;           /* 0x0F1 */
    Pe1U8 cd_transition_phase;      /* 0x0F2 */
    Pe1U8 cd_range_state;          /* 0x0F3: CdRom_FindNextDataSector */
    Pe1U8 memcard_init_phase;     /* 0x0F4: Boot_MemCardStepInit, phases 0 and 0x37..0x3B. */
    Pe1U8 display_list_modes;       /* 0x0F5: bit 0 mode 1, bit 1 mode 2 (Render_InitDisplayLists) */
    Pe1U8 bank_value_f6, bank_value_f7;
    unsigned short bank_value_f8, bank_value_fa;
    Pe1U8 unk_0fc[2];
    Pe1U8 track_volumes[2];         /* 0x0FE: per scene audio slot (CdRom_RegisterAudioSlot) */
    Pe1U32 pe_image_base_lba;        /* 0x100: g_PeImageBaseLba */
    Pe1FadeTile fade_tile;           /* 0x104: full-screen fade TILE */
    Pe1FadeTpage fade_tpage;         /* 0x114: draw mode for the fade tile */
    RenderObjectHeader *scene_object_model;        /* 0x11C */
    void *room_geometry_table;       /* 0x120: key 0xCAAD0704 */
    void *bank_asset_table;           /* 0x124: resolved bank asset table */
    u8 *bank_work_base;              /* 0x128 */
    u8 *bank_work_end;               /* 0x12C */
    u8 *bank_work_far_end;           /* 0x130: voice bank base + 0x2800 */
    void *scene_object_tables[3];    /* 0x134 */
    /* Startup resource tables selected by keys 0x57D40D84 and 0x57D41D84. */
    void *startup_resource_tables[2]; /* 0x140 */
    u8 *startup_resource_blob;       /* 0x148 */
    void *bank_asset_source;         /* 0x14C */
    u8 *voice_bank_base;             /* 0x150 */
    u8 *voice_bank_base_1400;        /* 0x154 */
    union SceneAssetView *entity_texture_blob; /* 0x158: voice bank base + 0x2800 */
    u8 *scene_object_work;         /* 0x15C */
    /* 0x160: the frame ordering tables (g_OtBufferTable) double as the
     * background texture load buffer while a scene loads. The complete ten-slot
     * renderer view aliases the original load-time pointers through 0x184. */
    union {
        RenderBufferPrefix render_buffers;
        struct {
            u8 *ordering[2];
            u8 *load_buffers[3];
            u8 *scratch[1];
            u8 *front[1];
            u8 *back[1];
            u8 *animation[2];
        } render_buffer_names;
        struct {
            union {
                unsigned char *ordering_tables[2];
                struct SceneAssetBlob *bg_texture_blob;
            } shared_buffers;
            union SceneAssetView *texture_load_scratch; /* 0x168: second room read */
            struct SceneAssetBlob *object_texture_blob; /* 0x16C */
            u8 *unk_170;                      /* 0x170: work buffer set by the boot memory layout */
            struct TimFile *bg_tim;                    /* 0x174 */
            unsigned short *save_background_source;      /* 0x178 */
            unsigned short *save_background_destination; /* 0x17C */
            struct TimFile *hud_tim;                   /* 0x180 */
            u8 *script_sprite_buffer;        /* 0x184: FieldFx_ClearScriptSprites starts the sprite list below it */
        };
    };
    struct PmSlotBanks *scene_process_slots; /* 0x188 */
    union { /* 0x18C: two typed views of the same loaded archive pointer */
        union SceneAssetView *loaded_scene_assets;
        struct SceneAssetBlob *loaded_scene_archive;
    };
    u8 *str_file_directory;          /* 0x190: g_StrFileDirBuffer */
    u8 *scene_load_scratch;        /* 0x194: g_SceneLoadScratchBuffer */
    /* Reset bounds establish these arrays; individual element roles unknown. */
    void *bank_slots[10];            /* 0x198 */
    void *bank_rows[10][48];         /* 0x1C0 */
    Pe1U32 bank_reset_940[1];
    void *bank_reset_944[1];         /* 0x944: relocated task block */
    void *bank_reset_948[1];
    union { /* 0x94C: room geometry/camera block */
        void *bank_reset_94c[1];
        struct GeomState *volatile geometry_state;
    };
    void *bank_reset_950[2];
    Pe1U32 bank_reset_958[1];
} Pe1GameState;

/* Audio records keep their fixed scalar format; GPU packets use native tags. */
PE1_STATIC_ASSERT(sizeof(Pe1SceneAudioState) == 16, scene_audio_state_size);
PE1_STATIC_ASSERT(sizeof(Pe1PrimTag) == 2 * sizeof(uintptr_t), game_primitive_tag_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1FadeTile, r) == sizeof(Pe1PrimTag), game_fade_tile_size);
PE1_STATIC_ASSERT(sizeof(Pe1FadeTpage) == 3 * sizeof(uintptr_t), game_fade_page_size);

PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1GameState, render_buffers) == PE1_OFFSETOF(Pe1GameState, shared_buffers), game_render_ordering_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1GameState, render_buffers.other_buffers[0]) == PE1_OFFSETOF(Pe1GameState, texture_load_scratch), game_render_scratch_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1GameState, render_buffers.packets[0]) == PE1_OFFSETOF(Pe1GameState, hud_tim), game_render_packets_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1GameState, render_buffers.packets[1]) == PE1_OFFSETOF(Pe1GameState, script_sprite_buffer), game_render_sprite_alias);

PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1GameState, render_buffer_names.ordering) == PE1_OFFSETOF(Pe1GameState, shared_buffers), game_render_named_ordering_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1GameState, render_buffer_names.scratch) == PE1_OFFSETOF(Pe1GameState, bg_tim), game_render_named_scratch_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1GameState, render_buffer_names.front) == PE1_OFFSETOF(Pe1GameState, save_background_source), game_render_named_front_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1GameState, render_buffer_names.back) == PE1_OFFSETOF(Pe1GameState, save_background_destination), game_render_named_back_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1GameState, render_buffer_names.animation) == PE1_OFFSETOF(Pe1GameState, render_buffers.packets), game_render_named_animation_alias);

#endif
