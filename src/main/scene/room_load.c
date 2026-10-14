#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pe1/geom_state.h"
#include "pe1/room_fx_model.h"
#include "pe1/render_camera.h"
#include "pe1/akao.h"
#include "pe1/asset_tim.h"
#include "pe1/boot_disc_check.h"
#include "pe1/cdrom.h"
#include "pe1/game_state.h"
#include "pe1/entity_animation.h"
#include "pe1/entity_frame_update.h"
#include "pe1/field_actor.h"
#include "pe1/field_bg_load.h"
#include "pe1/game_state.h"
#include "pe1/gpu_image.h"
#include "pe1/menu_memcard_display.h"
#include "pe1/menu_state.h"
#include "pe1/pe_image.h"
#include "pe1/player_entity.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_prim.h"
#include "pe1/resident_tables.h"
#include "pe1/save_tail_state.h"
#include "pe1/scene_assets.h"
#include "pe1/scene_room_load.h"
#include "pe1/scene_room.h"
#include "pe1/scene_transition.h"

/* COMMON supplies small-data metadata to MASPSX: these words are reached
 * through $gp, every other extern stays absolute. */
u8 D_8009CDC8;

typedef struct { char b[16]; } __attribute__((aligned(1), packed)) Copy16u;
typedef struct { u32 w[4]; } Copy16a;

/* Block copy cursor: retail tests the two addresses' alignment as words. */
typedef union SceneCopyCursor {
    u8 *bytes;
    Copy16u *unaligned;
    Copy16a *aligned;
    u32 word;
} SceneCopyCursor;

void Scene_LoadSceneData(void) {
    RECT rect;
    Pe1GameState *state;
    s32 lba;
    u16 *p0;
    s32 t;
    s32 flag;
    SceneCopyCursor src;
    SceneCopyCursor dst;
    u8 *end;
    u8 *copyEnd;

    rect.w = 0x3FF;
    rect.x = 0;
    rect.y = 0;
    rect.h = 0x1FF;
    state = &g_GameState;
    lba = g_PeImageBaseLba;
    ClearImage(&rect, 0, 0, 1);

restart1:
    p0 = &g_PeImageSectorTable[2];
    while (CdRom_ReadSectorsFromLba(lba + p0[0], &g_ResidentDataPack, p0[1] - p0[0]) == -1) {}
    t = 1;
    do {
        if (t == -1) {
            goto restart1;
        }
        t = CdRom_PollReady();
    } while (t != 0);

    flag = 0;
restart2:
    p0 = &g_PeImageSectorTable[3];
    while (CdRom_ReadSectorsFromLba(lba + p0[0], state->scene_load_scratch, p0[1] - p0[0]) == -1) {}
    t = 1;
    do {
        if (flag == 0) {
            Menu_FullInit();
            flag = 1;
        }
        if (t == -1) {
            goto restart2;
        }
        t = CdRom_PollReady();
    } while (t != 0);

    src.bytes = state->scene_load_scratch;
    dst.bytes = g_StartupResourceBlob;
    if ((src.word | dst.word) & 3) {
        copyEnd = src.bytes + 0x10A50;
        do {
            *dst.unaligned = *src.unaligned;
            src.bytes += 0x10;
            dst.bytes += 0x10;
        } while (src.bytes != copyEnd);
    } else {
        copyEnd = src.bytes + 0x10A50;
        do {
            *dst.aligned = *src.aligned;
            src.bytes += 0x10;
            dst.bytes += 0x10;
        } while (src.bytes != copyEnd);
    }

    state->startup_resource_blob = g_StartupResourceBlob;
    state->startup_resource_tables[0] = Asset_FindTable08ByU32Key(g_StartupResourceBlob, 0x57D40D84);
    state->startup_resource_tables[1] = Asset_FindTable08ByU32Key(state->startup_resource_blob, 0x57D41D84);

restart3:
    p0 = &g_PeImageSectorTable[6];
    while (CdRom_ReadSectorsFromLba(lba + p0[0], state->scene_load_scratch, p0[1] - p0[0]) == -1) {}
    t = 1;
    do {
        if (t == -1) {
            goto restart3;
        }
        t = CdRom_PollReady() != 0;
    } while (t != 0);

    Spu_UploadSampleBlockBlocking(g_GameState.scene_load_scratch, 1);

restart4:
    p0 = &g_PeImageSectorTable[7];
    while (CdRom_ReadSectorsFromLba(lba + p0[0], state->scene_load_scratch, p0[1] - p0[0]) == -1) {}
    t = 1;
    do {
        if (t == -1) {
            goto restart4;
        }
    } while ((t = (CdRom_PollReady() != 0)) != 0);

    {
        SceneCopyCursor to, from;
        to.bytes = state->bank_work_far_end;

        from.bytes = state->scene_load_scratch;
        end = from.bytes + 0x1400;
        if ((from.word | to.word) & 3) {
            do {
                *to.unaligned = *from.unaligned;
                from.bytes += 0x10;
                to.bytes += 0x10;
            } while (from.bytes != end);
        } else {
            do {
                *to.aligned = *from.aligned;
                from.bytes += 0x10;
                to.bytes += 0x10;
            } while (from.bytes != end);
        }
    }
}

/* The adjacent field background loader shares the scene resource state. */

/* Uploads every TIM listed in a scene blob's directory. */
#define SCENE_BLOB_LOAD_TIMS(source)                                         \
    {                                                                        \
        unsigned int i;  \
                                                                             \
        blob = (source);                                                     \
        directory = SceneAsset_ResolveOffset(blob, blob->directoryOffset);   \
        tim = SceneAsset_ResolveOffset(blob, directory->timEntries & 0x3fffff); \
        for (i = 0; i < directory->timEntries >> 22; i++)                    \
            Draw_LoadTimAsset(&tim[i], blob);                                 \
    }

