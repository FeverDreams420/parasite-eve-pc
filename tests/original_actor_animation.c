#include "pe1/resident_tables.h"
#include "pe1/draw_area.h"
#include "pe1/textbox.h"
#include "pe1/memcard.h"
void DeliverEvent(unsigned event,unsigned spec);
#include "pe1/battle_runtime.h"
#include "pe1/field_sfx.h"
#include "pe1/psyq_callbacks.h"
#include "pe1/boot.h"
#include "pe1/cdrom_buffers.h"
#include "pe1/draw_buffers.h"
#include "pe1/save.h"
#include "pe1/game_timers.h"
#include "pe1/random.h"
#include <stdint.h>
void GameTime_InitCounter0(void);
void GameTime_InitCounter1(void);
void GameTime_InitCounter2(void);
void Inv_SetDefaultItemId(void);
int Inv_ScrambleGrid(void);
#include "pe1/entity_animation.h"
#include "pe1/room_m005_drifting_sprite.h"
#include "pe1/room_m005.h"
#include "pe1/room_script_record.h"
#include "pe1/room_m005_beacon.h"
#include "../src/overlays/room_m005_theater/RoomM005_FxOrbiter_externs.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_rotated_quad.h"
#include "pe1/field_shape_quads.h"
#include "pe1/field_star_fan.h"
#include "pe1/field_texture_tables.h"
#include "../src/overlays/room_lib/room_lib.h"
#include "pe1/game_state.h"
#include "pe1/player_entity.h"
#include "pe1/scene_assets.h"
#include <stdio.h>
#include <string.h>
#include "pe1/boot_services.h"
#include "pe1/font.h"
#include "pe1/cdrom.h"
#include "pe1/entity_allocation.h"
#include "pe1/render_buffers.h"
#include "pe1/render_packets.h"
#include "pe1/render_prim.h"
#include "pe1/render_matrix_slot.h"
#include "pe1/field_tile.h"
#include "pe1/field_textured_model.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_oriented_sprite.h"
#include "pe1/field_shaded_ring.h"
#include "pe1/field_shaded_quad.h"
#include "pe1/field_glow_layers.h"
#include "pe1/fieldfx_state.h"
#include "pe1/field_effect_modules.h"
#include "pe1/pm.h"
#include "pe1/akao/commands.h"
#include "pe1/akao_send.h"
#include "pe1/script_state.h"
#include "pe1/script_queue.h"
#include "pe1/akao/queue.h"
#include "pe1/fieldfx_particle.h"
#include "pe1/field_tri_test.h"
#include "pe1/field_collision.h"
#include "pe1/field_textured_strip.h"
#include "pe1/geom_state.h"
#include "pe1/field_movement.h"
#include "pe1/pad_state.h"
#include "pe1/scene_flags.h"
#include "pe1/entity_frame_update.h"
#include "pe1/cdrom.h"
#include "pe1/gte.h"
#include "pe1/menu_widget.h"
#include "pe1/menu_queue.h"
#include "pe1/draw_state.h"
#include "pe1/draw_level_bar.h"
#include "pe1/gpu_image.h"
#include "pe1/akao.h"
#include "pe1/memcard_save_state.h"
#include "pe1/save_tail_state.h"
#include "pe1/menu_dialog.h"
#include "pe1/text.h"
#include "pe1/menu_notification_dialog.h"
#include "pe1/menu_state.h"
#include "pe1/menu_inventory.h"
#include "pe1/inventory_slots.h"
#include "pe1/stat_modifiers.h"
#include "../src/overlays/menu_memcard/menu_memcard_screen.h"
#include "../src/overlays/menu_memcard/menu_memcard_image.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <limits.h>
#include <psyz/system.h>

/* Observe the original traversal while deliberately changing cursor state. */
static int widget_draw_order[4], widget_draw_x[4], widget_draw_y[4];
static unsigned widget_draw_count;
static void record_widget_draw(MenuWidgetNode *node)
{
    if (widget_draw_count>=4) abort();
    unsigned i=widget_draw_count++;
    widget_draw_order[i]=node->mode;
    widget_draw_x[i]=g_TextCursorX;
    widget_draw_y[i]=g_TextCursorY;
    g_TextCursorX=900; g_TextCursorY=-700;
}

static unsigned menu_event_count;
static int menu_event_nodes[8], menu_event_flags[8];
static int record_menu_event(MenuWidgetNode *node,unsigned flags)
{
    if(menu_event_count>=8)abort();
    menu_event_nodes[menu_event_count]=node->mode;
    menu_event_flags[menu_event_count++]=(int)flags;
    return node->mode==2;
}

static int boot_buffer_layout(void)
{
    Pe1GameState saved = g_GameState;
    Boot_InitGameState();
    if (g_GameState.bank_asset_source != g_BootWorkArea ||
        g_GameState.voice_bank_base != g_BootWorkArea + 0x1800 ||
        g_GameState.voice_bank_base_1400 != g_BootWorkArea + 0x6000 ||
        (void *)g_GameState.entity_texture_blob != g_BootWorkArea + 0x14000 ||
        g_GameState.scene_object_work != D_80120D08 ||
        g_GameState.str_file_directory != g_StrFileDirBuffer ||
        (void *)g_GameState.loaded_scene_assets != g_StrFileDirBuffer - 8 ||
        g_GameState.scene_load_scratch != g_SceneLoadScratchArea ||
        g_GameState.bank_work_base != g_GameState.voice_bank_base ||
        g_GameState.bank_work_end != g_GameState.voice_bank_base + 0x1400 ||
        g_GameState.bank_work_far_end != g_GameState.voice_bank_base + 0x2800 ||
        !g_GameState.scene_process_slots || !g_GameState.script_sprite_buffer ||
        !g_GameState.unk_170) return 0;
    DrawFrameBuffer saved_frames[2];
    memcpy(saved_frames,g_DrawFrameBuffers,sizeof(saved_frames));
    memset(g_DrawFrameBuffers,0xA5,sizeof(g_DrawFrameBuffers));
    Draw_InitBuffers();
    for(unsigned i=0;i<2;++i) {
        const DrawFrameBuffer *frame=&g_DrawFrameBuffers[i];
        if(frame->frontBufferBase!=(i?g_RenderBackBufferBase[0]:g_RenderFrontBufferBase[0]) ||
           (void *)frame->orderingTableBase!=(i?(void *)g_RenderOtBufferBaseAlt[0]:(void *)g_OtBufferTable[0]) ||
           frame->draw.clip.x || frame->draw.clip.y!=(i?224:0) ||
           frame->draw.clip.w!=320 || frame->draw.clip.h!=224 ||
           frame->draw.ofs[0] || frame->draw.ofs[1]!=(i?224:0) ||
           frame->draw.isbg!=1 || frame->draw.r0 || frame->draw.g0 || frame->draw.b0 ||
           frame->display.disp.x || frame->display.disp.y!=(i?0:224) ||
           frame->display.disp.w!=320 || frame->display.disp.h!=224 ||
           frame->display.screen.y!=8 || frame->display.screen.h!=224) return 0;
    }
    if(g_TextCursorX || g_TextCursorY || g_TextCursorStackPtr!=g_TextCursorStackBottom ||
       g_DrawPrimColor!=0x808080 || g_DrawColorShaded!=0x404040 ||
       g_DrawDigitFontTpageClut!=0x395D || g_DrawDigitFontBaseTexU!=0x84 ||
       g_DrawDigitFontBaseTexV!=0xA4 || g_DrawPresentImage) return 0;
    DISPENV saved_display[2];
    memcpy(saved_display,g_RenderDispEnvArray,sizeof(saved_display));
    if(g_DrawBaseX[0]!=g_GameState.scene_audio.tracks.reserved[0] ||
       g_DrawBaseY!=g_GameState.scene_audio.tracks.reserved[1]) return 0;
    Draw_SetBaseOffsetPosition(0,15-g_DrawBaseY);
    if(g_DrawBaseY!=15 || g_GameState.scene_audio.tracks.reserved[1]!=15 ||
       g_RenderDispEnvArray[0].screen.y!=15 || g_RenderDispEnvArray[1].screen.y!=15 ||
       Draw_GetBaseY()!=15) return 0;
    Draw_SetBaseOffsetPosition(0,1);
    if(g_DrawBaseY!=15) return 0;
    Draw_SetBaseOffsetPosition(0,-15);
    if(g_DrawBaseY || Draw_GetBaseY() || g_RenderDispEnvArray[0].screen.y ||
       g_RenderDispEnvArray[1].screen.y) return 0;
    Draw_SetBaseOffsetPosition(0,-1);
    if(g_DrawBaseY) return 0;
    memcpy(g_RenderDispEnvArray,saved_display,sizeof(saved_display));
    memcpy(g_DrawFrameBuffers,saved_frames,sizeof(saved_frames));
    RenderNativeTag *first = (RenderNativeTag *)g_OtBufferTable[0];
    RenderNativeTag *second = (RenderNativeTag *)g_RenderOtBufferBaseAlt[0];
    if (D_800B0E38.ordering[0]!=first || D_800B0E38.ordering[1]!=second ||
        (void *)g_GameState.bg_tim!=g_RenderScratchBufferBase[0] ||
        (void *)g_GameState.save_background_source!=g_RenderFrontBufferBase[0] ||
        (void *)g_GameState.save_background_destination!=g_RenderBackBufferBase[0] ||
        (void *)g_GameState.hud_tim!=g_RenderAnimBufferBase[0] ||
        (void *)g_GameState.script_sprite_buffer!=g_RenderAnimBufferBase[1] ||
        first + 4096 != second ||
        (void *)(second + 4096) != (void *)g_GameState.object_texture_blob)
        return 0;
    first[4095].length = 0x1234;
    second[0].length = 0x5678;
    second[4095].length = 0x9abc;
    memset(g_EntityAllocationTable, 0, sizeof(g_EntityAllocationTable));
    for (unsigned i = 0; i < 16; ++i) {
        u8 *block = Entity_AllocBlock(ENTITY_ALLOCATION_BLOCK_SIZE);
        if (!block || block + ENTITY_ALLOCATION_BLOCK_SIZE > g_RenderFrontBufferBase[0])
            return 0;
        block[ENTITY_ALLOCATION_BLOCK_SIZE - 1] = (u8)i;
    }
    if (Entity_AllocBlock(1) || first[4095].length != 0x1234 ||
        second[0].length != 0x5678 || second[4095].length != 0x9abc)
        return 0;
    memset(g_EntityAllocationTable, 0, sizeof(g_EntityAllocationTable));
    g_GameState = saved;
    return 1;
}

/* Scripted I/O exercises the original glyph-record loader, including retries. */
static u8 glyph_source[42 * 2048], glyph_scratch[42 * 2048];
static unsigned glyph_reads, glyph_polls;
static PeDisc *glyph_disc;
static PeDiscFile glyph_archive;
static int glyph_io_failed;
static int glyph_read_retry, glyph_poll_retry, glyph_pending;
int CdRom_ReadSectorsFromLba(u32 lba, void *buffer, u32 count)
{
    ++glyph_reads;
    if (glyph_disc) {
        if (lba < glyph_archive.sector || buffer != glyph_scratch ||
            count > sizeof(glyph_scratch) / 2048 ||
            !pe_disc_read(glyph_disc, &glyph_archive,
                          (uint64_t)(lba - glyph_archive.sector) * 2048,
                          buffer, (size_t)count * 2048)) {
            glyph_io_failed = 1;
            return -2;
        }
        return 1;
    }
    if (lba != 12352 || buffer != glyph_scratch || count != 42) return -2;
    if (glyph_read_retry) { glyph_read_retry = 0; return -1; }
    memcpy(buffer, glyph_source, sizeof(glyph_source));
    return 1;
}
int CdRom_PollReady(void)
{
    ++glyph_polls;
    if (glyph_pending) { glyph_pending = 0; return 1; }
    if (glyph_poll_retry) { glyph_poll_retry = 0; return -1; }
    return 0;
}
static int glyph_records(void)
{
    Pe1GameState saved = g_GameState;
    g_GameState.pe_image_base_lba = 12345;
    g_GameState.scene_load_scratch = glyph_scratch;
    g_PeImageSectorTable[79] = 7;
    g_PeImageSectorTable[80] = 49;
    for (unsigned i = 0; i < sizeof(glyph_source); ++i)
        glyph_source[i] = (u8)((i * 37 + i / 328) ^ (i >> 8));
    g_FontActiveGlyphTable.codeCount = 113;
    glyph_read_retry = glyph_poll_retry = glyph_pending = 1;
    for (unsigned code = 0; code < 256; ++code) {
        if (Render_LoadFontGlyph(code) || g_FontActiveGlyphTable.codeCount != 113 ||
            memcmp(&g_FontActiveGlyphTable.groups, glyph_source + 1 + code * 328, 328))
            return 0;
    }
    g_GameState = saved;
    return glyph_reads == 258 && glyph_polls == 258;
}

static int glyph_selection(void)
{
    Pe1GameState saved = g_GameState;
    FontGlyphRecord record = {0};
    const u8 indices[6] = {2, 0, 3, 1, 4, 3};
    const u8 groups[6] = {1, 1, 2, 2, 1, 2};
    record.groups.count = 5;
    for (unsigned i = 0; i < 5; ++i) record.groups.codes[i] = i;
    record.slots.count = 6;
    memcpy(record.slots.indices, indices, sizeof(indices));
    memcpy(record.slots.groupKeys, groups, sizeof(groups));
    memcpy(glyph_source + 1 + 328, &record, sizeof(record));
    memcpy(glyph_source + 1 + 656, &record, sizeof(record));
    g_GameState.pe_image_base_lba = 12345;
    g_GameState.scene_load_scratch = glyph_scratch;
    g_FontGlyphCodeSequence[2] = 1;
    g_FontGlyphCodeSequence[3] = 2;
    g_FontSelectionState.table = &g_FontActiveGlyphTable;
    g_FontSelectionState.active = 1;
    if (Render_SetFontGlyphByCode(2) != 3 || g_FontSelectionState.selected != 2 ||
        Render_DrawTextDigit(NULL, 0) != 2 || Render_GetFontSlotCode(4) != 4 ||
        Menu_GetEquipSlotStateOrIndex() != 2 || !Menu_IsEquipSlotActive()) return 0;
    if (Render_GetOrLoadFontGlyph(22) != 1 || g_FontSelectionState.selected != 3 ||
        Render_GetOrLoadFontGlyph(23) != 4 || g_FontSelectionState.selected != 4 ||
        Render_GetOrLoadFontGlyph(20) != 0 || g_FontSelectionState.selected != 1 ||
        Render_GetOrLoadFontGlyph(21) != 2 || g_FontSelectionState.selected != 0) return 0;
    if (Menu_FindSelectedEquipSlotItem() != 3 || g_FontSelectionState.selected != 2 ||
        Render_FindFontGlyphSlot() != 2 || g_FontSelectionState.codeIndex != 3 ||
        Render_StepFontLoad() != 1 || g_FontSelectionState.codeIndex != 2) return 0;
    g_FontSelectionState.codeIndex = 70;
    if (Render_FindFontGlyphSlot() != 255 || !g_FontSelectionState.loadFailed ||
        Menu_ConsumeEquipSlotFlag() != 0 || Menu_ConsumeEquipSlotFlag() != 255) return 0;
    g_FontSelectionState.seed = 123;
    if (Render_ConsumeFontShuffleSeed() != 0 || Render_ConsumeFontShuffleSeed() != 255 ||
        Menu_ResetEquipSlotState() || Menu_IsEquipSlotActive() ||
        g_FontSelectionState.table || g_FontSelectionState.selected != 255) return 0;
    g_GameState = saved;
    return 1;
}

static int glyph_asset_records(PeDisc *disc, const PeDiscFile *exe)
{
    Pe1GameState saved = g_GameState;
    u16 range[2];
    unsigned slots = 0;
    if (!pe_disc_find(disc, "PE.IMG", &glyph_archive) ||
        !pe_disc_read(disc, exe, 0x800930d8u - 0x80010000u + 0x800 + 79 * 2,
                      range, sizeof(range)) || range[1] <= range[0]) return 0;
    size_t bytes = (size_t)(range[1] - range[0]) * 2048;
    if (bytes > sizeof(glyph_source) ||
        !pe_disc_read(disc, &glyph_archive, (uint64_t)range[0] * 2048,
                      glyph_source, bytes)) return 0;
    unsigned records = glyph_source[0];
    if (!records || 1 + records * 328 > bytes) return 0;
    g_PeImageSectorTable[79] = range[0];
    g_PeImageSectorTable[80] = range[1];
    g_GameState.pe_image_base_lba = glyph_archive.sector;
    g_GameState.scene_load_scratch = glyph_scratch;
    g_FontSelectionState.table = &g_FontActiveGlyphTable;
    g_FontActiveGlyphTable.codeCount = records;
    glyph_disc = disc;
    int ok = 1;
    for (unsigned code = 0; code < records && ok; ++code) {
        const u8 *raw = glyph_source + 1 + code * 328;
        if (Render_LoadFontGlyph(code) || glyph_io_failed ||
            memcmp(&g_FontActiveGlyphTable.groups, raw, 328) ||
            raw[2] > 24 || raw[27] > 100) { ok = 0; break; }
        for (unsigned slot = 0; slot < raw[27]; ++slot) {
            unsigned index = raw[28 + slot];
            if (index >= raw[2] || Render_GetFontSlotCode(slot) != raw[3 + index]) {
                ok = 0; break;
            }
            ++slots;
        }
        for (unsigned mode = 0; mode < 5; ++mode) {
            for (unsigned digit = 0; digit < 10; ++digit) {
                unsigned sought = mode == 0 ? 3 : mode == 1 ? (digit != 0) : mode == 2 ? 2 : 4;
                unsigned group = 0, expected = 255;
                g_FontSelectionState.codeIndex = digit;
                for (unsigned i = 0; i < raw[2]; ++i)
                    if (raw[3 + i] == sought) { group = i; break; }
                for (unsigned i = 0; i < raw[27]; ++i)
                    if (raw[28 + i] == group) { expected = i; break; }
                if (Render_DrawTextDigit(NULL, mode) != expected) ok = 0;
            }
        }
    }
    static const u32 seeds[] = {0, 1, 32767, 0xffffffffu};
    for (unsigned trial = 0; trial < 4 && ok; ++trial) {
        u8 sequence[71], seen[256] = {0};
        g_FontSelectionState.loadFailed = 1;
        g_FontSelectionState.seed = seeds[trial];
        int selected = Render_LoadFontGlyphAlt();
        unsigned rng_state = Psyz_RandState();
        memcpy(sequence, g_FontGlyphCodeSequence, sizeof(sequence));
        if (glyph_io_failed || !g_FontSelectionState.active ||
            g_FontSelectionState.codeIndex != 2 ||
            g_FontSelectionState.code != sequence[2] ||
            g_FontSelectionState.selected >= g_FontActiveGlyphTable.slots.count) {
            ok = 0; break;
        }
        for (unsigned i = 2; i < 71; ++i) {
            unsigned value = sequence[i];
            unsigned limit = i < 10 ? 104 : i < 20 ? 109 : 113;
            if (i % 10 == 0) {
                if (value != i / 10 - 1) ok = 0;
            } else {
                if (value < 7 || value >= limit || value >= records || seen[value]) ok = 0;
                seen[value] = 1;
            }
        }
        g_FontSelectionState.seed = seeds[trial];
        if (Render_LoadFontGlyphAlt() != selected ||
            memcmp(sequence, g_FontGlyphCodeSequence, sizeof(sequence)) ||
            Psyz_RandState() != rng_state) ok = 0;
    }
    glyph_disc = NULL;
    g_GameState = saved;
    Menu_ResetEquipSlotState();
    if (ok) printf("Original glyph loader: %u real records, %u slots, %u mode queries\n",
                   records, slots, records * 50);
    return ok;
}

/* Record positional sound requests; no replacement audio ships with the game. */
static unsigned sounds;
int Asset_Find08w(int id,int mode,int x,int y,int z)
{
    (void)mode;(void)x;(void)y;(void)z;if(id==1000)++sounds;return 0;
}
static int host_event(Uint32 type, SDL_Scancode key) {
    SDL_Event event={0};event.type=type;event.key.scancode=key;
    if(!SDL_PushEvent(&event))return 0;
    while(SDL_PollEvent(&event))Boot_HandlePadEvent(&event);
    return 1;
}
/* Reset must clear the whole native render object while keeping mounted data
 * and allocated buffers. Poison catches the historical 49-word partial clear. */
static int game_state_reset(void)
{
    Pe1GameState saved = g_GameState;
    static u8 voice[0x3000];
    u8 zero[sizeof(RenderObjectEntity)] = {0};
    u8 tail[8] = {0};
    int ok;
    memset(&g_GameState, 0x5a, sizeof(g_GameState));
    g_GameState.voice_bank_base = voice;
    g_GameState.scene_load_scratch = voice + 32;
    g_GameState.bank_rows[9][47] = voice + 64;
    g_GameState.pe_image_base_lba = 12345;
    Asset_ResetGameState();
    ok = 1;
    if (!(!memcmp(&g_GameState.scene_object, &zero, sizeof(zero)))) { fprintf(stderr, "game-state reset check 0 failed\n"); ok = 0; }
    if (!(!memcmp(g_GameState.reserved_scene_object_tail, tail, sizeof(tail)))) { fprintf(stderr, "game-state reset check 1 failed\n"); ok = 0; }
    if (!(g_GameState.flags == 3)) { fprintf(stderr, "game-state reset check 2 failed\n"); ok = 0; }
    if (!(g_GameState.saved_state_004 == 10)) { fprintf(stderr, "game-state reset check 3 failed\n"); ok = 0; }
    if (!(g_GameState.field_bg_cache_id == -1)) { fprintf(stderr, "game-state reset check 4 failed\n"); ok = 0; }
    if (!(g_GameState.room_type == 2)) { fprintf(stderr, "game-state reset check 5 failed\n"); ok = 0; }
    if (!(g_GameState.scene_area_type_backup == 0)) { fprintf(stderr, "game-state reset check 6 failed\n"); ok = 0; }
    if (!(g_GameState.pe_image_base_lba == 12345)) { fprintf(stderr, "game-state reset check 7 failed\n"); ok = 0; }
    if (!(g_GameState.voice_bank_base == voice)) { fprintf(stderr, "game-state reset check 8 failed\n"); ok = 0; }
    if (!(g_GameState.bank_work_base == voice)) { fprintf(stderr, "game-state reset check 9 failed\n"); ok = 0; }
    if (!(g_GameState.bank_work_end == voice + 0x1400)) { fprintf(stderr, "game-state reset check 10 failed\n"); ok = 0; }
    if (!(g_GameState.bank_work_far_end == voice + 0x2800)) { fprintf(stderr, "game-state reset check 11 failed\n"); ok = 0; }
    if (!(g_GameState.scene_load_scratch == voice + 32)) { fprintf(stderr, "game-state reset check 12 failed\n"); ok = 0; }
    if (!(g_GameState.bank_rows[9][47] == voice + 64)) { fprintf(stderr, "game-state reset check 13 failed\n"); ok = 0; }
    if (!(g_GameState.scene_audio.tracks.keys[0][0] == -1)) { fprintf(stderr, "game-state reset check 14 failed\n"); ok = 0; }
    if (!(g_GameState.scene_audio.tracks.keys[1][1] == -1)) { fprintf(stderr, "game-state reset check 15 failed\n"); ok = 0; }
    if (!(g_GameState.scene_audio.tracks.movie_frame == -1)) { fprintf(stderr, "game-state reset check 16 failed\n"); ok = 0; }
    if (!(g_GameState.pending_sample_bank == -1)) { fprintf(stderr, "game-state reset check 17 failed\n"); ok = 0; }
    if (!(g_GameState.scene_init_phase == 0x20)) { fprintf(stderr, "game-state reset check 18 failed\n"); ok = 0; }
    if (!(g_GameState.track_volumes[0] == 0x7f)) { fprintf(stderr, "game-state reset check 19 failed\n"); ok = 0; }
    if (!(g_GameState.track_volumes[1] == 0x7f)) { fprintf(stderr, "game-state reset check 20 failed\n"); ok = 0; }
    if (g_GameState.fade_tile.tag.len != 3 || g_GameState.fade_tpage.tag.len != 1 ||
        g_GameState.fade_tpage.mode != 0xe1000440u || g_GameState.fade_tile.code != 0x62 ||
        g_GameState.fade_tile.r != 32 || g_GameState.fade_tile.g != 32 ||
        g_GameState.fade_tile.b != 32 || g_GameState.fade_tile.x || g_GameState.fade_tile.y ||
        g_GameState.fade_tile.w != 320 || g_GameState.fade_tile.h != 224) {
        fprintf(stderr, "game-state fade packet initialization failed\n"); ok = 0;
    }
    g_GameState = saved;
    return ok;
}

static int bios_random(void)
{
    static const int expected[] = {16838, 5758, 10113, 17515, 31051, 5627, 23010, 7419};
    srand(1);
    for (unsigned i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i)
        if (rand() != expected[i]) return 0;
    if (Psyz_RandState() != 0x9cfbae39u) return 0;
    unsigned saved = Psyz_RandState();
    int next = rand();
    srand(saved);
    if (rand() != next) return 0;
    static const u32 seeds[] = {0, 1, 0x80000000u, 0xffffffffu, 12345};
    for (unsigned seed = 0; seed < sizeof(seeds) / sizeof(seeds[0]); ++seed) {
        uint64_t reference = seeds[seed];
        srand(seeds[seed]);
        for (unsigned i = 0; i < 10000; ++i) {
            reference = (reference * 1103515245ULL + 12345) % 4294967296ULL;
            if (rand() != (int)((reference / 65536) % 32768) ||
                Psyz_RandState() != reference) return 0;
        }
    }
    return 1;
}

/* Controlled draw-buffer input for the original packet linker. */

static RoomEnt *expected_room_context;
static unsigned room_callback_count;
static void record_room_callback(RoomEnt *context)
{
    if(context!=expected_room_context) abort();
    ++room_callback_count;
    context->w04+=7;
}
static int room_owner_state(void)
{
    FieldActor actor={0}; FieldActorState child={0};
    RoomEnt room={0}; room.link=(RoomLink *)&actor; actor.state=&child;
    struct FieldFxTaskOwner *owner=(struct FieldFxTaskOwner *)&room;
    room.t19=4; actor.mode=1;
    if(FieldFx_IsOwnerBusy(owner)!=1) return 0;
    actor.mode=2; room.t19=2; actor.flags=0x88000;
    if(FieldFx_IsOwnerBusy(owner)!=1 || actor.flags!=0x8000 || FieldFx_IsOwnerBusy(owner)!=0) return 0;
    room.t19=1; actor.flags=0x48000;
    if(FieldFx_IsOwnerBusy(owner)!=1 || actor.flags!=0x8000 || FieldFx_IsOwnerBusy(owner)!=0) return 0;
    room.t19=0;
    child.core_flags=0x1800;
    if(FieldFx_IsOwnerBusy(owner)!=1) return 0;
    for(unsigned phase=1;phase<8;++phase) {
        child.core_flags=phase<<1;
        if(FieldFx_IsOwnerBusy(owner)!=1) return 0;
    }
    child.core_flags=0;
    if(FieldFx_IsOwnerBusy(owner)!=0) return 0;
    actor.action=10; actor.anim_prev=10<<16;
    room.active=2; room.t18=3;
    FieldFx_LoopLinkedAnimation(&room);
    if(room.active!=1 || actor.anim.fixed!=3) return 0;
    room.active=-1; room.t18=-1;
    FieldFx_LoopLinkedAnimation(&room);
    if(room.active!=-1 || actor.anim.fixed!=-1) return 0;
    room.active=0; actor.anim.fixed=123;
    FieldFx_LoopLinkedAnimation(&room);
    if(actor.anim.fixed!=123) return 0;
    room.active=2; actor.anim_prev=9<<16;
    FieldFx_LoopLinkedAnimation(&room);
    if(room.active!=2 || actor.anim.fixed!=123) return 0;
    puts("Original room owner: native busy flags, child phases and linked animation repeats passed");
    return 1;
}