int Scene_LoadFieldBg(void)
{
    Pe1GameState *state = &g_GameState;
    u32 lba;
    unsigned int slot;
    FieldBgImageRecord *image;
    SceneAssetBlob *blob;
    SceneAssetDirectory *directory;
    TimUploadRecord *tim;
    int status;
    int done;

    if (!(state->flags & 1))
        return 0;

    lba = g_PeImageBaseLba;

retry1:
    while (CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[9], state->shared_buffers.bg_texture_blob,
               g_PeImageSectorTable[10] - g_PeImageSectorTable[9]) == -1)
        ;
    status = 1;
    do {
        if (status == -1)
            goto retry1; /* recorded debt: CD retry restart */
        status = CdRom_PollReady();
    } while (status != 0);

    done = 0;
retry2:
    while ((status = CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[10], state->bg_tim,
               g_PeImageSectorTable[11] - g_PeImageSectorTable[10])) == -1)
        ;
    status = 1;
    do {
        if (!done) {
            SCENE_BLOB_LOAD_TIMS(state->shared_buffers.bg_texture_blob);
            for (slot = 2; slot < 4; slot++) {
                g_FieldBgTextureSlots[slot].tpage = BG_SLOT_TPAGE(g_FieldBgTextureSlots[slot].x, g_FieldBgTextureSlots[slot].y);
                g_FieldBgTextureSlots[slot].clut = BG_SLOT_CLUT(g_FieldBgTextureSlots[slot].clut_x, g_FieldBgTextureSlots[slot].clut_y);
            }
            image = Asset_FindTable08ByU32Key(blob, 0xABADC06C);
            while (image->length != 0) {
                LoadImage(&image->rect, image->pixels);
                image = (FieldBgImageRecord *)(&image->length + (image->length >> 2));
            }
            done = 1;
        }
        if (status == -1)
            goto retry2; /* recorded debt: CD retry restart */
        status = CdRom_PollReady();
    } while (status != 0);

    done = 0;
retry3:
    while ((status = CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[11], state->hud_tim,
               g_PeImageSectorTable[12] - g_PeImageSectorTable[11])) == -1)
        ;
    status = 1;
    do {
        if (!done) {
            Asset_LoadTimImage(state->bg_tim);
            for (slot = 0; slot < 2; slot++) {
                g_FieldBgTextureSlots[slot].tpage = BG_SLOT_TPAGE(g_FieldBgTextureSlots[slot].x, g_FieldBgTextureSlots[slot].y);
                g_FieldBgTextureSlots[slot].clut = BG_SLOT_CLUT(g_FieldBgTextureSlots[slot].clut_x, g_FieldBgTextureSlots[slot].clut_y);
            }
            done = 1;
        }
        if (status == -1)
            goto retry3; /* recorded debt: CD retry restart */
        status = CdRom_PollReady();
    } while (status != 0);

    done = 0;
retry4:
    while ((status = CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[12], state->bank_asset_source,
               g_PeImageSectorTable[13] - g_PeImageSectorTable[12])) == -1)
        ;
    status = 1;
    do {
        if (!done) {
            done = 1;
            Asset_LoadTimImage(state->hud_tim);
            Battle_DrawHPBar();
        }
        if (status == -1)
            goto retry4; /* recorded debt: CD retry restart */
        status = CdRom_PollReady();
    } while (status != 0);

    done = 0;
retry5:
    while ((status = CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[4], state->object_texture_blob,
               g_PeImageSectorTable[5] - g_PeImageSectorTable[4])) == -1)
        ;
    status = 1;
    do {
        if (!done) {
            state->scene_object_model =
                Asset_FindTable08ByU32Key(state->bank_asset_source, 0xC4B5BA04);
            done = 1;
            state->room_geometry_table =
                Asset_FindTable08ByU32Key(state->bank_asset_source, 0xCAAD0704);
            state->bank_asset_table =
                Asset_FindTable08ByU32Key(state->bank_asset_source, 0x5EAF6804);
        }
        if (status == -1)
            goto retry5; /* recorded debt: CD retry restart */
        status = CdRom_PollReady();
    } while (status != 0);

    done = 0;
retry6:
    while ((status = CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[39], state->scene_process_slots,
               g_PeImageSectorTable[40] - g_PeImageSectorTable[39])) == -1)
        ;
    status = 1;
    do {
        if (!done) {
            SCENE_BLOB_LOAD_TIMS(state->object_texture_blob);
            done = 1;
        }
        if (status == -1)
            goto retry6; /* recorded debt: CD retry restart */
        status = CdRom_PollReady();
    } while (status != 0);

    {
        unsigned int i;

        blob = (SceneAssetBlob *)state->scene_process_slots;
        directory = SceneAsset_ResolveOffset(blob, blob->directoryOffset);
        tim = SceneAsset_ResolveOffset(blob, directory->timEntries & 0x3fffff);
        i = 0;
        if (directory->timEntries >> 22) {
            do {
                Draw_LoadTimAsset(&tim[i], blob);
                i++;
            } while (i < directory->timEntries >> 22);
        }
    }

    Akao_Cmd_F1();
    DrawSync(0);
    ResetGraph(1);
    VSync(0);
    PutDispEnv(&g_RenderDispEnvArray[g_ActiveDrawSlot]);
    SetDispMask(1);
    state->field_bg_cache_id = -1;
    state->loaded_entity_bank = 0;
    state->current_story_day = -1;
    state->field_bg_cache_bank = -1;
    state->pending_sample_bank = -1;
    state->pending_stream_banks[1] = 0;
    state->pending_stream_banks[0] = 0;
    if (!(state->flags & 0x40)) {
        state->scene_audio.tracks.banks[0] = -1;
        state->scene_audio.tracks.keys[0][1] = -1;
        state->scene_audio.tracks.keys[0][0] = -1;
    }
    if (!(state->flags & 0x80)) {
        state->scene_audio.tracks.banks[1] = -1;
        state->scene_audio.tracks.keys[1][1] = -1;
        state->scene_audio.tracks.keys[1][0] = -1;
    }
    state->flags &= ~1;
    return 0;
}