typedef struct NativeListState { void *identity; int id, ticks; } NativeListState;
static unsigned list_draws;
static int record_list_callback(int mode,void *payload,intptr_t argument)
{
    NativeListState *state=payload;
    if(state->identity!=state || argument!=42 || (uintptr_t)payload%_Alignof(NativeListState)) abort();
    if(mode==1) {
        if(g_FieldFxCurrentAge!=state->ticks) abort();
        ++state->ticks;
        return state->id==1 && state->ticks==2;
    }
    if(mode!=2 || g_FieldFxCurrentAge!=state->ticks) abort();
    ++list_draws;
    return 0;
}
static int callback_list_storage(void)
{
    _Alignas(void *) u8 arena[256]; memset(arena,0xa5,sizeof(arena));
    int bytes=FieldFx_InitCallbackList((char *)arena,sizeof(NativeListState),3,record_list_callback);
    size_t stride=(sizeof(FieldFxCallbackListEntry)+sizeof(NativeListState)+_Alignof(FieldFxCallbackListEntry)-1)/
                  _Alignof(FieldFxCallbackListEntry)*_Alignof(FieldFxCallbackListEntry);
    if(bytes!=(int)(sizeof(FieldFxCallbackList)+3*stride)) return 0;
    NativeListState *states[3];
    for(unsigned i=0;i<3;++i) {
        states[i]=FieldFx_AcquireCallbackSlot((char *)arena);
        if((u8 *)states[i]!=arena+sizeof(FieldFxCallbackList)+i*stride+sizeof(FieldFxCallbackListEntry)) return 0;
        states[i]->identity=states[i]; states[i]->id=i+1; states[i]->ticks=0;
    }
    if(FieldFx_AcquireCallbackSlot((char *)arena)) return 0;
    FieldFxTaskContext context={0}; context.argument=42;
    g_FieldFxTaskContext=&context; g_FieldFxCurrentAge=91;
    if(FieldFx_UpdateCallbackList(arena)!=3 || g_FieldFxCurrentAge!=91) return 0;
    if(FieldFx_DrawCallbackList((char *)arena)!=3 || list_draws!=3 || g_FieldFxCurrentAge!=91) return 0;
    if(FieldFx_UpdateCallbackList(arena)!=3 || g_FieldFxCurrentAge!=91) return 0;
    if(FieldFx_DrawCallbackList((char *)arena)!=2 || list_draws!=5 || g_FieldFxCurrentAge!=91) return 0;
    NativeListState *reused=FieldFx_AcquireCallbackSlot((char *)arena);
    if(reused!=states[0] || reused->ticks!=2) return 0;
    FieldFxCallbackListEntry *entry=(FieldFxCallbackListEntry *)((u8 *)reused-sizeof(FieldFxCallbackListEntry));
    if(entry->age!=0 || entry->active!=1) return 0;
    for(size_t i=bytes;i<sizeof(arena);++i) if(arena[i]!=0xa5) return 0;
    FieldFxTaskSlot slot={0}; slot.end=(char *)arena;
    g_FieldFxActiveTaskSlot=&slot;
    char *out=NULL;
    if(FieldFx_AllocCallbackList(&out,0,sizeof(NativeListState),3,record_list_callback)!=bytes || out!=(char *)arena) return 0;
    if(FieldFx_InitCallbackList((char *)arena,-1,3,record_list_callback)!=-1 ||
       FieldFx_InitCallbackList((char *)arena,16,-1,record_list_callback)!=-1 ||
       FieldFx_InitCallbackList((char *)arena,16,INT_MAX,record_list_callback)!=-1 ||
       FieldFx_InitCallbackList((char *)arena+1,16,3,record_list_callback)!=-1) return 0;
    g_FieldFxActiveTaskSlot=NULL;
    if(FieldFx_AllocCallbackList(&out,0,16,3,record_list_callback)!=-1) return 0;
    FieldFxTaskContext room_context={0},*old_room_context=g_FieldFxTaskContext;
    g_FieldFxTaskContext=&room_context;room_context.variables[0]=7;
    intptr_t record=RoomLib_SetTaskVar0(0,0,0,0);
    if(record!=(intptr_t)(g_RoomM005ScriptAnchorView-4) || g_RoomM005ScriptFadeView!=(u8 *)record+12 ||
        room_context.variables[0]!=7 || RoomLib_SetTaskVar0(1,0,0,0)!=record || room_context.variables[0]!=1)return 0;
    room_context.flags=0x5a;room_context.argument=record;
    if(FX_TASK_AS(RoomM005EventState)->triggered!=1 || FX_TASK_AS(RoomDlgState)->h12!=1 ||
        FX_TASK_AS(RoomM005BeaconEvent)->active!=0x5a || FX_TASK_AS(RoomM005BeaconEvent)->variant!=1)return 0;
    FX_TASK_AS(RoomM005BeaconEvent)->variant=23;FX_TASK_AS(RoomDlgState)->bD=0x42;
    if(room_context.variables[0]!=23 || room_context.flags!=0x42 || room_context.argument!=record ||
        room_context.pc.script || room_context.cursor)return 0;
    GteShortVector *anchor=(void *)g_RoomM005ScriptAnchorView,*fade=(void *)g_RoomM005ScriptFadeView;
    *anchor=(GteShortVector){-123,456,-789,0};*fade=(GteShortVector){12,-34,56,0};
    s16 *record_words=(s16 *)record;
    if(record_words[2]!=-123 || record_words[3]!=456 || record_words[4]!=-789 ||
        record_words[6]!=12 || record_words[7]!=-34 || record_words[8]!=56)return 0;
    FieldFxTaskProgram room_program={.initialize=RoomLib_SetTaskVar0};
    FieldFxTaskOwner room_owner={0};FieldFxObjectPrefix *old_room_owner=g_FieldFxOwner;
    room_owner.tasks.table=&room_program.table;
    FieldFx_InitTaskContext((PmSlotHeader *)&room_owner);
    if(FieldFx_InitTaskProgram((PmSlotHeader *)&room_owner,0,1,0,0,0)!=0 ||
        room_owner.tasks.argument!=record || room_owner.tasks.variables[0]!=1)return 0;
    memset((void *)record,0,20);g_FieldFxTaskContext=old_room_context;g_FieldFxOwner=old_room_owner;
    if(&D_800F336C!=&g_FieldFxSpriteParams.palette || &D_800F3374!=&g_FieldFxSpriteParams.depth) return 0;
    if(&g_FieldFxTpageSlot1!=g_FieldFxTpageSlots+1 || &g_FieldFxTpageSlot2!=g_FieldFxTpageSlots+2 ||
        &g_FieldFxTpageSlot3!=g_FieldFxTpageSlots+3 || &g_FieldFxTpageSlot4!=g_FieldFxTpageSlots+4 ||
        &g_FieldFxTpageSlot9!=g_FieldFxTpageSlots+9 || &g_FieldFxTpageSlot11!=g_FieldFxTpageSlots+11 ||
        &g_FieldFxPaletteEntry3!=g_FieldFxPaletteTable+3 || &g_FieldFxTpageTableEntry1!=g_FieldFxTpageTable+1)return 0;
    /* Run the real M005 drift callback through the original list dispatcher:
     * age 6 moves once more before terminating; halfword motion wraps. */
    memset(arena,0xa5,sizeof(arena));
    bytes=FieldFx_InitCallbackList((char *)arena,sizeof(RoomM005DriftingSpriteState),3,RoomM005_BeaconDriftCallback);
    RoomM005DriftingSpriteState *drifts[3];
    for(unsigned i=0;i<3;++i) {
        drifts[i]=FieldFx_AcquireCallbackSlot((char *)arena);
        *drifts[i]=(RoomM005DriftingSpriteState){65530,3,32767,0,13,65534,32769};
    }
    for(unsigned frame=1;frame<=7;++frame) {
        if(FieldFx_UpdateCallbackList(arena)!=3 || g_FieldFxCurrentAge!=91) return 0;
        for(unsigned i=0;i<3;++i) {
            if(drifts[i]->x!=(u16)(65530+13*frame) || drifts[i]->y!=(u16)(3+65534*frame) ||
                drifts[i]->z!=(u16)(32767+32769*frame)) return 0;
        }
    }
    if(FieldFx_UpdateCallbackList(arena)!=0) return 0;
    RoomM005DriftingSpriteState *drift_reused=FieldFx_AcquireCallbackSlot((char *)arena);
    if(drift_reused!=drifts[0]) return 0;
    FieldFxCallbackListEntry *drift_entry=(void *)((u8 *)drift_reused-sizeof(FieldFxCallbackListEntry));
    if(drift_entry->age || drift_entry->active!=1) return 0;
    for(size_t i=bytes;i<sizeof(arena);++i)if(arena[i]!=0xa5)return 0;
    puts("Original M005 drift: native callback ABI, halfword motion, terminal frame and slot reuse passed");
    _Alignas(void *) u8 original_pool[1024];memset(original_pool,0xa5,sizeof(original_pool));
    RoomM005OrbiterState controller={.armed=1,.frame=12};
    FieldFxTaskSlot original_slot={.start=(char *)&controller,.end=(char *)original_pool};
    g_FieldFxActiveTaskSlot=&original_slot;
    int original_bytes=RoomM005_Orbiter(0,&controller,0);
    FieldFxCallbackList *original_list=(void *)original_pool;
    if(original_bytes<=0 || original_bytes>sizeof(original_pool) || controller.armed || controller.frame ||
        original_list->count!=24 || original_list->callback!=RoomM005_FxOrbiter)return 0;
    for(unsigned i=original_bytes;i<sizeof(original_pool);++i)if(original_pool[i]!=0xa5)return 0;
    g_FieldFxActiveTaskSlot=NULL;
    g_FieldFxTaskContext=NULL;
    puts("Original callback lists: aligned pointer payloads, update/draw ages, completion/reuse and guards passed");
    return 1;
}

typedef struct NativeTaskState { void *identity; int ticks; } NativeTaskState;
static unsigned task_draws;
static int task_pointer_value;
static intptr_t record_native_task_argument(int mode,intptr_t value,intptr_t b,intptr_t c)
{
    if(mode!=7 || value!=(intptr_t)&task_pointer_value || b!=value+sizeof(int) || c!=-9)abort();
    return value;
}
static int observe_native_task(int mode,void *payload,intptr_t argument)
{
    NativeTaskState *state=payload;
    if(argument==(intptr_t)&task_pointer_value) {
        if(mode==1)++*(int *)argument;
        if(mode==2)*(int *)argument+=10;
        return 0;
    }
    if(argument!=42 || (uintptr_t)payload%_Alignof(NativeTaskState)) abort();
    if(mode==0) { state->identity=state; state->ticks=0; return 0; }
    if(state->identity!=state) abort();
    if(mode==1) {
        if(g_FieldFxCurrentAge!=state->ticks) abort();
        return ++state->ticks==2;
    }
    if(mode!=2 || g_FieldFxCurrentAge!=state->ticks) abort();
    ++task_draws; return 0;
}
static int reject_task_allocation(int mode,void *payload,intptr_t argument)
{
    (void)mode; (void)argument;
    return FieldFx_InitCallbackList((char *)payload,16,1000,record_list_callback);
}
static int native_task_cycle(void)
{
    struct { FieldFxTaskOwner owner; u8 guard[16]; } storage;
    memset(&storage,0xa5,sizeof(storage));
    FieldFxTaskOwner *owner=&storage.owner;
    FieldFxTaskTable table={0};
    u16 script[]={1,0,9,2,0,0,4,2,0,0,0,0};
    table.callbacks[0]=observe_native_task; table.sizes[0]=sizeof(NativeTaskState);
    owner->tasks.table=&table; owner->tasks.pc.script=script; owner->tasks.argument=42;
    owner->prefix.actor=NULL; owner->prefix.reserved00=0;
    FieldFx_InitTaskContext((PmSlotHeader *)owner);
    if(owner->tasks.cursor!=owner->tasks.arena || owner->tasks.used || owner->tasks.count)
        return 0;
    for(unsigned slot=0;slot<8;++slot)
        if(owner->tasks.slots[slot].id!=0xffff || owner->tasks.slots[slot].start || owner->tasks.slots[slot].end)
            return 0;
    if(owner->tasks.table!=&table || owner->tasks.pc.script!=script) return 0;
    GteMatrix identity={0}; identity.m[0][0]=identity.m[1][1]=identity.m[2][2]=4096;
    GteMatrix *saved_view=g_ViewMatrixSlot.value; g_ViewMatrixSlot.value=&identity;
    if(FieldFx_UpdateTasks((PmSlotHeader *)owner)!=0 || owner->tasks.variables[0]!=9 || owner->tasks.count!=1 ||
       owner->tasks.delay!=2 || owner->tasks.slots[0].age!=1) return 0;
    if(FieldFx_DrawTasks((PmSlotHeader *)owner)!=0 || task_draws!=1) return 0;
    if(FieldFx_UpdateTasks((PmSlotHeader *)owner)!=0 || owner->tasks.count || owner->tasks.delay!=1 || owner->tasks.slots[0].id!=0xffff)
        return 0;
    if(FieldFx_UpdateTasks((PmSlotHeader *)owner)!=0 || owner->prefix.reserved00!=4 || owner->tasks.delay) return 0;
    g_ViewMatrixSlot.value=saved_view;
    FieldFxTaskProgram program={0};
    program.table.callbacks[0]=observe_native_task;program.table.sizes[0]=sizeof(NativeTaskState);
    program.initialize=record_native_task_argument;program.script=script;
    void *old_program=g_FieldFxTaskPrograms[32];g_FieldFxTaskPrograms[32]=&program;
    if(FieldFx_BindTaskProgram(owner,32)!=0 || owner->tasks.table!=&program.table || owner->tasks.pc.script!=script)return 0;
    FieldFx_InitTaskContext((PmSlotHeader *)owner);owner->tasks.argument=42;
    if(FieldFx_InitTaskProgram((PmSlotHeader *)owner,1,0,0,0,0)!=0 || owner->tasks.argument!=42)return 0;
    if(FieldFx_InitTaskProgram((PmSlotHeader *)owner,0,7,(intptr_t)&task_pointer_value,(intptr_t)&task_pointer_value+sizeof(int),-9)!=0 ||
        owner->tasks.argument!=(intptr_t)&task_pointer_value)return 0;
    task_pointer_value=0;
    if(FieldFx_SpawnTask(0)!=(int)sizeof(NativeTaskState))return 0;
    owner->tasks.pc.script=script+9;
    if(FieldFx_UpdateTasks((PmSlotHeader *)owner)!=0 || task_pointer_value!=1)return 0;
    g_ViewMatrixSlot.value=&identity;
    if(FieldFx_DrawTasks((PmSlotHeader *)owner)!=0 || task_pointer_value!=11)return 0;
    g_ViewMatrixSlot.value=saved_view;
    {
        PmPrimarySlot bank[11]={0};PmTables old_tables=g_PmTables;
        Pe1GameState old_game=g_GameState;u32 old_flags=g_GameStateFlags;
        g_PmTables=(PmTables){g_PmCmdHandlerArray,bank,NULL};
        g_GameStateFlags=0x80;g_GameState.flags&=~8u;g_GameState.tim_load_state=0;
        if(g_PmCmdHandlerArray[85]!=&g_FieldFxTaskClass)return 0;
        program.script=script+9;task_pointer_value=0;
        if(Scene_LoadRoomAssets(117,NULL)!=0 || bank[0].header.command!=117 || bank[0].header.state!=1 ||
            bank[0].task.tasks.table!=&program.table)return 0;
        if(Pm_SendCmd(0,0,7,(intptr_t)&task_pointer_value,(intptr_t)&task_pointer_value+sizeof(int),-9)!=0)return 0;
        g_FieldFxTaskContext=&bank[0].task.tasks;
        if(FieldFx_SpawnTask(0)!=(int)sizeof(NativeTaskState))return 0;
        g_ViewMatrixSlot.value=&identity;
        if(Pm_Start(0)!=0 || Pm_Exec(0)!=0 || task_pointer_value!=11)return 0;
        if(Pm_Stop(0,NULL,0)!=0 || bank[0].header.state)return 0;
        memset(bank,0,sizeof(bank));g_FieldFxTaskPrograms[32]=NULL;
        if(Scene_LoadRoomAssets(117,NULL)!=-8 || bank[0].header.state ||
            Scene_LoadRoomAssets(191,NULL)!=-8 || bank[0].header.state ||
            FieldFx_BindTaskProgram(owner,-1)!=-1 || FieldFx_BindTaskProgram(owner,104)!=-1)return 0;
        g_ViewMatrixSlot.value=saved_view;g_PmTables=old_tables;g_GameState=old_game;g_GameStateFlags=old_flags;
    }
    g_FieldFxTaskPrograms[32]=old_program;
    FieldFx_InitTaskContext((PmSlotHeader *)owner);owner->tasks.table=&table;owner->tasks.argument=42;
    u16 bad_scripts[][6]={{1,7,99,0,0,0},{1,(u16)-1,99,0,0,0},{4,7,1,0,0,0},{3,0,7,0,0,0}};
    for(unsigned invalid=0;invalid<sizeof(bad_scripts)/sizeof(bad_scripts[0]);++invalid) {
        FieldFx_InitTaskContext((PmSlotHeader *)owner); owner->tasks.pc.script=bad_scripts[invalid];
        if(FieldFx_UpdateTasks((PmSlotHeader *)owner)!=-1 || owner->tasks.count || owner->tasks.used) return 0;
    }
    u16 decrement[]={3,0,0,0,0,0};
    FieldFx_InitTaskContext((PmSlotHeader *)owner); owner->tasks.pc.script=decrement; owner->tasks.variables[0]=2;
    if(FieldFx_UpdateTasks((PmSlotHeader *)owner)!=-1 || owner->tasks.variables[0]!=1 || owner->tasks.pc.script!=decrement+3) return 0;
    owner->tasks.pc.script=decrement; owner->tasks.variables[0]=1;
    if(FieldFx_UpdateTasks((PmSlotHeader *)owner)!=0 || owner->tasks.variables[0] || owner->tasks.pc.script!=decrement+3) return 0;
    FieldFx_InitTaskContext((PmSlotHeader *)owner); g_FieldFxTaskContext=&owner->tasks;
    for(unsigned slot=0;slot<8;++slot)
        if(FieldFx_SpawnTask(0)!=(int)sizeof(NativeTaskState)) return 0;
    if(owner->tasks.count!=8 || FieldFx_SpawnTask(0)!=-1 || FieldFx_SpawnTask(8)!=-1 || FieldFx_SpawnTask(-1)!=-1)
        return 0;
    FieldFx_InitTaskContext((PmSlotHeader *)owner);
    table.sizes[0]=sizeof(owner->tasks.arena)+1;
    if(FieldFx_SpawnTask(0)!=-1 || owner->tasks.count || owner->tasks.used) return 0;
    table.sizes[0]=sizeof(NativeTaskState);
    owner->tasks.used=sizeof(owner->tasks.arena)-1;
    owner->tasks.cursor=owner->tasks.arena+owner->tasks.used;
    if(FieldFx_SpawnTask(0)!=(int)sizeof(NativeTaskState) || owner->tasks.used!=sizeof(NativeTaskState) ||
       owner->tasks.slots[0].start!=owner->tasks.arena) return 0;
    FieldFx_InitTaskContext((PmSlotHeader *)owner);
    owner->tasks.used=1; owner->tasks.cursor=owner->tasks.arena+1;
    int padding=(int)_Alignof(void *)-1;
    if(FieldFx_SpawnTask(0)!=(int)sizeof(NativeTaskState)+padding ||
       owner->tasks.slots[0].start!=owner->tasks.arena+_Alignof(void *)) return 0;
    FieldFx_InitTaskContext((PmSlotHeader *)owner);
    table.callbacks[0]=reject_task_allocation;
    if(FieldFx_SpawnTask(0)!=-1 || owner->tasks.count || owner->tasks.used || owner->tasks.cursor!=owner->tasks.arena ||
       owner->tasks.slots[0].id!=0xffff) return 0;
    for(unsigned byte=0;byte<sizeof(storage.guard);++byte) if(storage.guard[byte]!=0xa5) return 0;
    g_FieldFxTaskContext=NULL; g_FieldFxActiveTaskSlot=NULL; g_FieldFxOwner=NULL;
    puts("Original effect tasks: init, script spawn/wait/stop, update/draw/completion, eight-slot limit, native alignment/wrap and allocation failure passed");
    return 1;
}

static int canonical_render_buffers(void)
{
    RenderBufferPrefix saved=D_800B0E38;
    static RenderGpuTag tags[2];
    static char bytes[8];
    for(unsigned i=0;i<2;++i) {
        g_GameState.shared_buffers.ordering_tables[i]=(unsigned char *)&tags[i];
        if(D_800B0E38.ordering[i]!=&tags[i]) return 0;
    }
    g_GameState.texture_load_scratch=(union SceneAssetView *)&bytes[0];
    g_GameState.object_texture_blob=(struct SceneAssetBlob *)&bytes[1];
    g_GameState.unk_170=(u8 *)&bytes[2];
    g_GameState.bg_tim=(struct TimFile *)&bytes[3];
    g_GameState.save_background_source=(unsigned short *)&bytes[4];
    g_GameState.save_background_destination=(unsigned short *)&bytes[5];
    for(unsigned i=0;i<6;++i) if(D_800B0E38.other_buffers[i]!=&bytes[i]) return 0;
    D_800B0E38.packets[0]=&bytes[6]; D_800B0E38.packets[1]=&bytes[7];
    if((void *)g_GameState.hud_tim!=&bytes[6] || g_GameState.script_sprite_buffer!=(u8 *)&bytes[7]) return 0;
    D_800B0E38=saved;
    puts("Original render buffer symbol: canonical game state aliases all ten native slots");
    return 1;
}

static int native_effect_packet_links(void)
{
    struct { RenderTintMode mode; u8 guard[16]; } buffer;
    RenderTintMode *modes=&buffer.mode;
    RenderBufferPrefix saved=D_800B0E38;
    int saved_slot=g_ActiveDrawSlot, saved_offset=g_DrawPacketOffset;
    for(unsigned slot=0;slot<2;++slot) for(int mode=0;mode<4;++mode) {
        memset(&buffer,0xa5,sizeof(buffer));
        RenderGpuTag ordering={.address=(uintptr_t)0xffffff,.length=0};
        FieldTilePoint packet={0}; SetTile1(&packet);
        packet.r=128; packet.g=64; packet.b=32; packet.x=7; packet.y=9;
        D_800B0E38.packets[slot]=(char *)modes;
        g_ActiveDrawSlot=slot; g_DrawPacketOffset=0;
        FieldFx_LinkBlendedPacket(&ordering,&packet,mode);
        if(ordering.address!=(uintptr_t)modes || modes->tag.address!=(uintptr_t)&packet ||
           packet.tag.address!=0xffffff || modes->tag.length!=1 || packet.tag.length!=2 ||
           packet.command!=0x6a || g_DrawPacketOffset!=sizeof(RenderTintMode)) return 0;
        if(modes->command!=(u32)(0xe1000200|GetTPage(0,mode,0,0))) return 0;
        for(unsigned i=0;i<sizeof(buffer.guard);++i) if(buffer.guard[i]!=0xa5) return 0;
        ordering.address=0xffffff; g_DrawPacketOffset=0;
        FieldFx_LinkBlendedPacket(&ordering,&packet,255);
        if(ordering.address!=(uintptr_t)&packet || packet.tag.address!=0xffffff || g_DrawPacketOffset) return 0;
    }
    D_800B0E38=saved; g_ActiveDrawSlot=saved_slot; g_DrawPacketOffset=saved_offset;
    puts("Original effect packets: full native links, both slots, four blend modes, payload offsets and arena guards passed");
    return 1;
}

int CdRom_InitScreenState(void);
void Render_ResetScene(int width,int height);
int Draw_InitDisplay(int mode);
static int original_display_start(void)
{
    Pe1GameState saved=g_GameState;
    GeomScrollRecord scroll=g_GeomScroll;
    const int modes[]={0,1,2,3};
    g_GameState.flags=0;
    Render_ResetScene(320,224);
    for(unsigned i=0;i<4;++i) {
        CdRom_InitScreenState();
        g_SceneDispatchToken=0x12345678;
        g_GameState.scene_init_subphase=0xA5;
        if(Draw_InitDisplay(modes[i]) || g_ActiveDrawSlot || g_ScreenTint.fadeMode!=1 ||
           g_ScreenTint.frame!=2 || g_ScreenTint.duration!=2 ||
           g_SceneInitSubphase || g_GameState.bank_asset_source!=g_BootWorkArea ||
           g_GameState.scene_load_scratch!=g_SceneLoadScratchArea ||
           g_DrawFrameBuffers[0].frontBufferBase!=g_RenderFrontBufferBase[0] ||
           g_DrawFrameBuffers[1].frontBufferBase!=g_RenderBackBufferBase[0] ||
           g_SceneDispatchToken!=(modes[i]==1?0xA80830C8u:modes[i]==3?0xA80651C8u:0x12345678u)) return 0;
    }
    g_GameState=saved; g_GeomScroll=scroll;
    puts("Original display start: four modes, memory layout, two fade frames and scene-token selection passed");
    return 1;
}

/* RGB sequences from executing USA Render_SetCDDCSlot 80068E24 in MIPS. */
static int original_startup_fade(void)
{
    static const struct { unsigned duration, count; u8 rgb[8][3]; } cases[]={
        {0,2,{{0,200,255},{255,128,0}}},
        {1,2,{{0,200,255},{255,128,0}}},
        {2,3,{{0,200,255},{255,128,0},{255,128,0}}},
        {4,5,{{0,200,255},{85,176,170},{170,152,85},{255,128,0},{255,128,0}}},
        {7,8,{{0,200,255},{42,188,213},{85,176,170},{127,164,128},{170,152,85},{212,140,43},{255,128,0},{255,128,0}}},
    };
    GeomScrollRecord saved_scroll=g_GeomScroll;
    RenderBufferPrefix saved_buffers=D_800B0E38;
    int saved_slot=g_ActiveDrawSlot;
    RenderGpuTag ordering[2][4];
    int valid=1;
    for(unsigned slot=0;slot<2;++slot) for(unsigned stop=0;stop<2;++stop)
    for(unsigned blend=0;blend<4;++blend)
    for(unsigned c=0;c<sizeof(cases)/sizeof(cases[0]);++c) {
        CdRom_InitScreenState();
        for(unsigned i=0;i<2;++i) {
            const RenderTintTile *tile=&g_ScreenTint.tiles[i];
            if(tile->tag.length!=3 || tile->command!=0x62 || tile->r!=255 ||
               tile->g!=255 || tile->b!=255 || tile->x || tile->y ||
               tile->width!=320 || tile->height!=224) valid=0;
            D_800B0E38.ordering[i]=(char *)ordering[i];
        }
        g_ActiveDrawSlot=slot;
        g_ScreenTint.fadeMode=stop?6:2; g_ScreenTint.blendMode=blend;
        g_ScreenTint.duration=cases[c].duration;
        g_ScreenTint.targetR=255; g_ScreenTint.targetG=128; g_ScreenTint.targetB=0;
        g_ScreenTint.startR=0; g_ScreenTint.startG=200; g_ScreenTint.startB=255;
        for(unsigned frame=0;frame<cases[c].count;++frame) {
            for(unsigned i=0;i<2;++i) for(unsigned j=0;j<4;++j)
                ordering[i][j]=(RenderGpuTag){0xFFFFFF,0x9876};
            unsigned inactive=(g_ScreenTint.fadeMode&3)==0;
            Render_SetCDDCSlot();
            if(inactive) {
                if(ordering[slot][3].address!=0xFFFFFF) valid=0;
            } else {
                const RenderTintTile *tile=&g_ScreenTint.tiles[slot];
                const RenderTintMode *mode=&g_ScreenTint.modes[slot];
                if(ordering[slot][3].address!=(uintptr_t)mode ||
                   mode->tag.address!=(uintptr_t)tile || tile->tag.address!=0xFFFFFF ||
                   mode->tag.length!=1 || tile->tag.length!=3 ||
                   mode->command!=(0xE1000400u|(blend<<5)) ||
                   memcmp(&tile->r,cases[c].rgb[frame],3)) valid=0;
            }
            for(unsigned i=0;i<2;++i) for(unsigned j=0;j<4;++j) {
                if(ordering[i][j].length!=0x9876 ||
                   ((i!=slot || j!=3) && ordering[i][j].address!=0xFFFFFF)) valid=0;
            }
        }
        if(g_ScreenTint.fadeMode!=(stop?0:1) ||
           g_ScreenTint.frame!=(cases[c].duration?cases[c].duration:1)) valid=0;
    }
    g_GeomScroll=saved_scroll; D_800B0E38=saved_buffers; g_ActiveDrawSlot=saved_slot;
    if(!valid) fprintf(stderr,"Original startup fade RGB/native links differ from MIPS reference\n");
    return valid;
}

static int native_screen_tint_packets(void)
{
    static RenderGpuTag ordering[4096];
    struct { RenderTintTile tile; RenderTintMode mode; u8 guard[16]; } buffer;
    RenderBufferPrefix saved=D_800B0E38;
    int saved_slot=g_ActiveDrawSlot, saved_offset=g_DrawPacketOffset;
    u8 rgb[3]={200,100,50};
    for(unsigned slot=0;slot<2;++slot) {
        D_800B0E38.ordering[slot]=(char *)ordering;
        D_800B0E38.packets[slot]=(char *)&buffer;
        g_ActiveDrawSlot=slot;
        for(unsigned depth=0;depth<4096;++depth) {
            ordering[depth].address=0xffffff;
            ordering[depth].length=0;
        }
        for(int blend=0;blend<5;++blend) for(unsigned depth=0;depth<4096;++depth) {
            memset(&buffer,0xa5,sizeof(buffer));
            g_DrawPacketOffset=0;
            ordering[depth].address=0xffffff;
            if(STRIP_OT(depth)!=&ordering[depth]) return 0;
            FieldFx_DrawScreenTint(rgb,64,blend==4?255:blend,depth);
            RenderTintTile *tile=&buffer.tile;
            if(tile->tag.address!=0xffffff || tile->tag.length!=3 ||
               tile->r!=100 || tile->g!=50 || tile->b!=25 ||
               tile->x || tile->y || tile->width!=320 || tile->height!=240) return 0;
            if(blend==4) {
                if(ordering[depth].address!=(uintptr_t)tile || tile->command!=0x60 ||
                   g_DrawPacketOffset!=sizeof(*tile)) return 0;
                const u8 *unused=(const u8 *)&buffer.mode;
                for(unsigned i=0;i<sizeof(buffer.mode);++i) if(unused[i]!=0xa5) return 0;
            } else {
                if(ordering[depth].address!=(uintptr_t)&buffer.mode ||
                   buffer.mode.tag.address!=(uintptr_t)tile || buffer.mode.tag.length!=1 ||
                   buffer.mode.command!=(u32)(0xe1000200|GetTPage(0,blend,0,0)) ||
                   tile->command!=0x62 ||
                   g_DrawPacketOffset!=sizeof(*tile)+sizeof(buffer.mode)) return 0;
            }
            for(unsigned i=0;i<sizeof(buffer.guard);++i) if(buffer.guard[i]!=0xa5) return 0;
            ordering[depth].address=0xffffff;
        }
        for(unsigned depth=0;depth<4096;++depth)
            if(ordering[depth].address!=0xffffff || ordering[depth].length) return 0;
        memset(&buffer,0xa5,sizeof(buffer)); g_DrawPacketOffset=0;
        FieldFx_DrawScreenTint(rgb,64,0,4096);
        if(g_DrawPacketOffset!=sizeof(RenderTintTile)) return 0;
        for(unsigned depth=0;depth<4096;++depth)
            if(ordering[depth].address!=0xffffff || ordering[depth].length) return 0;
    }
    D_800B0E38=saved; g_ActiveDrawSlot=saved_slot; g_DrawPacketOffset=saved_offset;
    puts("Original screen tint: all 4096 OT depths, both slots, blend modes, RGB and packet guards passed");
    return 1;
}

static int native_effect_model_packets(void)
{
    _Alignas(16) u8 blob[128]={0}, packets[512];
    static RenderGpuTag ordering[4096];
    RenderBufferPrefix saved=D_800B0E38;
    int saved_slot=g_ActiveDrawSlot, saved_offset=g_DrawPacketOffset;
    GteMatrix view={.m={{4096,0,0},{0,4096,0},{0,0,4096}}};
    GteMatrix *saved_view=g_GeomScroll.words.viewMatrix;
    g_GeomScroll.words.viewMatrix=&view;
    GteMatrix placement=view; placement.t[2]=512;
    FieldTexturedModel *model=(FieldTexturedModel *)blob;
    model->header.flatTris=model->header.shadedTris=1;
    model->header.flatQuads=model->header.shadedQuads=1;
    model->header.colorOffset=104; model->header.colorCount=4;
    FieldModelTri *tri=model->header.tris;
    FieldModelQuad *quad=(FieldModelQuad *)(tri+2);
    for(unsigned i=0;i<2;++i) {
        for(unsigned j=0;j<3;++j) { tri[i].vertex[j]=72+8*j; tri[i].uv[j]=0x1000+j; }
        for(unsigned j=0;j<4;++j) { quad[i].vertex[j]=72+8*j; quad[i].uv[j]=0x2000+j; }
    }
    FieldModelVertex *v=(FieldModelVertex *)(blob+72);
    for(unsigned j=0;j<4;++j) {
        v[j].x=(j&1)?32:-32; v[j].y=(j&2)?32:-32;
        v[j].color=104+4*j;
        ((u32 *)(blob+104))[j]=0x00302010+j;
    }
    Psyz_GteCtrlWrite(26,256); Psyz_GteCtrlWrite(29,1365); Psyz_GteCtrlWrite(30,1024);
    gte_SetGeomOffset(160,120);
    FieldFx_ModelSetTexture(0x123,0x456);
    const u16 factors[]={0,64,128,512,65535};
    const u32 expected_colors[5][4]={
        {0,0,0,0},
        {0x181008,0x181008,0x181009,0x181009},
        {0x302010,0x302011,0x302012,0x302013},
        {0xc08040,0xc08044,0xc08048,0xc0804c},
        {0xffffff,0xffffff,0xffffff,0xffffff},
    };
    u8 original_model[sizeof(blob)];memcpy(original_model,blob,sizeof(blob));
    for(unsigned slot=0;slot<2;++slot) for(unsigned semi=0;semi<2;++semi) for(unsigned factor=0;factor<5;++factor) {
        FieldFx_ModelSaveColors((char *)blob);
        FieldFx_ModelScaleColors((char *)blob,factors[factor]);
        if(memcmp(blob+104,expected_colors[factor],16))return 0;
        memset(packets,0xa5,sizeof(packets));
        memset(ordering,0,sizeof(ordering));
        for(unsigned i=0;i<4096;++i) ordering[i].address=0xffffff;
        D_800B0E38.ordering[slot]=ordering; D_800B0E38.packets[slot]=(char *)packets;
        g_ActiveDrawSlot=slot; g_DrawPacketOffset=0;
        FieldFx_ModelSetSemiTrans(semi); g_FieldFxModelDepthBias=0;
        FieldFx_ModelDraw(model,&placement);
        size_t expected=sizeof(FieldModelFt3Packet)+sizeof(FieldModelGt3Packet)+sizeof(FieldModelFt4Packet)+sizeof(FieldModelGt4Packet);
        if(g_DrawPacketOffset!=expected) { fprintf(stderr,"effect model bytes %d expected %zu\n",g_DrawPacketOffset,expected); return 0; }
        unsigned sizes[4]={sizeof(FieldModelFt3Packet),sizeof(FieldModelGt3Packet),sizeof(FieldModelFt4Packet),sizeof(FieldModelGt4Packet)};
        unsigned lengths[4]={7,9,9,12}, codes[4]={0x24,0x34,0x2c,0x3c};
        size_t offset=0; uintptr_t previous=0xffffff;
        for(unsigned i=0;i<4;++i) {
            RenderGpuTag *tag=(RenderGpuTag *)(packets+offset);
            if(tag->address!=previous || tag->length!=lengths[i] || packets[offset+sizeof(*tag)+3]!=(codes[i]|(semi?2:0))) return 0;
            u32 rendered_color;memcpy(&rendered_color,packets+offset+sizeof(*tag),4);
            if((rendered_color&0xffffff)!=expected_colors[factor][0])return 0;
            previous=(uintptr_t)tag; offset+=sizes[i];
        }
        unsigned used=0;
        for(unsigned depth=0;depth<4096;++depth) if(ordering[depth].address!=0xffffff) {
            if(ordering[depth].address!=previous) return 0;
            ++used;
        }
        if(used!=1) return 0;
        for(size_t i=expected;i<sizeof(packets);++i) if(packets[i]!=0xa5) return 0;
        FieldFx_ModelRestoreColors((char *)blob);
        if(memcmp(blob,original_model,sizeof(blob)))return 0;
    }
    D_800B0E38=saved; g_ActiveDrawSlot=saved_slot; g_DrawPacketOffset=saved_offset;
    g_GeomScroll.words.viewMatrix=saved_view;
    puts("Original effect model: all packet types, both slots, transparency and save/scale/draw/restore saturation passed");
    return 1;
}

static int native_oriented_sprite_packets(void)
{
    struct { FieldStripPacket packet; u8 guard[16]; } buffer;
    static RenderGpuTag ordering[4096];
    RenderBufferPrefix saved=D_800B0E38;
    int saved_slot=g_ActiveDrawSlot, saved_offset=g_DrawPacketOffset;
    GteMatrix *saved_view=g_GeomScroll.words.viewMatrix;
    GteMatrix view={.m={{4096,0,0},{0,4096,0},{0,0,4096}}};
    g_GeomScroll.words.viewMatrix=&view;
    FieldOrientedSprite sprite={.position={0,0,512},.scale={4096,4096,4096},
        .rgb={80,40,20},.cell=0x23,.clut=0x12,.brightness=128};
    Psyz_GteCtrlWrite(26,256); Psyz_GteCtrlWrite(29,1365); Psyz_GteCtrlWrite(30,1024);
    gte_SetGeomOffset(160,120);
    FieldFx_SetSpriteCellSize(2,3);
    if(g_FieldFxCellQuad[0].x!=-32 || g_FieldFxCellQuad[0].y!=-48 ||
       g_FieldFxCellQuad[3].x!=32 || g_FieldFxCellQuad[3].y!=48) return 0;
    for(unsigned slot=0;slot<2;++slot) for(unsigned preset=0;preset<4;++preset)
    for(unsigned color=0;color<2;++color) for(unsigned blend=0;blend<5;++blend) {
        memset(&buffer,0xa5,sizeof(buffer));
        memset(ordering,0,sizeof(ordering));
        for(unsigned i=0;i<4096;++i) ordering[i].address=0xffffff;
        D_800B0E38.ordering[slot]=ordering;
        D_800B0E38.packets[slot]=(char *)&buffer;
        g_ActiveDrawSlot=slot; g_DrawPacketOffset=0;
        FieldFx_SetSpriteTexturePreset(preset); FieldFx_SetSpriteColorMode(color?256:16);
        FieldFx_SetSpriteBlendMode(blend);
        FieldFx_DrawOrientedSprite(&sprite);
        FieldStripPacket *p=&buffer.packet;
        if(g_DrawPacketOffset!=sizeof(*p) || p->tag.length!=9 || p->tag.address!=0xffffff ||
           p->code!=(blend?0x2e:0x2c) || p->r0!=80 || p->g0!=40 || p->b0!=20 ||
           p->u0!=48 || p->u1!=49 || p->u2!=48 || p->u3!=49 ||
           p->v0!=32 || p->v1!=32 || p->v2!=34 || p->v3!=34 ||
           p->tpage!=GetTPage(color,blend?blend-1:0,preset<3?0x340:0x380,preset==1?0x160:0x100) ||
           p->clut!=GetClut(32,(int[]){0x1d7,0x1db,0x1d6,0x1c8}[preset]+1)) return 0;
        unsigned used=0;
        for(unsigned depth=0;depth<4096;++depth) if(ordering[depth].address!=0xffffff) {
            if(ordering[depth].address!=(uintptr_t)p) return 0;
            ++used;
        }
        if(used!=1) return 0;
        for(unsigned i=0;i<sizeof(buffer.guard);++i) if(buffer.guard[i]!=0xa5) return 0;
    }
    struct { FieldGt4Packet packet; u8 guard[16]; } rotated;
    Psyz_GteCtrlWrite(27,1); Psyz_GteCtrlWrite(28,0);
    GteShortVector anchor={0,0,512}; GteRotation rotation={1024,0,0,0};
    RenderColor first={80,40,20,0},second={60,30,10,0};
    RenderEffectParameters saved_params=g_FieldFxSpriteParams;
    g_FieldFxSpriteParams.tpage=0x100;g_FieldFxSpriteParams.depth=0;
    for(unsigned slot=0;slot<2;++slot) for(unsigned mode=0;mode<5;++mode) {
        memset(&rotated,0xa5,sizeof(rotated));
        for(unsigned i=0;i<4096;++i)ordering[i]=(RenderGpuTag){.address=0xffffff};
        D_800B0E38.ordering[slot]=ordering;D_800B0E38.packets[slot]=(char *)&rotated;
        g_ActiveDrawSlot=slot;g_DrawPacketOffset=0;
        FieldFx_DrawRotatedQuad(&anchor,&rotation,100,100,7,11,12,16,123,&first,&second,128,mode==4?255:mode);
        FieldGt4Packet *p=&rotated.packet;
        if(g_DrawPacketOffset!=sizeof(*p) || p->tag.length!=12 || p->tag.address!=0xffffff ||
            p->code!=(mode==4?0x3c:0x3e) || p->r0!=80 || p->g0!=40 || p->b0!=20 ||
            p->r1!=80 || p->r2!=60 || p->g2!=30 || p->b2!=10 || p->r3!=60 ||
            p->u0!=7 || p->u1!=7 || p->u2!=18 || p->u3!=18 ||
            p->v0!=11 || p->v2!=11 || p->v1!=26 || p->v3!=26 || p->clut!=123 ||
            p->tpage!=(mode==4?0x100:0x100|GetTPage(0,mode,0,0))) return 0;
        unsigned linked=0;
        for(unsigned depth=0;depth<4096;++depth)if(ordering[depth].address!=0xffffff) {
            if(ordering[depth].address!=(uintptr_t)p)return 0;
            ++linked;
        }
        if(linked!=1)return 0;
        for(unsigned i=0;i<sizeof(rotated.guard);++i)if(rotated.guard[i]!=0xa5)return 0;
    }
    GteShortVector old_center=g_RoomM005BeaconCenter,old_offset=g_RoomM005BeaconOffset;
    int old_alt=g_FieldFxAltPaletteFlag,old_age=g_FieldFxCurrentAge;
    FieldFxTaskContext context={0},*old_context=g_FieldFxTaskContext;
    g_FieldFxTaskContext=&context;
    g_RoomM005BeaconCenter=(GteShortVector){0,0,512};g_RoomM005BeaconOffset=(GteShortVector){0};
    _Alignas(void *) u8 drift_arena[128];
    FieldFx_InitCallbackList((char *)drift_arena,sizeof(RoomM005DriftingSpriteState),1,RoomM005_BeaconDriftCallback);
    RoomM005DriftingSpriteState *drift=FieldFx_AcquireCallbackSlot((char *)drift_arena);
    *drift=(RoomM005DriftingSpriteState){1024,0,0,0,0,0,0};
    FieldFxCallbackListEntry *entry=(void *)((u8 *)drift-sizeof(FieldFxCallbackListEntry));
    for(unsigned slot=0;slot<2;++slot)for(unsigned kind=3;kind<=4;++kind)
    for(unsigned alt=0;alt<2;++alt)for(unsigned age=0;age<=6;++age) {
        memset(&rotated,0xa5,sizeof(rotated));
        for(unsigned i=0;i<4096;++i)ordering[i]=(RenderGpuTag){.address=0xffffff};
        D_800B0E38.ordering[slot]=ordering;D_800B0E38.packets[slot]=(char *)&rotated;
        g_ActiveDrawSlot=slot;g_DrawPacketOffset=0;
        g_FieldFxSpriteParams.palette=kind;g_FieldFxAltPaletteFlag=alt;entry->age=age;
        if(FieldFx_DrawCallbackList((char *)drift_arena)!=1)return 0;
        FieldGt4Packet *p=&rotated.packet;
        u8 intensity=rcos((age<<10)/6)/24;
        unsigned u=(age&1)<<7,v=((age<<3)&16)+32;
        if(p->tag.length!=12 || p->tag.address!=0xffffff || p->code!=0x3e ||
            p->r0!=intensity || p->g0!=intensity || p->b0!=intensity || p->r3!=intensity ||
            p->clut!=GetClut(0,g_FieldFxPaletteTable[kind]+(kind==4&&alt?4:0)) ||
            p->u0!=u || p->u2!=(u8)(u+126) || p->v0!=v || p->v1!=v+14)return 0;
        unsigned linked=0;
        for(unsigned depth=0;depth<4096;++depth)if(ordering[depth].address!=0xffffff) {
            if(ordering[depth].address!=(uintptr_t)p)return 0;
            ++linked;
        }
        if(linked!=1 || g_DrawPacketOffset!=sizeof(*p))return 0;
        for(unsigned i=0;i<sizeof(rotated.guard);++i)if(rotated.guard[i]!=0xa5)return 0;
    }
    g_FieldFxTaskContext=old_context;g_FieldFxCurrentAge=old_age;
    g_FieldFxAltPaletteFlag=old_alt;g_RoomM005BeaconCenter=old_center;g_RoomM005BeaconOffset=old_offset;
    g_FieldFxSpriteParams=saved_params;
    D_800B0E38=saved; g_ActiveDrawSlot=saved_slot; g_DrawPacketOffset=saved_offset;
    g_GeomScroll.words.viewMatrix=saved_view;
    puts("Original sprites: both buffers, presets/blends, native rotated-quad payloads and M005 drift ages/palettes passed");
    return 1;
}

static int native_ring_packets(void)
{
    struct RingPair { FieldRingPacket packet; RenderTintMode mode; };
    struct { struct RingPair pairs[4]; u8 guard[16]; } buffer;
    static RenderGpuTag ordering[4096];
    GteShortVector points[8]={0};
    FieldShadedRing ring={.source=points,.color0={80,40,20},.color1={200,100,50},
        .mode=4,.extent0=16,.extent1=32,.intensity=64};
    RenderBufferPrefix saved=D_800B0E38;
    int saved_slot=g_ActiveDrawSlot, saved_offset=g_DrawPacketOffset;
    GteMatrix *saved_view=g_GeomScroll.words.viewMatrix;
    GteMatrix view={.m={{4096,0,0},{0,4096,0},{0,0,4096}}};
    g_GeomScroll.words.viewMatrix=&view;
    Psyz_GteCtrlWrite(26,256); Psyz_GteCtrlWrite(29,1365); Psyz_GteCtrlWrite(30,1024);
    gte_SetGeomOffset(160,120);
    FieldFx_BuildRingGeometry(&ring);
    if(points[0].x || points[0].y!=32 || points[4].x || points[4].y!=16) return 0;
    for(unsigned slot=0;slot<2;++slot) for(unsigned transform=0;transform<2;++transform)
    for(unsigned blend=0;blend<5;++blend) {
        memset(&buffer,0xa5,sizeof(buffer)); memset(ordering,0,sizeof(ordering));
        for(unsigned i=0;i<4096;++i) ordering[i].address=0xffffff;
        D_800B0E38.ordering[slot]=ordering; D_800B0E38.packets[slot]=(char *)&buffer;
        g_ActiveDrawSlot=slot; g_DrawPacketOffset=0;
        FieldFx_SetSpriteBlendMode(blend);
        GteMatrix placement=view; placement.t[2]=512;
        FieldFx_DrawShadedRing(&ring,&placement,transform);
        if(g_DrawPacketOffset!=sizeof(buffer.pairs)) return 0;
        uintptr_t previous=0xffffff;
        for(unsigned i=0;i<4;++i) {
            FieldRingPacket *p=&buffer.pairs[i].packet;
            RenderTintMode *m=&buffer.pairs[i].mode;
            if(p->tag.address!=previous || p->tag.length!=8 ||
               m->tag.address!=(uintptr_t)p || m->tag.length!=1 ||
               m->command!=(u32)(0xe1000000|((blend?blend-1:0)<<5)) ||
               p->c0.word!=(u32)((blend?0x3a:0x38)<<24|25<<16|50<<8|100) ||
               p->c1.word!=(u32)(25<<16|50<<8|100) ||
               p->c2.word!=(u32)(10<<16|20<<8|40) || p->c3.word!=p->c2.word) return 0;
            previous=(uintptr_t)m;
        }
        unsigned used=0;
        for(unsigned i=0;i<4096;++i) if(ordering[i].address!=0xffffff) {
            if(ordering[i].address!=previous) return 0;
            ++used;
        }
        if(used!=1) return 0;
        for(unsigned i=0;i<sizeof(buffer.guard);++i) if(buffer.guard[i]!=0xa5) return 0;
    }
    D_800B0E38=saved; g_ActiveDrawSlot=saved_slot; g_DrawPacketOffset=saved_offset;
    g_GeomScroll.words.viewMatrix=saved_view;
    puts("Original rings: builder, both slots/transforms, all blend modes, paired packet links and guards passed");
    return 1;
}

static int native_effect_collision_coordinates(void)
{
    BattleEntity player={0}, *saved=D_8009D254;
    GteShortVector quad[4]={{0,0,0},{16,0,0},{0,0,16},{16,0,16}};
    D_8009D254=&player;
    for(int x=-2;x<=18;++x) for(int z=-2;z<=18;++z) {
        GteShortVector point={x,99,z};
        player.posX.parts.integer=x; player.posY.parts.integer=99; player.posZ.parts.integer=z;
        if(FieldFx_IsPlayerInTriangleXZ(quad)!=FieldFx_IsPointInTriangleXZ(&point,quad) ||
           FieldFx_IsPlayerInQuadXZ(quad)!=FieldFx_IsPointInQuadXZ(&point,quad)) return 0;
    }
    FieldActor actor={0};FieldActorState actor_state={0};RenderMatrix matrices[21]={0};
    actor.state=&actor_state;actor.render_object.matrices=matrices;
    GteShortVector input={-7,9,11,0},output={0},angles={0};
    for(unsigned index=0;index<=20;index+=20)for(unsigned turn=0;turn<2;++turn) {
        RenderMatrix *matrix=&matrices[index];
        memset(matrix,0,sizeof(*matrix));
        matrix->rotation[0][turn?2:0]=4096;matrix->rotation[1][1]=4096;
        matrix->rotation[2][turn?0:2]=turn?-4096:4096;
        matrix->translation[0]=40000+index;matrix->translation[1]=-50000;matrix->translation[2]=65530;
        FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)&actor,index,&input,&output);
        if(output.x!=(s16)(40000+index+(turn?11:-7)) || output.y!=(s16)-49991 ||
            output.z!=(s16)(turn?65537:65541))return 0;
        FieldFx_GetTransformLookAngles((RoomFxTransformOwner *)&actor,index,&angles);
        if(angles.x || angles.y!=(turn?3072:2048) || angles.z)return 0;
        if(((RoomM005BeaconPool *)&actor)->matrix!=(GteMatrix *)matrices)return 0;
    }
    actor_state.flags=0x1234;
    ((RoomM005BeaconActor *)&actor_state)->flags|=0x4000;
    if(actor_state.flags!=0x5234)return 0;
    u8 action=1;actor_state.action_state=&action;
    if(((RoomM005BeaconPool *)&actor)->node->state!=&action)return 0;
    D_8009D254=saved;
    puts("Original effect collision: player/native coordinate paths agree across 441 triangle/quad positions");
    return 1;
}

static int original_glow_resident_constants(void)
{
    const char *path=getenv("PE_TEST_DISC");
    if(!path) return !getenv("PE_REQUIRE_GAME_ASSETS");
    struct { u32 address; const void *value; size_t size; } rows[]={
        {0x800c2154,&g_GlowEffectLayersSpinA,sizeof(GteShortVector)},
        {0x800c215c,&g_GlowLayers1SpinB,sizeof(GteShortVector)},
        {0x800c2164,&g_GlowLayers1Scale,sizeof(GteVector)},
        {0x800c2184,&g_GlowEffectBLayersSpinA,sizeof(GteShortVector)},
        {0x800c218c,&g_GlowEffectBSpinB,sizeof(GteShortVector)},
        {0x800c2194,&g_GlowEffectBLayersScale,sizeof(GteVector)},
        {0x800c21a4,&g_GlowEffectCLayersSpinA,sizeof(GteShortVector)},
        {0x800c21ac,&g_GlowEffectCLayersSpinB,sizeof(GteShortVector)},
        {0x800c21b4,&g_GlowLayers3Scale,sizeof(GteVector)},
        {0x800c21d4,&g_FourLayerGlowSpinA,sizeof(GteShortVector)},
        {0x800c21dc,&g_FourLayerGlowSpinB,sizeof(GteShortVector)},
        {0x800c21e4,&g_FourLayerGlowSpinC,sizeof(GteShortVector)},
        {0x800c21ec,&g_FourLayerGlowSpinD,sizeof(GteShortVector)},
        {0x800c21f4,&g_FourLayerGlowScale,sizeof(GteVector)},
        {0x800c214c,&g_GlowEffectPointOffset,sizeof(GteShortVector)},
        {0x800c2174,&g_GlowEffectSpriteRotation,sizeof(GteShortVector)},
        {0x800e085c,g_GlowEffectSlotSizes,7*sizeof(u16)},
        {0x800e0888,g_GlowEffectScaleByAction,16*sizeof(s16)},
        {0x800e08a8,g_GlowEffectFlareOffsets,8*sizeof(GteShortVector)},
        {0x800e08e8,g_GlowEffectHaloOffsets,8*sizeof(GteShortVector)},
        {0x800e0928,g_GlowEffectScript,25*sizeof(int)},
        {0x800c217c,&g_GlowEffectBPointOffset,sizeof(GteShortVector)},
        {0x800e09c8,g_GlowEffectBSlotSizes,5*sizeof(u16)},
        {0x800e09f0,g_GlowEffectBScaleByAction,16*sizeof(s16)},
        {0x800e0a10,g_GlowEffectBFlareOffsets,8*sizeof(GteShortVector)},
        {0x800e0a50,g_GlowEffectBScript,13*sizeof(int)},
        {0x800e0ab4,g_GlowEffectCSlotSizes,4*sizeof(u16)},
        {0x800e0ad8,g_GlowEffectCScaleByAction,16*sizeof(s16)},
        {0x800e0af8,g_GlowEffectCFlareOffsets,8*sizeof(GteShortVector)},
        {0x800e0b38,g_GlowEffectCScript,12*sizeof(int)},
        {0x800c21c4,&g_FourLayerGlowPointOffsetA,sizeof(GteShortVector)},
        {0x800c21cc,&g_FourLayerGlowPointOffsetB,sizeof(GteShortVector)},
        {0x800c2204,&g_FourLayerGlowSpriteRotation,sizeof(GteShortVector)},
        {0x800e0bbc,g_FourLayerGlowSlotSizes,7*sizeof(u16)},
        {0x800e0be8,g_FourLayerGlowScaleByAction,16*sizeof(s16)},
        {0x800e0c08,g_FourLayerGlowFlareOffsets,8*sizeof(GteShortVector)},
        {0x800e0c48,g_FourLayerGlowHaloOffsets,8*sizeof(GteShortVector)},
        {0x800e0c88,g_FourLayerGlowScript,25*sizeof(int)},
        {0x800e0d40,g_PointSpriteSlotSizes,7*sizeof(u16)},
        {0x800e0d6c,g_PointSpriteScript,22*sizeof(int)},
        {0x800c22a0,&g_FieldFxBlackColor,sizeof(RenderColor)},
        {0x800e1204,g_FieldFxPaletteTable,6*sizeof(u16)},
        {0x800e11e4,g_FieldFxTpageSlots,sizeof(g_FieldFxTpageSlots)},
        {0x800c2258,&D_800C2258,sizeof(GteShortVector)},
        {0x800c2260,&g_FieldFxZeroVector,sizeof(GteShortVector)},
        {0x80093162,g_SceneAssetLbaTbl,(2*sizeof(u16))},
        {0x800c2268,&g_FieldFxZeroRotation,sizeof(GteRotation)},
        {0x800c2290,&g_FieldFxUnitScale,sizeof(GteVector)},
        {0x800e1210,g_FieldFxStripPacketCounts,6*sizeof(u16)},
        {0x800e121c,g_FieldFxShapeVertexTable[0],4*sizeof(GteShortVector)},
        {0x800e123c,g_FieldFxShapeVertexTable[1],8*sizeof(GteShortVector)},
        {0x800e127c,g_FieldFxShapeVertexTable[2],8*sizeof(GteShortVector)},
        {0x800e12bc,g_FieldFxShapeVertexTable[3],8*sizeof(GteShortVector)},
        {0x800e12fc,g_FieldFxShapeVertexTable[4],8*sizeof(GteShortVector)},
        {0x800e133c,g_FieldFxShapeVertexTable[5],16*sizeof(GteShortVector)}
    };
    PeDisc disc={0}; PeDiscFile file;
    u8 header[32], raw[128];
    if(!pe_disc_open(&disc,path)) return 0;
    if(!pe_disc_find(&disc,"SLUS_006.62",&file) ||
       !pe_disc_read(&disc,&file,0,header,sizeof(header))) { pe_disc_close(&disc); return 0; }
    u32 base=(u32)header[24]|(u32)header[25]<<8|(u32)header[26]<<16|(u32)header[27]<<24;
    for(unsigned i=0;i<sizeof(rows)/sizeof(rows[0]);++i) {
        if(rows[i].address<base || !pe_disc_read(&disc,&file,0x800ull+rows[i].address-base,raw,rows[i].size) ||
           memcmp(raw,rows[i].value,rows[i].size)) { pe_disc_close(&disc); return 0; }
    }
    const u32 resident_ids[8]={0x800e086c,0x800e09d4,0x800e0abc,0x800e0bcc,0x800e0f50,0x800e0e9c,0x800e0fe0,0x800e0d50};
    if(!pe_disc_read(&disc,&file,0x800ull+0x80094188u-base,raw,sizeof(resident_ids)) ||
       memcmp(raw,resident_ids,sizeof(resident_ids))) { pe_disc_close(&disc); return 0; }
    u32 task_descriptor;
    const u32 task_methods[]={0x800d4860,0x800d4620,0x800d4698,0x800d4704,0x800d413c,0x800d4850,0};
    if(!pe_disc_read(&disc,&file,0x800ull+0x80094188u-base+85*4,&task_descriptor,4) || task_descriptor!=0x800e13d4 ||
        !pe_disc_read(&disc,&file,0x800ull+task_descriptor-base,raw,sizeof(task_methods)) ||
        memcmp(raw,task_methods,sizeof(task_methods))) {pe_disc_close(&disc);return 0;}
    PeDiscFile archive;u32 room_range[2];u8 room_bytes[0x2000];
    if(!pe_disc_find(&disc,"PE.IMG",&archive) ||
        !pe_disc_read(&disc,&file,0x800ull+0x80093378u-base+4*8,room_range,sizeof(room_range))) {pe_disc_close(&disc);return 0;}
    u64 room_sector=(u64)room_range[0]+(room_range[1]&255)+((room_range[1]>>8)&4095);
    if(!pe_disc_read(&disc,&archive,room_sector*2048,room_bytes,sizeof(room_bytes))) {pe_disc_close(&disc);return 0;}
    struct {u32 address;const void *value;size_t size;} room_rows[]={
        {0x8018eff4,&g_RoomM005OrbiterJointAnchor,8},
        {0x8018effc,&g_RoomM005CrossFlareFlashAnchor,8},
        {0x8018f004,&g_RoomM005CrossFlareTwinAnchor,8},
        {0x8018f00c,&g_RoomM005BeaconTint,4},
        {0x8018f010,&g_RoomM005JointBeaconAnchor,8},
        {0x80190ad4,RoomM005_Rec2,sizeof(RoomM005_Rec2)},
        {0x80190af4,g_RoomM005CrossFlareColorTrack,sizeof(g_RoomM005CrossFlareColorTrack)},
        {0x80190b80,g_RoomScriptRecordWords,20},
    };
    for(unsigned i=0;i<sizeof(room_rows)/sizeof(room_rows[0]);++i)
        if(memcmp(room_bytes+(room_rows[i].address-0x8018efe8u),room_rows[i].value,room_rows[i].size)) {pe_disc_close(&disc);return 0;}
    pe_disc_close(&disc);
    puts("Original glow constants: resident PM IDs, spins, scales, offsets, slot sizes and authored script match the supplied USA disc");
    return 1;
}

static int native_glow_layer_packets(void)
{
    struct { FieldStripPacket packets[4]; u8 guard[16]; } buffer;
    static RenderGpuTag ordering[4096];
    RenderBufferPrefix saved=D_800B0E38;
    int saved_slot=g_ActiveDrawSlot, saved_offset=g_DrawPacketOffset;
    GteMatrix *saved_view=g_GeomScroll.words.viewMatrix;
    GteMatrix view={.m={{4096,0,0},{0,4096,0},{0,0,4096}}};
    g_GeomScroll.words.viewMatrix=&view;
    FieldGlowLayers glow={.depth=128,.z=1024,.matrix=view};
    int (*draws[])(void *,void *,void *)={GlowEffect_DrawFlareLayers,
        GlowEffectB_DrawFlareLayers,GlowEffectC_DrawFlareLayers,FourLayerGlow_DrawFlareLayers};
    FieldGlowSprite *sprites[]={&g_GlowEffectSprite2,&g_GlowEffectBSprite2,&g_GlowEffectCSprite1,&g_FourLayerGlowSprite2};
    Psyz_GteCtrlWrite(26,256); Psyz_GteCtrlWrite(29,1365); Psyz_GteCtrlWrite(30,1024);
    gte_SetGeomOffset(160,120);
    for(unsigned slot=0;slot<2;++slot) for(unsigned kind=0;kind<4;++kind) {
        memset(&buffer,0xa5,sizeof(buffer)); memset(ordering,0,sizeof(ordering));
        for(unsigned i=0;i<4096;++i) ordering[i].address=0xffffff;
        *sprites[kind]=(FieldGlowSprite){.r=80,.g=80,.b=80,.cell=0xae,.clut=7,.offset=-50};
        D_800B0E38.ordering[slot]=ordering; D_800B0E38.packets[slot]=(char *)&buffer;
        g_ActiveDrawSlot=slot; g_DrawPacketOffset=0;
        draws[kind](NULL,NULL,&glow);
        unsigned count=kind==3?4:2;
        if(g_DrawPacketOffset!=count*sizeof(FieldStripPacket)) { fprintf(stderr,"glow kind %u slot %u bytes %d expected %zu\n",kind,slot,g_DrawPacketOffset,count*sizeof(FieldStripPacket)); return 0; }
        unsigned linked=0, seen=0;
        for(unsigned depth=0;depth<4096;++depth) {
            uintptr_t address=ordering[depth].address;
            while(address!=0xffffff) {
                if(address<(uintptr_t)buffer.packets || address>=(uintptr_t)&buffer.packets[count] ||
                   (address-(uintptr_t)buffer.packets)%sizeof(FieldStripPacket) || linked>=count) return 0;
                FieldStripPacket *packet=(FieldStripPacket *)address;
                unsigned bit=1u<<(unsigned)(packet-buffer.packets);
                if(seen&bit) return 0;
                seen|=bit;
                unsigned brightness=(kind==3 && packet<&buffer.packets[2])?40:80;
                if(packet->tag.length!=9 || packet->code!=0x2e ||
                   packet->r0!=brightness || packet->g0!=brightness || packet->b0!=brightness) {
                    fprintf(stderr,"glow kind %u packet %td RGB %u/%u/%u code %x\n",kind,packet-buffer.packets,packet->r0,packet->g0,packet->b0,packet->code); return 0;
                }
                ++linked; address=packet->tag.address;
            }
        }
        if(linked!=count) return 0;
        const u8 *bytes=(const u8 *)&buffer;
        for(size_t i=count*sizeof(FieldStripPacket);i<sizeof(buffer);++i) if(bytes[i]!=0xa5) return 0;
    }
    D_800B0E38=saved; g_ActiveDrawSlot=saved_slot; g_DrawPacketOffset=saved_offset;
    g_GeomScroll.words.viewMatrix=saved_view;
    puts("Original glow layers: three paired variants and four-layer variant, both slots, native chains and guards passed");
    return 1;
}

static unsigned object_init_calls, object_draw_calls, object_update_calls;
static FieldFxObject *expected_effect_object;
static int object_slot_init(void *object, void *slot, void *data)
{
    FieldFxSlot *entry=slot;
    if(object!=expected_effect_object || entry!=&expected_effect_object->slots[0] ||
       data!=expected_effect_object->data || entry->counter || entry->flag!=1) abort();
    memcpy(data,&object,sizeof(object)); ++object_init_calls;
    return 0;
}
static int object_slot_draw(void *object, void *slot, void *data)
{
    void *saved=NULL; memcpy(&saved,data,sizeof(saved));
    if(saved!=object || object!=expected_effect_object || slot!=&expected_effect_object->slots[0]) abort();
    ++object_draw_calls; return 0;
}
static int object_slot_update(void *object, void *slot, void *data)
{
    object_slot_draw(object,slot,data); --object_draw_calls;
    FieldFxSlot *entry=slot;
    if(entry->counter==1) entry->flag=2;
    ++object_update_calls; return 0;
}
static int native_object_script_cycle(void)
{
    struct { FieldFxObject object; u8 guard[16]; } buffer;
    FieldActor actor={0};
    FieldFxHandler init[]={object_slot_init},draw[]={object_slot_draw},update[]={object_slot_update};
    u16 sizes[]={sizeof(void *)};
    int commands[]={0x100009,0x10000,0x20002,-1};
    memset(&buffer,0xa5,sizeof(buffer));
    FieldFxObject *object=&buffer.object;
    object->prefix.asset_type=7; object->prefix.actor=&actor;
    expected_effect_object=object;
    void **script=FieldFx_GetSlot(object);
    if(script!=(void **)&object->state.commands || object->prefix.asset_type!=7 ||
       object->prefix.actor!=&actor || object->prefix.owner_kind || object->prefix.live_slots) return 0;
    for(size_t i=0;i<sizeof(object->data);++i) if(object->data[i]) return 0;
    for(unsigned i=0;i<64;++i) if(object->slots[i].flag || object->slots[i].counter || object->slots[i].data_offset) return 0;
    object->state.commands=commands;
    object_init_calls=object_draw_calls=object_update_calls=0;
    if(FieldFx_RunScript(object,init,sizes) || object_init_calls!=1 ||
       object->state.regs[0]!=9 || object->state.slot_count!=1 || object->state.pc!=3 || object->state.wait!=2) return 0;
    FieldFx_SetScriptContext(&actor);
    if(FieldFx_GetScriptContext()!=&actor || FieldFx_RegPtr(0)!=&object->state.regs[0] ||
       FieldFx_VarPtr(0)!=&object->state.vars[0]) return 0;
    if(FieldFx_Register(object,draw) || object_draw_calls!=1) return 0;
    if(FieldFx_UpdateSlots(object,update) || object_update_calls!=1 || object->slots[0].counter!=1) return 0;
    if(FieldFx_RunScript(object,init,sizes) || object->state.wait!=1 || object_init_calls!=1) return 0;
    if(FieldFx_UpdateSlots(object,update) || object_update_calls!=2 || object->slots[0].flag || object->state.slot_count) return 0;
    if(FieldFx_RunScript(object,init,sizes) || object->state.wait) return 0;
    if(FieldFx_RunScript(object,init,sizes)!=-1 || !object->state.halted || object->state.pc!=3) return 0;
    FieldFx_GetSlot(object);
    int immediate[]={0x10000,-1}; object->state.commands=immediate;
    if(FieldFx_RunScript(object,init,sizes) || !FieldFx_IsLastSlotHalted()) return 0;
    if(FieldFx_FreeAllSlots()!=-1 || FieldFx_IsLastSlotHalted() || object->state.slot_count) return 0;
    for(unsigned i=0;i<sizeof(buffer.guard);++i) if(buffer.guard[i]!=0xa5) return 0;
    puts("Original object scripts: native reset, init/draw/update/free callbacks, wait/halt, context and guards passed");
    return 1;
}