void Akao_LoadVoiceBankAlt(void)
{
    Pe1GameState *state = &g_GameState;
    int i, j;
    u8 *base;
    AkaoVoiceBankEntry *pair;

    for (i = 9; i >= 0; --i)
        state->bank_slots[i] = 0;
    for (i = 0; i < 10; ++i)
        for (j = 47; j >= 0; --j)
            state->bank_rows[i][j] = 0;
    for (i = 0; i < 1; ++i)
        state->bank_reset_940[i] = 0;
    for (i = 0; i < 1; ++i)
        state->bank_reset_944[i] = 0;
    for (i = 0; i < 1; ++i)
        state->bank_reset_948[i] = 0;
    for (i = 0; i < 1; ++i)
        state->bank_reset_94c[i] = 0;
    for (i = 1; i >= 0; --i)
        state->bank_reset_950[i] = 0;
    for (i = 0; i < 1; ++i)
        state->bank_reset_958[i] = 0;

    base = state->voice_bank_base;
    state->bank_work_base = base;
    state->bank_work_end = base + 0x1400;
    state->bank_asset_table =
        Asset_FindTable08ByU32Key(state->bank_asset_source, 0x5EAF6804);
    state->scene_audio.bytes[8] = 0x27;
    state->scene_audio.bytes[9] = 13;
    state->bank_state_11 = 0;
    state->bank_state_12 = 0;
    state->flags &= ~0x400000;

    for (i = 0, pair = g_AkaoVoiceBankEntries; i < 4; ++i, ++pair) {
        pair->ids[1] = 0;
        g_AkaoVoiceBankEntries[i].ids[0] = 0;
    }
    state->bank_value_f6 = 0x30;
    state->bank_value_f7 = 0x7F;
    state->bank_value_f8 = 0x100;
    state->bank_value_fa = 0x800;
}

/*
 * Scene_LoadRoom (0x8006B4F8, 2160 bytes).
 * Streams the three room ranges of PE.IMG, uploads the TIM lists of the
 * first two while the next read runs, then relocates the room directory
 * tables into g_GameState and queues the stream/sample bank reads.
 *
 * maspsx -G1: retail reads the 4-byte g_PmCmdHandlerTable pointer
 * absolutely while the 1-byte room letter D_8009CDC8 is gp-relative.
 */

int Scene_LoadRoom(unsigned int roomId)
{
    static GeomState *geometry_owner;
    GeomState *next_geometry = NULL;
    char name[8];
    unsigned int base;
    int map;
    int ready;
    int loaded;
    unsigned int i;
    unsigned int slot;
    int flags;
    SceneAssetView *room;
    SceneRoomDirectory *directory;
    TimUploadRecord *tim;
    SceneRoomRecord *record;
    SceneRoomShortRecord *shortRecord;
    Pe1GameState *state;

    name[0] = D_8009CDC8;
    memset(name + 1, 0, 6);
    flags = 0;
    state = &g_GameState;
    base = g_GameState.pe_image_base_lba;
    Text_EncodeBase32(name, roomId);

retry_scratch:
    {
        map = Text_ParseMapNumber((signed char *)name) - 1;
        while ((ready = CdRom_ReadSectorsFromLba(base + g_RoomSectorTable[map].start,
                                        state->scene_load_scratch,
                                        g_RoomSectorTable[map].scratchSectors)) == -1) {
        }
        for (ready = 1; ready != 0; ready = CdRom_PollReady()) {
            if (ready == -1) {
                /* Recorded debt: CD retry restart (goto). */
                goto retry_scratch;
            }
        }
    }

    loaded = 0;
retry_texture:
    {
        while ((ready = CdRom_ReadSectorsFromLba(base + g_RoomSectorTable[map].start +
                                            g_RoomSectorTable[map].scratchSectors,
                                        state->texture_load_scratch,
                                        g_RoomSectorTable[map].textureSectors)) == -1) {
        }
        for (ready = 1; ready != 0; ready = CdRom_PollReady()) {
            if (!loaded) {
                room = (SceneAssetView *)state->scene_load_scratch;
                directory = SceneAsset_ResolveOffset(room, room->header.directoryOffset);
                tim = SceneAsset_ResolveOffset(room, directory->tims & 0x3FFFFF);
                for (i = 0; i < directory->tims >> 22; i++) {
                    Draw_LoadTimAsset(&tim[i], room);
                }
                loaded = 1;
            }
            if (ready == -1) {
                /* Recorded debt: CD retry restart (goto). */
                goto retry_texture;
            }
        }
    }

    loaded = 0;
retry_room:
    {
        while ((ready = CdRom_ReadSectorsFromLba(base + g_RoomSectorTable[map].start +
                                            g_RoomSectorTable[map].scratchSectors +
                                            g_RoomSectorTable[map].textureSectors,
                                        state->loaded_scene_assets,
                                        g_RoomSectorTable[map].roomSectors)) == -1) {
        }
        for (ready = 1; ready != 0; ready = CdRom_PollReady()) {
            if (!loaded) {
                room = state->texture_load_scratch;
                directory = SceneAsset_ResolveOffset(room, room->header.directoryOffset);
                tim = SceneAsset_ResolveOffset(room, directory->tims & 0x3FFFFF);
                for (i = 0; i < directory->tims >> 22; i++) {
                    Draw_LoadTimAsset(&tim[i], room);
                }
                loaded = 1;
            }
            if (ready == -1) {
                /* Recorded debt: CD retry restart (goto). */
                goto retry_room;
            }
        }
    }

    /* Room code is linked statically on the host; no PS1 cache flush. */

    room = state->loaded_scene_assets;
    directory = SceneAsset_ResolveOffset(room, room->header.directoryOffset);
    /* Validate all native bindings before mutating the scene banks. */
    record = SceneAsset_ResolveOffset(room, directory->scripts & 0x3FFFFF);
    for (i = 0; i < directory->scripts >> 22; i++) {
        PmCommand *native = NULL;
        slot = record[i].source.bytes.slot;
        /* These seven USA rooms contain the same original mirror module.
         * All other serialized descriptors still need native bindings. */
        if (slot == 69 && record[i].u.handlerAddress == 0x8018fb44u) {
            switch (map + 1) {
            case 17: case 18: case 21: case 45: case 102: case 151: case 319:
                native = &g_RoomFxMirrorHandlers;
                break;
            }
        }
        if (!native) {
            fprintf(stderr, "Scene_LoadRoom: native descriptor missing for M%03d slot %u (PS1 %08x)\n",
                    map + 1, slot, record[i].u.handlerAddress);
            return -2;
        }

    }

    /* Expand disk mesh records to native pointer width. Keep the original
     * byte arrays and view/control offsets, and append the native entries. */
    {
        SceneRoomRecord *aux = SceneRoom_FirstRecord(room, directory->taskAux);
        size_t offset = aux->source.offsetAndSlot & 0xffffff;
        u32 payload_bytes, declared;
        memcpy(&payload_bytes, aux, 4); payload_bytes &= 0xffffff;
        size_t room_bytes = (size_t)g_RoomSectorTable[map].roomSectors * 2048;
        if (offset > room_bytes || payload_bytes > room_bytes - offset || payload_bytes < sizeof(GeomState))
            goto invalid_geometry;
        const u8 *source = (const u8 *)room + offset;
        GeomState header;
        memcpy(&header, source, sizeof(header));
        memcpy(&declared, source, 4);
        if (declared < sizeof(GeomState) || declared > payload_bytes || header.entry_offset < 0 ||
            (size_t)header.entry_offset > declared ||
            header.entry_count06 > (declared - (size_t)header.entry_offset) / 0x38)
            goto invalid_geometry;
        u16 view_count;
        memcpy(&view_count, source + 10, sizeof(view_count));
        if (!view_count || header.entry_offset_1C < (s32)sizeof(GeomState) ||
            (size_t)header.entry_offset_1C > declared ||
            view_count > (declared - (size_t)header.entry_offset_1C) / sizeof(CameraViewport) ||
            header.ctrl_offset < 0 || (size_t)header.ctrl_offset > declared ||
            header.entry_count > (declared - (size_t)header.ctrl_offset) / sizeof(GeomCtrlEntry))
            goto invalid_geometry;
        for (unsigned control_index = 0; control_index < header.entry_count; ++control_index) {
            size_t control_offset = (size_t)header.ctrl_offset + control_index * sizeof(GeomCtrlEntry);
            GeomCtrlEntry control;
            memcpy(&control, source + control_offset, sizeof(control));
            s64 slots_offset = (s64)control_offset + control.slot_offset;
            u32 slots = control.head.packed >> 8;
            if (!slots) slots = 1; /* Reset reads the first slot even for an idle control. */
            if (slots_offset < 0 || (u64)slots_offset > declared ||
                slots > (declared - (size_t)slots_offset) / sizeof(GeomAnimationSlot))
                goto invalid_geometry;
            for (u32 slot_index = 0; slot_index < slots; ++slot_index)
                if (source[slots_offset + slot_index * sizeof(GeomAnimationSlot)] >= header.entry_count06)
                    goto invalid_geometry;
        }
        size_t entries_offset = (declared + _Alignof(GeomEntry) - 1) & ~((size_t)_Alignof(GeomEntry) - 1);
        size_t native_bytes = entries_offset + (size_t)header.entry_count06 * sizeof(GeomEntry);
        if (native_bytes > INT32_MAX) goto invalid_geometry;
        next_geometry = calloc(1, native_bytes);
        if (!next_geometry) goto invalid_geometry;
        memcpy(next_geometry, source, declared);
        next_geometry->entry_offset = (s32)entries_offset;
        u32 native_size = (u32)native_bytes;
        memcpy(next_geometry, &native_size, 4);
        GeomEntry *entries = (GeomEntry *)((u8 *)next_geometry + entries_offset);
        for (unsigned entry = 0; entry < header.entry_count06; ++entry) {
            size_t old_offset = (size_t)header.entry_offset + entry * 0x38;
            GeomEntry *native = &entries[entry];
            memcpy(native, source + old_offset, 0x30);
            if (native->prim_count) {
                s64 position = (s64)old_offset + native->u28.pos_ptr;
                s64 texture = (s64)old_offset + native->u2C.uv_ptr;
                if (position < 0 || texture < 0 || (position & 3) || (texture & 3) ||
                    (u64)position > declared || (u64)texture > declared ||
                    native->prim_count > (declared - (size_t)position) / sizeof(u32) ||
                    native->prim_count > (declared - (size_t)texture) / 8)
                    goto invalid_geometry;
                native->u28.pos_ptr = (s32)(position - ((u8 *)native - (u8 *)next_geometry));
                native->u2C.uv_ptr = (s32)(texture - ((u8 *)native - (u8 *)next_geometry));
            }
        }
    }

    state->requested_entity_bank = directory->entityBank;
    state->room_type = directory->roomType;

    record = SceneAsset_ResolveOffset(room, directory->bankRoots & 0x3FFFFF);
    i = 0;
    if (directory->bankRoots >> 22) do {
        state->bank_slots[record[i].source.bytes.slot] =
            SCENE_ROOM_PAYLOAD(room, &record[i]);
    } while (++i < directory->bankRoots >> 22);

    record = SceneAsset_ResolveOffset(room, directory->bankRows & 0x3FFFFF);
    for (i = 0; i < directory->bankRows >> 22; i++) {
        state->bank_rows[record[i].u.row.group][record[i].source.bytes.slot] =
            SCENE_ROOM_PAYLOAD(room, &record[i]);
    }

    state->bank_reset_944[0] =
        Script_RelocBlock(SCENE_ROOM_PAYLOAD(room, SceneRoom_FirstRecord(room, directory->taskBlock)));
    state->bank_reset_948[0] = SCENE_ROOM_PAYLOAD(room, SceneRoom_FirstRecord(room, directory->taskData));
    free(geometry_owner);
    geometry_owner = next_geometry;
    next_geometry = NULL;
    state->geometry_state = geometry_owner;

    shortRecord = SceneAsset_ResolveOffset(room, directory->taskSlots & 0x3FFFFF);
    for (i = 0; i < directory->taskSlots >> 22; i++) {
        state->bank_reset_950[shortRecord[i].source.bytes.slot] =
            SCENE_ROOM_PAYLOAD(room, &shortRecord[i]);
    }

    record = SceneAsset_ResolveOffset(room, directory->scripts & 0x3FFFFF);
    for (i = 0; i < directory->scripts >> 22; i++) {
        slot = record[i].source.bytes.slot;
        if (!g_PmCmdHandlerTable[slot]) g_PmCmdHandlerTable[slot] = &g_RoomFxMirrorHandlers;
    }

    if (directory->samples & 0xFFC00000) {
        SceneRoomRecord *sample = SceneAsset_ResolveOffset(room, directory->samples & 0x3FFFFF);
        for (i = 0; i < directory->samples >> 22; i++) {
            if (sample[i].flags & 0x20) {
                int key = sample[i].u.track.key;

                /* The id words of the voice-bank entries, indexed as one
                 * halfword run from entry 1. */
                if ((u16)key & 1) {
                    g_AkaoVoiceBankEntries[1].ids[sample[i].source.bytes.slot] = key;
                    g_AkaoVoiceBankEntries[1].ids[sample[i].source.bytes.slot + 8] = sample[i].u.track.key;
                } else {
                    g_AkaoVoiceBankEntries[1].ids[sample[i].source.bytes.slot - 4] = key;
                    g_AkaoVoiceBankEntries[1].ids[sample[i].source.bytes.slot + 4] = sample[i].u.track.key;
                }
            }
        }
    }

    if (directory->tracks & 0xFFC00000) {
        SceneRoomRecord *track = SceneAsset_ResolveOffset(room, directory->tracks & 0x3FFFFF);
        for (i = 0; i < directory->tracks >> 22; i++) {
            if (track[i].flags & 0x10) {
                state->scene_audio.tracks.pending_key = track[i].u.track.key;
                state->scene_audio.tracks.pending_bank = track[i].u.track.bank;
                state->bank_asset_table =
                    SceneAsset_ResolveOffset(state->loaded_scene_assets, track[i].source.offsetAndSlot & 0xFFFFFF);
            }
        }
    }

    if (directory->streams & 0xFFC00000) {
        SceneRoomStreamRecord *stream = SceneAsset_ResolveOffset(room, directory->streams & 0x3FFFFF);

        for (i = 0; i < directory->streams >> 22; i++) {
            if (stream[i].flags & 0x10) {
                continue;
            }
            if (stream[i].flags & 0x20) {
                if (stream[i].channel == 0) {
                    flags |= 0x10;
                    if (state->pending_stream_banks[0] != stream[i].bank.value) {
                        flags |= 1;
                        state->pending_stream_banks[0] = stream[i].bank.low;
                    }
                } else if (stream[i].channel == 1) {
                    flags |= 0x20;
                    if (state->pending_stream_banks[1] != stream[i].bank.value) {
                        flags |= 2;
                        state->pending_stream_banks[1] = stream[i].bank.low;
                    }
                }
            } else {
                u16 bank = stream[i].bank.value;

                flags |= 0x40;
                if (state->pending_sample_bank != bank) {
                    flags |= 4;
                    state->pending_sample_bank = bank;
                }
            }
        }
    }

    loaded = 1;
    if ((flags & 1) && state->pending_stream_banks[0] != 0) {
        CdRom_ReadAssetSectors(2, state->pending_stream_banks[0], 0,
                       state->scene_load_scratch, 0x21, loaded);
    } else if (!(flags & 0x10)) {
        state->pending_stream_banks[0] = 0;
    }
    if ((flags & 2) && state->pending_stream_banks[1] != 0) {
        CdRom_ReadAssetSectors(2, state->pending_stream_banks[1], 1,
                       state->scene_load_scratch, 0x21, 1);
    } else if (!(flags & 0x20)) {
        state->pending_stream_banks[1] = 0;
    }
    if ((flags & 4) && state->pending_sample_bank != -1) {
        CdRom_ReadAssetSectors(1, state->pending_sample_bank, 0,
                       state->scene_load_scratch, 0x21, 1);
    } else if (!(flags & 0x40)) {
        state->pending_sample_bank = -1;
    }
    return 0;

invalid_geometry:
    free(next_geometry);
    fprintf(stderr, "Scene_LoadRoom: invalid native geometry for M%03d\n", map + 1);
    return -3;
}