static int authored_m005_effect_cycle(void)
{
    PmPrimarySlot bank[11]={0};FieldActor actor={0};FieldActorState state={0};u8 action=1;
    RenderMatrix matrices[21]={0};
    for(unsigned i=0;i<21;++i) {
        for(unsigned axis=0;axis<3;++axis)matrices[i].rotation[axis][axis]=4096;
        matrices[i].translation[1]=-100;matrices[i].translation[2]=1024;
    }
    actor.state=&state;actor.mode=2;actor.render_object.matrices=matrices;
    state.action_state=&action;state.control10.command_value=1;
    PmTables old_tables=g_PmTables;Pe1GameState old_game=g_GameState;u32 old_flags=g_GameStateFlags;
    RenderBufferPrefix old_buffers=D_800B0E38;GteMatrix *old_view=g_GeomScroll.words.viewMatrix;
    int old_slot=g_ActiveDrawSlot,old_offset=g_DrawPacketOffset,old_age=g_FieldFxCurrentAge;
    void *old_program=g_FieldFxTaskPrograms[32];u32 old_random=Psyz_RandState();
    FieldFxTaskContext *old_context=g_FieldFxTaskContext;FieldFxObjectPrefix *old_owner=g_FieldFxOwner;
    FieldFxTaskSlot *old_task=g_FieldFxActiveTaskSlot;
    GteMatrix view={.m={{4096,0,0},{0,4096,0},{0,0,4096}}};
    static RenderGpuTag ordering[4096];
    struct { _Alignas(void *) u8 bytes[65536];u8 guard[16]; } packets;
    g_PmTables=(PmTables){g_PmCmdHandlerArray,bank,NULL};g_FieldFxTaskPrograms[32]=&g_RoomM005Effects;
    g_GameStateFlags=0x80;g_GameState.flags&=~8u;g_GameState.tim_load_state=0;
    struct {SceneAssetBlob header;SceneAssetDirectory directory;} archive={0};
    archive.header.directoryOffset=sizeof(SceneAssetBlob);
    g_GameState.loaded_scene_archive=&archive.header;
    g_GeomScroll.words.viewMatrix=&view;
    Psyz_GteCtrlWrite(26,256);Psyz_GteCtrlWrite(27,1);Psyz_GteCtrlWrite(28,0);
    Psyz_GteCtrlWrite(29,1365);Psyz_GteCtrlWrite(30,1024);gte_SetGeomOffset(160,120);
    FieldFx_InitTexturePageTable();srand(1);
    memset(g_RoomM005ScriptAnchorView-4,0,20);
    if(Scene_LoadRoomAssets(117,&actor)!=0 || Pm_SendCmd(0,0,0,0,0,0)!=0)return 0;
    unsigned drawn=0,linked=0,ring_links=0,blend_links=0;
    for(unsigned frame=0;frame<76;++frame) {
        memset(&packets,0xa5,sizeof(packets));
        for(unsigned depth=0;depth<4096;++depth)ordering[depth]=(RenderGpuTag){.address=0xffffff};
        g_ActiveDrawSlot=frame&1;g_DrawPacketOffset=0;
        D_800B0E38.ordering[g_ActiveDrawSlot]=ordering;D_800B0E38.packets[g_ActiveDrawSlot]=(char *)packets.bytes;
        if(frame==30 && Pm_SendCmd(0,0,1,0,0,0)!=0)return 0;
        if(Pm_Start(0)!=0 || Pm_Exec(0)!=0 || bank[0].task.tasks.count!=((frame<41 || frame>=62)?1:2))return 0;
        if(g_DrawPacketOffset<0 || g_DrawPacketOffset>sizeof(packets.bytes))return 0;
        drawn+=g_DrawPacketOffset!=0;
        for(unsigned depth=0;depth<4096;++depth) {
            uintptr_t address=ordering[depth].address;unsigned chain=0;
            while(address!=0xffffff) {
                if(address<(uintptr_t)packets.bytes || address+sizeof(RenderGpuTag)>(uintptr_t)packets.bytes+g_DrawPacketOffset ||
                    address%_Alignof(RenderGpuTag) || ++chain>512)return 0;
                RenderGpuTag *tag=(void *)address;
                if(tag->length!=9 && tag->length!=8 && tag->length!=1)return 0;
                ring_links+=tag->length==8;blend_links+=tag->length==1;
                ++linked;address=tag->address;
            }
        }
        for(unsigned i=0;i<16;++i)if(packets.guard[i]!=0xa5)return 0;
    }
    RoomM005OrbiterState *orbiter=(void *)bank[0].task.tasks.slots[0].start;
    if(!drawn || !linked || !ring_links || !blend_links || orbiter->armed!=1 || orbiter->frame!=32 ||
        bank[0].task.tasks.slots[0].id!=0xffff || bank[0].task.tasks.slots[1].id!=1 ||
        bank[0].task.tasks.slots[1].age!=35 || bank[0].task.tasks.variables[0]!=1)return 0;
    s16 *anchor_words=(void *)g_RoomM005ScriptAnchorView,*fade_words=(void *)g_RoomM005ScriptFadeView;
    if(anchor_words[0] || anchor_words[1]!=-100 || anchor_words[2]!=1024 ||
        fade_words[0] || fade_words[1]!=-100 || fade_words[2]!=964)return 0;
    g_FieldFxCurrentAge=35;
    if(RoomM005_CrossFlare(1,bank[0].task.tasks.slots[1].start,bank[0].task.tasks.argument)!=1)return 0;
    if(Pm_Stop(0,&actor,0)!=0 || bank[0].header.state)return 0;
    g_FieldFxTaskContext=old_context;g_FieldFxOwner=old_owner;g_FieldFxActiveTaskSlot=old_task;g_FieldFxCurrentAge=old_age;
    g_PmTables=old_tables;g_GameState=old_game;g_GameStateFlags=old_flags;g_FieldFxTaskPrograms[32]=old_program;
    D_800B0E38=old_buffers;g_GeomScroll.words.viewMatrix=old_view;g_ActiveDrawSlot=old_slot;g_DrawPacketOffset=old_offset;
    srand(old_random);memset(g_RoomM005ScriptAnchorView-4,0,20);
    puts("Original M005: 76 PM frames, real orbiter/cross-flare script, overlapping tasks, OT rings/blends and arena guards passed");
    return 1;
}