s32 Scene_StreamTexturePage(void)
{
    Pe1GameState *state;
    PeSectorTableAddress ranges;
    PeSectorTableAddress nextRanges;
    PeSectorTableAddress first;
    PeSectorTableAddress last;
    s32 retry;
    s32 offset;
    u16 start;
    s32 poll;

    state = &g_GameState;
    if ((s8)g_GameState.room_type != g_LoadedTexturePageId) {
        DrawSync(0);
        /* Matching debt: the CD retry restarts from the table address;
         * a loop lets loop.c hoist it (docs/CODE_ORGANIZATION.md). */
restart:
        ranges.entry = g_PeImageSectorTable;
        nextRanges.entry = ranges.entry + 1;
        retry = -1;
        do {
            offset = ((s8)state->room_type + 0x2B) * 2;
            first.word = offset + ranges.word;
            last.word = offset + nextRanges.word;
            start = *first.entry;
        } while (CdRom_ReadSectorsFromLba(state->pe_image_base_lba + start, state->scene_load_scratch,
                                          *last.entry - start) == retry);
        do {
            poll = CdRom_PollReady();
            if (poll == 0) {
                break;
            }
            if (poll == -1) {
                goto restart;
            }
        } while (1);
        Asset_LoadTimImage((TimFile *)state->scene_load_scratch);
        state->field_bg_cache_bank = (s8)state->room_type;
    }
    return 0;
}

/* Disc-change gating for scene-area transitions. */

int CdRom_DetectDiscChange(void) {
    unsigned int *state = &g_GameState.flags;
    int offset = g_SceneAreaType - 0xA;

    if ((unsigned int)offset < 5) {
        int value = g_SavedSceneAreaType;

        if (g_SceneAreaType != (unsigned char)value) {
            *state |= 0x200000;
        }

        if (((unsigned int)offset >> 1) != ((value - 0xA) / 2)) {
            g_DiscChangeFlags |= 4;
        }
    }

    return 0;
}

/* Matching debt: empty barriers, one retry jump, and a
 * 32-byte unused stack reserve retain retail allocation and scheduling.
 * The shifted state view preserves the bank-row address calculation. */
int Scene_LoadEntityTexture(void)
{
    int bank = g_SceneAreaType;
    u32 lba = g_GameState.pe_image_base_lba;
    Pe1GameState *state = &g_GameState;
    SceneAssetBlob *blob;
    SceneAssetDirectory *directory;
    TimUploadRecord *tim;
    TimUploadRecord *firstTim;
    u32 timOffset;
    SceneBankAssetRecord *record;
    SceneBankAssetRecord *firstRecord;
    unsigned i;
    u32 packed;
    u32 offsetMask;
    int index;
    int result;
    u32 matchingStackReserve[8];
retry:
    switch (state->entity_texture_phase) {
    case 0:
        if (state->flags & 0x200000)
            state->entity_texture_phase = 1;
        else
            state->entity_texture_phase = 6;
        break;
    case 1:
        index = bank + 3;
        if (CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[index],
                state->scene_load_scratch,
                g_PeImageSectorBoundsNext[index] - g_PeImageSectorTable[index]) != -1)
            state->entity_texture_phase = 2;
        return 1;
    case 2:
        result = CdRom_PollReady();
        if (result == -1) {
            state->entity_texture_phase = 1;
            return 1;
        }
        if (result != 0)
            return 1;
        if (state->flags & 0x20000) {
            if (!(state->flags & 0x80000) && g_AyaActionStep < 2)
                return 1;
            state->flags |= 0x40000;
        }
        state->entity_texture_phase = 3;
        break;
    case 3:
        blob = (SceneAssetBlob *)state->scene_load_scratch;
        directory = SceneAsset_ResolveOffset(blob, blob->directoryOffset);
        timOffset = directory->timEntries & 0x3fffff;
        firstTim = SceneAsset_ResolveOffset(blob, timOffset);
        i = 0;
        if (directory->timEntries >> 22) {
            tim = firstTim;
            do {
                Draw_LoadTimAsset(tim, blob);
                packed = directory->timEntries;
                ++i;
                ++tim;
            } while (i < (packed >> 22));
        }
        state->entity_texture_phase = 4;
        break;
    case 4:
        index = bank + 8;
        if (CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[index],
                state->voice_bank_base_1400,
                g_PeImageSectorBoundsNext[index] - g_PeImageSectorTable[index]) != -1)
            state->entity_texture_phase = 5;
        return 1;
    case 5:
        result = CdRom_PollReady();
        if (result == -1) {
            state->entity_texture_phase = 4;
            return 1;
        }
        /* Reuse the counter scratch for readiness before the next loop phase. */
        i = result != 0;
        if (i)
            return 1;
        state->entity_texture_phase = 6;
        break;
    case 6:
        blob = (SceneAssetBlob *)state->voice_bank_base_1400;
        directory = SceneAsset_ResolveOffset(blob, blob->directoryOffset);
        {
            SceneBankAssetRecord *root = SceneAsset_ResolveOffset(blob, directory->bankRootEntries & 0x3fffff);
            state->bank_slots[0] = SceneAsset_ResolveOffset(blob, root->source.offsetAndId & 0xffffff);
        }
        firstRecord = SceneAsset_ResolveOffset(blob, directory->bankRowEntries & 0x3fffff);
        i = 0;
        if (directory->bankRowEntries >> 22) {
            offsetMask = 0xffffff;
            record = firstRecord;
            do {
                unsigned id;
                union { Pe1GameState *state; u32 address; } destination;
                id = record->source.bytes.id;
                /* The row id indexes the flattened bank row table. */
                destination.state = state;
                destination.address = id * 4 + destination.address;
                destination.state->bank_rows[0][0] = SceneAsset_ResolveOffset(
                    blob, record->source.offsetAndId & offsetMask);
                packed = directory->bankRowEntries;
                ++i;
                ++record;
            } while (i < (packed >> 22));
        }
        state->entity_texture_phase = 0;
        state->loaded_entity_bank = state->requested_entity_bank;
        state->flags &= ~0x200000;
        return 0;
    default:
        return 0;
    }
    goto retry;
}

/* Scene start-up steps 0x20..0x27: area type and disc swap bookkeeping,
 * the entity texture banks, then the player's model and render object. */