static int authored_glow_script_cycle(void)
{
    PmCommand *classes[]={&g_GlowEffectClass,&g_GlowEffectBClass,&g_GlowEffectCClass,&g_FourLayerGlowClass,&g_PointSpriteClass,&g_BurstEffectClass,&g_BurstEffectBClass,&g_BurstEffectCClass};
    unsigned commands[]={0,1,2,3,7,5,4,6};
    for(unsigned variant=0;variant<8;++variant) for(unsigned player_action=1;player_action<=7;++player_action) {
    struct { PmPrimarySlot primary[11]; u8 guard[16]; } buffer={0};
    RenderMatrix matrices[20]={0};
    FieldActor owner={0}; FieldActorState owner_state={0}; u8 owner_action=1;
    BattleEntity player={0}, *saved_player=D_8009D254;
    Combatant core={0}; BattleAction action={0};
    for(unsigned i=0;i<20;++i) {
        for(unsigned j=0;j<3;++j) matrices[i].rotation[j][j]=4096;
        matrices[i].translation[2]=1024;
    }
    owner.render_object.matrices=matrices; owner.pos_z=1024*65536;
    FieldActor targets_data[2]={0}; FieldActor *targets[]={&targets_data[0],&targets_data[1],NULL};
    for(unsigned i=0;i<2;++i) {
        targets_data[i].render_object.target_x=20+i*40;
        targets_data[i].render_object.target_y=0;
        targets_data[i].render_object.target_z=1024+i*100;
    }
    owner.state=&owner_state; owner_state.control10.command_value=1;
    owner_state.action_state=&owner_action;
    player.core=&core; core.action=&action; action.actionCode.actionId=player_action;
    D_8009D254=&player;
    FieldFxObject *object=&buffer.primary[0].effect;
    object->prefix.asset_type=7; object->prefix.actor=&owner;
    memset(buffer.guard,0xa5,sizeof(buffer.guard));
    RenderBufferPrefix saved=D_800B0E38;
    int saved_slot=g_ActiveDrawSlot, saved_offset=g_DrawPacketOffset;
    GteMatrix *saved_view=g_GeomScroll.words.viewMatrix;
    GteMatrix view={.m={{4096,0,0},{0,4096,0},{0,0,4096}}};
    g_GeomScroll.words.viewMatrix=&view;
    s32 *saved_projection=g_GeomScroll.words.projectionDistance, projection=256;
    g_GeomScroll.words.projectionDistance=&projection;
    static RenderGpuTag ordering[4096];
    _Alignas(16) u8 packets[32768];
    Psyz_GteCtrlWrite(26,256); Psyz_GteCtrlWrite(29,1365); Psyz_GteCtrlWrite(30,1024);
    gte_SetGeomOffset(160,120);
    g_FieldFxLoadedTexturePage=0;
    PmTables previous=g_PmTables; Pe1GameState previous_game=g_GameState;
    u32 previous_flags=g_GameStateFlags;
    g_PmTables=(PmTables){g_PmCmdHandlerArray,buffer.primary,NULL};
    if(g_PmCmdHandlerArray[commands[variant]]!=classes[variant]) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    g_GameStateFlags=0x80; g_GameState.flags&=~8u; g_GameState.tim_load_state=0;
    g_GameState.cd_range_read_mode=0;
    classes[variant]->init();
    if(Scene_LoadRoomAssets(commands[variant],&owner)!=0 || buffer.primary[0].header.owner!=&owner ||
       buffer.primary[0].header.command!=commands[variant]) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    unsigned seen=0, drawn=0;
    if(variant>=5 && Pm_SendCmd(0,0,1,(intptr_t)targets,0,0)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    if(Pm_SendCmd(0,0,0,variant==4?0:1,0x1234,0x5678)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    for(unsigned frame=0;frame<160;++frame) {
        unsigned slot=frame&1;
        for(unsigned i=0;i<4096;++i) { ordering[i].address=0xffffff; ordering[i].length=0; }
        D_800B0E38.ordering[slot]=ordering; D_800B0E38.packets[slot]=(char *)packets;
        g_ActiveDrawSlot=slot; g_DrawPacketOffset=0;
        if(variant<4 && frame==20 && Pm_SendCmd(0,0,0,variant==4?0:1,0,0)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
        if(variant!=4 && frame==40 && Pm_SendCmd(0,0,0,2,0,0)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
        if(Pm_Start(0) || Pm_Exec(0)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
        if(frame==0 && owner_action!=((variant==4)?1:2)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
        for(unsigned i=0;i<64;++i) if(object->slots[i].flag==1) {
            seen|=1u<<object->slots[i].handler_id;
            if(variant==4 && object->slots[i].handler_id==1) {
                FieldFxRadialParticles *data=(void *)(object->data+object->slots[i].data_offset);
                if(data->count!=(player_action==3?8:16)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
            }
            if(variant==4 && object->slots[i].handler_id==4) {
                FieldFxScatteredParticles *data=(void *)(object->data+object->slots[i].data_offset);
                if(data->points.count!=(player_action==3?4:16)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
            }
        }
        if(g_DrawPacketOffset) ++drawn;
        if(g_DrawPacketOffset>sizeof(packets)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
        for(unsigned i=0;i<sizeof(buffer.guard);++i) if(buffer.guard[i]!=0xa5) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    }
    if(seen!=(unsigned[]){0x3f,0xf,0x7,0x3f,0x3f,0xf,0x7,0x3}[variant] || !object->state.halted || object->state.slot_count || !drawn || buffer.primary[0].header.state || buffer.primary[0].header.owner) {
        fprintf(stderr,"authored effect %u handlers %x halted %d slots %d drawn %u\n",variant,seen,object->state.halted,object->state.slot_count,drawn); do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    }
    if(Scene_LoadRoomAssets(commands[variant],&owner)!=0) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    if(variant>=5 && Pm_SendCmd(0,0,1,(intptr_t)targets,0,0)) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    if(Pm_SendCmd(0,0,0,variant==4?0:1,0,0) || Pm_Exec(0) || !object->state.slot_count) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    if(Pm_Stop(0,&player,0) || buffer.primary[0].header.state!=2) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    if(Pm_Stop(0,&owner,0) || buffer.primary[0].header.state || buffer.primary[0].header.owner) do { fprintf(stderr,"authored effect failure line %d variant %u action %u\n",__LINE__,variant,player_action); return 0; } while(0);
    classes[variant]->unload(); g_PmTables=previous;
    g_GameState=previous_game; g_GameStateFlags=previous_flags;
    D_8009D254=saved_player; D_800B0E38=saved;
    g_ActiveDrawSlot=saved_slot; g_DrawPacketOffset=saved_offset; g_GeomScroll.words.viewMatrix=saved_view;
    g_GeomScroll.words.projectionDistance=saved_projection;
    }
    puts("Original authored effects: eight resident PM IDs and native factory allocations, seven actions, send/start/exec/close/release passed");
    return 1;
}

int Script_CopyEntityRenderObject(int **args);
static int original_actor_render_opcodes(void)
{
    FieldActor *previous=g_CurrentEntity,*previous_player=g_PlayerEntity,*previous_head=g_FieldActorListHead;
    ScriptNode *previous_node=g_ScriptNodePool;u32 *previous_cursor=g_ScriptCursor;
    RenderObjectEntity previous_battle=g_PlayerBattleEntity;
    int previous_lock=g_FieldMoveLock;
    u32 script[16]={0};
    s16 rates[]={0,1,32,-2,32767};
    for(unsigned player=0;player<2;++player) for(unsigned i=0;i<5;++i) {
        FieldActor actor={.flags=0x8040};ScriptNode node={.flags=0x8000};
        int rate=rates[i];int *args[]={&rate};
        g_CurrentEntity=&actor;g_PlayerEntity=player?&actor:NULL;g_ScriptNodePool=&node;
        g_PlayerBattleEntity=(RenderObjectEntity){0};g_FieldMoveLock=0x82;
        g_ScriptCursor=script+8;
        int duration=rate?rate:1;
        if(Script_PlayEntityAnim(args)!=0 || actor.flags!=0x8000 || node.flags!=0x8020 || node.active!=1 ||
           g_ScriptCursor!=script+5 || !(actor.render_object.flags_9C&4) ||
           actor.render_object.fade_duration!=(s8)duration || actor.render_object.fade_red_step!=(u8)(128/duration)) do { fprintf(stderr,"render opcode failure line %d\n",__LINE__); return 0; } while(0);
        if(player && (g_FieldMoveLock!=0x80 || g_PlayerBattleEntity.fade_duration!=(s8)duration ||
           !(g_PlayerBattleEntity.flags_9C&4))) do { fprintf(stderr,"render opcode failure line %d\n",__LINE__); return 0; } while(0);
        if(!player && (g_FieldMoveLock!=0x82 || g_PlayerBattleEntity.flags_9C)) do { fprintf(stderr,"render opcode failure line %d\n",__LINE__); return 0; } while(0);
        rate=123;g_ScriptCursor=script+8;
        if(Script_PlayEntityAnim(args)!=0 || g_ScriptCursor!=script+5 || actor.render_object.fade_duration!=(s8)duration) do { fprintf(stderr,"render opcode failure line %d\n",__LINE__); return 0; } while(0);
        actor.render_object.flags_9C&=~4u;g_ScriptCursor=script+8;
        if(Script_PlayEntityAnim(args)!=1 || node.flags!=0x8000 || g_ScriptCursor!=script+8) do { fprintf(stderr,"render opcode failure line %d\n",__LINE__); return 0; } while(0);
    }
    u32 saved_view[9];memcpy(saved_view,g_EntityRenderScratch,sizeof(saved_view));
    GteMatrix view={.m={{4096,0,0},{0,4096,0},{0,0,4096}}};
    memset(g_EntityRenderScratch,0,sizeof(saved_view));memcpy(g_EntityRenderScratch,&view,sizeof(view));
    for(unsigned player=0;player<2;++player) for(unsigned index=0;index<2;++index) {
        RenderMatrix matrices[2]={0};RenderVec3s bounds[4]={0};
        for(unsigned j=0;j<2;++j) {
            for(unsigned k=0;k<3;++k) matrices[j].rotation[k][k]=4096;
            matrices[j].translation[0]=-100-j*10;matrices[j].translation[1]=200+j*10;matrices[j].translation[2]=-300-j*10;
            bounds[2*j]=(RenderVec3s){-7,9,-11,10};
        }
        FieldActor owner={.type_id=7,.sub_id=11},dead={.type_id=7,.sub_id=11,.flags=0x10,.next=&owner};
        FieldActor actor={.flags=0x8000};
        owner.render_object.bounds_vertices=bounds;owner.render_object.matrices=matrices;
        g_CurrentEntity=&actor;g_PlayerEntity=player?&owner:NULL;g_FieldActorListHead=&dead;
        int type=player?0:7,id=11,part=index;int *args[]={&type,&id,&part};
        if(Script_CopyEntityRenderObject(args)!=1 || actor.parent!=&owner || actor.flags!=0xa000 ||
           actor.render_object.animation_source!=&owner.render_object || actor.render_object.table_index!=index ||
           actor.pos_x!=(-107-(int)index*10)*65536 || actor.pos_y!=(209+(int)index*10)*65536 ||
           actor.pos_z!=(-311-(int)index*10)*65536) do { fprintf(stderr,"render opcode failure line %d\n",__LINE__); return 0; } while(0);
    }
    memcpy(g_EntityRenderScratch,saved_view,sizeof(saved_view));
    g_CurrentEntity=previous;g_PlayerEntity=previous_player;g_FieldActorListHead=previous_head;
    g_ScriptNodePool=previous_node;g_ScriptCursor=previous_cursor;g_PlayerBattleEntity=previous_battle;g_FieldMoveLock=previous_lock;
    puts("Original actor render opcodes: animation start/wait/finish, player battle mirror and native inherited transforms with negative coordinates passed");
    return 1;
}

static int original_actor_camera_opcodes(void)
{
    FieldActor *previous=g_CurrentEntity,*previous_player=g_PlayerEntity;
    ScriptNode *previous_node=g_ScriptNodePool;
    u32 *previous_cursor=g_ScriptCursor;
    u32 script[16]={0};
    unsigned angles[]={0,1,511,1024,2047,2048,2049,3584,4095};
    for(unsigned from=0;from<9;++from) for(unsigned to=0;to<9;++to) {
        unsigned start=angles[from],end=angles[to];
        FieldActor actor={.rot_y=start}; ScriptNode node={.flags=0x8000};
        int target=end,step=64;int *args[]={&target,&step};
        g_CurrentEntity=&actor;g_ScriptNodePool=&node;
        unsigned calls=0;
        for(;;) {
            unsigned before=actor.rot_y&0xfff;
            g_ScriptCursor=script+8;
            int finished=Render_CameraStepAngleFade(args);
            if(++calls>80) do { fprintf(stderr,"camera opcode failure line %d\n",__LINE__); return 0; } while(0);
            if(finished) {
                if(actor.rot_y!=end || node.flags!=0x8000 || g_ScriptCursor!=script+8) do { fprintf(stderr,"camera opcode failure line %d\n",__LINE__); return 0; } while(0);
                break;
            }
            if(calls==1) {
                int span=(int)end-(int)start;
                int direction=span<0?-1:1;
                if(span>2048 || span<-2048) direction=-direction;
                if(node.target14!=64*direction) return 0;
            }
            if(!(node.flags&0x20) || node.target18.coordinate!=(int)end || node.active!=1 ||
               g_ScriptCursor!=script+4 || (actor.rot_y&0xfff)!=((before+node.target14)&0xfff)) do { fprintf(stderr,"camera opcode failure line %d\n",__LINE__); return 0; } while(0);
            target=123;step=1; /* A yielded opcode retains its original operands. */
        }
    }
    for(unsigned player=0;player<2;++player) {
        FieldActor actor={.pos_x=100*65536,.pos_z=200*65536,.rot_y=4095,
                          .move_factor=65536,.move_speed=4096};
        ScriptNode node={.flags=0x8000};
        int x=-2*65536,z=3*65536,turn=0;int *args[]={&x,&z,&turn};
        g_CurrentEntity=&actor;g_PlayerEntity=player?&actor:NULL;g_ScriptNodePool=&node;
        unsigned calls=0;
        for(;;) {
            g_ScriptCursor=script+8;
            int finished=Render_CameraTrackRelativeOffset(args);
            if(++calls>200 || node.target18.coordinate!=98*65536 || node.target1c!=203*65536) do { fprintf(stderr,"camera opcode failure line %d\n",__LINE__); return 0; } while(0);
            if(finished) {
                if(actor.pos_x!=98*65536 || actor.pos_z!=203*65536 || actor.motion_x || actor.motion_z ||
                   node.flags!=0x8000 || g_ScriptCursor!=script+8) do { fprintf(stderr,"camera opcode failure line %d\n",__LINE__); return 0; } while(0);
                break;
            }
            if(!(node.flags&0x20) || node.active!=1 || g_ScriptCursor!=script+3) do { fprintf(stderr,"camera opcode failure line %d\n",__LINE__); return 0; } while(0);
            actor.pos_x=(s32)((u32)actor.pos_x+(u32)actor.motion_x);
            actor.pos_z=(s32)((u32)actor.pos_z+(u32)actor.motion_z);
            x=z=0;turn=1;
        }
    }
    for(unsigned player=0;player<2;++player) {
        FieldActor actor={.pos_x=100*65536,.pos_z=200*65536,.rot_y=4095,
                          .move_factor=65536,.move_speed=4096};
        ScriptNode node={.flags=0x8000};
        int x=-100*65536,z=150*65536,turn=64;int *args[]={&x,&z,&turn};
        g_CurrentEntity=&actor;g_PlayerEntity=player?&actor:NULL;g_ScriptNodePool=&node;
        for(unsigned frame=0;frame<3;++frame) {
            g_ScriptCursor=script+8;
            if(Render_CameraTrackRelativeOffset(args)!=0 || actor.rot_y!=63+64*frame ||
               node.target18.coordinate!=0 || node.target1c!=350*65536 || node.target14!=64 ||
               node.flags!=0x8020 || node.active!=1 || g_ScriptCursor!=script+3) return 0;
            actor.pos_x=(s32)((u32)actor.pos_x+(u32)actor.motion_x);
            actor.pos_z=(s32)((u32)actor.pos_z+(u32)actor.motion_z);
            x=z=0;turn=1;
        }
    }
    g_CurrentEntity=previous;g_PlayerEntity=previous_player;g_ScriptNodePool=previous_node;g_ScriptCursor=previous_cursor;
    puts("Original actor camera opcodes: angle wrap, retained operands, native cursor rewinds, relative movement and player speed path passed");
    return 1;
}

static int original_absolute_movement_opcode(void)
{
    FieldActor *previous=g_CurrentEntity,*previous_player=g_PlayerEntity;
    ScriptNode *previous_node=g_ScriptNodePool;u32 *previous_cursor=g_ScriptCursor;
    u32 script[16]={0};
    const int ranges[][4]={{0,0,1,0},{0,1,2,0},{2,1,-2,0},{1,3,1,1},{3,0,1,1},{0,3,-1,1},{2,2,-1,0}};
    for(unsigned i=0;i<7;++i) {
        FieldActor actor={.action=3,.anim_step=ranges[i][2]*65536,.anim_frame_target=ranges[i][1]};
        actor.anim.fixed=ranges[i][0]*65536;
        ScriptNode node={.flags=0x8000};
        g_CurrentEntity=&actor;g_ScriptNodePool=&node;g_ScriptCursor=script+8;
        if(Script_WaitAnimRange(NULL)!=0 || node.active!=1 || node.flags!=0x8000 ||
           g_ScriptCursor!=script+(ranges[i][3]?6:8)) return 0;
    }
    for(unsigned player=0;player<2;++player) for(unsigned turns=0;turns<2;++turns) {
        FieldActor absolute={.pos_x=100*65536,.pos_z=200*65536,.rot_y=4095,
                             .move_factor=65536,.move_speed=4096};
        FieldActor relative=absolute;
        ScriptNode absolute_node={.flags=0x8000},relative_node=absolute_node;
        int dx=-2*65536,dz=3*65536,turn=turns?64:0;
        int x=98*65536,z=203*65536,absolute_turn=turn;
        int *abs_args[]={&x,&z,&absolute_turn},*rel_args[]={&dx,&dz,&turn};
        for(unsigned frame=0;frame<80;++frame) {
            g_CurrentEntity=&absolute;g_PlayerEntity=player?&absolute:NULL;
            g_ScriptNodePool=&absolute_node;g_ScriptCursor=script+8;
            int abs_result=Script_MoveTowardPoint(abs_args);
            u32 *abs_cursor=g_ScriptCursor;
            g_CurrentEntity=&relative;g_PlayerEntity=player?&relative:NULL;
            g_ScriptNodePool=&relative_node;g_ScriptCursor=script+8;
            int rel_result=Render_CameraTrackRelativeOffset(rel_args);
            if(abs_result!=rel_result || abs_cursor!=g_ScriptCursor ||
               absolute.pos_x!=relative.pos_x || absolute.pos_z!=relative.pos_z ||
               absolute.motion_x!=relative.motion_x || absolute.motion_z!=relative.motion_z ||
               absolute.rot_y!=relative.rot_y || absolute_node.flags!=relative_node.flags ||
               absolute_node.active!=relative_node.active) return 0;
            if(abs_result) {
                if(absolute.pos_x!=98*65536 || absolute.pos_z!=203*65536 || absolute.motion_x || absolute.motion_z) return 0;
                break;
            }
            if(abs_cursor!=script+3 || absolute_node.target14!=98*65536 ||
               absolute_node.target18.coordinate!=203*65536 || absolute_node.target1c!=(turns?64:0) || absolute_node.active!=1) return 0;
            if(!turns && frame==79) return 0;
            absolute.pos_x=(s32)((u32)absolute.pos_x+(u32)absolute.motion_x);
            absolute.pos_z=(s32)((u32)absolute.pos_z+(u32)absolute.motion_z);
            relative.pos_x=absolute.pos_x;relative.pos_z=absolute.pos_z;
            x=z=dx=dz=0;turn=absolute_turn=1;
        }
    }
    g_CurrentEntity=previous;g_PlayerEntity=previous_player;g_ScriptNodePool=previous_node;g_ScriptCursor=previous_cursor;
    puts("Original absolute movement opcode: relative/absolute targets agree across player speed, turning, yields, cached operands and completion");
    return 1;
}

static int original_resident_script_program(void)
{
    FieldActor *previous=g_CurrentEntity,*previous_player=g_PlayerEntity;
    ScriptNode *previous_node=g_ScriptNodePool;u32 *previous_cursor=g_ScriptCursor;
    u32 previous_variable=g_ScriptVariableBank[7],previous_id=g_ScriptVariableBank[8],previous_sfx=g_ScriptVariableBank[9];
    if(g_ScriptQueueOpcodeHandlers[68]!=Script_MoveTowardPoint ||
       g_ScriptQueueOpcodeHandlers[97]!=Render_CameraTrackRelativeOffset ||
       g_ScriptQueueOpcodeHandlers[146]!=Script_PlayEntityAnim) do { fprintf(stderr,"resident script failure line %d\n",__LINE__); return 0; } while(0);
    for(unsigned player=0;player<2;++player) {
        u32 script[]={39|(1u<<13),0,300,38|(1u<<13)|(2u<<17),0,8,
                      37|(1u<<13)|(2u<<17),0,9,68|(3u<<13),0,98*65536,203*65536,0,
                      10|(2u<<13)|(2u<<17),0,7,77,32,0};
        FieldActor actor={.pos_x=100*65536,.pos_z=200*65536,.rot_y=4095,
                          .move_factor=65536,.move_speed=4096};
        ScriptNode node={.current.script=script,.active=1};
        actor.field_sfx_id=54321;
        g_CurrentEntity=&actor;g_PlayerEntity=player?&actor:NULL;g_ScriptVariableBank[7]=0;
        unsigned ticks=0;
        while(!(node.flags&0x10)) {
            if(++ticks>80) do { fprintf(stderr,"resident script failure line %d\n",__LINE__); return 0; } while(0);
            g_ScriptNodePool=&node;Script_RunQueue();
            if(g_ScriptNodePool!=NULL) do { fprintf(stderr,"resident script failure line %d\n",__LINE__); return 0; } while(0);
            if(node.flags&0x10) {
                if(actor.pos_x!=98*65536 || actor.pos_z!=203*65536 || actor.motion_x || actor.motion_z ||
                   g_ScriptVariableBank[7]!=77 || node.current.script!=script+20 ||
                   actor.sub_id!=44 || g_ScriptVariableBank[8]!=44 || g_ScriptVariableBank[9]!=54321) do { fprintf(stderr,"resident script failure line %d\n",__LINE__); return 0; } while(0);
            } else {
                if(node.current.script!=script+9 || node.active!=1 || g_ScriptVariableBank[7]) do { fprintf(stderr,"resident script failure line %d\n",__LINE__); return 0; } while(0);
                actor.pos_x=(s32)((u32)actor.pos_x+(u32)actor.motion_x);
                actor.pos_z=(s32)((u32)actor.pos_z+(u32)actor.motion_z);
            }
        }
    }
    g_CurrentEntity=previous;g_PlayerEntity=previous_player;g_ScriptNodePool=previous_node;
    g_ScriptCursor=previous_cursor;g_ScriptVariableBank[7]=previous_variable;
    g_ScriptVariableBank[8]=previous_id;g_ScriptVariableBank[9]=previous_sfx;
    puts("Original resident interpreter: real dispatcher, movement, assignment, yields and node termination passed");
    return 1;
}

static int original_actor_audio_state(void)
{
    Pe1GameState previous=g_GameState;
    FieldActor actor={.type_id=7,.sub_id=11};
    FieldActor *previous_actor=g_CurrentEntity;
    int op=0x15e,a=0x12345,b=0x187,c=-2,d=-129,result=123;
    AkaoCommandArgs args={&op,&a,&b,&c,&d,&result};
    g_CurrentEntity=&actor;
    if(Akao_ProcessCommand(&args)!=1 || g_GameState.bank_value_f8!=(u16)a ||
       g_GameState.bank_value_f6!=(u8)b || g_GameState.bank_value_fa!=(u16)c ||
       g_GameState.bank_value_f7!=(u8)d || result!=123) return 0;
    op=0x195; g_GameState.bank_state_12=0;
    if(Akao_ProcessCommand(&args)!=1 || g_GameState.bank_state_12!=1) return 0;
    g_GameState.bank_state_11=0;
    for(unsigned i=0;i<18;++i) {
        op=i&1?0x197:0x196; a=0x180+i;b=-129;c=0x12345;d=-2;
        if(Akao_ProcessCommand(&args)!=1 || g_GameState.bank_state_11!=(i<16?i+1:16)) return 0;
        if(i<16) {
            DeferredAkaoCommand *command=&g_DeferredAkaoCommands[i];
            if(command->actor_type!=7 || command->actor_id!=11 ||
               command->command!=(s8)a || command->arg1!=(s8)b ||
               command->arg2!=(s16)c || command->arg3!=(s16)(i&1?d:c)) return 0;
        }
    }
    /* The cache hit uses the same bank pointer the original loader installs. */
    u16 sample_bank[12]={0x4b41,0x4f41,42,0,7,0};
    g_GameState.bank_work_base=(u8 *)sample_bank;
    g_GameState.bank_work_end=(u8 *)sample_bank;
    struct { u32 root[2]; SceneAssetDirectory directory; } archive={0};
    archive.root[1]=sizeof(archive.root);
    g_GameState.loaded_scene_assets=(SceneAssetView *)&archive;
    for(unsigned slot=0;slot<2;++slot) {
        g_GameState.scene_audio.tracks.keys[0][0]=slot? -1:23;
        g_GameState.scene_audio.tracks.keys[1][0]=slot?23:-1;
        g_GameState.flags&=~0xc0u;g_GameState.cd_track_phase=0;
        g_AkaoMessageQueueCount=0;
        op=0xcb;a=23;b=0x55;c=1;
        if(Akao_ProcessCommand(&args)!=1 || result!=42 || g_AkaoMessageQueueCount!=2 ||
           g_AkaoMessageQueue[0].opcode.word!=0x19 ||
           g_AkaoMessageQueue[0].arg0.sample_data!=(u8 *)(sample_bank+8) ||
           g_AkaoMessageQueue[1].opcode.word!=0xc0 ||
           g_AkaoMessageQueue[1].arg0.value!=0x55 ||
           g_GameState.scene_audio.tracks.keys[slot][1]!=42 ||
           g_GameState.track_volumes[slot]!=0x55) return 0;
    }
    u32 sound_archive[32]={0};
    sound_archive[1]=8;
    sound_archive[(8+0x30)/4]=(1u<<22)|64;
    u8 *record=(u8 *)sound_archive+64;
    record[3]=0x10;
    sound_archive[(64+4)/4]=96;
    record[8]=0xff;record[10]=0x34;record[11]=0x12;
    g_GameState.loaded_scene_assets=(SceneAssetView *)sound_archive;
    op=0x190;a=0x1234;
    if(Akao_ProcessCommand(&args)!=1 || (u8)g_GameState.scene_audio.tracks.pending_key!=0x34 ||
       (u8)g_GameState.scene_audio.tracks.pending_bank!=0xff ||
       g_GameState.bank_asset_table!=(u8 *)sound_archive+96) return 0;
    op=0x199;a=0x198;
    if(Akao_ProcessCommand(&args)!=1 || g_GameState.scene_audio.tracks.movie_volume!=0x98) return 0;
    g_AkaoMessageQueueCount=0;g_GameState=previous;g_CurrentEntity=previous_actor;
    puts("Original actor audio: canonical positional/deferral/archive/volume state, sixteen-command limit and both cached native bank pointers passed");
    return 1;
}

static int original_burst_target_selection(void)
{
    FieldFxHandler initialize[]={BurstEffect_InitTarget,BurstEffectB_InitTarget,BurstEffectC_InitTarget};
    FieldActor **selected[]={&g_BurstEffectTarget,&g_BurstEffectBTarget,&g_BurstEffectCTarget};
    FieldActor actors[2]={0}; FieldActor *targets[]={&actors[0],&actors[1],NULL};
    unsigned int sound_header[4]={0x4f414b41u,0,0,0};
    Pe1GameState previous=g_GameState;
    int previous_page=g_FieldFxLoadedTexturePage;
    for(unsigned variant=0;variant<3;++variant) for(unsigned gate=0;gate<3;++gate) {
        FieldFxObject object={0};
        FieldFx_GetSlot(&object);
        *FieldFx_VarPtr(1)=(intptr_t)targets;
        g_GameState.cd_range_read_mode=(u8[]){0,1,255}[gate];
        g_GameState.scene_object_tables[2]=sound_header;
        g_FieldFxLoadedTexturePage=0;
        for(unsigned index=0;index<2;++index) {
            actors[index].render_object.target_x=0x1234+index;
            actors[index].render_object.target_y=0xabcd-index;
            actors[index].render_object.target_z=0x8000+index;
            g_AkaoMessageQueueCount=0;
            if(initialize[variant](&object,NULL,NULL) || *selected[variant]!=&actors[index] ||
               *FieldFx_RegPtr(13)!=(int)index+1 || *FieldFx_RegPtr(14)!=(int)index) return 0;
            u16 x=variant==0?g_BurstEffectTargetPosition.x:variant==1?g_BurstEffectBTargetPosition.x:g_BurstEffectCTargetPosition.x;
            u16 y=variant==0?g_BurstEffectTargetPosition.y:variant==1?g_BurstEffectBTargetPosition.y:g_BurstEffectCTargetPosition.y;
            u16 z=variant==0?g_BurstEffectTargetPosition.z:variant==1?g_BurstEffectBTargetPosition.z:g_BurstEffectCTargetPosition.z;
            if(x!=(u16)(0x1234+index) || y!=(u16)(0xabcd-index) || z!=(u16)(0x8000+index)) return 0;
            int expected=variant<2 && gate!=0;
            if(g_AkaoMessageQueueCount!=expected) return 0;
            if(expected && (g_AkaoMessageQueue[0].opcode.word!=0x24 ||
               g_AkaoMessageQueue[0].arg0.sequence!=(u8 *)(sound_header+1) ||
               g_AkaoMessageQueue[0].arg1!=0 || g_AkaoMessageQueue[0].arg2!=0x80 ||
               g_AkaoMessageQueue[0].arg3!=0x7f)) return 0;
        }
    }
    g_AkaoMessageQueueCount=0; g_GameState=previous; g_FieldFxLoadedTexturePage=previous_page;
    g_BurstEffectTarget=g_BurstEffectBTarget=g_BurstEffectCTarget=NULL;
    puts("Original bursts: native target lists, coordinates, final-target marker and canonical sound gate enqueue through original AKAO passed");
    return 1;
}

static int native_effect_spawn_arguments(void)
{
    struct { FieldFxObject object; u8 guard[16]; } buffer;
    PmSendCallback spawn[]={GlowEffect_Spawn6,GlowEffectB_Spawn6,
        GlowEffectC_Spawn6,FourLayerGlow_Spawn6,PointSprite_Spawn6,BurstEffect_Spawn6,BurstEffectB_Spawn6,BurstEffectC_Spawn6};
    for(unsigned method=0;method<8;++method) for(unsigned index=0;index<9;++index) {
        memset(&buffer,0xa5,sizeof(buffer));
        FieldFxState expected=buffer.object.state;
        int value=index&1?INT_MIN:INT_MAX;
        expected.vars[index]=value;
        if(spawn[method]((PmSlotHeader *)&buffer.object,0x1234,index,value,0x5678,0x9abc) ||
           memcmp(&expected,&buffer.object.state,sizeof(expected)) ||
           g_FieldFxScriptState!=&buffer.object.state) return 0;
        for(unsigned i=0;i<sizeof(buffer.guard);++i) if(buffer.guard[i]!=0xa5) return 0;
        FieldActor target={0}; FieldActor *targets[2]={&target,NULL};
        if(spawn[method]((PmSlotHeader *)&buffer.object,0,index,(intptr_t)targets,0,0) ||
           *FieldFx_VarPtr(index)!=(intptr_t)targets) return 0;
        FieldActor **received=(FieldActor **)*FieldFx_VarPtr(index);
        if(received[0]!=&target || received[1]!=NULL) return 0;
        FieldFxState saved=buffer.object.state;
        if(FieldFx_Spawn6((intptr_t)&buffer.object,0,-1,42,0,0)!=-1 ||
           FieldFx_Spawn6((intptr_t)&buffer.object,0,9,42,0,0)!=-1 ||
           FieldFx_Spawn6(0,0,0,42,0,0)!=-1 || memcmp(&saved,&buffer.object.state,sizeof(saved))) return 0;
    }
    puts("Original spawn arguments: eight entry points, native owners/target-list addresses, all variables, signed values and invalid bounds passed");
    return 1;
}

static size_t slot_write_size;
static unsigned slot_write_calls;
static int object_slot_write(void *object, void *slot, void *data)
{
    FieldFxObject *owner=object; FieldFxSlot *entry=slot;
    if(data!=owner->data+entry->data_offset || entry->data_offset<0 ||
       entry->data_offset+slot_write_size>sizeof(owner->data)) abort();
    memset(data,0x6c,slot_write_size); ++slot_write_calls;
    return 0;
}
static int native_object_slot_bounds(void)
{
    struct { FieldFxObject object; u8 guard[16]; } buffer={0};
    FieldFxObject *object=&buffer.object;
    FieldFxHandler handlers[]={object_slot_write,object_slot_write};
    u16 sizes[]={1,FIELDFX_DATA_SIZE+1};
    object->prefix.asset_type=7;
    memset(buffer.guard,0xa5,sizeof(buffer.guard));
    FieldFx_GetSlot(object); slot_write_size=1; slot_write_calls=0;
    for(unsigned i=0;i<64;++i) {
        if(FieldFx_StartSlot(object,0,sizes,handlers)!=&object->data[i] ||
           object->slots[i].flag!=1 || object->slots[i].data_offset!=i) return 0;
    }
    if(object->state.slot_count!=64 || FieldFx_StartSlot(object,0,sizes,handlers) ||
       object->state.regs[14]!=1 || slot_write_calls!=64) return 0;
    object->state.halted=1;
    if(FieldFx_FreeAllSlots()!=-1 || object->state.slot_count || FieldFx_FreeSlot(0) ||
       FieldFx_FreeSlot(64) || object->state.slot_count) return 0;
    FieldFx_GetSlot(object); slot_write_calls=0;
    FieldFxState saved=object->state;
    if(FieldFx_StartSlot(object,1,sizes,handlers) || slot_write_calls ||
       memcmp(&saved,&object->state,sizeof(saved)) || object->slots[0].flag) return 0;
    FieldFx_ActivateSlot(64,0,1); FieldFx_ActivateSlot(0,0,FIELDFX_DATA_SIZE+1);
    if(memcmp(&saved,&object->state,sizeof(saved)) || object->slots[0].flag) return 0;
    object->state.data_next=-1; saved=object->state;
    if(FieldFx_StartSlot(object,0,sizes,handlers) || memcmp(&saved,&object->state,sizeof(saved))) return 0;
    FieldFx_GetSlot(object);
    sizes[0]=FIELDFX_DATA_SIZE-1; slot_write_size=sizes[0];
    if(FieldFx_StartSlot(object,0,sizes,handlers)!=object->data ||
       object->state.data_next!=FIELDFX_DATA_SIZE-1) return 0;
    sizes[0]=2; slot_write_size=2;
    if(FieldFx_StartSlot(object,0,sizes,handlers)!=object->data || object->state.data_next!=2 ||
       object->slots[1].data_offset) return 0;
    FieldFx_GetSlot(object); object->state.data_next=8;
    sizes[0]=FIELDFX_DATA_SIZE; slot_write_size=sizes[0];
    if(FieldFx_StartSlot(object,0,sizes,handlers)!=object->data ||
       object->state.data_next!=FIELDFX_DATA_SIZE) return 0;
    for(unsigned i=0;i<FIELDFX_DATA_SIZE;++i) if((u8)object->data[i]!=0x6c) return 0;
    for(unsigned i=0;i<sizeof(buffer.guard);++i) if(buffer.guard[i]!=0xa5) return 0;
    puts("Original object slots: 64-slot limit, full arena write, original wrap, invalid reservations and double-free guards passed");
    return 1;
}

static int process_task_bank_failures(void)
{
    Pe1GameState saved=g_GameState;
    u16 saved_extent[2]={g_SceneAssetLbaTbl[0],g_SceneAssetLbaTbl[1]};void *saved_buffer=g_PmTaskProgramBuffer;
    void *programs[104]; memcpy(programs,g_FieldFxTaskPrograms,sizeof(programs));
    g_GameState.pe_image_base_lba=12345; g_GameState.flags&=~0x10000u;
    g_SceneAssetLbaTbl[0]=7; g_SceneAssetLbaTbl[1]=49;
    g_PmTaskProgramBuffer=glyph_scratch;
    for(unsigned command=108;command<=114;++command) {
        glyph_reads=glyph_polls=0; glyph_read_retry=1; glyph_poll_retry=glyph_pending=0;
        if(Scene_LoadRoomAssets(command,NULL)!=-13 || glyph_reads!=1 || glyph_polls ||
           (g_GameState.flags&0x10000) || memcmp(programs,g_FieldFxTaskPrograms,sizeof(programs))) return 0;
        glyph_reads=glyph_polls=0; glyph_poll_retry=1; glyph_pending=1;
        if(Scene_LoadRoomAssets(command,NULL)!=-13 || glyph_reads!=1 || glyph_polls!=2 ||
           (g_GameState.flags&0x10000) || memcmp(programs,g_FieldFxTaskPrograms,sizeof(programs))) return 0;
    }
    glyph_reads=glyph_polls=0;
    g_SceneAssetLbaTbl[1]=7;
    if(Scene_LoadRoomAssets(108,NULL)!=-13 || glyph_reads || glyph_polls) return 0;
    g_SceneAssetLbaTbl[1]=6;
    if(Scene_LoadRoomAssets(108,NULL)!=-13 || glyph_reads || glyph_polls) return 0;
    g_SceneAssetLbaTbl[1]=49; g_GameState.pe_image_base_lba=UINT32_MAX-10;
    if(Scene_LoadRoomAssets(108,NULL)!=-13 || glyph_reads || glyph_polls) return 0;
    g_GameState.pe_image_base_lba=12345; g_PmTaskProgramBuffer=NULL;
    if(Scene_LoadRoomAssets(108,NULL)!=-13 || glyph_reads || glyph_polls) return 0;
    g_PmTaskProgramBuffer=glyph_scratch; g_SceneAssetLbaTbl[0]=8; g_SceneAssetLbaTbl[1]=50;
    if(Scene_LoadRoomAssets(108,NULL)!=-13 || glyph_reads!=1 || glyph_polls) return 0;
    g_SceneAssetLbaTbl[0]=7;g_SceneAssetLbaTbl[1]=49;
    for(unsigned command=108;command<=114;++command) {
        glyph_reads=glyph_polls=0;glyph_read_retry=glyph_poll_retry=glyph_pending=0;
        if(Scene_LoadRoomAssets(command,NULL)!=-13 || glyph_reads!=1 || glyph_polls!=1 ||
            (g_GameState.flags&0x10000) || memcmp(programs,g_FieldFxTaskPrograms,sizeof(programs)))return 0;
    }
    g_SceneAssetLbaTbl[0]=saved_extent[0];g_SceneAssetLbaTbl[1]=saved_extent[1];g_PmTaskProgramBuffer=saved_buffer;
    g_GameState=saved; glyph_reads=glyph_polls=0; glyph_read_retry=glyph_poll_retry=glyph_pending=0;
    puts("Original process task-bank I/O: seven commands, read/poll failures and invalid extents return without retry or registry mutation");
    return 1;
}

static int process_texture_bank_failures(void)
{
    Pe1GameState saved=g_GameState; PmTables tables=g_PmTables;
    static PmCommand *handlers[86];
    g_PmTables.handlers=handlers;
    g_GameState.pe_image_base_lba=12345; g_GameState.scene_load_scratch=glyph_scratch;
    g_GameState.loaded_scene_assets=(SceneAssetView *)glyph_scratch;
    g_PeImageSectorTable[5]=7; g_PeImageSectorTable[6]=49;
    g_GameState.flags|=8;
    g_GameState.tim_load_state=0x34; glyph_reads=glyph_polls=0; glyph_read_retry=1;
    if(Scene_LoadRoomAssets(0,NULL)!=-14 || g_GameState.tim_load_state!=0x34 ||
       glyph_reads!=1 || glyph_polls || !(g_GameState.flags&8)) return 0;
    g_GameState.tim_load_state=0x35; glyph_reads=glyph_polls=0; glyph_poll_retry=glyph_pending=1;
    if(Scene_LoadRoomAssets(0,NULL)!=-14 || g_GameState.tim_load_state!=0x34 ||
       glyph_reads || glyph_polls!=2 || !(g_GameState.flags&8)) return 0;
    glyph_reads=glyph_polls=0; g_PeImageSectorTable[5]=8; g_PeImageSectorTable[6]=50;
    if(Asset_LoadTimTextures(0)!=-1 || g_GameState.tim_load_state!=0x34 || glyph_reads!=1 || glyph_polls) return 0;
    glyph_reads=0; g_PeImageSectorTable[6]=8;
    if(Asset_LoadTimTextures(0)!=-1 || glyph_reads || glyph_polls) return 0;
    g_PeImageSectorTable[6]=7;
    if(Asset_LoadTimTextures(0)!=-1 || glyph_reads || glyph_polls) return 0;
    g_PeImageSectorTable[5]=7; g_PeImageSectorTable[6]=49;
    g_GameState.pe_image_base_lba=UINT32_MAX-10;
    if(Asset_LoadTimTextures(0)!=-1 || glyph_reads || glyph_polls) return 0;
    g_GameState.pe_image_base_lba=12345; g_GameState.scene_load_scratch=NULL;
    if(Asset_LoadTimTextures(0)!=-1 || glyph_reads || glyph_polls) return 0;
    g_GameState.scene_load_scratch=glyph_scratch;
    memset(glyph_source,0,sizeof(glyph_source));
    SceneAssetBlob empty={.directoryOffset=sizeof(SceneAssetBlob)};
    memcpy(glyph_source,&empty,sizeof(empty));
    glyph_pending=1;
    /* A successful empty upload completes first; only then the missing PM
     * handler is reported. No fake handler or program is executed. */
    if(Scene_LoadRoomAssets(0,NULL)!=-8 || g_GameState.tim_load_state ||
       (g_GameState.flags&8) || glyph_reads!=1 || glyph_polls!=2) return 0;
    g_GameState=saved; g_PmTables=tables; glyph_reads=glyph_polls=0;
    glyph_read_retry=glyph_poll_retry=glyph_pending=0;
    puts("Original process texture I/O: read/pending-poll errors, retry phase, extent checks and empty-bank recovery passed");
    return 1;
}

static int native_actor_effect_gates(void)
{
    FieldFxObject object={0}; FieldActor actor={0}; FieldActorState state={0};
    u8 action=1; FieldFxHandler handlers[1]={0};
    actor.state=&state; state.control10.command_value=1; state.action_state=&action;
    object.prefix.actor=&actor;
    FieldFx_GetSlot(&object);
    for(unsigned selector=0;selector<8;++selector) for(unsigned enabled=0;enabled<2;++enabled)
    for(unsigned value=0;value<2;++value) {
        state.count_limit=state.progress_step=0; state.flags=0;
        memset(state.transition_timers,0,sizeof(state.transition_timers));
        memset(state.number_desc_1,0,sizeof(state.number_desc_1));
        memset(state.reserved74,0,sizeof(state.reserved74));
        state.substate=NULL; state.script_value8c=0;
        switch(selector) {
        case 0: state.count_limit=value; break;
        case 1: state.progress_step=value; break;
        case 2: state.transition_timers[0]=value; break;
        case 3: state.flags=value; break;
        case 4: state.number_desc_1[4]=value; break;
        case 5: state.substate=(void *)(uintptr_t)(0x100+value); break;
        case 6: state.reserved74[8]=value; break;
        case 7: state.script_value8c=value; break;
        }
        state.core_flags=(selector<<21)|(enabled?2:0); action=1;
        if(FieldFx_UpdateSlots(&object,handlers)!=(enabled && !value?-1:0) || action!=2) return 0;
    }
    puts("Original actor effect gates: all eight native fields, active masks, zero/nonzero values and action transition passed");
    return 1;
}

static int room_actor_view(void)
{
    FieldActor actor={0}, parent={0};
    FieldActorState state={0};
    u8 action_state=1;
    RenderMatrix matrices[2]={0};
    actor.state=&state; state.action_state=&action_state;
    actor.parent=&parent; actor.next=&parent; actor.prev=&parent;
    actor.type_id=7; actor.sub_id=9; actor.flags=0x400; actor.mode=3;
    actor.anim.parts.integer=19; actor.anim_prev=7<<16;
    actor.pos_x=123<<16; actor.pos_y=-456*65536; actor.pos_z=789<<16;
    actor.rot_y=0x123;
    actor.motion_x=11; actor.accel_y=22; actor.gravity_z=33;
    actor.render_object.matrices=matrices;
    actor.render_object.model_matrix.translation[2]=42;
    RoomLink *view=(RoomLink *)&actor;
    FieldActorNode *node=(FieldActorNode *)&actor;
    if(node->w00!=&state || node->next!=(FieldActorNode *)&parent || node->w08!=(FieldActorNode *)&parent ||
       node->b0C!=7 || node->b0D!=9 || node->w98!=0x400) return 0;
    if(view->variant!=3 || view->winLo!=19 || view->winHi!=7 || view->link18C!=(RoomLink *)&parent)
        return 0;
    if(view->pos[0]!=actor.pos_x || view->pos[1]!=actor.pos_y || view->rotation[1]!=actor.rot_y ||
       view->vel[0]!=11 || view->move[1]!=22 || view->accel[2]!=33 || view->p238!=matrices ||
       view->modelTranslation[2]!=42) return 0;
    view->pos[2]=-5*65536; view->posMirror[1]=17; view->h250=0x400;
    view->target->flags=0x40000000;
    view->target->startFlag=9;
    if(actor.pos_z!=-5*65536 || actor.base_y!=17 || actor.render_object.flags_9C!=0x400 ||
       state.core_flags!=0x40000000 || state.reserveda8[4]!=9) return 0;
    FieldActor snapshot=actor;
    int (*release[])(RoomEnt *)={RoomLib_ReleaseHandlerB,RoomLib_ReleaseHandlerC};
    for(unsigned method=0;method<2;++method) {
        int signal=1;
        RoomEnt object={0};
        object.link=view; object.active=1; object.flag3=1; object.sub.signal=&signal;
        action_state=1;
        if(release[method](&object)!=0 || action_state!=4 || signal || object.flag3 || object.state!=4)
            return 0;
        if(memcmp(&actor,&snapshot,sizeof(actor))) return 0;
    }
    puts("Original room actor view: canonical position, animation, motion, parent/model pointers and B/C release state passed");
    return 1;
}

static int room_callback_context(void)
{
    RoomEnt object={0};
    expected_room_context=&object;
    object.sub.cb=record_room_callback;
    int (*updates[])(RoomEnt *)={RoomLib_UpdateHandlerA,RoomLib_UpdateHandlerB,RoomLib_UpdateHandlerC};
    for(unsigned method=0;method<3;++method) {
        if(updates[method](&object)!=0 || room_callback_count!=method+1 || object.w04!=(int)(method+1)*7)
            return 0;
    }
    puts("Original room actor classes: A/B/C update callbacks received their native object context");
    return 1;
}

/* USA Save_SelectMetadataWindow selects C0DE0 or C20A4 (+10 for armor). */
static int original_save_metadata_storage(void)
{
    SaveRuntimeState saved=g_SaveRuntimeState;
    int index=g_SaveMetadataWindowIndex, length=g_SaveMetadataWindowLength;
    u8 *cursor=g_SaveMetadataCursor;
    ItemDataRecord *source=g_SaveMetadataSourceRecord;
    ItemDataRecord item={0};
    int valid=1;
    memset(&g_SaveRuntimeState,0xFF,sizeof(g_SaveRuntimeState));
    for(int window=0;window<2;++window) {
        g_SaveMetadataWindowIndex=window;
        Save_SelectMetadataWindow(NULL);
        if(g_SaveMetadataCursor!=g_SaveRuntimeState.windows.metadata[window].text ||
           g_SaveMetadataWindowLength!=8 || g_SaveMetadataSourceRecord) valid=0;
        Save_AppendMetadataChar(0x30+window);
        if(g_SaveRuntimeState.windows.metadata[window].text[0]!=0x30+window ||
           g_SaveRuntimeState.windows.metadata[window].text[1]!=0xFF) valid=0;
        item.kind=window?ITEM_KIND_ARMOR:0;
        Save_SelectMetadataWindow(&item);
        if(g_SaveMetadataCursor!=g_CursorRenderMetadataWindows[window].text ||
           g_SaveMetadataSourceRecord!=&item) valid=0;
        Save_AppendMetadataChar(0x40+window);
        if(((u8 *)&g_SaveRuntimeState)[0x12C4+window*16]!=0x40+window ||
           g_CursorRenderDataBlock[window*16]!=0x40+window) valid=0;
        /* Retail replaces the eighth glyph when a ninth is entered. */
        for(unsigned i=1;i<8;++i) Save_AppendMetadataChar(0x50+i);
        Save_AppendMetadataChar(0x60);
        if(g_CursorRenderMetadataWindows[window].text[7]!=0x60 ||
           g_CursorRenderMetadataWindows[window].text[8]!=0xFF) valid=0;
    }
    g_SaveRuntimeState=saved;
    g_SaveMetadataWindowIndex=index; g_SaveMetadataWindowLength=length;
    g_SaveMetadataCursor=cursor; g_SaveMetadataSourceRecord=source;
    if(!valid) fprintf(stderr,"Original save metadata storage/selection failed\n");
    return valid;
}

static char card_test_root[128];
static int card_test_path(char *destination,const char *source,int maximum)
{
    if(strncmp(source,"bu",2) || (source[2]!='0' && source[2]!='1') || source[3]!='0' || source[4]!=':') return -1;
    const char *name=source+5;
    return snprintf(destination,(size_t)maximum,"%s/bu%c0/%s",card_test_root,source[2],*name=='*'?"":name);
}
static int original_card_file_services(const u8 *image)
{
    snprintf(card_test_root,sizeof(card_test_root),"pe-card-io-%llx",(unsigned long long)SDL_GetPerformanceCounter());
    SDL_PathInfo info;
    if(SDL_GetPathInfo(card_test_root,&info) || !SDL_CreateDirectory(card_test_root)) return 0;
    int valid=1;
    u8 expected[0x2000],actual[0x2000];
    if(image)memcpy(expected,image,sizeof(expected));
    else for(unsigned i=0;i<sizeof(expected);++i)expected[i]=(u8)(i*37+(i>>8));
    Psyz_AdjustPathCB(card_test_path);
    for(unsigned port=0;port<2;++port) {
        char directory[160],host[256];
        snprintf(directory,sizeof(directory),"%s/bu%u0",card_test_root,port);
        if(!SDL_CreateDirectory(directory)) {valid=0;break;}
        snprintf(g_MemCardFilePath,sizeof(g_MemCardFilePath),g_MemCardFilenameFormatPtr,port,'0'+port,'O');
        if(strlen(g_MemCardFilePath)!=25 || strlen(g_MemCardFilePath+5)!=20) valid=0;
        int fd=open(g_MemCardFilePath,0x10200);
        if(fd<0) {valid=0;break;}
        close(fd);
        fd=open(g_MemCardFilePath,2);
        if(fd<0) {valid=0;break;}
        memcpy(g_MemCardFileBuffer,expected,sizeof(expected));
        if(write(fd,(char *)g_MemCardFileBuffer,sizeof(expected))!=sizeof(expected)) valid=0;
        if(close(fd)) valid=0;
        fd=open(g_MemCardFilePath,1);
        if(fd<0) {valid=0;break;}
        if(read(fd,actual,sizeof(actual))!=sizeof(actual) || memcmp(actual,expected,sizeof(actual))) valid=0;
        if(lseek(fd,0x1FF0,0)!=0x1FF0 || read(fd,actual,16)!=16 || memcmp(actual,expected+0x1FF0,16)) valid=0;
        if(read(fd,actual,1)!=0) valid=0;
        close(fd);
        fd=open(g_MemCardFilePath,3);
        if(fd<0) {valid=0;break;}
        u8 marker=0xC7;
        if(lseek(fd,0x123,0)!=0x123 || write(fd,(char *)&marker,1)!=1 ||
           lseek(fd,0x123,0)!=0x123 || read(fd,actual,1)!=1 || actual[0]!=marker) valid=0;
        close(fd);
        char search[8];snprintf(search,sizeof(search),"bu%u0:*",port);
        MemCardDirEntry entry;
        memset(&entry,0xA5,sizeof(entry));
        if(!firstfile(search,&entry) || entry.size!=0x2000 || memcmp(entry.name,g_MemCardFilePath+5,20) || nextfile(&entry)) valid=0;
        Psyz_AdjustPath(host,g_MemCardFilePath,sizeof(host));
        if(!SDL_RemovePath(host) || !SDL_RemovePath(directory))valid=0;
    }
    if(open("bu00:missing-file",1)>=0 || read(-1,actual,1)!=-1 || write(-1,(char *)actual,1)!=-1)valid=0;
    Psyz_AdjustPathCB(NULL);
    if(!SDL_RemovePath(card_test_root)) valid=0;
    memset(g_MemCardFileBuffer,0,sizeof(g_MemCardFileBuffer));
    if(valid)puts("Original card file ABI: both ports, exact 20-byte names, 8 KiB write/reopen/read, seek/update, directory entries and I/O errors passed");
    return valid;
}

void MemCard_InitManager(void);
static int original_system_start(void)
{
    g_MemCardPortStates[0].present=0xA5; g_MemCardPortStates[1].present=0xA5;
    Boot_InitSystem();
    if(!g_MemCardManagerReady || g_MemCardPortStates[0].present || g_MemCardPortStates[1].present ||
       g_PadPackets[0].header!=0x4100 || g_PadPackets[0].buttons!=0xFFFF ||
       g_PadPackets[0].leftX!=128 || g_PadPackets[0].rightY!=128 ||
       g_PadPackets[1].header!=0x00FF) return 0;
    long handles[MEMCARD_EVENT_COUNT]; memcpy(handles,g_MemCardEventHandles,sizeof(handles));
    int *flags[]={&g_MemCardEventF400Spec0004Flag,&g_MemCardRemovedEventPending,
        &g_MemCardRemovedEventPending,&g_MemCardEventF400Spec2000Flag,
        &g_MemCardEventF000Spec0004Flag,&g_MemCardEventF000Spec8000Flag,
        &g_MemCardEventF000Spec8000Flag,&g_MemCardEventF000Spec2000Flag};
    const unsigned specs[]={4,0x8000,0x100,0x2000};
    for(unsigned i=0;i<MEMCARD_EVENT_COUNT;++i) {
        if(handles[i]==-1) return 0;
        for(unsigned j=0;j<i;++j) if(handles[i]==handles[j]) return 0;
        for(unsigned j=0;j<MEMCARD_EVENT_COUNT;++j) *flags[j]=0;
        DeliverEvent(i<4?0xF4000001u:0xF0000011u,specs[i%4]);
        if(*flags[i]!=1) return 0;
    }
    g_MemCardPortStates[0].present=g_MemCardPortStates[1].present=1;
    MemCard_InitManager();
    if(memcmp(handles,g_MemCardEventHandles,sizeof(handles)) ||
       g_MemCardPortStates[0].present || g_MemCardPortStates[1].present) return 0;
    memset(g_MemCardPortStates,0xA5,2*sizeof(MemCardPortState));
    g_MemCardServicedPort=g_MemCardInfoPollCountdown=g_MemCardConnectDebounce=7;
    g_MemCardConnectPending[0]=g_MemCardConnectPending[1]=7;
    g_McOpPending=g_MemCardActivePortOneBased=g_MemCardSavePollTimeout=7;
    g_MemCardActivePromptPending=g_SaveTitleStyleFlag=g_MemCardLoadSucceeded=g_MemCardReadContext=7;
    g_MemCardActiveState=&g_MemCardPortStates[1];
    MemCard_InitState();
    MemCardPortState expected_ports[2];memset(expected_ports,0,sizeof(expected_ports));
    expected_ports[0].fileDescriptor=expected_ports[1].fileDescriptor=-1;
    if(memcmp(g_MemCardPortStates,expected_ports,sizeof(expected_ports)) || g_MemCardServicedPort ||
       g_MemCardInfoPollCountdown || g_MemCardConnectDebounce || g_MemCardConnectPending[0] || g_MemCardConnectPending[1] ||
       g_McOpPending || g_MemCardActivePortOneBased || g_MemCardSavePollTimeout || g_MemCardActivePromptPending ||
       g_SaveTitleStyleFlag || g_MemCardLoadSucceeded || g_MemCardReadContext || g_MemCardActiveState ||
       (u8 *)&g_MemCardSaveStateBuffer!=g_MemCardFileBuffer+0x100 || g_MemCardIconPalette!=(u8 *)g_MemCardIconHeader.palette)
        return 0;
    if(!original_card_file_services(NULL)) return 0;
    puts("Original system start: GPU/GTE/SPU, native pad reset and eight real card-event callbacks passed");
    return 1;
}

void Draw_AllocPrimRectFull(s32 x,s32 y,s32 width,s32 height);
void Draw_AllocPrimRect(void);
static int original_menu_draw_areas(void)
{
    static _Alignas(16) u8 arena[0x4000+32];
    u8 *saved_cursor=g_DrawPacketCursor,*saved_arena=g_DrawPacketArenaBase;
    u32 *saved_ordering=g_DrawOrderingTableEntry;
    int saved_slot=g_DrawBufferIndex,valid=1;
    MenuWidgetNode *saved_head=g_MenuWidgetActiveListHead;
    MenuWidgetNode parent={0},node={0};
    parent.x=17;parent.y=19;parent.children[0]=&node;
    node.x=11;node.y=13;node.draw_state=7;node.grid_width=5;
    node.disabled=9;node.visible_rows=3;
    g_MenuWidgetActiveListHead=&parent;
    for(unsigned slot=0;slot<2;++slot) {
        RenderNativeTag ordering={0xFFFFFF,0x1234};
        memset(arena,0xA5,sizeof(arena));
        g_DrawPacketArenaBase=arena;g_DrawPacketCursor=arena;
        g_DrawOrderingTableEntry=(u32 *)&ordering;g_DrawBufferIndex=slot;
        Draw_AllocPrimRectFull(21,33,89,41);
        Draw_AllocPrimRect();
        Draw_AllocPrimWithMask(&node);
        const unsigned rects[3][4]={{21,33,89,41},{0,0,320,224},{28,32,35,27}};
        for(unsigned i=0;i<3;++i) {
            GpuCmdPacket *packet=(GpuCmdPacket *)(arena+i*sizeof(*packet));
            unsigned x=rects[i][0],y=rects[i][1]+slot*224;
            unsigned lastx=x+rects[i][2]-1,lasty=y+rects[i][3]-1;
            if(packet->length!=2 || packet->field4!=(0xE3000000u|(y<<10)|x) ||
               packet->field8!=(0xE4000000u|(lasty<<10)|lastx) ||
               packet->u0.tag!=(i?(uintptr_t)(arena+(i-1)*sizeof(*packet)):0xFFFFFF)) valid=0;
        }
        uintptr_t head=(uintptr_t)(arena+2*sizeof(GpuCmdPacket));
        if(ordering.address!=head || ordering.length!=0x1234 ||
           g_DrawPacketCursor!=arena+3*sizeof(GpuCmdPacket)) valid=0;
        g_DrawPacketCursor=arena+0x4000-sizeof(GpuCmdPacket);
        Draw_AllocPrimRectFull(1,2,3,4);Draw_AllocPrimRect();Draw_AllocPrimWithMask(&node);
        if(ordering.address!=head || ordering.length!=0x1234 ||
           g_DrawPacketCursor!=arena+0x4000-sizeof(GpuCmdPacket)) valid=0;
        for(size_t i=3*sizeof(GpuCmdPacket);i<sizeof(arena);++i) if(arena[i]!=0xA5) valid=0;
    }
    g_DrawPacketCursor=saved_cursor;g_DrawPacketArenaBase=saved_arena;
    g_DrawOrderingTableEntry=saved_ordering;g_DrawBufferIndex=saved_slot;
    g_MenuWidgetActiveListHead=saved_head;
    if(!valid) fprintf(stderr,"Original menu draw-area payload/native links/arena guard failed\n");
    return valid;
}

void Draw_EmitDigitSprite(int digit);
static int original_digit_packets(void)
{
    /* Payload hashes from USA Draw_SetFontVariant/Draw_EmitDigitSprite
     * 8005F844/8005F874, executed in MIPS, excluding the platform DMA tag. */
    static const uint64_t hashes[2][2]={{UINT64_C(0x4453c0728d2e749d),UINT64_C(0x22072416028e569d)},
        {UINT64_C(0xdc84dc2c95ba4c69),UINT64_C(0x338cd32cbc4986f1)}};
    const int positions[3][2]={{23,45},{-17,-9},{65533,65534}};
    const int digits[]={-1,-10,0,1,9,10,79,INT_MAX};
    static _Alignas(16) u8 arena[0x4000+16];
    u8 *cursor=g_DrawPacketCursor,*base=g_DrawPacketArenaBase;
    u32 *entry=g_DrawOrderingTableEntry;
    int x=g_TextCursorX,y=g_TextCursorY,dim=g_DrawTextDimmed;
    u32 color=g_DrawPrimColor,shaded=g_DrawColorShaded;
    int clut=g_DrawDigitFontTpageClut,u=g_DrawDigitFontBaseTexU,v=g_DrawDigitFontBaseTexV;
    int valid=1;
    for(unsigned font=0;font<2;++font) for(unsigned dark=0;dark<2;++dark) {
        RenderNativeTag ordering={0xFFFFFF,0x9876};
        uintptr_t previous=ordering.address;
        memset(arena,0xA5,sizeof(arena));
        g_DrawPacketCursor=g_DrawPacketArenaBase=arena;g_DrawOrderingTableEntry=(u32 *)&ordering;
        Draw_SetFontVariant(font);Draw_SetColor(0xE1A25C);Draw_SetTextDimmed(dark);
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned pos=0;pos<3;++pos) for(unsigned d=0;d<sizeof(digits)/sizeof(digits[0]);++d) {
            Draw_SetCursor(positions[pos][0],positions[pos][1]);
            RenderTexturedQuad *quad=(RenderTexturedQuad *)g_DrawPacketCursor;
            Draw_EmitDigitSprite(digits[d]);
            if(quad->tag.length!=9 || quad->tag.address!=previous ||
               ordering.address!=(uintptr_t)quad || ordering.length!=0x9876 ||
               g_DrawPacketCursor!=(u8 *)quad+sizeof(*quad) ||
               g_TextCursorX!=positions[pos][0] || g_TextCursorY!=positions[pos][1]) valid=0;
            previous=(uintptr_t)quad;
            const u8 *payload=(const u8 *)quad+sizeof(RenderNativeTag);
            for(unsigned i=0;i<36;++i) hash=(hash^payload[i])*UINT64_C(1099511628211);
        }
        if(hash!=hashes[font][dark]) valid=0;
        for(size_t i=24*sizeof(RenderTexturedQuad);i<sizeof(arena);++i) if(arena[i]!=0xA5) valid=0;
        g_DrawPacketCursor=arena+0x4000-sizeof(RenderTexturedQuad);
        Draw_EmitDigitSprite(7);
        if(ordering.address!=previous || ordering.length!=0x9876 ||
           g_DrawPacketCursor!=arena+0x4000-sizeof(RenderTexturedQuad)) valid=0;
    }
    g_DrawPacketCursor=cursor;g_DrawPacketArenaBase=base;g_DrawOrderingTableEntry=entry;
    g_TextCursorX=x;g_TextCursorY=y;g_DrawTextDimmed=dim;g_DrawPrimColor=color;g_DrawColorShaded=shaded;
    g_DrawDigitFontTpageClut=clut;g_DrawDigitFontBaseTexU=u;g_DrawDigitFontBaseTexV=v;
    if(!valid) fprintf(stderr,"Original digit payload/native links differ from USA MIPS reference\n");
    return valid;
}

static int original_color_quad_packets(void)
{
    static _Alignas(16) u8 arena[0x4000+32];
    u8 *cursor=g_DrawPacketCursor,*base=g_DrawPacketArenaBase;u32 *entry=g_DrawOrderingTableEntry;
    u32 flags=g_GameStateFlags,color=g_DrawPrimColor,shaded=g_DrawColorShaded;int dim=g_DrawTextDimmed;
    const int widths[]={-32768,-1,0,1,7,28,55,56,57,1000,32767};
    const int heights[]={-1,0,1,7,28,56,1000,32767};
    size_t bytes=2*sizeof(RenderGouraudQuad)+sizeof(RenderColorTilePacket)+sizeof(RenderDrawModePacket);
    uint64_t hash=UINT64_C(14695981039346656037);int valid=1;
    for(unsigned flag=0;flag<2;++flag)for(unsigned w=0;w<11;++w)for(unsigned h=0;h<8;++h) {
        memset(arena,0xA5,sizeof(arena));RenderNativeTag ordering={0xFFFFFF,0xAB};
        g_DrawPacketArenaBase=g_DrawPacketCursor=arena;g_DrawOrderingTableEntry=(u32 *)&ordering;
        g_GameStateFlags=flag*2;Draw_AllocColorQuad(widths[w],heights[h]);
        RenderGouraudQuad *first=(void *)arena,*second=(void *)(arena+sizeof(*first));
        RenderColorTilePacket *tile=(void *)(arena+2*sizeof(*first));
        RenderDrawModePacket *mode=(void *)((u8 *)tile+sizeof(*tile));
        if(g_DrawPacketCursor!=arena+bytes || ordering.address!=(uintptr_t)mode || ordering.length!=0xAB ||
           first->tag.address!=0xFFFFFF || first->tag.length!=8 || second->tag.address!=(uintptr_t)first || second->tag.length!=8 ||
           tile->tag.address!=(uintptr_t)second || tile->tag.length!=3 || mode->tag.address!=(uintptr_t)tile || mode->tag.length!=1 ||
           g_DrawPrimColor!=0x808080 || g_DrawColorShaded!=0x404040 || g_DrawTextDimmed)valid=0;
        const u8 *payloads[]={(u8 *)first+sizeof(RenderNativeTag),(u8 *)second+sizeof(RenderNativeTag),
                             (u8 *)tile+sizeof(RenderNativeTag),(u8 *)&mode->command};
        const unsigned sizes[]={32,32,12,4};
        for(unsigned p=0;p<4;++p)for(unsigned i=0;i<sizes[p];++i)hash=(hash^payloads[p][i])*UINT64_C(1099511628211);
        for(unsigned i=bytes;i<sizeof(arena);++i)if(arena[i]!=0xA5)valid=0;
    }
    /* All four GPU payloads from 176 executions of USA 80061044 in MIPS. */
    if(hash!=UINT64_C(0xa60c172cc58948fd)) {
        fprintf(stderr,"Original color quad hash %016llx differs from USA MIPS\n",(unsigned long long)hash);valid=0;
    }
    for(unsigned remaining=0;remaining<=bytes+8;remaining+=8) {
        memset(arena,0xA5,sizeof(arena));RenderNativeTag ordering={0xFFFFFF,0xAB};
        u8 *start=arena+0x4000-remaining;
        g_DrawPacketArenaBase=arena;g_DrawPacketCursor=start;g_DrawOrderingTableEntry=(u32 *)&ordering;
        g_DrawPrimColor=0x123456;g_DrawColorShaded=0x654321;g_DrawTextDimmed=1;
        Draw_AllocColorQuad(28,56);
        if(remaining<=bytes) {
            if(g_DrawPacketCursor!=start || ordering.address!=0xFFFFFF || ordering.length!=0xAB ||
               g_DrawPrimColor!=0x123456 || g_DrawColorShaded!=0x654321 || g_DrawTextDimmed!=1)valid=0;
            for(unsigned i=0;i<sizeof(arena);++i)if(arena[i]!=0xA5)valid=0;
        } else {
            if(g_DrawPacketCursor!=start+bytes)valid=0;
            for(u8 *p=arena;p<start;++p)if(*p!=0xA5)valid=0;
            for(u8 *p=start+bytes;p<arena+sizeof(arena);++p)if(*p!=0xA5)valid=0;
        }
    }
    /* Feed the same native chain to PsyZ and observe VRAM headlessly.
     * Check the complete footprint, background blend and all gauge pixels. */
    static u16 saved_pixels[320*240],pixels[320*240];
    DRAWENV old_draw,draw;RECT area={0,0,320,240};
    GetDrawEnv(&old_draw);StoreImage(&area,saved_pixels);DrawSync(0);
    SetDefDrawEnv(&draw,0,0,320,240);draw.isbg=0;draw.dtd=0;PutDrawEnv(&draw);
    for(unsigned flag=0;flag<2;++flag)for(unsigned fill=0;fill<3;++fill) {
        RenderNativeTag ordering={0xFFFFFF,0};memset(arena,0xA5,sizeof(arena));
        g_DrawPacketArenaBase=g_DrawPacketCursor=arena;g_DrawOrderingTableEntry=(u32 *)&ordering;
        g_GameStateFlags=flag*2;Draw_AllocColorQuad(fill*28,56);
        ClearImage(&area,0,0,0);DrawOTag((u_int *)&ordering);DrawSync(0);
        StoreImage(&area,pixels);DrawSync(0);
        int offset=flag?0:7;
        for(int y=0;y<240;++y)for(int x=0;x<320;++x) {
            int background=x>=221 && x<305 && y>=166+offset && y<177+offset;
            int gauge=x>=229 && x<285 && y>=170+offset && y<173+offset;
            u16 value=pixels[y*320+x];
            if((!background && value) || (background && !gauge && value!=0x0C63) ||
               (gauge && (!value || value==0x0C63))) {
                if(valid)fprintf(stderr,"Original color gauge VRAM (%d,%d)=%04x, background=%d gauge=%d\n",x,y,value,background,gauge);
                valid=0;
            }
        }
    }
    LoadImage(&area,saved_pixels);PutDrawEnv(&old_draw);DrawSync(0);
    g_DrawPacketCursor=cursor;g_DrawPacketArenaBase=base;g_DrawOrderingTableEntry=entry;
    g_GameStateFlags=flags;g_DrawPrimColor=color;g_DrawColorShaded=shaded;g_DrawTextDimmed=dim;
    if(valid)puts("Original color gauge: 176 four-command payloads match USA MIPS, native links, arena limits and six complete headless VRAM footprints passed");
    return valid;
}

static int original_save_card_images(PeDisc *disc)
{
    PeDiscFile archive,exe;
    if(!pe_disc_find(disc,"PE.IMG",&archive) || !pe_disc_find(disc,"SLUS_006.62",&exe)) return 0;
    CdRom_BindImage(disc,&archive);
    if(Asset_OpenPeImage()!=0) return 0;
    MenuWidgetSimpleDescriptor simple[0x41];MenuWidgetGridDescriptor grid[0x41];
    if(!pe_disc_read(disc,&exe,0x92478-0x10000+0x800,simple,sizeof(simple)) ||
       !pe_disc_read(disc,&exe,0x92888-0x10000+0x800,grid,sizeof(grid)) ||
       memcmp(simple,g_MenuWidgetSimpleDescriptors,sizeof(simple)) ||
       memcmp(grid,g_MenuWidgetGridDescriptors,sizeof(grid))) return 0;
    /* Complete USA 80040B80 execution with retail bzero/memcpy/strcpy
     * replaced by their ordinary memory operations in the MIPS reference. */
    const uint64_t hashes[]={UINT64_C(0x936685394f80bede),UINT64_C(0xe5f81aab0f7bf953),UINT64_C(0xb61aaf5cd3ece884),UINT64_C(0x7c2fb8b4765a32aa),UINT64_C(0xcec572cd58919327),UINT64_C(0x35824b0bdf247d70),UINT64_C(0x7dce7284bb2cb877),UINT64_C(0xe89666a7d248ca42),UINT64_C(0x65bcde6175fd7dd),UINT64_C(0x38786e1cec33fc5b),UINT64_C(0x6b73adf727616b9e),UINT64_C(0x306ce1fd0927f959),UINT64_C(0x982926f768839710),UINT64_C(0x73a40725c205f9fa),UINT64_C(0xf21eb1b1eef2a01c),UINT64_C(0x5cd836de2ed5054c),UINT64_C(0x85fc33ce4d3ac0d6),UINT64_C(0x2e35122d654262a8),UINT64_C(0xd0abcde83a45c645),UINT64_C(0x2456ac875900e287),UINT64_C(0xa1789c690daf8765),UINT64_C(0x4125cc56ff810af9),UINT64_C(0xe416e4a8fa0d45b3),UINT64_C(0xba006f1659b2ff21)};
    GameTimerEntry saved_timers[4];memcpy(saved_timers,g_GameTimers,sizeof(saved_timers));
    ItemDataRecord saved_keys[9];memcpy(saved_keys,g_KeyItemRecords,sizeof(saved_keys));
    u32 saved_blend[2];memcpy(saved_blend,g_DrawBlendColorPair,sizeof(saved_blend));
    DISPENV saved_display[2];memcpy(saved_display,g_RenderDispEnvArray,sizeof(saved_display));
    TextboxNameGlyphs saved_name=g_TextboxNameGlyphs;
    MenuWidgetSavedLayout saved_layouts[72];memcpy(saved_layouts,g_MenuWidgetColumnLayoutTable,sizeof(saved_layouts));
    int saved_layout_mode=MenuWidget_GetColumnLayoutMode(),saved_context=g_MenuItemContextFlag;
    int saved_window=g_SaveMetadataWindowIndex,saved_success=g_MemCardLoadSucceeded;
    unsigned saved_submenu=g_MenuActionSubmenuSel;
    SaveRuntimeState saved_runtime=g_SaveRuntimeState;
    Pe1GameState saved_state=g_GameState;
    AyaBattleState saved_aya=g_AyaBattleState;
    BattleStateTail saved_tail=g_SavedBattleStateTail;
    BattleAttributes saved_attributes=g_BattleEquipStateBlock;
    u32 saved_variables[512];memcpy(saved_variables,g_ScriptVariableBank,sizeof(saved_variables));
    int saved_move=D_8009D2E8,saved_token=g_SceneDispatchToken,saved_style=g_SaveTitleStyleFlag;
    u32 saved_flags=g_GameStateFlags;
    u8 saved_fade=g_ScreenTint.fadeMode,*saved_cursor=g_SaveIoCursor;
    SaveSjisFormatArguments saved_arguments=g_SaveSjisFormatArguments;
    u8 title[0x44];memcpy(title,g_SaveTitleBuffer,sizeof(title));
    int valid=&g_PlayTimeFrameCounter==g_ScriptVariableBank+74;
    const unsigned seconds[]={0,3661,359999},play[]={0,400,800};
    unsigned index=0;
    for(unsigned style=0;style<2;++style) for(unsigned links=0;links<4;++links)
    for(unsigned t=0;t<3;++t,++index) {
        for(size_t i=0;i<sizeof(g_SaveRuntimeState);++i)((u8 *)&g_SaveRuntimeState)[i]=(u8)(i*17+3);
        g_SaveRuntimeState.windows.header.playTimeMinutes=seconds[t];
        for(unsigned i=0;i<0x800;++i)((u8 *)g_ScriptVariableBank)[i]=(u8)(i*13+9);
        g_PlayTimeFrameCounter=play[t];
        for(unsigned i=0;i<16;++i)((u8 *)&g_GameState)[i]=(u8)(i*11+0x55);
        g_GameStateFlags=0xA5A55A5A;D_8009D2E8=0x01020309;g_SceneDispatchToken=0xCD4002A8;
        g_ScreenTint.fadeMode=0xD3;
        u8 aya[0x70];for(unsigned i=0;i<sizeof(aya);++i)aya[i]=(u8)(i*7+19);
        memset(&g_AyaBattleState,0,sizeof(g_AyaBattleState));
        memcpy(g_AyaBattleState.words,aya,0x1C);
        memcpy((u8 *)g_AyaBattleState.words+PE1_OFFSETOF(Combatant,maxHP),aya+0x1C,0x4C);
        g_AyaBattleState.action=links&1?&g_SavedBattleStateTail:NULL;
        g_AyaBattleState.attributes=links&2?&g_BattleEquipStateBlock:NULL;
        for(unsigned i=0;i<sizeof(g_SavedBattleStateTail);++i)((u8 *)&g_SavedBattleStateTail)[i]=(u8)(i*5+23);
        for(unsigned i=0;i<sizeof(g_BattleEquipStateBlock);++i)((u8 *)&g_BattleEquipStateBlock)[i]=(u8)(i*3+31);
        AyaBattleState expected_aya=g_AyaBattleState;
        g_SaveTitleStyleFlag=style;
        char path[]="bu00:BASLUS-006620000001O";
        if(Save_BuildCardFile(path)!=0 || g_SaveIoCursor!=g_MemCardFileBuffer+0x1C90)valid=0;
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<sizeof(g_MemCardFileBuffer);++i)hash=(hash^g_MemCardFileBuffer[i])*UINT64_C(1099511628211);
        if(hash!=hashes[index]) {
            fprintf(stderr,"Original save image %u hash %016llx differs from USA MIPS\n",index,(unsigned long long)hash);valid=0;
        }
        /* Independent bitwise CRC, with its four on-disc bytes excluded. */
        u16 crc=0xFFFF;
        for(unsigned i=0;i<sizeof(g_MemCardFileBuffer);++i) {
            unsigned value=i>=0x1C8C && i<0x1C90?0:g_MemCardFileBuffer[i];
            crc^=value<<8;
            for(unsigned b=0;b<8;++b)crc=(u16)((crc&0x8000)?(crc<<1)^0x1021:crc<<1);
        }
        u32 recorded;memcpy(&recorded,g_MemCardFileBuffer+0x1C8C,4);
        if(recorded!=(u32)(crc^0xFFFF))valid=0;
        static u8 tail[0x8A8+16];memset(tail,0xA5,sizeof(tail));
        g_SaveIoCursor=tail+1;
        if(Save_SerializeTail()!=0 || g_SaveIoCursor!=tail+1+0x8A8 ||
           memcmp(tail+1,g_MemCardFileBuffer+0x13E4,0x8A8))valid=0;
        for(unsigned i=1+0x8A8;i<sizeof(tail);++i)if(tail[i]!=0xA5)valid=0;
        if(tail[0]!=0xA5)valid=0;
        u8 expected_fields[15];memcpy(expected_fields,&g_GameState,sizeof(expected_fields));
        BattleStateTail expected_tail=g_SavedBattleStateTail;BattleAttributes expected_attributes=g_BattleEquipStateBlock;
        memset(g_ScriptVariableBank,0xA5,0x800);memset(&g_AyaBattleState,0xA5,sizeof(g_AyaBattleState));
        memset((u8 *)&g_GameState+4,0xA5,11);g_GameStateFlags=0xFFFFFFFF;
        D_8009D2E8=-1;g_SceneDispatchToken=0;g_ScreenTint.fadeMode=0;
        memset(&g_SavedBattleStateTail,0xA5,sizeof(g_SavedBattleStateTail));
        memset(&g_BattleEquipStateBlock,0xA5,sizeof(g_BattleEquipStateBlock));
        g_SaveIoCursor=tail+1;
        if(Save_DeserializeTail()!=0 || g_SaveIoCursor!=tail+1+0x8A8 ||
           memcmp(&g_AyaBattleState,&expected_aya,sizeof(expected_aya)) ||
           memcmp(g_ScriptVariableBank,g_MemCardFileBuffer+0x13E4,0x800) ||
           memcmp(expected_fields,&g_GameState,sizeof(expected_fields)) ||
           memcmp(&g_SavedBattleStateTail,&expected_tail,sizeof(expected_tail)) ||
           memcmp(&g_BattleEquipStateBlock,&expected_attributes,sizeof(expected_attributes)) ||
           (u32)g_SceneDispatchToken!=0xCD4002A8u || g_ScreenTint.fadeMode!=0xD3 ||
           g_GameStateFlags!=(0xA5A55A5Au&0xFFFF2679u) || D_8009D2E8!=0x01020300)valid=0;
        Pe1GameState before_state=g_GameState;AyaBattleState before_aya=g_AyaBattleState;
        u32 before_variables[512];memcpy(before_variables,g_ScriptVariableBank,sizeof(before_variables));
        u32 bad=0x12345678;memcpy(tail+1+0x818+0x68,&bad,4);g_SaveIoCursor=tail+1;
        if(Save_DeserializeTail()!=-1 || g_SaveIoCursor!=tail+1 ||
           memcmp(&g_GameState,&before_state,sizeof(before_state)) || memcmp(&g_AyaBattleState,&before_aya,sizeof(before_aya)) ||
           memcmp(g_ScriptVariableBank,before_variables,sizeof(before_variables)))valid=0;
        g_AyaBattleState.action=(BattleStateTail *)(uintptr_t)1;g_SaveIoCursor=tail+1;
        memset(tail,0xA5,sizeof(tail));
        if(Save_SerializeTail()!=-1 || g_SaveIoCursor!=tail+1)valid=0;
        for(unsigned i=0;i<sizeof(tail);++i)if(tail[i]!=0xA5)valid=0;
    }
    /* Execute the complete original loader, including real modal widgets.
     * Synthetic image payloads deliberately exercise 32-bit timer wrapping. */
    static u8 image[0x2000];memcpy(image,g_MemCardFileBuffer,sizeof(image));
    int queued=g_AkaoMessageQueueCount;
    g_SaveMetadataWindowIndex=0;
    MenuWidget_InitPool();Menu_CreateMemCardProgressWidget(0);
    Save_LoadCardFileIntoRuntime();
    MenuWidgetNode *child=MenuWidget_GetCurrentNode();
    if(!g_MemCardLoadSucceeded || memcmp(&g_SaveRuntimeState,image+0x100,sizeof(g_SaveRuntimeState)) ||
       g_SaveIoCursor!=g_MemCardFileBuffer+0x1C90 || g_MenuDeferredCallback!=Save_CancelUiFlow ||
       MenuWidget_FindByModeAndSelectedBase(1,0x27) || !child || !child->parent ||
       child->parent->selected_base!=0x28 || child->parent->update!=Menu_HandleDeferredCallbackInput ||
       GAME_TIME_COUNTER(2)!=(s32)((u32)g_SaveRuntimeState.windows.header.playTimeMinutes*60u) ||
       GAME_TIME_COUNTER(0)!=(s32)((u32)g_SaveRuntimeState.windows.header.gameTimeMinutes*60u))valid=0;
    for(unsigned i=0;i<6;++i)if(g_KeyItemRecords[i].ammo!=g_SaveRuntimeState.categoryCounts[i])valid=0;
    const unsigned damaged[]={0,4,0x100,0x13E4,0x1C8C,0x1C90,0x1FFF};
    for(unsigned fault=0;fault<8;++fault) {
        memcpy(g_MemCardFileBuffer,image,sizeof(image));
        if(fault<7) g_MemCardFileBuffer[damaged[fault]]^=0x40;
        else {
            u32 bad=0x12345678;memcpy(g_MemCardFileBuffer+0x13E4+0x818+0x68,&bad,4);
            memset(g_MemCardFileBuffer+0x1C8C,0,4);
            u16 crc=0xFFFF;
            for(unsigned i=0;i<sizeof(image);++i) {
                crc^=(u16)g_MemCardFileBuffer[i]<<8;
                for(unsigned b=0;b<8;++b)crc=(u16)((crc&0x8000)?(crc<<1)^0x1021:crc<<1);
            }
            u32 stored=crc^0xFFFFu;memcpy(g_MemCardFileBuffer+0x1C8C,&stored,4);
        }
        SaveRuntimeState before_runtime=g_SaveRuntimeState;Pe1GameState before_game=g_GameState;
        AyaBattleState before_aya=g_AyaBattleState;u32 before_variables[512];
        memcpy(before_variables,g_ScriptVariableBank,sizeof(before_variables));
        u32 before_flags=g_GameStateFlags;int before_lock=D_8009D2E8,before_scene=g_SceneDispatchToken;
        GameTimerEntry timers[4];memcpy(timers,g_GameTimers,sizeof(timers));
        MenuWidget_InitPool();Menu_CreateMemCardProgressWidget(0);g_MemCardLoadSucceeded=1;
        Save_LoadCardFileIntoRuntime();
        child=MenuWidget_GetCurrentNode();
        if(g_MemCardLoadSucceeded || memcmp(&before_runtime,&g_SaveRuntimeState,sizeof(before_runtime)) ||
           memcmp(&before_game,&g_GameState,sizeof(before_game)) || memcmp(&before_aya,&g_AyaBattleState,sizeof(before_aya)) ||
           memcmp(before_variables,g_ScriptVariableBank,sizeof(before_variables)) ||
           memcmp(timers,g_GameTimers,sizeof(timers)) || g_GameStateFlags!=before_flags || D_8009D2E8!=before_lock ||
           g_SceneDispatchToken!=before_scene || g_MenuDeferredCallback || MenuWidget_FindByModeAndSelectedBase(1,0x27) ||
           !child || !child->parent || child->parent->draw!=Menu_DrawTwoLineDialogText)valid=0;
        const u8 *line0=Text_LookupTable4(0x55),*line1=Text_LookupTable4(0x56);
        unsigned len0=0,len1=0;while(line0[len0]!=0xFF && len0<63)++len0;while(line1[len1]!=0xFF && len1<63)++len1;
        if(memcmp(line0,g_MenuTwoLineDialogText[0],len0+1) || memcmp(line1,g_MenuTwoLineDialogText[1],len1+1))valid=0;
        if(child && child->parent) {
            MenuWidgetNode *parent=child->parent;
            if(parent->update(parent,0x10040)!=1 || g_MenuWidgetActiveListHead || MenuWidget_GetCurrentNode())valid=0;
        }
    }
    memcpy(g_MemCardFileBuffer,image,sizeof(image));MenuWidget_InitPool();Menu_CreateMemCardProgressWidget(0);
    Save_LoadCardFileIntoRuntime();
    if(!g_MemCardLoadSucceeded || memcmp(image,g_MemCardFileBuffer,sizeof(image)))valid=0;
    MenuWidget_InitPool();g_MenuDeferredCallback=NULL;g_AkaoMessageQueueCount=queued;
    if(!original_card_file_services(g_MemCardFileBuffer))valid=0;
    /* A valid New Game image can also finish the original success callback.
     * At the title screen no field actor exists yet; scene spawn follows later. */
    FieldActor *saved_player=g_PlayerEntity;g_PlayerEntity=NULL;
    int scale[7];memcpy(scale,g_StatScaleBase,sizeof(scale));
    int saved_bonus=g_InvBonusSlotCount,saved_limit=g_InvSlotLimit;
    s16 *saved_items=g_InvItemPtr,*saved_override=g_InvActiveListOverride;
    int saved_override_limit=g_InvOverrideSlotLimit,saved_bit_words=g_InvSelectionBitWords;
    u32 *saved_bits=g_InvSelectionBits;
    Inv_BuildItemGridFromCategory();Save_InitMetadataState();Save_BuildHeader();
    char path[]="bu00:BASLUS-006620000001O";
    if(Save_BuildCardFile(path)!=0)valid=0;
    SaveRuntimeState initialized=g_SaveRuntimeState;
    memset(&g_SaveRuntimeState,0xA5,sizeof(g_SaveRuntimeState));
    MenuWidget_InitPool();Menu_CreateMemCardProgressWidget(0);Menu_ClearCommandResult();
    Save_LoadCardFileIntoRuntime();child=MenuWidget_GetCurrentNode();
    if(!g_MemCardLoadSucceeded || memcmp(&initialized,&g_SaveRuntimeState,sizeof(initialized)) ||
       !child || !child->parent || g_MenuDeferredCallback!=Save_CancelUiFlow)valid=0;
    else {
        MenuWidgetNode *parent=child->parent;
        if(parent->update(parent,0)!=1 || !g_MenuDeferredCallback || Menu_GetCommandResult()!=0)valid=0;
        if(parent->update(parent,0x10040)!=1 || g_MenuDeferredCallback || Menu_GetCommandResult()!=2 ||
           MenuWidget_GetCurrentNode() || g_MenuWidgetActiveListHead || g_InvItemPtr!=g_AyaInventoryItems ||
           g_AyaSaveState.max_hp!=Aya_LookupLevelStats(0)->hp ||
           g_AyaEquippedWeaponSlot[0]!=0 || g_AyaEquippedArmorSlot[0]!=1)valid=0;
    }
    Menu_ClearCommandResult();g_AkaoMessageQueueCount=queued;
    /* Exercise the newly linked original screen-adjust callbacks through the
     * actual widget input ABI, including both limits and cancel/confirm. */
    for(int base_y=0;base_y<16;++base_y)for(int direction=-1;direction<=1;direction+=2)
    for(unsigned confirm=0;confirm<2;++confirm) {
        Draw_SetBaseOffsetPosition(0,base_y-Draw_GetBaseY());
        MenuWidget_InitPool();Menu_OpenScreenAdjustView(NULL);
        MenuWidgetNode *node=MenuWidget_GetCurrentNode();
        int expected=base_y+direction;if(expected<0)expected=0;if(expected>15)expected=15;
        if(!node || !node->update)valid=0;
        else {
            if(node->update(node,0)!=1 || Draw_GetBaseY()!=base_y)valid=0;
            if(node->update(node,direction<0?0x1000:0x4000)!=1 || Draw_GetBaseY()!=expected)valid=0;
            if(node->update(node,confirm?0x10000:0x40)!=1 ||
               Draw_GetBaseY()!=(confirm?expected:base_y) || MenuWidget_GetCurrentNode() || g_MenuWidgetActiveListHead)valid=0;
        }
    }
    int paused=g_MenuInputPollingPaused,pending=g_MenuInputPausePending;
    for(unsigned flags=0;flags<8;++flags) {
        g_DiscChangeFlags=flags;MenuInput_SetPollingPaused(1);g_MenuInputPausePending=1;
        Menu_DeferredExec();
        if(g_MenuInputPollingPaused!=((flags&3)!=0) || g_MenuInputPausePending!=((flags&3)!=0))valid=0;
    }
    MenuInput_SetPollingPaused(paused);g_MenuInputPausePending=pending;g_AkaoMessageQueueCount=queued;
    g_PlayerEntity=saved_player;memcpy(g_StatScaleBase,scale,sizeof(scale));
    g_InvBonusSlotCount=saved_bonus;g_InvSlotLimit=saved_limit;g_InvItemPtr=saved_items;
    g_InvActiveListOverride=saved_override;g_InvOverrideSlotLimit=saved_override_limit;
    g_InvSelectionBitWords=saved_bit_words;g_InvSelectionBits=saved_bits;
    memcpy(g_GameTimers,saved_timers,sizeof(saved_timers));memcpy(g_KeyItemRecords,saved_keys,sizeof(saved_keys));
    memcpy(g_DrawBlendColorPair,saved_blend,sizeof(saved_blend));memcpy(g_RenderDispEnvArray,saved_display,sizeof(saved_display));
    g_TextboxNameGlyphs=saved_name;g_MenuItemContextFlag=saved_context;g_MenuActionSubmenuSel=saved_submenu;
    MenuWidget_SetColumnLayoutMode(saved_layout_mode);memcpy(g_MenuWidgetColumnLayoutTable,saved_layouts,sizeof(saved_layouts));
    g_SaveMetadataWindowIndex=saved_window;g_MemCardLoadSucceeded=saved_success;
    g_SaveRuntimeState=saved_runtime;g_GameState=saved_state;g_GameStateFlags=saved_flags;g_AyaBattleState=saved_aya;
    g_SavedBattleStateTail=saved_tail;g_BattleEquipStateBlock=saved_attributes;
    memcpy(g_ScriptVariableBank,saved_variables,sizeof(saved_variables));D_8009D2E8=saved_move;
    g_SceneDispatchToken=saved_token;g_SaveTitleStyleFlag=saved_style;g_ScreenTint.fadeMode=saved_fade;
    g_SaveIoCursor=saved_cursor;g_SaveSjisFormatArguments=saved_arguments;memcpy(g_SaveTitleBuffer,title,sizeof(title));
    if(valid)puts("Original save image: 24 full 8 KiB USA MIPS images, SJIS titles/icons, CRC, unaligned tails and full loader with modal dialogs, CRC/link rejection and retry passed");
    return valid;
}

static int original_icon_packets(PeDisc *disc,PeDiscFile *executable)
{
    PeDiscFile archive; u8 range[4];
    if(!pe_disc_find(disc,"PE.IMG",&archive) ||
       !pe_disc_read(disc,executable,0x930D8-0x10000+0x800+4,range,sizeof(range))) return 0;
    unsigned first=range[0]|(unsigned)range[1]<<8,last=range[2]|(unsigned)range[3]<<8;
    if(last<=first || (last-first)*2048>sizeof(g_ResidentDataPack)) return 0;
    ResidentDataPack resident=g_ResidentDataPack;
    if(!pe_disc_read(disc,&archive,(uint64_t)first*2048,&g_ResidentDataPack,(last-first)*2048)) return 0;
    unsigned count=(g_ResidentDataPack.directory.itemBaseData-g_ResidentDataPack.directory.glyphDescriptors)/8;
    const uint64_t hashes[]={UINT64_C(0xd4de41434c6d79f9),UINT64_C(0xbdb11c6f91760571)};
    const int positions[3][2]={{23,45},{-17,-9},{65533,65534}};
    static _Alignas(16) u8 arena[0x4000+16];
    u8 *cursor=g_DrawPacketCursor,*base=g_DrawPacketArenaBase;u32 *entry=g_DrawOrderingTableEntry;
    int x=g_TextCursorX,y=g_TextCursorY,dim=g_DrawTextDimmed;
    u32 color=g_DrawPrimColor,shaded=g_DrawColorShaded;
    if(count!=156) {g_ResidentDataPack=resident;return 0;}
    int valid=1;
    int text_mode=g_TextRenderMode,advance=g_DrawGlyphAdvance;
    for(unsigned dark=0;dark<2;++dark) {
        RenderNativeTag ordering={0xFFFFFF,0x9876};
        g_DrawPacketArenaBase=arena;g_DrawOrderingTableEntry=(u32 *)&ordering;
        Draw_SetColor(0xE1A25C);Draw_SetTextDimmed(dark);
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned pos=0;pos<3;++pos) for(unsigned mode=0;mode<3;++mode)
        for(unsigned glyph=0;glyph<count;++glyph) {
            memset(arena,0xA5,sizeof(RenderTexturedQuad)+16);g_DrawPacketCursor=arena;
            ordering.address=0xFFFFFF;Draw_SetCursor(positions[pos][0],positions[pos][1]);
            Draw_EmitGlyph(glyph,mode);
            RenderTexturedQuad *quad=(RenderTexturedQuad *)arena;
            if(quad->tag.length!=9 || quad->tag.address!=0xFFFFFF ||
               ordering.address!=(uintptr_t)quad || ordering.length!=0x9876 ||
               g_DrawPacketCursor!=arena+sizeof(*quad)) valid=0;
            const u8 *payload=arena+sizeof(RenderNativeTag);
            for(unsigned i=0;i<36;++i) hash=(hash^payload[i])*UINT64_C(1099511628211);
            for(size_t i=sizeof(RenderNativeTag)+36;i<sizeof(*quad)+16;++i) if(arena[i]!=0xA5) valid=0;
        }
        if(hash!=hashes[dark]) valid=0;
        g_DrawPacketCursor=arena+0x4000-sizeof(RenderTexturedQuad);
        uintptr_t head=ordering.address;Draw_EmitGlyph(123,2);
        if(ordering.address!=head || g_DrawPacketCursor!=arena+0x4000-sizeof(RenderTexturedQuad)) valid=0;
    }
    /* USA 8005EB64: all descriptor sprite payloads and draw-page commands. */
    const uint64_t sprite_hashes[]={UINT64_C(0x0215f3979ba18173),UINT64_C(0x54f0e01143881453)};
    for(unsigned dark=0;dark<2;++dark) {
        RenderNativeTag ordering={0xFFFFFF,0x9876};
        g_DrawPacketArenaBase=arena;g_DrawOrderingTableEntry=(u32 *)&ordering;
        Draw_SetColor(0xE1A25C);Draw_SetTextDimmed(dark);
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned pos=0;pos<3;++pos) for(unsigned glyph=0;glyph<count;++glyph) {
            memset(arena,0xA5,sizeof(RenderTexturedQuad)+16);g_DrawPacketCursor=arena;
            ordering.address=0xFFFFFF;Draw_SetCursor(positions[pos][0],positions[pos][1]);
            Draw_AllocSprite(glyph);
            RenderSpritePacket *sprite=(RenderSpritePacket *)arena;
            RenderDrawModePacket *page=(RenderDrawModePacket *)(arena+sizeof(*sprite));
            if(sprite->tag.length!=4 || sprite->tag.address!=0xFFFFFF || page->tag.length!=1 ||
               page->tag.address!=(uintptr_t)sprite || ordering.address!=(uintptr_t)page ||
               ordering.length!=0x9876 || page->command>UINT32_MAX ||
               g_DrawPacketCursor!=(u8 *)page+sizeof(*page)) valid=0;
            for(unsigned i=0;i<16;++i) hash=(hash^arena[sizeof(RenderNativeTag)+i])*UINT64_C(1099511628211);
            for(unsigned byte=0;byte<4;++byte) hash=(hash^((page->command>>(byte*8))&255))*UINT64_C(1099511628211);
            for(size_t i=sizeof(*sprite)+sizeof(*page);i<sizeof(RenderTexturedQuad)+16;++i)
                if(arena[i]!=0xA5) valid=0;
        }
        if(hash!=sprite_hashes[dark]) valid=0;
        size_t pair=sizeof(RenderSpritePacket)+sizeof(RenderDrawModePacket);
        for(unsigned missing=0;missing<=8;missing+=8) {
            ordering.address=0xFFFFFF;
            g_DrawPacketCursor=arena+0x4000-pair+missing;
            u8 *failed_cursor=g_DrawPacketCursor;
            memset(failed_cursor,0xA5,pair-missing+16);
            Draw_AllocSprite(0x77);
            if(g_DrawPacketCursor!=failed_cursor || ordering.address!=0xFFFFFF || ordering.length!=0x9876) valid=0;
            for(size_t i=0;i<pair-missing+16;++i) if(failed_cursor[i]!=0xA5) valid=0;
        }
    }
    /* USA 8005EED4: text quad bytes plus cursor/page/advance state,
     * including all six page-prefix values and control bytes FA..FF.
     * Page 1/code 0 now includes its sprite/page packet pair. */
    const uint64_t text_hashes[]={UINT64_C(0x3ee449b4194055b7),UINT64_C(0xa2b4884fca9feb47)};
    for(unsigned dark=0;dark<2;++dark) {
        RenderNativeTag ordering={0xFFFFFF,0x9876};
        g_DrawPacketArenaBase=arena;g_DrawOrderingTableEntry=(u32 *)&ordering;
        Draw_SetColor(0xE1A25C);Draw_SetTextDimmed(dark);
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned pos=0;pos<3;++pos) for(unsigned page=0;page<6;++page)
        for(unsigned code=0;code<256;++code) {
            memset(arena,0xA5,sizeof(RenderTexturedQuad)+16);g_DrawPacketCursor=arena;
            ordering.address=0xFFFFFF;Draw_SetCursor(positions[pos][0],positions[pos][1]);
            g_TextRenderMode=page;g_DrawGlyphAdvance=99;
            Draw_AllocTexturedQuad(code);
            RenderTexturedQuad *quad=(RenderTexturedQuad *)arena;
            int special=page==1 && code==0;
            if(special) {
                RenderSpritePacket *sprite=(RenderSpritePacket *)arena;
                RenderDrawModePacket *mode=(RenderDrawModePacket *)(arena+sizeof(*sprite));
                if(sprite->tag.length!=4 || sprite->tag.address!=0xFFFFFF || mode->tag.length!=1 ||
                   mode->tag.address!=(uintptr_t)sprite || ordering.address!=(uintptr_t)mode ||
                   g_DrawPacketCursor!=(u8 *)mode+sizeof(*mode)) valid=0;
            } else if(code<250) {
                if(quad->tag.length!=9 || quad->tag.address!=0xFFFFFF ||
                   ordering.address!=(uintptr_t)quad || g_DrawPacketCursor!=arena+sizeof(*quad)) valid=0;
            } else if(ordering.address!=0xFFFFFF || g_DrawPacketCursor!=arena) valid=0;
            if(ordering.length!=0x9876) valid=0;
            const u8 *payload=arena+sizeof(RenderNativeTag);
            for(unsigned i=0;i<(special?16u:36u);++i) hash=(hash^payload[i])*UINT64_C(1099511628211);
            if(special) {
                RenderDrawModePacket *mode=(RenderDrawModePacket *)(arena+sizeof(RenderSpritePacket));
                for(unsigned byte=0;byte<4;++byte) hash=(hash^((mode->command>>(byte*8))&255))*UINT64_C(1099511628211);
            }
            const u32 state[]={(u32)g_TextCursorX,(u32)g_TextCursorY,(u32)g_TextRenderMode,(u32)g_DrawGlyphAdvance};
            for(unsigned i=0;i<4;++i) for(unsigned byte=0;byte<4;++byte)
                hash=(hash^((state[i]>>(byte*8))&255))*UINT64_C(1099511628211);
            size_t used=special?sizeof(RenderSpritePacket)+sizeof(RenderDrawModePacket):sizeof(RenderNativeTag)+36;
            for(size_t i=used;i<sizeof(*quad)+16;++i) if(arena[i]!=0xA5) valid=0;
        }
        if(hash!=text_hashes[dark]) valid=0;
        g_TextRenderMode=0;g_DrawPacketCursor=arena+0x4000-sizeof(RenderTexturedQuad);
        uintptr_t head=ordering.address;Draw_AllocTexturedQuad(7);
        if(ordering.address!=head || g_DrawPacketCursor!=arena+0x4000-sizeof(RenderTexturedQuad)) valid=0;
    }
    /* Whole USA 8006062C execution, including nested cursor stack, number,
     * optional minus sprite, bar slices, colour reset and final draw page. */
    const uint64_t bar_hashes[]={UINT64_C(0xd0110c3a0c5a55a9),UINT64_C(0x6fe60f48763f6639)};
    const int values[]={INT_MIN,-99,-10,-1,0,1,9,10,99,100,999,INT_MAX};
    int *stack=g_TextCursorStackPtr,stack_data[28];
    memcpy(stack_data,g_TextCursorStackBottom,sizeof(stack_data));
    int font_clut=g_DrawDigitFontTpageClut,font_u=g_DrawDigitFontBaseTexU,font_v=g_DrawDigitFontBaseTexV;
    for(unsigned dark=0;dark<2;++dark) {
        RenderNativeTag ordering={0xFFFFFF,0x9876};
        g_DrawPacketArenaBase=arena;g_DrawOrderingTableEntry=(u32 *)&ordering;
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned font=0;font<2;++font) for(unsigned pos=0;pos<3;++pos)
        for(unsigned value=0;value<sizeof(values)/sizeof(values[0]);++value)
        for(int width=-1;width<50;++width) {
            memset(arena,0xA5,1024);g_DrawPacketCursor=arena;ordering.address=0xFFFFFF;
            Draw_SetColor(0xE1A25C);Draw_SetTextDimmed(dark);Draw_SetFontVariant(font);
            Draw_SetCursor(positions[pos][0],positions[pos][1]);
            g_TextCursorStackPtr=g_TextCursorStackBottom;
            Draw_AllocTexturedRectAlt(values[value],width);
            u8 *packet=arena;uintptr_t previous=0xFFFFFF;unsigned packets=0;
            while(packet<g_DrawPacketCursor && packets<16) {
                RenderNativeTag *tag=(RenderNativeTag *)packet;
                size_t bytes=tag->length==9?sizeof(RenderTexturedQuad):
                    tag->length==4?sizeof(RenderSpritePacket):
                    tag->length==1?sizeof(RenderDrawModePacket):0;
                if(!bytes || bytes>(size_t)(g_DrawPacketCursor-packet)) {valid=0;break;}
                if(tag->address!=previous) valid=0;
                hash=(hash^tag->length)*UINT64_C(1099511628211);
                if(tag->length==1) {
                    uintptr_t command=((RenderDrawModePacket *)packet)->command;
                    if(command>UINT32_MAX) valid=0;
                    for(unsigned b=0;b<4;++b) hash=(hash^((command>>(b*8))&255))*UINT64_C(1099511628211);
                } else for(unsigned b=0;b<tag->length*4;++b)
                    hash=(hash^packet[sizeof(RenderNativeTag)+b])*UINT64_C(1099511628211);
                previous=(uintptr_t)packet;packet+=bytes;++packets;
            }
            if(packet!=g_DrawPacketCursor || packets<8 || packets>12 ||
               ordering.address!=previous || ordering.length!=0x9876 ||
               g_TextCursorStackPtr!=g_TextCursorStackBottom) valid=0;
            for(size_t i=(size_t)(packet-arena);i<1024;++i) if(arena[i]!=0xA5) valid=0;
            const u32 state[]={(u32)g_TextCursorX,(u32)g_TextCursorY,
                (u32)((u8 *)g_TextCursorStackPtr-(u8 *)g_TextCursorStackBottom),g_DrawPrimColor,g_DrawColorShaded};
            for(unsigned i=0;i<5;++i) for(unsigned b=0;b<4;++b)
                hash=(hash^((state[i]>>(b*8))&255))*UINT64_C(1099511628211);
        }
        if(hash!=bar_hashes[dark]) {
            fprintf(stderr,"Original bar dim %u hash %016llx differs from USA MIPS\n",dark,(unsigned long long)hash);
            valid=0;
        }
    }
    /* Every aligned amount of free space across the complete bar size:
     * dropped packets must leave a valid chain and restore the cursor stack. */
    for(unsigned remaining=0;remaining<=768;remaining+=8) {
        RenderNativeTag ordering={0xFFFFFF,0x9876};
        memset(arena,0xA5,sizeof(arena));
        g_DrawPacketArenaBase=arena;g_DrawPacketCursor=arena+0x4000-remaining;
        u8 *start=g_DrawPacketCursor;
        g_DrawOrderingTableEntry=(u32 *)&ordering;g_TextCursorStackPtr=g_TextCursorStackBottom;
        Draw_SetCursor(23,45);Draw_SetFontVariant(0);Draw_SetTextDimmed(0);Draw_SetColor(0xE1A25C);
        Draw_AllocTexturedRectAlt(-99,24);
        if(g_TextCursorX!=23 || g_TextCursorY!=45 || g_TextCursorStackPtr!=g_TextCursorStackBottom ||
           g_DrawPacketCursor<start || g_DrawPacketCursor>arena+0x4000 || ordering.length!=0x9876) valid=0;
        uintptr_t previous=0xFFFFFF;u8 *packet=start;
        for(unsigned count=0;packet<g_DrawPacketCursor && count<16;++count) {
            RenderNativeTag *tag=(RenderNativeTag *)packet;
            size_t bytes=tag->length==9?sizeof(RenderTexturedQuad):
                tag->length==4?sizeof(RenderSpritePacket):tag->length==1?sizeof(RenderDrawModePacket):0;
            if(!bytes || bytes>(size_t)(g_DrawPacketCursor-packet)) {valid=0;break;}
            if(tag->address!=previous) valid=0;
            previous=(uintptr_t)packet;packet+=bytes;
        }
        if(packet!=g_DrawPacketCursor || ordering.address!=previous) valid=0;
        for(u8 *p=arena;p<start;++p) if(*p!=0xA5) valid=0;
        for(u8 *p=g_DrawPacketCursor;p<arena+sizeof(arena);++p) if(*p!=0xA5) valid=0;
    }
    g_DrawDigitFontTpageClut=font_clut;g_DrawDigitFontBaseTexU=font_u;g_DrawDigitFontBaseTexV=font_v;
    memcpy(g_TextCursorStackBottom,stack_data,sizeof(stack_data));g_TextCursorStackPtr=stack;
    if(!original_save_card_images(disc))valid=0;
    g_TextRenderMode=text_mode;g_DrawGlyphAdvance=advance;
    g_ResidentDataPack=resident;g_DrawPacketCursor=cursor;g_DrawPacketArenaBase=base;g_DrawOrderingTableEntry=entry;
    g_TextCursorX=x;g_TextCursorY=y;g_DrawTextDimmed=dim;g_DrawPrimColor=color;g_DrawColorShaded=shaded;
    if(valid) puts("Original sprites/icon/text: 156 real descriptors, complete text and 7344 whole-bar cases match USA MIPS; 97 arena limits preserve chains/cursors");
    else fprintf(stderr,"Original icon payload/native links differ from USA MIPS 8005ED18\n");
    return valid;
}

/* Compare actual headless GPU output at every original logo VSync. */
static u16 title_expected_pixels[320*480],title_actual_pixels[320*480];
static const u8 *title_logo_pixels,*title_logo_palette;
static unsigned title_logo_frame,title_logo_failures;
static void title_logo_observe(void)
{
    unsigned frame=title_logo_frame++;
    if(frame>=480) {++title_logo_failures;return;}
    unsigned shade=frame<32?frame*4:frame<392?128:frame<424?(424-frame)*4:0;
    unsigned page=frame&1;
    for(unsigned y=0;y<64;++y) for(unsigned x=0;x<256;++x) {
        unsigned packed=title_logo_pixels[y*128+x/2];
        unsigned index=(packed>>((x&1)*4))&15;
        u16 texel=(u16)title_logo_palette[index*2]|(u16)title_logo_palette[index*2+1]<<8;
        if(!texel) continue;
        u16 color=texel&0x8000;
        for(unsigned c=0;c<3;++c) color|=(((texel>>(c*5))&31)*shade/128)<<(c*5);
        title_expected_pixels[(page*240+y+88)*320+x+32]=color;
    }
    MemcardScreenBuffer *screen=g_MemcardScreenBuffers[!page];
    for(unsigned y=0;y<2;++y) for(unsigned x=0;x<6;++x) {
        const u8 *p=screen->overlayImage+(y*6+x)*2;
        title_expected_pixels[(page*240+y+7)*320+x+6]=(u16)p[0]|(u16)p[1]<<8;
    }
    RECT area={0,0,320,480};
    StoreImage(&area,title_actual_pixels);DrawSync(0);
    if(memcmp(title_actual_pixels,title_expected_pixels,sizeof(title_actual_pixels))) {
        if(!title_logo_failures) for(unsigned i=0;i<320*480;++i)
            if(title_actual_pixels[i]!=title_expected_pixels[i]) {
                fprintf(stderr,"Logo frame %u pixel %u: %04x != %04x\n",frame,i,
                        title_actual_pixels[i],title_expected_pixels[i]);break;
            }
        ++title_logo_failures;
    }
}
static unsigned title_node_index(MemcardImageNode *node)
{
    if(!node) return 0;
    if(node<g_MemcardImageNodePool || node>=g_MemcardImageNodePool+8) return UINT_MAX;
    return (unsigned)(node-g_MemcardImageNodePool)+1;
}
static unsigned title_callback_index(uintptr_t callback)
{
    const uintptr_t functions[]={0,(uintptr_t)MenuCard_UpdateFadeCounter,(uintptr_t)MenuCard_AdvanceFade,
        (uintptr_t)MenuCard_UpdateFadeDisplay,(uintptr_t)MenuCard_DecreaseFade,(uintptr_t)MenuCard_StepFadeToImages,
        (uintptr_t)MenuCard_CreateFadeImageNode,(uintptr_t)MenuCard_AttachFadePayload,(uintptr_t)MenuCard_InitFadeForNode,
        (uintptr_t)MenuCard_BlendImage,(uintptr_t)MenuCard_BlendRotatedImage};
    for(unsigned i=0;i<sizeof(functions)/sizeof(functions[0]);++i) if(functions[i]==callback) return i;
    return UINT_MAX;
}
static int original_title_screen(PeDisc *disc)
{
    PeDiscFile archive;
    if(!pe_disc_find(disc,"PE.IMG",&archive)) return 0;
    u32 flags=g_GameState.flags,base=g_PeImageBaseLba;
    unsigned modes=g_PeImageMountFlags;
    Pe1SceneAudioState audio=g_GameState.scene_audio;
    int queued=g_AkaoMessageQueueCount;
    CdRom_BindImage(disc,&archive);
    if(Asset_OpenPeImage()!=0 || CdRom_LoadInitialImage()!=0) return 0;
    const u8 *logo=g_MemcardImageBundle+g_MemcardImageOffsets[8];
    unsigned palette_bytes=(unsigned)logo[8]|(unsigned)logo[9]<<8;
    if(logo[0]!=16 || logo[4]!=8 || palette_bytes!=44) return 0;
    title_logo_palette=logo+20;title_logo_pixels=logo+8+palette_bytes+12;
    DRAWENV old_draw;GetDrawEnv(&old_draw);
    RECT clear={0,0,320,480};ClearImage(&clear,0,0,0);DrawSync(0);
    memset(title_expected_pixels,0,sizeof(title_expected_pixels));
    for(unsigned p=0;p<2;++p) {
        MemcardScreenBuffer *screen=g_MemcardScreenBuffers[p];
        SetDefDrawEnv(&screen->draw,0,p*240,320,240);screen->draw.isbg=0;
        SetDefDispEnv(&screen->disp,0,!p*240,320,240);screen->disp.isrgb24=1;
        screen->overlay=(RECT){4,7,4,2};screen->dirty=(RECT){0};
        for(unsigned i=0;i<24;++i) screen->overlayImage[i]=(u8)(p*71+i+1);
    }
    g_MemcardScreenPage=0;PutDrawEnv(&g_MemcardScreenBuffers[0]->draw);
    title_logo_frame=title_logo_failures=0;
    VSyncCallback(title_logo_observe);MenuCard_ShowLogoFade();VSyncCallback(Boot_VsyncCallback);
    int valid=title_logo_frame==480 && !title_logo_failures && g_MemcardScreenPage==0 &&
        g_MemcardCurrentScreen==g_MemcardScreenBuffers[0] &&
        g_MemcardScreenBuffers[0]->disp.isrgb24==1 && g_MemcardScreenBuffers[1]->disp.isrgb24==1;
    /* Original two-page background restore plus per-page dirty overlays. */
    for(unsigned p=0;p<2;++p) g_MemcardScreenBuffers[p]->overlay.w=0;
    MenuCard_FlipScreenPages();
    const u8 *background=g_MemcardImageBundle+g_MemcardImageOffsets[0]+sizeof(MemcardImage);
    static u16 restored[480*204];RECT background_area={0,20,480,204};
    for(unsigned p=0;p<2;++p) {
        background_area.y=p*240+20;StoreImage(&background_area,restored);DrawSync(0);
        if(memcmp(restored,background,sizeof(restored))) valid=0;
    }
    if(g_MemcardScreenPage!=0 || g_MemcardCurrentScreen!=g_MemcardScreenBuffers[0]) valid=0;
    /* Full original node/input/fade/render sequence versus USA MIPS, with
     * pointer/function identities normalized to pool slots and callback IDs. */
    const uint64_t node_hashes[]={UINT64_C(0x82bcdaa44ad52f19),UINT64_C(0x44a261b8d9567021),
        UINT64_C(0x6f787be356e1bf1a),UINT64_C(0x6f787be356e1bf1a),
        UINT64_C(0x8473fe8902c4b46e),UINT64_C(0x12d571482a868159)};
    MemCardPortState saved_ports[2];memcpy(saved_ports,g_MemCardPortStates,sizeof(saved_ports));
    void *saved_bgm[2]={g_AkaoBgmHandle[0],g_AkaoBgmHandle[1]};g_AkaoBgmHandle[0]=g_AkaoBgmHandle[1]=NULL;
    u32 saved_pad=g_FieldPadBits;
    for(unsigned scenario=0;scenario<6;++scenario) {
        memset(g_MemCardPortStates,0,sizeof(saved_ports));
        if(scenario && scenario!=5) {
            MemCardPortState *port=&g_MemCardPortStates[scenario==3];
            port->present=1;port->managerState=15;
            if(scenario>=2) {port->slots[14].state=MEMCARD_SLOT_OCCUPIED;port->slots[14].titleStyleFlag=1;}
        }
        if(MemCard_HasAnyPresentFile()!=(scenario>=2 && scenario!=5)) valid=0;
        memset(g_MemcardImageNodePool,0,sizeof(g_MemcardImageNodePool));
        for(unsigned i=0;i<7;++i)g_MemcardImageNodePool[i].next=&g_MemcardImageNodePool[i+1];
        g_MemcardFreeImageNodes=g_MemcardImageNodePool;
        g_MemcardActiveNodeHead=g_MemcardActiveNodeTail=g_MemcardPendingNodeHead=g_MemcardPendingNodeTail=NULL;
        for(unsigned p=0;p<2;++p) {
            g_MemcardScreenBuffers[p]->overlay=(RECT){0};g_MemcardScreenBuffers[p]->dirty=(RECT){0};
            memset(g_MemcardScreenBuffers[p]->overlayImage,0,sizeof(g_MemcardScreenBuffers[p]->overlayImage));
        }
        MenuCard_CreateImageNode(1)->field14=MenuCard_CreateFadeImageNode;
        g_MemcardPrevPad=0;g_MemcardScreenTimer=1;
        uint64_t hash=UINT64_C(14695981039346656037);
        unsigned frames=scenario>=4?240:142;
        for(unsigned frame=0;frame<frames;++frame) {
            if(scenario==4 && frame==190) memset(g_MemCardPortStates,0,sizeof(saved_ports));
            if(scenario==5 && frame==100) {
                MemCardPortState *port=&g_MemCardPortStates[1];
                port->present=1;port->managerState=15;
                port->slots[14].state=MEMCARD_SLOT_OCCUPIED;port->slots[14].titleStyleFlag=1;
            }
            g_FieldPadBits=frame==40?4:frame==110 || frame==130?8:frame==120?0x20:frame==frames-2?0x20000000:0;
            g_MemcardScreenPage=!(frame&1);g_MemcardCurrentScreen=g_MemcardScreenBuffers[g_MemcardScreenPage];
            MenuCard_HandleListInput();MenuCard_RedrawImageNodes();
            MemcardImageNode *node=g_MemcardActiveNodeHead,*previous=NULL;
            while(node) {
                MemcardImageNode *next=node->next;
                if(node->freePending) {
                    if(previous)previous->next=next;else g_MemcardActiveNodeHead=next;
                    if(g_MemcardActiveNodeTail==node)g_MemcardActiveNodeTail=previous;
                    node->next=g_MemcardFreeImageNodes;g_MemcardFreeImageNodes=node;
                } else previous=node;
                node=next;
            }
            if(g_MemcardPendingNodeHead) {
                if(g_MemcardActiveNodeTail)g_MemcardActiveNodeTail->next=g_MemcardPendingNodeHead;
                else g_MemcardActiveNodeHead=g_MemcardPendingNodeHead;
                g_MemcardActiveNodeTail=g_MemcardPendingNodeTail;
                g_MemcardPendingNodeHead=g_MemcardPendingNodeTail=NULL;
            }
            ++g_MemcardScreenTimer;
            for(unsigned i=0;i<8;++i) {
                MemcardImageNode *n=&g_MemcardImageNodePool[i];
                const u32 fields[]={title_node_index(n->next),(u16)n->x|((u32)(u16)n->y<<16),
                    (u16)n->width|((u32)(u16)n->height<<16),title_callback_index((uintptr_t)n->draw),
                    title_callback_index((uintptr_t)n->update),title_callback_index((uintptr_t)n->field14),
                    n->image?(u32)((u8 *)n->image-g_MemcardImageBundle):0,
                    (u32)n->intensity,(u32)n->targetY,(u32)n->stepY,(u32)n->parameter,(u32)n->kind,(u32)n->freePending};
                for(unsigned f=0;f<13;++f) for(unsigned b=0;b<4;++b)
                    hash=(hash^((fields[f]>>(b*8))&255))*UINT64_C(1099511628211);
            }
            const u32 state[]={title_node_index(g_MemcardFreeImageNodes),title_node_index(g_MemcardActiveNodeHead),
                title_node_index(g_MemcardActiveNodeTail),title_node_index(g_MemcardPendingNodeHead),
                title_node_index(g_MemcardPendingNodeTail),(u32)g_MemcardScreenTimer,(u32)g_MemcardPrevPad};
            for(unsigned f=0;f<7;++f) for(unsigned b=0;b<4;++b)
                hash=(hash^((state[f]>>(b*8))&255))*UINT64_C(1099511628211);
            for(unsigned p=0;p<2;++p) {
                const RECT rects[]={g_MemcardScreenBuffers[p]->overlay,g_MemcardScreenBuffers[p]->dirty};
                for(unsigned r=0;r<2;++r) {
                    const u16 values[]={(u16)rects[r].x,(u16)rects[r].y,(u16)rects[r].w,(u16)rects[r].h};
                    for(unsigned f=0;f<4;++f) for(unsigned b=0;b<2;++b)
                        hash=(hash^((values[f]>>(b*8))&255))*UINT64_C(1099511628211);
                }
            }
            RECT *area=&g_MemcardCurrentScreen->overlay;
            if(area->w>0 && area->h>0) {
                size_t bytes=(size_t)area->w*area->h*3;
                if(bytes>sizeof(g_MemcardCurrentScreen->overlayImage)) {valid=0;break;}
                for(size_t b=0;b<bytes;++b)hash=(hash^g_MemcardCurrentScreen->overlayImage[b])*UINT64_C(1099511628211);
            }
        }
        if(hash!=node_hashes[scenario] || g_MemcardScreenTimer!=1003 ||
           !g_MemcardActiveNodeHead || g_MemcardActiveNodeHead->kind!=7 || g_MemcardActiveNodeHead->y!=160) {
            fprintf(stderr,"Original title nodes scenario %u hash %016llx differs from USA MIPS\n",scenario,(unsigned long long)hash);
            valid=0;
        }
    }
    for(unsigned available=0;available<5;++available) {
        memset(g_MemcardImageNodePool,0,sizeof(g_MemcardImageNodePool));
        for(unsigned i=1;i<available;++i)g_MemcardImageNodePool[i-1].next=&g_MemcardImageNodePool[i];
        g_MemcardFreeImageNodes=available?g_MemcardImageNodePool:NULL;
        g_MemcardActiveNodeHead=g_MemcardActiveNodeTail=g_MemcardPendingNodeHead=g_MemcardPendingNodeTail=NULL;
        MemcardImageNode saved_nodes[8];memcpy(saved_nodes,g_MemcardImageNodePool,sizeof(saved_nodes));
        MemcardImageNode *free=g_MemcardFreeImageNodes;
        MenuCard_CreateImageGroup();
        if(MenuCard_CreateImageNode(-1) || MenuCard_CreateImageNode(8)) valid=0;
        if(!available) {MenuCard_CreateFixedImageNode();if(MenuCard_CreateImageNode(1))valid=0;}
        if(g_MemcardFreeImageNodes!=free || g_MemcardPendingNodeHead || g_MemcardPendingNodeTail ||
           memcmp(saved_nodes,g_MemcardImageNodePool,sizeof(saved_nodes))) valid=0;
    }
    memcpy(g_MemCardPortStates,saved_ports,sizeof(saved_ports));g_FieldPadBits=saved_pad;
    g_AkaoBgmHandle[0]=saved_bgm[0];g_AkaoBgmHandle[1]=saved_bgm[1];
    PutDrawEnv(&old_draw);g_AkaoMessageQueueCount=queued;
    CdRom_BindImage(NULL,NULL);g_GameState.flags=flags;g_PeImageBaseLba=base;
    g_PeImageMountFlags=modes;g_GameState.scene_audio=audio;
    if(valid) puts("Original title screen: all 480 logo frames match full framebuffer pixels, both RGB24 backgrounds and 1048 input/node/fade/render frames match USA MIPS headlessly");
    else fprintf(stderr,"Original title screen failed: %u frames, %u pixel mismatches\n",title_logo_frame,title_logo_failures);
    return valid;
}

static int original_subsystem_start(void)
{
    g_GameStateFlags=0xA5A5A5A5; g_FrameRngCounter=123;
    for(unsigned i=0;i<7;++i) g_BattleEnemySlots[i].active=123;
    g_BattleActionQueueCount=17; g_BattleActionIndex=11; g_BattleRemainingActions=3;
    memset(g_ScriptVariableBank,0xA5,0x200*sizeof(u32));
    memset(g_ScriptNodeActiveFlags,0xA5,sizeof(g_ScriptNodeActiveFlags));
    Script_QueueFieldSfx(1,2,3,4,5);
    Boot_InitSubsystems();
    VSyncCallback(NULL);
    if(g_GameStateFlags!=0x4000 || g_FrameRngCounter || g_ActiveDrawSlot ||
       g_BattleActionQueueCount || g_BattleActionIndex || g_BattleRemainingActions ||
       g_FieldSfxQueueCount || g_EntityFreePoolCount || g_FieldActorListHead ||
       g_PlayerEntity || g_EntitySerialCounter) {fprintf(stderr,"subsystem check failed at %d\n",__LINE__);return 0;}
    for(unsigned i=0;i<7;++i) if(g_BattleEnemySlots[i].active) {fprintf(stderr,"subsystem check failed at %d\n",__LINE__);return 0;}
    for(unsigned i=0;i<0x200;++i) if(g_ScriptVariableBank[i]) {fprintf(stderr,"subsystem check failed at %d\n",__LINE__);return 0;}
    /* USA 80034F40..80034F4C repeatedly clears only the first flag;
     * the free-list initializer later clears all 64. Preserve that order. */
    for(unsigned i=0;i<64;++i) if((u32)g_ScriptNodeActiveFlags[i]!=(i?0xA5A5A5A5u:0u))
        {fprintf(stderr,"subsystem flag reset differs at %u\n",i);return 0;}
    if(g_ScriptNodeFreeListHead || g_ScriptNodes[71].next) {fprintf(stderr,"subsystem check failed at %d\n",__LINE__);return 0;}
    uint64_t hash=UINT64_C(14695981039346656037);
    for(unsigned i=0;i<33;++i) for(unsigned byte=0;byte<4;++byte)
        hash=(hash^((g_DrawPrimGeneratorWords[i]>>(byte*8))&255))*UINT64_C(1099511628211);
    if(hash!=UINT64_C(0x2a702d05777cd111)) {fprintf(stderr,"subsystem check failed at %d\n",__LINE__);return 0;}
    puts("Original subsystem start: RNG warm-up, battle reset, script/entity pools and SFX reset passed");
    return 1;
}

/* Actual MIPS init/step execution, including the negative tail and the
 * boot warm-up of 2000 draws. Hashes serialize each word little-endian. */
static int original_frame_random(void)
{
    static const struct { unsigned draws; uint64_t state, stream; s32 head, tail; } cases[]={
        {0,UINT64_C(0x2cb83249daca1179),UINT64_C(0xcbf29ce484222325),64,16},
        {1,UINT64_C(0x982b0feedeac8cfd),UINT64_C(0x8346f5632112ce24),60,12},
        {5,UINT64_C(0x36a1e402d88fc5),UINT64_C(0x6cf2a888882e39ef),44,-4},
        {17,UINT64_C(0xb0944a452a2679dd),UINT64_C(0xeefebd61ca8aed13),64,-52},
        {2000,UINT64_C(0x2a702d05777cd111),UINT64_C(0x9b10472970d605e0),20,-48},
        {8192,UINT64_C(0xe822a05d2e530a78),UINT64_C(0x3d7e90f6c6b75720),4,-48},
    };
    u32 saved[33]; memcpy(saved,g_DrawPrimGeneratorWords,sizeof(saved));
    int valid=1;
    Draw_InitGpuHwRegs();
    uint64_t stream=UINT64_C(14695981039346656037);
    unsigned next=0;
    for(unsigned draws=0;draws<=8192;++draws) {
        if(draws==cases[next].draws) {
            uint64_t state=UINT64_C(14695981039346656037);
            for(unsigned i=0;i<33;++i) for(unsigned byte=0;byte<4;++byte)
                state=(state^((g_DrawPrimGeneratorWords[i]>>(byte*8))&255))*UINT64_C(1099511628211);
            if(state!=cases[next].state || stream!=cases[next].stream ||
               (s32)g_DrawPrimGeneratorWords[14]!=cases[next].head ||
               (s32)g_DrawPrimGeneratorWords[15]!=cases[next].tail) valid=0;
            if(++next==sizeof(cases)/sizeof(cases[0])) break;
        }
        u32 value=Draw_FlushPrimQueue();
        for(unsigned byte=0;byte<4;++byte)
            stream=(stream^((value>>(byte*8))&255))*UINT64_C(1099511628211);
    }
    Draw_InitGpuHwRegs();
    for(unsigned i=0;i<16;++i) {
        static const u32 prefix[]={0x17a,0x264,0x3de,0x642,0xa20,0x9,0x3d,0x441042,
            0x3e0003f,0x4b107e,0xb5c90,0x214eb,0x5989,0x1274,0x3fc00,0x450655};
        if(Draw_FlushPrimQueue()!=prefix[i]) valid=0;
    }
    memcpy(g_DrawPrimGeneratorWords,saved,sizeof(saved));
    if(!valid) fprintf(stderr,"Original frame RNG differs from USA MIPS execution\n");
    return valid;
}

/* Goldens from executing USA EXE 800528F0/80052B2C in MIPS Unicorn.
 * 4096 draws cross seven complete table refresh boundaries. */
static int original_item_random(void)
{
    static const struct { u32 frames; uint64_t table_hash, stream_hash; } cases[] = {
        {UINT32_C(0), UINT64_C(0x72d912fe482091f1), UINT64_C(0x6a8e50bbaca9ed24)},
        {UINT32_C(60), UINT64_C(0x3d536800677af255), UINT64_C(0xa1d70666c8a26a26)},
        {UINT32_C(3600), UINT64_C(0x681d934fc438838), UINT64_C(0xce743e2b365ad07f)},
        {UINT32_C(4294967295), UINT64_C(0x30ad6f89bad09868), UINT64_C(0x32d6b1fd5d134ec)},
    };
    GameTimerEntry saved_timers[4];
    u8 saved_table[521];
    s32 saved_index=g_ItemDropRngIndex;
    int valid=1;
    memcpy(saved_timers,g_GameTimers,sizeof(saved_timers));
    memcpy(saved_table,g_ItemDropRngTable,sizeof(saved_table));
    GameTime_InitCounter0(); GameTime_InitCounter1(); GameTime_InitCounter2();
    if(g_GameClockFlags!=1 || g_GameTimeTable!=0 || g_GameClockLimit!=21600000 ||
       g_GameTimers[1].flags!=1 || g_GameTimers[1].current || g_GameTimers[1].limit ||
       g_GameTimers[2].flags!=1 || g_GameTimers[2].current || g_GameTimers[2].limit) valid=0;
    for(unsigned c=0;c<sizeof(cases)/sizeof(cases[0]);++c) {
        memcpy(&g_GameTimers[0].current,&cases[c].frames,sizeof(u32));
        Inv_SetDefaultItemId();
        uint64_t table=UINT64_C(14695981039346656037);
        uint64_t stream=table;
        for(unsigned i=0;i<521;++i) table=(table^g_ItemDropRngTable[i])*UINT64_C(1099511628211);
        if(g_ItemDropRngIndex!=520 || table!=cases[c].table_hash) valid=0;
        for(unsigned i=0;i<4096;++i) {
            int byte=Inv_ScrambleGrid();
            if(byte<0 || byte>255 || g_ItemDropRngIndex!=(s32)(i%521)) valid=0;
            stream=(stream^(unsigned)byte)*UINT64_C(1099511628211);
        }
        if(stream!=cases[c].stream_hash) valid=0;
    }
    memcpy(g_GameTimers,saved_timers,sizeof(saved_timers));
    memcpy(g_ItemDropRngTable,saved_table,sizeof(saved_table));
    g_ItemDropRngIndex=saved_index;
    if(!valid) fprintf(stderr,"Original item RNG/timer reset differs from USA EXE\n");
    return valid;
}

int main(int argc, char **argv)
{
    if(!room_callback_context() || !room_actor_view() || !room_owner_state() || !callback_list_storage() || !native_task_cycle() || !canonical_render_buffers() || !native_effect_packet_links() || !native_screen_tint_packets() || !native_effect_model_packets() || !native_oriented_sprite_packets() || !native_ring_packets() || !native_effect_collision_coordinates() || !original_glow_resident_constants() || !native_glow_layer_packets() || !native_object_script_cycle() || !authored_glow_script_cycle() || !authored_m005_effect_cycle() || !original_actor_render_opcodes() || !original_actor_camera_opcodes() || !original_absolute_movement_opcode() || !original_resident_script_program() || !original_actor_audio_state() || !original_burst_target_selection() || !native_effect_spawn_arguments() || !native_object_slot_bounds() || !process_task_bank_failures() || !process_texture_bank_failures() || !native_actor_effect_gates()) return 123;
    (void)argv;
    if (!original_startup_fade()) return 126;
    if (!original_save_metadata_storage()) return 125;
    if (!original_frame_random()) return 128;
    if (!original_item_random()) return 124;
    if (!bios_random()) return 58;
    if (!glyph_records()) return 55;
    if (!glyph_selection()) return 56;
    if (!boot_buffer_layout()) return 54;
    if (!game_state_reset()) return 53;
    if(!SDL_Init(SDL_INIT_EVENTS))return 43;
    if(!original_system_start())return 130;
    if(!original_display_start())return 127;
    if(!original_menu_draw_areas())return 131;
    if(!original_digit_packets() || !original_color_quad_packets())return 132;
    if(!original_subsystem_start())return 129;
    const u32 lzcs[]={0,1,0x7fffffff,0x80000000,0xffffffff};
    const u32 lzcr[]={32,31,1,1,32};
    for(unsigned i=0;i<5;++i) {
        Psyz_GteDataWrite(30,lzcs[i]);
        if(Psyz_GteDataRead(30)!=lzcs[i] || Psyz_GteDataRead(31)!=lzcr[i])return 33;
        Psyz_GteDataWrite(31,123);
        if(Psyz_GteDataRead(31)!=lzcr[i])return 34;
    }
    if(argc>1) {
        const char *path=getenv("PE_TEST_DISC");
        if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};PeDiscFile exe;u32 pointers[50];
        if(!pe_disc_open(&disc,path) || !pe_disc_find(&disc,"SLUS_006.62",&exe) ||
           !pe_disc_read(&disc,&exe,0x943c0-0x10000+0x800,pointers,sizeof(pointers)))return 20;
        if (!glyph_asset_records(&disc, &exe)) return 57;
        if (!original_icon_packets(&disc,&exe)) return 133;
        if (!original_title_screen(&disc)) return 134;
        for(unsigned mode=0;mode<50;++mode) {
            SceneFlagRecord *native=g_EntityInputRulesByMode[mode];
            if(!pointers[mode]) {if(native)return 21;continue;}
            if(!native)return 22;
            for(unsigned i=0;i<4;++i) {
                u32 raw[4];
                if(!pe_disc_read(&disc,&exe,pointers[mode]-0x80010000+0x800+i*16,raw,sizeof(raw)))return 23;
                void (*callback)(BattleEntity *,int *)=0;
                switch(raw[3]) {
                case 0:break;
                case 0x80071034:callback=Scene_Init;break;
                case 0x800710a4:callback=Scene_UpdatePlayerEntity;break;
                case 0x8007136c:callback=Scene_UpdateEntityFacing;break;
                case 0x800716a4:callback=Scene_SyncEntityAction;break;
                case 0x80071754:callback=Scene_UpdateEntityFacingFromPad;break;
                default:return 24;
                }
                if(native[i].flags!=raw[0] || native[i].mask_all!=raw[1] ||
                   native[i].mask_any!=raw[2] || native[i].callback!=callback)return 25;
                if(!raw[0])break;
                if(i==3)return 26;
            }
        }
        pe_disc_close(&disc);
        puts("All 50 original input table slots match executable records");
    }
    BattleEntity parent={0},child={0};
    RenderAnimationDataHeader actions[2]={{0}};
    actions[0].frame_count=4;actions[1].frame_count=8;
    for (unsigned i=0;i<2;++i) {
        g_GameState.bank_rows[i][0]=actions;g_GameState.bank_rows[i][1]=actions+1;
    }
    parent.next=&child;child.parent=&parent;child.modelIndex=1;
    g_FieldActorListHead=(FieldActor *)&parent;
    parent.entityFlags=0x100000;child.entityFlags=0x200000;
    Entity_SetActionMode(&parent,1);
    if(parent.animLastFrame!=7 || child.animLastFrame!=7 || child.actionMode!=1 || parent.actionData!=actions+1) return 1;
    parent.animFrame=0x18000;Entity_SetAction(&parent,1);
    if(parent.animFrame!=0x18000) return 2;
    parent.animStep=0x10000;Entity_AdvanceAnim(&child);
    if(parent.animFrame!=0x28000 || child.animFrame!=parent.animFrame || !(parent.entityFlags&0x800000)) return 3;
    parent.entityFlags=0;parent.animFrame=7<<16;Entity_AdvanceAnim(&parent);
    if(parent.animFrame || !(parent.entityFlags&8)) return 4;
    parent.entityFlags=0;parent.animStep=-0x10000;Entity_AdvanceAnim(&parent);
    if(parent.animFrame!=(7<<16) || !(parent.entityFlags&8)) return 5;
    parent.entityFlags=0x200;parent.animFrame=0;parent.animStep=0x30000;parent.animStopFrame=2;
    Entity_AdvanceAnim(&parent);
    if(parent.animFrame!=(2<<16)) return 6;
    parent.entityFlags=0;parent.actionMode=1;parent.animFrame=2<<16;parent.animPrev.fixed=0;parent.animStep=0x10000;
    g_PlayerEntity=(FieldActor *)&parent;
    g_AkaoVoiceBankEntries[0].index=1;g_AkaoVoiceBankEntries[0].frame=1;g_AkaoVoiceBankEntries[0].ids[0]=1000;
    Entity_AdvanceAnim(&parent);
    if(sounds!=1) return 7;
    g_PlayerEntity=0;
    g_GameState.bank_state_11=1;g_GameState.bank_state_12=1;
    g_AkaoVoiceBankEntries[4].group=parent.modelIndex;
    g_AkaoVoiceBankEntries[4].set=parent.teamId;
    g_AkaoVoiceBankEntries[4].index=parent.actionMode;
    g_AkaoVoiceBankEntries[4].frame=1;g_AkaoVoiceBankEntries[4].ids[1]=1000;
    parent.entityFlags=0;parent.animFrame=0;parent.animPrev.fixed=3<<16;parent.animStep=-0x10000;
    Entity_AdvanceAnim(&parent);
    if(sounds!=2) return 8;
    g_GameState.bank_state_11=0;g_GameState.bank_state_12=0;g_FieldActorListHead=0;
    {
        BattleEntity actor={0}; Combatant core={0};
        RenderAnimationDataHeader action={0};action.frame_count=8;
        for(unsigned mode=0;mode<24;++mode) g_GameState.bank_rows[0][mode]=&action;
        actor.core=&core;actor.moveFactor=0xCCC0;actor.moveSpeed=4096;
        g_PlayerEntity=(FieldActor *)&actor;
        g_GeomScroll.state.position.cameraBasis.m[0][0]=4096;
        g_GeomScroll.state.position.cameraBasis.m[1][1]=4096;
        g_GeomScroll.state.position.cameraBasis.m[2][2]=4096;
        const unsigned pads[]={FIELD_PAD_UP,FIELD_PAD_LEFT,FIELD_PAD_DOWN,FIELD_PAD_RIGHT};
        const int dx[]={0,-1,0,1},dz[]={-1,0,1,0};
        for(unsigned run=0;run<2;++run) for(unsigned dir=0;dir<4;++dir) {
            int state=-1, speed=run?FIELD_RUN_SPEED:FIELD_WALK_SPEED;
            g_FieldPadBits=pads[dir];
            if(run)Scene_UpdateEntityFacing(&actor,&state);
            else Scene_UpdatePlayerEntity(&actor,&state);
            if(actor.motionX!=dx[dir]*speed || actor.motionZ!=dz[dir]*speed || actor.motionY ||
               actor.facingAngle!=dir*1024 || actor.actionMode!=(run?23:22) ||
               actor.actionData!=&action) return 9;
        }
        int state=22;g_FieldPadBits=0;
        Scene_UpdatePlayerEntity(&actor,&state);
        if(actor.motionX || actor.motionY || actor.motionZ) return 10;
        g_FieldPadBits=FIELD_PAD_LEFT;g_FieldMoveLock=FIELD_MOVE_CONFUSED;
        Scene_UpdatePlayerEntity(&actor,&state);
        if(actor.motionX!=FIELD_WALK_SPEED || actor.facingAngle!=1024) return 11;
        g_FieldMoveLock=0;
        g_PadPackets[0].header=0x7000;
        const unsigned ax[]={0,128,255,128},ay[]={128,0,128,255};
        const int mx[]={-1,0,1,0},mz[]={0,1,0,-1};
        for(unsigned dir=0;dir<4;++dir) {
            g_PadPackets[0].leftX=ax[dir];g_PadPackets[0].leftY=ay[dir];
            Scene_UpdatePlayerEntity(&actor,&state);
            if(actor.motionX!=mx[dir]*FIELD_WALK_SPEED || actor.motionZ!=mz[dir]*FIELD_WALK_SPEED) return 12;
        }
        Scene_Init(&actor,&state);
        if(actor.actionMode!=21 || actor.motionX || actor.motionZ) return 13;
        g_PadPackets[0].header=0;
        actor.posX.fixed=actor.posY.fixed=actor.posZ.fixed=0;
        Boot_BuildRenderFlagTable();
        Boot_ResetPadInput();
        if(!host_event(SDL_EVENT_KEY_DOWN,SDL_SCANCODE_S))return 44;
        Field_HandleStateTransition();
        if(g_FieldPadBits!=FIELD_PAD_UP || g_FieldPadPressed!=FIELD_PAD_UP){fprintf(stderr,"pad %x pressed %x flags %x\n",g_FieldPadBits,g_FieldPadPressed,g_GameStateFlags);return 30;}
        Entity_IntegratePositionFull((FieldActor *)&actor);
        if(actor.posZ.fixed!=-FIELD_WALK_SPEED || actor.actionMode!=22)return 14;
        if(!host_event(SDL_EVENT_KEY_DOWN,SDL_SCANCODE_LSHIFT))return 45;
        Field_HandleStateTransition();
        if(!(g_FieldPadBits&1) || !(g_FieldPadPressed&1))return 31;
        Entity_IntegratePositionFull((FieldActor *)&actor);
        if(actor.posZ.fixed!=-FIELD_WALK_SPEED-FIELD_RUN_SPEED || actor.actionMode!=23)return 15;
        if(!host_event(SDL_EVENT_KEY_UP,SDL_SCANCODE_S) ||
           !host_event(SDL_EVENT_KEY_UP,SDL_SCANCODE_LSHIFT))return 46;
        Field_HandleStateTransition();
        if(g_FieldPadBits || !(g_FieldPadReleased&FIELD_PAD_UP))return 32;
        Entity_IntegratePositionFull((FieldActor *)&actor);
        if(actor.posZ.fixed!=-FIELD_WALK_SPEED-FIELD_RUN_SPEED || actor.actionMode!=21 || actor.motionZ)return 16;
        g_PadPackets[0].header=0x7000;
        g_PadPackets[0].leftX=89;g_PadPackets[0].leftY=128;
        Field_HandleStateTransition();
        if(g_FieldPadBits!=FIELD_PAD_LEFT)return 35;
        Field_HandleStateTransition();
        if(g_FieldPadPressed || g_FieldPadReleased)return 36;
        g_PadPackets[0].leftX=19;Field_HandleStateTransition();
        if(g_FieldPadBits!=(FIELD_PAD_LEFT|1))return 37;
        MenuWidget_InitPool();
        for (unsigned slot=0; slot<24; ++slot) {
            MenuWidgetNode *allocated=MenuWidget_AllocNode(slot?&g_MenuWidgetNodePool[0]:NULL,
                                                        slot>0 && slot<5?&g_MenuWidgetNodePool[0]:NULL);
            if (allocated!=&g_MenuWidgetNodePool[slot] ||
                allocated->next!=(slot?&g_MenuWidgetNodePool[slot-1]:NULL)) return 86;
            for (unsigned child=0; child<4; ++child)
                if (allocated->children[child]) return 87;
        }
        if (g_MenuWidgetFreeListHead || g_MenuWidgetActiveListHead!=&g_MenuWidgetNodePool[23]) return 88;
        for (unsigned child=0; child<4; ++child)
            if (g_MenuWidgetNodePool[0].children[child]!=&g_MenuWidgetNodePool[child+1]) return 89;
        memset(g_MenuWidgetColumnLayoutTable, 0x5a, sizeof(g_MenuWidgetColumnLayoutTable));
        Menu_InitStateBuffer();
        for (unsigned slot=0;slot<72;++slot) {
            int unsaved=slot==6 || slot==16 || slot==20 || slot==22 ||
                        slot==24 || slot==25 || slot==49 || slot==53;
            MenuWidgetSavedLayout *layout=&g_MenuWidgetColumnLayoutTable[slot];
            if (layout->cursorX!=(unsaved?-1:0) || layout->cursorY ||
                layout->scrollY || layout->reserved) return 98;
        }
        MenuWidgetNode *root=&g_MenuWidgetNodePool[0], *child=&g_MenuWidgetNodePool[1];
        child->mode=2; child->aux_index=6; child->layout_flags=0x20;
        child->cursor_x=1; child->cursor_y=9; child->scroll_y=7;
        g_MenuWidgetCurrentNode=&g_MenuWidgetNodePool[2];
        g_MenuWidgetNodePool[23].mode=2;
        g_MenuWidgetNodePool[23].linkedNext=root;
        g_MenuWidgetNodePool[23].linkedPrevious=child;
        MenuWidget_DestroyNodeRecursive(root);
        if (g_MenuWidgetCurrentNode || g_MenuWidgetNodePool[23].linkedNext ||
            g_MenuWidgetNodePool[23].linkedPrevious) return 91;
        if (g_MenuWidgetColumnLayoutTable[6].cursorX!=1 ||
            g_MenuWidgetColumnLayoutTable[6].cursorY!=9 ||
            g_MenuWidgetColumnLayoutTable[6].scrollY!=7) return 92;
        unsigned remaining=0;
        for (MenuWidgetNode *node=g_MenuWidgetActiveListHead;node;node=node->next) {
            if (node->parent || ++remaining>19) return 93;
        }
        if (remaining!=19) return 94;
        for (unsigned slot=0;slot<5;++slot)
            if (MenuWidget_AllocNode(NULL,NULL)!=&g_MenuWidgetNodePool[4-slot]) return 95;
        while (g_MenuWidgetActiveListHead) MenuWidget_DestroyNodeRecursive(g_MenuWidgetActiveListHead);
        unsigned free_mask=0;
        for (MenuWidgetNode *node=g_MenuWidgetFreeListHead;node;node=node->next) {
            unsigned slot=(unsigned)(node-g_MenuWidgetNodePool);
            if (slot>=24 || (free_mask & (1u<<slot))) return 96;
            free_mask|=1u<<slot;
        }
        if (free_mask!=0xffffff) return 97;
        MenuWidget_InitPool();
        if (g_MenuWidgetFreeListHead!=g_MenuWidgetNodePool || g_MenuWidgetNodePool[23].next) return 90;
        MenuWidgetNode draw_nodes[4]={0};
        draw_nodes[0].children[0]=&draw_nodes[1];
        draw_nodes[0].children[2]=&draw_nodes[3];
        draw_nodes[1].children[1]=&draw_nodes[2];
        for (unsigned i=0;i<4;++i) {
            draw_nodes[i].mode=i;
            draw_nodes[i].x=(int)i+1; draw_nodes[i].y=-((int)i+1);
            draw_nodes[i].draw=record_widget_draw;
        }
        g_TextCursorX=10; g_TextCursorY=20;
        g_TextCursorStackPtr=g_TextCursorStackBottom;
        for(unsigned i=0;i<12;++i)g_TextCursorStackTop[i]=0x12345678;
        Draw_PushPrimToList(draw_nodes);
        const int want_x[4]={11,13,16,15},want_y[4]={19,17,14,15};
        if(widget_draw_count!=4 || g_TextCursorX!=10 || g_TextCursorY!=20 ||
           g_TextCursorStackPtr!=g_TextCursorStackBottom)return 99;
        for(unsigned i=0;i<4;++i)
            if(widget_draw_order[i]!=(int)i || widget_draw_x[i]!=want_x[i] || widget_draw_y[i]!=want_y[i])return 100;
        for(unsigned i=0;i<12;++i)if(g_TextCursorStackTop[i]!=0x12345678)return 101;
        MenuInput_InitQueue();
        for(int i=0;i<20;++i)Queue_Enqueue(i&1?2:1,100+i);
        if(g_MenuEventQueueFreeList)return 102;
        Queue_Enqueue(1,999);
        MenuQueueEntry event;
        Queue_DequeueByMask(2,&event);
        if(event.payload.input.flags!=101)return 103;
        Queue_Enqueue(4,123);
        for(int i=0;i<20;++i) {
            Queue_DequeueByMask(-1,&event);
            int want=i==19?123:100+i+(i>0);
            if(event.payload.input.flags!=want)return 104;
        }
        if(g_MenuEventQueueHead || g_MenuEventQueueTail)return 105;
        MenuWidgetNode event_parent={0},event_child={0};
        event_parent.mode=2;event_parent.update=record_menu_event;
        event_child.mode=1;event_child.update=record_menu_event;event_child.parent=&event_parent;
        g_MenuWidgetCurrentNode=&event_child;g_MenuInputPollingPaused=1;
        Queue_Enqueue(1,0x40);MenuInput_DispatchQueuedEvents();
        Queue_Enqueue(2,0x1000);MenuInput_DispatchQueuedEvents();
        Queue_Enqueue(4,0x20);MenuInput_DispatchQueuedEvents();
        if(menu_event_count!=6)return 106;
        for(unsigned i=0;i<6;++i)
            if(menu_event_nodes[i]!=(i%2?2:1) || menu_event_flags[i]!=(i<2?0x40:i<4?0x21000:0x10000))return 107;
        g_MenuWidgetCurrentNode=NULL;g_MenuInputPollingPaused=0;
        MenuInput_DispatchQueuedEvents();
        MenuWidgetNode menu={0};menu.mode=1;g_MenuWidgetActiveListHead=&menu;
        MenuWidgetListNavigation navigation={0};
        navigation.list=&menu;
        menu.visible_rows=4; menu.y_limit=16; menu.disabled=8; menu.scroll_y=4;
        MenuWidget_EaseNodePosition(&navigation);
        if (navigation.draw_state!=8 || navigation.visible_rows!=8 ||
            navigation.list!=&menu || navigation.mode!=0) return 85;
        Field_HandleStateTransition();
        if(g_FieldPadBits!=FIELD_PAD_LEFT)return 38;
        g_PadPackets[0].leftX=20;Field_HandleStateTransition();
        if(g_FieldPadBits)return 39;
        g_MenuWidgetActiveListHead=0;
        g_PadPackets[0].header=0x00ff;Field_HandleStateTransition();
        if(g_FieldPadPressed!=4 || g_FieldPadBits!=4 || !(g_GameStateFlags&0x4000))return 40;
        Field_HandleStateTransition();
        if(g_FieldPadBits || g_FieldPadPressed)return 41;
        g_PadPackets[0].header=0x4000;Field_HandleStateTransition();
        if(g_FieldPadBits || (g_GameStateFlags&0x4000))return 42;
        Boot_ResetPadInput();
        if(!host_event(SDL_EVENT_KEY_DOWN,SDL_SCANCODE_W) ||
           !host_event(SDL_EVENT_KEY_DOWN,SDL_SCANCODE_UP) ||
           !host_event(SDL_EVENT_KEY_UP,SDL_SCANCODE_W))return 47;
        if(g_PadPackets[0].buttons&0x10)return 48;
        if(!host_event(SDL_EVENT_WINDOW_FOCUS_LOST,SDL_SCANCODE_UNKNOWN))return 49;
        Field_HandleStateTransition();
        if(g_FieldPadBits || g_PadPackets[0].buttons!=0xffff)return 50;
        if(!host_event(SDL_EVENT_KEY_DOWN,SDL_SCANCODE_W) ||
           !host_event(SDL_EVENT_WINDOW_FOCUS_GAINED,SDL_SCANCODE_UNKNOWN))return 51;
        Field_HandleStateTransition();
        if(g_FieldPadBits)return 52;
        /* SDL events -> original pad mapper -> original menu repeat/dispatch. */
        Boot_ResetPadInput(); Boot_BuildRenderFlagTable(); MenuInput_InitQueue();
        menu_event_count=0;
        event_parent.flags=0; g_MenuWidgetCurrentNode=&event_parent;
        Field_HandleStateTransition(); MenuInput_DispatchQueuedEvents();
        if(menu_event_count || !g_MenuInputActive)return 108;
        if(!host_event(SDL_EVENT_KEY_DOWN,SDL_SCANCODE_UP))return 109;
        Field_HandleStateTransition(); MenuInput_DispatchQueuedEvents();
        if(menu_event_count!=1 || menu_event_flags[0]!=0x1000)return 110;
        for(unsigned frame=1;frame<=9;++frame) {
            Field_HandleStateTransition(); MenuInput_DispatchQueuedEvents();
            if(menu_event_count!=(frame<9?1u:2u))return 111;
        }
        if(menu_event_flags[1]!=0x21000)return 112;
        if(!host_event(SDL_EVENT_KEY_UP,SDL_SCANCODE_UP))return 113;
        Field_HandleStateTransition(); MenuInput_DispatchQueuedEvents();
        if(menu_event_count!=2)return 114;
        if(!host_event(SDL_EVENT_KEY_DOWN,SDL_SCANCODE_X))return 115;
        Field_HandleStateTransition(); MenuInput_DispatchQueuedEvents();
        if(menu_event_count!=3 || menu_event_flags[2]!=0x20) { fprintf(stderr,"X press: count=%u flags=%x pad=%x\n",menu_event_count,menu_event_flags[2],g_FieldPadBits); return 116; }
        if(!host_event(SDL_EVENT_KEY_UP,SDL_SCANCODE_X))return 117;
        Field_HandleStateTransition(); MenuInput_DispatchQueuedEvents();
        if(menu_event_count!=4 || menu_event_flags[3]!=0x10000)return 118;
        if(!host_event(SDL_EVENT_KEY_DOWN,SDL_SCANCODE_V))return 119;
        Field_HandleStateTransition(); MenuInput_DispatchQueuedEvents();
        if(menu_event_count!=5 || menu_event_flags[4]!=0x80)return 120;
        if(!host_event(SDL_EVENT_WINDOW_FOCUS_LOST,SDL_SCANCODE_UNKNOWN))return 121;
        Field_HandleStateTransition(); MenuInput_DispatchQueuedEvents();
        if(menu_event_count!=5 || g_MenuInputHeldStatusMask || g_FieldPadBits)return 122;
        g_MenuWidgetCurrentNode=NULL;
        g_PlayerEntity=0;
    }
    puts("Original action/clock/audio trigger and digital/analog player motion passed");
    SDL_Quit();
    return 0;
}