s32 Scene_InitEntityPlayer(s32 resume) {
    Pe1GameState *gameState = &g_GameState;
    s8 *renderSetup;
    u32 area;
    u8 saved;

    switch (g_GameState.scene_init_phase) {
    case 32:
        g_GameState.flags |= 0x20000;
        if (resume) {
            saved = g_SceneAreaType;
            g_SceneAreaType = 0xE;
            g_GameState.scene_area_type_backup = saved;
        } else {
            g_SceneAreaType = g_GameState.scene_area_type_backup;
        }
        gameState->scene_init_phase = 0x21;
        return 1;
    case 33:
        gameState->scene_init_phase = 0x22;
        return 1;
    case 34:
        gameState->scene_init_phase = 0x23;
        return 1;
    case 35:
        area = g_SceneAreaType - 0xA;
        if (area < 5) {
            saved = g_SavedSceneAreaType;
            if (g_SceneAreaType != saved) {
                g_GameStateFlagsWord = g_GameState.flags | 0x200000;
            }
            if ((area >> 1) != ((saved - 0xA) / 2)) {
                g_DiscChangeFlags |= 4;
            }
        }
        gameState->scene_init_phase = 0x24;
        return 1;
    case 36:
        if (Scene_LoadEntityTexture() == 1) {
            return 1;
        }
        if (resume) {
            gameState->scene_init_phase = 0x27;
            return 1;
        }
        gameState->scene_init_phase = 0x25;
        return 1;
    case 37:
        g_GameState.story_day_flags |= 4;
        Scene_SetStoryDay(g_CurrentStoryDay[0]);
        gameState->scene_init_phase = 0x26;
        return 1;
    case 38:
        if (Scene_LoadEntityTextures() == 1 && gameState->scene_init_subphase < 0xB) {
            return 1;
        }
        gameState->scene_init_phase = 0x27;
        return 1;
    case 39:
        {
            FieldActor *player = g_PlayerEntity;
            player->model_state.model_header = g_GameState.bank_slots[0];
            player->action_data = g_GameState.bank_rows[0][0x15];
            Entity_SetActionMode((BattleEntity *)player, 0x15);
        }
        {
            FieldActor *player = g_PlayerEntity;
            Render_SetupEntityPrims(&player->render_object, player->model_state.model_header,
                                    (u8 *)(player->allocation_block + 0x50), 0x3C0, 0x100, 0,
                                    0x1C0, 2, &renderSetup, 1);
        }
        {
            FieldActor *player = g_PlayerEntity;
            Render_InitRoomPrimState(&player->render_object);
        }
        {
            FieldActor *player = g_PlayerEntity;
            Render_DrawWithAnim(&player->render_object, player->action_data, 0,
                                &g_RenderLightingState);
        }
        {
            FieldActor *player = g_PlayerEntity;
            player->render_object.header->shadow_radius = player->render_object.hit_cylinder.radius * 2;
        }
        g_GameStateFlagsWord = g_GameState.flags & ~0x60000;
        if (resume) {
            gameState->flags |= 0x80000;
        } else {
            gameState->flags &= ~0x80000;
        }
        gameState->scene_init_phase = 0x20;
        /* fallthrough */
    default:
        return 0;
    }
}

/* Existing byte symbols for the shared state's pending/current day and flags.
 * Preserve the unsigned pending-byte read before its signed-day conversion. */

int Scene_SetStoryDay(s32 storyDay) {
    Pe1GameState *gameState = &g_GameState;
    u8 flags;

    if (storyDay == -1) {
        u8 pending;
        u8 storyFlags;
        pending = g_PendingStoryDay;
        storyFlags = g_DiscChangeFlags;
        storyDay = (s8)pending;
        g_CurrentStoryDay[0] = pending;
        g_DiscChangeFlags = storyFlags | 3;
    }

    if (((g_GameStateFlags & 2) != 0) || ((gameState->flags & 2) != 0)) {
        g_DiscChangeFlags |= 2;
        g_FieldMoveLock &= ~2U;
    }

    flags = gameState->story_day_flags;
    if ((flags & 4) != 0) {
        gameState->story_day_flags = (flags | 3) & ~4;
    }

    if ((storyDay - 1U) < 8U) {
        if (storyDay != gameState->pending_story_day) {
            s8 newStoryDay;

            newStoryDay = storyDay;
            gameState->pending_story_day = newStoryDay;
            gameState->current_story_day = newStoryDay;
            gameState->story_day_flags |= 1;
        }
    }

    return 0;
}


/* Streams the area entity bank and rebuilds the scene render object. */
int Scene_LoadEntityTextures(void)
{
    int area = g_SceneAreaType;
    int day = g_CurrentStoryDay[0];
    u32 lba = g_PeImageBaseLba;
    Pe1GameState *state = &g_GameState;
    SceneAssetView *view;
    SceneAssetDirectory *directory;
    TimUploadRecord *tim;
    TimUploadRecord *firstTim;
    SceneBankAssetRecord *firstRecord;
    SceneBankAssetRecord *root;
    RenderObjectHeader *model;
    u32 streamEntries, packed;
    u32 i;
    int result;
    int sectorIndex;
    s8 *textureData;

    if (area - 10U >= 5)
        return 0;
retry:
    switch (state->scene_init_subphase) {
    case 0:
        if (state->story_day_flags & 1) {
            state->scene_init_subphase = 1;
            break;
        }
        if (state->story_day_flags & 2) {
            state->scene_init_subphase = 11;
            break;
        }
        Scene_UpdateBgDraw();
        return 0;
    case 1:
        if (day - 2U >= 5) {
            state->scene_init_subphase = 4;
            break;
        }
        sectorIndex = day + 0x26;
        result = CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[sectorIndex],
            state->scene_load_scratch,
            g_PeImageSectorBoundsNext[sectorIndex] - g_PeImageSectorTable[sectorIndex]);
        Scene_UpdateBgDraw();
        if (result != -1)
            state->scene_init_subphase = 2;
        return 1;
    case 2:
        result = CdRom_PollReady();
        if (result == -1) {
            state->scene_init_subphase = 1;
            return 1;
        }
        if (result != 0) {
            if (!(g_GameStateFlags & 2))
                Scene_UpdateBgDraw();
            return 1;
        }
        state->scene_init_subphase = 3;
        break;
    case 3:
        view = state->scene_load_scratch;
        directory = SCENE_ASSET_AT(view, view->header.directoryOffset);
        firstTim = SCENE_ASSET_AT(view, directory->timEntries & 0x3fffff);
        i = 0;
        if (directory->timEntries >> 22) {
            tim = firstTim;
            do {
                Draw_LoadTimAsset(tim, view);
                packed = directory->timEntries;
                i++;
                tim++;
            } while (i < packed >> 22);
        }
        state->scene_init_subphase = 4;
        break;
    case 4:
        sectorIndex = (state->requested_entity_bank - 10) / 2 * 8 + day + 0x16;
        if (CdRom_ReadSectorsFromLba(lba + g_PeImageSectorTable[sectorIndex],
                state->entity_texture_blob,
                g_PeImageSectorBoundsNext[sectorIndex] - g_PeImageSectorTable[sectorIndex]) != -1)
            state->scene_init_subphase = 5;
        if (!(g_GameStateFlags & 2))
            Scene_UpdateBgDraw();
        return 1;
    case 5:
        result = CdRom_PollReady();
        if (result == -1) {
            state->scene_init_subphase = 4;
            return 1;
        }
        if (result != 0) {
            if (!(g_GameStateFlags & 2))
                Scene_UpdateBgDraw();
            return 1;
        }
        state->scene_init_subphase = 6;
        break;
    case 6:
        view = state->entity_texture_blob;
        directory = SCENE_ASSET_AT(view, view->header.directoryOffset);
        firstRecord = SCENE_ASSET_AT(view, directory->bankRowEntries & 0x3fffff);
        i = 0;
        if (directory->bankRowEntries >> 22) {
            do {
                state->bank_rows[0][firstRecord[i].source.bytes.id] = SCENE_ASSET_AT(view, firstRecord[i].source.offsetAndId & 0xffffff);
                packed = directory->bankRowEntries;
                i++;
            } while (i < packed >> 22);
        }
        state->cd_range_read_mode = 0;
        for (i = 0; i < 3; i++)
            state->scene_object_tables[i] = 0;
        streamEntries = directory->streamEntries;
        if (streamEntries & 0xffc00000) {
            SceneCdStreamRecord *first = SCENE_ASSET_AT(view, streamEntries & 0x3fffff);
            i = 0;
            if (streamEntries >> 22) {
                do {
                    state->scene_object_tables[i] = SCENE_ASSET_AT(view, first[i].offset & 0xffffff);
                    i++;
                } while (i < directory->streamEntries >> 22);
            }
            state->cd_range_read_mode = first->cdIndex;
        }
        if (g_GameStateFlags & 2) {
            state->scene_init_subphase = 7;
            return 1;
        }
        if (state->story_day_flags & 2) {
            state->scene_init_subphase = 11;
            break;
        }
        Scene_UpdateBgDraw();
        state->scene_init_subphase = 0;
        state->story_day_flags &= ~3;
        return 0;
    case 7:
        if (CdRom_ReadAssetSectors(1, state->cd_range_read_mode, 0,
                state->scene_load_scratch, 0x21, 0) == 0) {
            state->scene_init_subphase = 11;
            break;
        }
        return 1;
    case 11:
        state->story_day_flags |= 1;
        state->scene_init_subphase = 12;
        return 1;
    case 12:
        state->scene_init_subphase = 13;
        return 1;
    case 13:
        view = state->entity_texture_blob;
        directory = SCENE_ASSET_AT(view, view->header.directoryOffset);
        firstRecord = SCENE_ASSET_AT(view, directory->bankRowEntries & 0x3fffff);
        i = 0;
        if (directory->bankRowEntries >> 22) {
            do {
                state->bank_rows[0][firstRecord[i].source.bytes.id] = SCENE_ASSET_AT(view, firstRecord[i].source.offsetAndId & 0xffffff);
                packed = directory->bankRowEntries;
                i++;
            } while (i < packed >> 22);
        }
        state->cd_range_read_mode = 0;
        for (i = 0; i < 3; i++)
            state->scene_object_tables[i] = 0;
        streamEntries = directory->streamEntries;
        if (streamEntries & 0xffc00000) {
            SceneCdStreamRecord *first = SCENE_ASSET_AT(view, streamEntries & 0x3fffff);
            i = 0;
            if (streamEntries >> 22) {
                do {
                    state->scene_object_tables[i] = SCENE_ASSET_AT(view, first[i].offset & 0xffffff);
                    i++;
                } while (i < directory->streamEntries >> 22);
            }
            state->cd_range_read_mode = first->cdIndex;
        }
        if (g_PlayerEntity == 0) {
            u32 sceneFlag = state->flags & 2;
            return sceneFlag == 0;
        }
        if (g_GameStateFlags & 2) {
            root = SCENE_ASSET_AT(view, directory->bankRootEntries & 0x3fffff);
            model = SCENE_ASSET_AT(view, root->source.offsetAndId & 0xffffff);
        } else {
            model = state->scene_object_model;
        }
        Render_SetupEntityPrims(&state->scene_object, model,
            state->scene_object_work, 0x2C0, 0x80, 0, 0x1C2, 0, &textureData, 1);
        if (state->scene_object.animation_source == 0 && g_PlayerEntity != 0) {
            state->scene_object.animation_source = &g_PlayerEntity->render_object;
            state->scene_object.animation_state = 3;
            state->scene_object.animation_id = 0x12;
        }
        state->scene_object.shade = 0x80;
        state->scene_object.lightNegativeY = 0xC;
        state->scene_object.lightPositiveY = 0x18;
        Render_InitRoomPrimState(&state->scene_object);
        Render_DrawWithAnim(&state->scene_object, 0, 0, &g_RenderLightingState);
        state->scene_init_subphase = 0;
        state->story_day_flags &= ~3;
        return 1;
    default:
        return 0;
    }
    goto retry;
}

/* Draws the scene's background object animated from the player's render
 * object. */
int Scene_UpdateBgDraw(void) {
    Pe1GameState *state = &g_GameState;
    FieldActor *entity;

    if (state->flags & 0xC0000) {
        return 0;
    }

    entity = g_PlayerEntity;
    if (entity == 0) {
        return 0;
    }

    if (entity->flags & 0x20000040) {
        return 0;
    }

    if (!(g_GameStateFlags & 2) && (g_FieldMoveLock & 2)) {
        return 0;
    }

    if (g_GameState.scene_object.animation_source == 0) {
        g_GameState.scene_object.animation_source = &entity->render_object;
        g_GameState.scene_object.animation_state = 3;
        g_GameState.scene_object.animation_id = 0x12;
    }

    Render_SetGteScreenOffset();
    Render_TransformVertices(&state->scene_object);
    Render_TransformMorphVertices(&state->scene_object, g_EntityRenderScratch);

    if ((g_GameStateFlags & 2) || g_PlayerEntity->render_object.variant_visible != 0) {
        Render_DrawEntity(&state->scene_object, g_EntityRenderScratch);
    }

    Render_ResetGteScreenOffset();
    return 0;
}
