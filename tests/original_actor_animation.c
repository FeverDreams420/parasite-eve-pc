#include "../src/overlays/menu_memcard/menu_memcard_video.h"
#include "pe1/resident_tables.h"
#include "pe1/draw_area.h"
#include "pe1/textbox.h"
#include "pe1/textbox_open.h"
#include "pe1/menu_flags.h"
#include "pe1/draw_wipe_rect.h"
#include "pe1/memcard.h"
void DeliverEvent(unsigned event,unsigned spec);
#include "pe1/battle_runtime.h"
#include "pe1/battle_entity_anim.h"
#include "pe1/battle_level_up.h"
#include "pe1/battle_update.h"
#include "pe1/battle_status.h"
#include "pe1/battle_main.h"
#include "pe1/field_sfx.h"
#include "pe1/psyq_callbacks.h"
#include "pe1/boot.h"
#include "pe1/boot_disc_check.h"
#include "pe1/cdrom_buffers.h"
#include "pe1/draw_buffers.h"
#include "pe1/save.h"
#include "pe1/game_timers.h"
#include "pe1/menu_background.h"
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
#include "pe1/render_anim_position.h"
#include "pe1/render_animation_frame.h"
#include "pe1/field_glow_line.h"
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
#include "pe1/akao/globals.h"
#include "pe1/akao/init_state.h"
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
#include <psyz/spu.h>

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
        size_t glyph_bytes=TEXTBOX_GLYPH_PACKET_CAPACITY*sizeof(TextboxGlyphPacket);
        if((u8 *)D_800B0E38.other_buffers[2]!=(u8 *)D_800B0E38.other_buffers[1]+glyph_bytes ||
           g_RenderScratchBufferBase[0]!=(u8 *)D_800B0E38.other_buffers[2]+glyph_bytes ||
           g_RenderAnimBufferBase[1]!=g_RenderAnimBufferBase[0]+RENDER_ANIM_BUFFER_BYTES ||
           (u8 *)g_GameState.scene_process_slots!=g_RenderAnimBufferBase[1]+RENDER_ANIM_BUFFER_BYTES)return 0;
        RenderBufferPrefix display_layout=D_800B0E38;
        Boot_InitMemoryLayout();
        if(memcmp(&display_layout,&D_800B0E38,sizeof(display_layout)))return 0;
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
    g_PlayerEntity=(FieldActor *)(&player);
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
    g_PlayerEntity=(FieldActor *)(saved);
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
    g_PlayerEntity=(FieldActor *)(&player);
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
    g_PlayerEntity=(FieldActor *)(saved_player); D_800B0E38=saved;
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


/* USA 8003495C/80034DE0, including the real wipe callees: 18,560 cases.
 * Compare the two numbers referenced by the HP glyph stream. Retail reads
 * past its local terminator into unused number slots; those are excluded. */

/* Execute the original parser and real SDK packet helpers, comparing state
 * and every linked command against unhooked USA 371B0/37870. */

/* CPU algorithm from USA 8007041C with COP2 forwarded to the same PsyZ GTE;
 * this isolates CPU/layout fidelity, not independent hardware GTE accuracy. */


static int original_battle_target_list(void)
{
    BattleTarget saved_targets[BATTLE_TARGET_CAPACITY+1];memcpy(saved_targets,g_BattleTargetList,sizeof(saved_targets));
    BattleSavedPosition saved_position=g_AyaSavedPos;
    s8 saved_index=g_BattleTargetIndex;
    FieldActor *saved_head=g_FieldActorListHead,*saved_player=g_PlayerEntity;
    BattleEntity player={0},entities[9]={{0}};EnemyCombatant cores[9]={{0}};
    const u32 flag_patterns[]={0,0x40,0x2040,0x4000,0x4040,0x2000,0x80,UINT32_MAX};
    const unsigned missing_masks[]={0,0x55,0x7F};
    const s16 players[][2]={{0,0},{-32768,32767},{100,-100}};
    const s16 points[][7][2]={
        {{100,100},{100,100},{100,100},{100,100},{100,100},{100,100},{100,100}},
        {{700,50},{600,-50},{500,50},{400,-50},{300,50},{200,-50},{100,50}},
        {{-32768,32767},{32767,-32768},{-32768,-32768},{32767,32767},{0,32767},{-32768,0},{0,0}},
        {{100,0},{0,100},{-100,0},{0,-100},{70,70},{-70,70},{70,-70}}};
    g_PlayerEntity=(FieldActor *)&player;
    uint64_t hash=UINT64_C(14695981039346656037);int valid=1;unsigned cases=0,histogram[8]={0};
#define TARGET_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned mask=0;mask<128;++mask)for(unsigned pattern=0;pattern<9;++pattern)
    for(unsigned missing=0;missing<3;++missing)for(unsigned layout=0;layout<4;++layout)
    for(unsigned origin=0;origin<3;++origin)for(unsigned insert=0;insert<2;++insert){
        memset(g_BattleTargetList,0xA5,sizeof(g_BattleTargetList));
        g_AyaSavedPos=(BattleSavedPosition){0x11223344,0x55667788,(s32)UINT32_C(0x99AABBCC)};
        player.posX.parts.integer=players[origin][0];player.posZ.parts.integer=players[origin][1];
        BattleEntity *sequence[8];unsigned candidate=0;
        for(unsigned i=0;i<8;++i)sequence[i]=i==(insert?3:0)?&player:&entities[candidate++];
        for(unsigned i=0;i<8;++i)sequence[i]->next=i<7?sequence[i+1]:NULL;
        g_FieldActorListHead=(FieldActor *)sequence[0];
        for(unsigned i=0;i<7;++i){
            entities[i].core=missing_masks[missing]>>i&1?NULL:&cores[i];
            cores[i].hpAlive=mask>>i&1?(int)i*10+1:(i&1?-1:0);
            entities[i].entityFlags=pattern==8?0:flag_patterns[(i+pattern)%8];
            entities[i].renderObject.target_x=points[layout][i][0];entities[i].renderObject.target_z=points[layout][i][1];
        }
        int returned=Battle_BuildTargetList();unsigned n=(u8)g_BattleTargetIndex;
        if(returned!=(int)n)valid=0;
        if(n>7){valid=0;goto targets_restore;}++histogram[n];TARGET_PUT(n,1);
        for(unsigned i=0;i<n;++i){
            unsigned id=0;for(unsigned j=0;j<7;++j)if(g_BattleTargetList[i].actor==&entities[j])id=j+1;
            if(!id)valid=0;TARGET_PUT(id,1);TARGET_PUT(g_BattleTargetList[i].dist,4);TARGET_PUT(g_BattleTargetList[i].angle,2);
            TARGET_PUT(g_BattleTargetList[i].pad_0A[0],1);TARGET_PUT(g_BattleTargetList[i].pad_0A[1],1);
        }
        TARGET_PUT(g_BattleTargetList[n].actor==NULL,1);
        TARGET_PUT(g_AyaSavedPos.x,4);TARGET_PUT(g_AyaSavedPos.y,4);TARGET_PUT(g_AyaSavedPos.z,4);++cases;
    }
    /* Invalid population cannot overwrite the saved pose or the sentinel. */
    for(unsigned i=0;i<9;++i){entities[i].core=&cores[i];cores[i].hpAlive=1;entities[i].entityFlags=0;entities[i].next=i<8?&entities[i+1]:NULL;}
    g_FieldActorListHead=(FieldActor *)entities;
    g_AyaSavedPos=(BattleSavedPosition){1,2,3};Battle_BuildTargetList();
    if(g_BattleTargetIndex!=7 || g_BattleTargetList[7].actor || g_AyaSavedPos.x || g_AyaSavedPos.y!=2 || g_AyaSavedPos.z!=3)valid=0;
    g_PlayerEntity=NULL;Battle_BuildTargetList();if(g_BattleTargetIndex || g_BattleTargetList[0].actor)valid=0;
targets_restore:
#undef TARGET_PUT
    memcpy(g_BattleTargetList,saved_targets,sizeof(saved_targets));g_AyaSavedPos=saved_position;
    g_BattleTargetIndex=saved_index;g_FieldActorListHead=saved_head;g_PlayerEntity=saved_player;
    if(!valid || hash!=UINT64_C(0xcd7e48e9d9318f85)){
        fprintf(stderr,"Original battle targets: cases=%u hash=%016llx valid=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    puts("Original battle targets: 82944 unhooked USA cases, filtering, null/dead cores, distances, angles, ordering/ties, full seven-target sentinel and saved-X alias match");return 1;
}

static int original_script_sprites(void)
{
    Pe1GameState saved_state=g_GameState;GeomScrollRecord saved_scroll=g_GeomScroll;
    int saved_offset=g_DrawPacketOffset,saved_slot=g_ActiveDrawSlot;
    FieldScriptSprite *saved_list=g_FieldFxScriptSpriteList;
    s16 saved_count=g_FieldFxScriptSpriteCount;FieldScriptSprite saved_records[22]={0};
    if(saved_list)memcpy(saved_records,saved_list-21,sizeof(saved_records));
    FieldFx_ClearScriptSprites();FieldScriptSprite *list=g_FieldFxScriptSpriteList;
    GteMatrix view={0};view.m[0][0]=view.m[1][1]=view.m[2][2]=4096;
    int projection=320;g_ViewMatrixSlot.value=&view;g_GeomScroll.state.position.projectionDistance=&projection;
    union {uintptr_t alignment;u8 bytes[0x10000];} arena;
    RenderNativeTag ordering[4096];
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;int valid=1;
#define SPRITE_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
#define SPRITE_STATE() do {SPRITE_PUT(g_FieldFxScriptSpriteCount,2); \
    for(unsigned i_=0;i_<22;++i_){const u8 *p_=(const u8 *)&list[-(int)i_]; \
        for(unsigned j_=0;j_<20;++j_)SPRITE_PUT(p_[j_],1);}}while(0)
#define SPRITE_SETUP() do { \
    for(unsigned i_=0;i_<22;++i_){memset(&list[-(int)i_],0xA5,20);list[-(int)i_].active=0;} \
    g_FieldFxScriptSpriteCount=0;FieldFx_ClearScriptSprites(); \
    for(unsigned i_=0;i_<32;++i_){Psyz_GteDataWrite(i_,0);Psyz_GteCtrlWrite(i_,0);} \
    Psyz_GteCtrlWrite(24,160<<16);Psyz_GteCtrlWrite(25,112<<16);Psyz_GteCtrlWrite(26,320); \
}while(0)
#define SPRITE_STEP(slot) do { \
    memset(arena.bytes,0xA5,sizeof(arena.bytes)); \
    for(unsigned i_=0;i_<4096;++i_)ordering[i_]=(RenderNativeTag){0xFFFFFF,0xAB}; \
    g_DrawPacketOffset=0;g_ActiveDrawSlot=(slot); \
    g_RenderAnimBufferBase[0]=g_RenderAnimBufferBase[1]=arena.bytes; \
    D_800B0E38.ordering[0]=D_800B0E38.ordering[1]=ordering; \
    FieldFx_UpdateScriptSprites();SPRITE_STATE(); \
    if(g_DrawPacketOffset<0 || (size_t)g_DrawPacketOffset>sizeof(arena.bytes) || g_DrawPacketOffset%sizeof(RenderTexturedQuad)){valid=0;goto sprites_restore;} \
    unsigned count_=(unsigned)g_DrawPacketOffset/sizeof(RenderTexturedQuad);SPRITE_PUT(count_,2); \
    for(unsigned i_=0;i_<count_;++i_){RenderTexturedQuad *p_=(RenderTexturedQuad *)arena.bytes+i_; \
        if(p_->tag.length!=9)valid=0;const u8 *data_=(const u8 *)p_+sizeof(RenderNativeTag); \
        for(unsigned j_=0;j_<36;++j_)SPRITE_PUT(data_[j_],1);} \
    for(unsigned layer_=0;layer_<4096;++layer_){uintptr_t link_=ordering[layer_].address;unsigned links_=0; \
        if(ordering[layer_].length!=0xAB)valid=0;if(link_==0xFFFFFF)continue;SPRITE_PUT(layer_,2); \
        while(link_!=0xFFFFFF){ \
            if(link_<(uintptr_t)arena.bytes || link_>=(uintptr_t)(arena.bytes+g_DrawPacketOffset) || \
               (link_-(uintptr_t)arena.bytes)%sizeof(RenderTexturedQuad) || ++links_>22){valid=0;goto sprites_restore;} \
            SPRITE_PUT((link_-(uintptr_t)arena.bytes)/sizeof(RenderTexturedQuad),1);link_=((RenderNativeTag *)link_)->address;} \
        SPRITE_PUT(links_,1);}SPRITE_PUT(65535,2); \
    for(unsigned i_=g_DrawPacketOffset;i_<sizeof(arena.bytes);++i_)if(arena.bytes[i_]!=0xA5)valid=0; \
}while(0)
    const u32 masks[]={0,1,0x555555,0xAAAAAA,0xFFFFF,0x3FFFFF,1<<21};
    for(unsigned m=0;m<7;++m){
        SPRITE_SETUP();int count=0;
        for(unsigned i=0;i<22;++i){list[-(int)i].active=(masks[m]>>i)&1;count+=list[-(int)i].active;}
        g_FieldFxScriptSpriteCount=count>20?20:count;FieldFx_ClearScriptSprites();SPRITE_STATE();++cases;
    }
    const int delays[]={0,1,3,17,255},sizes[]={-32768,-1,0,1,32,32767},spawn_counts[]={1,4,20,21,22};
    for(unsigned mode=0;mode<3;++mode)for(unsigned delay=0;delay<5;++delay)for(unsigned size=0;size<6;++size)
    for(unsigned slot=0;slot<2;++slot)for(unsigned spawn=0;spawn<5;++spawn){
        SPRITE_SETUP();
        for(int i=0;i<spawn_counts[spawn];++i){s16 point[]={i*25-100,-i*7,1000+i*8};
            FieldFx_SpawnScriptSprite(point,mode,sizes[size],delays[delay],i*17+31,i*11+64,511-i*9);SPRITE_STATE();}
        for(unsigned tick=0;tick<8;++tick)SPRITE_STEP(slot);
        FieldFx_ClearScriptSprites();SPRITE_STATE();++cases;
    }
    const unsigned long_runs[][3]={{0,255,1030},{1,1,520},{1,3,200}};
    for(unsigned i=0;i<3;++i){
        SPRITE_SETUP();s16 point[]={-25,-100,1000};
        FieldFx_SpawnScriptSprite(point,long_runs[i][0],32,long_runs[i][1],200,128,64);SPRITE_STATE();
        for(unsigned tick=0;tick<long_runs[i][2];++tick)SPRITE_STEP(tick&1);
        ++cases;
    }

    /* Native arena exhaustion and projection guards do not alter pool timing. */
    for(unsigned remaining=0;remaining<=168;remaining+=8){
        SPRITE_SETUP();s16 point[]={0,0,1000};
        for(unsigned i=0;i<20;++i)FieldFx_SpawnScriptSprite(point,0,32,255,0,0,0);
        memset(arena.bytes,0xA5,sizeof(arena.bytes));
        for(unsigned i=0;i<4096;++i)ordering[i]=(RenderNativeTag){0xFFFFFF,0xAB};
        g_ActiveDrawSlot=0;g_RenderAnimBufferBase[0]=arena.bytes;D_800B0E38.ordering[0]=ordering;
        unsigned start=RENDER_ANIM_BUFFER_BYTES-remaining;g_DrawPacketOffset=start;
        FieldFx_UpdateScriptSprites();
        unsigned emitted=remaining/sizeof(RenderTexturedQuad);
        if(g_DrawPacketOffset!=(int)(start+emitted*sizeof(RenderTexturedQuad)) || g_FieldFxScriptSpriteCount!=20)valid=0;
        for(unsigned i=0;i<start;++i)if(arena.bytes[i]!=0xA5)valid=0;
        for(unsigned i=RENDER_ANIM_BUFFER_BYTES;i<sizeof(arena.bytes);++i)if(arena.bytes[i]!=0xA5)valid=0;
        unsigned linked=0;
        for(unsigned layer=0;layer<4096;++layer){uintptr_t link=ordering[layer].address;
            while(link!=0xFFFFFF){
                if(link<(uintptr_t)(arena.bytes+start) || link>=(uintptr_t)(arena.bytes+g_DrawPacketOffset) || ++linked>emitted){valid=0;break;}
                link=((RenderNativeTag *)link)->address;
            }
        }
        if(linked!=emitted)valid=0;
    }
    for(unsigned invalid=0;invalid<3;++invalid){
        SPRITE_SETUP();s16 point[]={0,0,invalid==2?20000:0};
        projection=invalid==0?0:320;
        FieldFx_SpawnScriptSprite(point,0,32,1,0,0,0);
        memset(arena.bytes,0xA5,sizeof(arena.bytes));
        for(unsigned i=0;i<4096;++i)ordering[i]=(RenderNativeTag){0xFFFFFF,0xAB};
        g_DrawPacketOffset=0;g_ActiveDrawSlot=0;
        FieldFx_UpdateScriptSprites();
        for(unsigned i=0;i<4096;++i)if(ordering[i].address!=0xFFFFFF || ordering[i].length!=0xAB)valid=0;
        for(unsigned i=sizeof(RenderTexturedQuad);i<sizeof(arena.bytes);++i)if(arena.bytes[i]!=0xA5)valid=0;
    }
sprites_restore:
#undef SPRITE_STEP
#undef SPRITE_SETUP
#undef SPRITE_STATE
#undef SPRITE_PUT
    memcpy(list-21,saved_records,sizeof(saved_records));g_FieldFxScriptSpriteList=saved_list;g_FieldFxScriptSpriteCount=saved_count;
    g_GameState=saved_state;g_GeomScroll=saved_scroll;g_DrawPacketOffset=saved_offset;g_ActiveDrawSlot=saved_slot;
    if(!valid || hash!=UINT64_C(0x1c171246cd9ca458)){
        fprintf(stderr,"Original script sprites: cases=%u hash=%016llx valid=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    puts("Original script sprites: 910 USA CPU/GTE-bridge scenarios, pool/clear/reuse, two modes, timers, RGB fades, long lifecycles and FT4/OT payloads match");return 1;
}

static int original_wireframe_renderer(void)
{
    GteMatrix view;
    GteMatrix *saved_view=g_ViewMatrixSlot.value;
    RenderNativeTag ordering[4096];
    union {uintptr_t alignment;u8 bytes[0x4000];} arena;
    const u32 scales[]={0,0x400000,0xC80000,0xFFFF0000};
    const int yaws[]={0,256,1024,2048,3072,4095};
    const unsigned materials[]={0,32,33,34,35};
    const s16 rotations[][9]={{4096,0,0,0,4096,0,0,0,4096},
        {0,0,4096,0,4096,0,-4096,0,0},{4096,0,0,0,4096,0,0,0,4096}};
    u32 saved_xy[256];s16 saved_depth[256];
    memcpy(saved_xy,g_WireScratch.screenXY,sizeof(saved_xy));memcpy(saved_depth,g_WireScratch.depth,sizeof(saved_depth));
    uint64_t hash=UINT64_C(14695981039346656037);int valid=1;unsigned samples=0;
#define WIRE_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned orientation=0;orientation<2;++orientation)for(unsigned floor=0;floor<8;++floor)
    for(unsigned material=0;material<5;++material)for(unsigned scale=0;scale<4;++scale)
    for(unsigned yaw=0;yaw<6;++yaw)for(unsigned position=0;position<3;++position)for(unsigned camera=0;camera<3;++camera){
        for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
        Psyz_GteCtrlWrite(24,160<<16);Psyz_GteCtrlWrite(25,112<<16);Psyz_GteCtrlWrite(26,320);
        memset(&view,0,sizeof(view));memcpy(view.m,rotations[camera],sizeof(view.m));
        if(camera==2){view.t[0]=23;view.t[1]=-17;view.t[2]=250;}g_ViewMatrixSlot.value=&view;
        union {u32 alignment;u8 bytes[128];} frame={0};
        frame.bytes[1]=3;frame.bytes[6]=16;frame.bytes[10]=1;frame.bytes[19]=2;
        const s16 xs[]={-100,100,0},zs[]={0,0,100};
        for(unsigned i=0;i<3;++i){s16 xyz[]={xs[i],floor>>i&1?0:-100,zs[i]};
            memcpy(frame.bytes+28+i*8,xyz,sizeof(xyz));frame.bytes[28+i*8+6]=i%2;}
        memcpy(frame.bytes+52,orientation?(u8[]){1,0,2,0}:(u8[]){0,1,2,0},4);
        memcpy(frame.bytes+56,(u8[]){255,17,129,18},4);frame.bytes[67]=frame.bytes[71]=materials[material];
        RenderAnimPositionInput point={0};point.x=(s16[]){-50,0,50}[position];point.z=(s16[]){1000,8000,15000}[position];
        memset(arena.bytes,0xA5,sizeof(arena.bytes));
        memset(g_WireScratch.screenXY,0xA5,sizeof(saved_xy));memset(g_WireScratch.depth,0xA5,sizeof(saved_depth));
        for(unsigned i=0;i<4096;++i)ordering[i]=(RenderNativeTag){0xFFFFFF,0xAB};
        int used=Render_DrawWireframeModel((struct RenderAnimFrame *)frame.bytes,&point,(s32)scales[scale],yaws[yaw],ordering,arena.bytes,sizeof(arena.bytes));
        u8 *cur=arena.bytes;unsigned count=0,old_used=0;RenderNativeTag *packets[16];
        if(used<0 || used>(int)sizeof(arena.bytes)){valid=0;goto wire_restore;}
        while(cur<arena.bytes+used){
            RenderNativeTag *tag=(RenderNativeTag *)cur;
            if(count>=16 || (tag->length!=0 && tag->length!=1 && tag->length!=4)){valid=0;goto wire_restore;}
            packets[count++]=tag;old_used+=(tag->length+1)*4;
            cur+=tag->length==0?sizeof(RenderNativeTag):tag->length==1?sizeof(RenderDrawModePacket):sizeof(FieldLineG2Packet);
        }
        if(cur!=arena.bytes+used){valid=0;goto wire_restore;}
        WIRE_PUT(old_used,4);
        for(unsigned i=0;i<3;++i){WIRE_PUT(g_WireScratch.screenXY[i],4);WIRE_PUT(g_WireScratch.depth[i],2);}
        for(unsigned i=56;i<60;++i)WIRE_PUT(frame.bytes[i],1);
        for(unsigned i=0;i<count;++i){RenderNativeTag *tag=packets[i];WIRE_PUT(tag->length,1);
            const u8 *payload=(const u8 *)tag+sizeof(*tag);for(unsigned j=0;j<tag->length*4;++j)WIRE_PUT(payload[j],1);}
        for(unsigned layer=0;layer<4096;++layer){
            uintptr_t link=ordering[layer].address;unsigned links=0;
            if(ordering[layer].length!=0xAB)valid=0;
            if(link==0xFFFFFF)continue;WIRE_PUT(layer,2);
            while(link!=0xFFFFFF){
                unsigned id=0;while(id<count && (uintptr_t)packets[id]!=link)++id;
                if(id==count || ++links>=20){valid=0;goto wire_restore;}
                WIRE_PUT(id,1);link=packets[id]->address;
            }
            WIRE_PUT(links,1);
        }
        WIRE_PUT(65535,2);++samples;
    }

    /* Allocation pressure with the three-edge, three-draw-mode reference
     * case, plus invalid face indices and clipped depths. */
    {
        union {u32 alignment;u8 bytes[4096];} mesh={0};
        mesh.bytes[1]=3;mesh.bytes[6]=16;mesh.bytes[10]=1;
        const s16 positions[][3]={{-100,0,0},{100,-100,0},{0,-100,100}};
        for(unsigned i=0;i<3;++i)memcpy(mesh.bytes+28+i*8,positions[i],6);
        memcpy(mesh.bytes+52,(u8[]){0,1,2,0},4);mesh.bytes[67]=32;
        memset(&view,0,sizeof(view));view.m[0][0]=view.m[1][1]=view.m[2][2]=4096;
        RenderAnimPositionInput point={0};point.z=8000;
        for(unsigned capacity=0;capacity<=176;capacity+=8){
            for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
            Psyz_GteCtrlWrite(24,160<<16);Psyz_GteCtrlWrite(25,112<<16);Psyz_GteCtrlWrite(26,320);
            memset(arena.bytes,0xA5,sizeof(arena.bytes));
            for(unsigned i=0;i<4096;++i)ordering[i]=(RenderNativeTag){0xFFFFFF,0xAB};
            int used=Render_DrawWireframeModel((struct RenderAnimFrame *)mesh.bytes,&point,0x400000,2048,ordering,arena.bytes,capacity);
            if(used<0 || (unsigned)used>capacity || (capacity>=168 && used!=168))valid=0;
            for(unsigned i=capacity;i<sizeof(arena.bytes);++i)if(arena.bytes[i]!=0xA5)valid=0;
            for(unsigned i=0;i<4096;++i){uintptr_t link=ordering[i].address;unsigned links=0;
                if(ordering[i].length!=0xAB)valid=0;
                while(link!=0xFFFFFF){
                    if(link<(uintptr_t)arena.bytes || link+sizeof(RenderNativeTag)>(uintptr_t)(arena.bytes+used) || ++links>6){valid=0;break;}
                    RenderNativeTag *tag=(RenderNativeTag *)link;
                    if((tag->length!=1 && tag->length!=4) || link+sizeof(*tag)+tag->length*4>(uintptr_t)(arena.bytes+used)){valid=0;break;}
                    link=tag->address;
                }
            }
        }
        mesh.bytes[52]=3;
        if(Render_DrawWireframeModel((struct RenderAnimFrame *)mesh.bytes,&point,0x400000,2048,ordering,arena.bytes,sizeof(arena.bytes)))valid=0;
        mesh.bytes[52]=0;
        for(unsigned i=0;i<3;++i){s16 zero=0;memcpy(mesh.bytes+28+i*8+2,&zero,2);}
        point.z=0;
        for(unsigned z=20000;z<=40000;z+=20000){
            view.t[2]=z;
            for(unsigned i=0;i<4096;++i)ordering[i]=(RenderNativeTag){0xFFFFFF,0xAB};
            if(Render_DrawWireframeModel((struct RenderAnimFrame *)mesh.bytes,&point,0x400000,0,ordering,arena.bytes,sizeof(arena.bytes)))valid=0;
            for(unsigned i=0;i<4096;++i)if(ordering[i].address!=0xFFFFFF || ordering[i].length!=0xAB)valid=0;
        }
        memset(mesh.bytes,0,sizeof(mesh.bytes));mesh.bytes[1]=255;
        memset(g_WireScratch.screenXY,0xA5,sizeof(saved_xy));memset(g_WireScratch.depth,0xA5,sizeof(saved_depth));
        if(Render_DrawWireframeModel((struct RenderAnimFrame *)mesh.bytes,&point,0x400000,0,ordering,arena.bytes,0) ||
           g_WireScratch.screenXY[255]!=0xA5A5A5A5 || (u16)g_WireScratch.depth[255]!=0xA5A5)valid=0;
    }
wire_restore:
#undef WIRE_PUT
    g_ViewMatrixSlot.value=saved_view;
    memcpy(g_WireScratch.screenXY,saved_xy,sizeof(saved_xy));memcpy(g_WireScratch.depth,saved_depth,sizeof(saved_depth));
    if(!valid || hash!=UINT64_C(0xdd1b134b66169daf)){
        fprintf(stderr,"Original wireframe renderer: samples=%u hash=%016llx valid=%d\n",samples,(unsigned long long)hash,valid);return 0;}
    puts("Original wireframe renderer: 17280 USA CPU/GTE-bridge cases, native LINE_G2/draw-mode payloads and full OT chains match");return 1;
}

static int original_wireframe_animation(void)
{
    Pe1GameState saved_state=g_GameState;GeomScrollRecord saved_scroll=g_GeomScroll;
    Combatant *saved_actor=g_ActiveActor;FieldActor *saved_player=g_PlayerEntity;
    int saved_offset=g_DrawPacketOffset,saved_slot=g_ActiveDrawSlot,saved_frame=g_BattleAttackAnimFrame;
    Combatant actor={0};BattleAction action={0};FieldActor player={0};actor.action=&action;
    GteMatrix view={0};view.m[0][0]=view.m[1][1]=view.m[2][2]=4096;
    g_ViewMatrixSlot.value=&view;g_ActiveActor=&actor;g_PlayerEntity=&player;
    RenderNativeTag ordering[4096];
    union {uintptr_t alignment;u8 bytes[0x4000];} arena;
    const int distances[]={-32768,-1,0,1,200,32767};
    const int frames[]={1,2,9,10,11,127,INT_MAX};
    const int modes[]={0,16,32,48};
    u32 saved_xy[256];s16 saved_depth[256];
    memcpy(saved_xy,g_WireScratch.screenXY,sizeof(saved_xy));memcpy(saved_depth,g_WireScratch.depth,sizeof(saved_depth));
    uint64_t hash=UINT64_C(14695981039346656037);int valid=1;unsigned samples=0;
#define WIRE_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned slot=0;slot<2;++slot)for(unsigned mode=0;mode<4;++mode)
    for(unsigned distance=0;distance<6;++distance)for(unsigned step=0;step<7;++step)for(unsigned camera=0;camera<2;++camera){
        for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
        Psyz_GteCtrlWrite(26,320);
        union {u32 alignment;u8 bytes[128];} frame={0};
        frame.bytes[1]=3;frame.bytes[6]=16;frame.bytes[10]=1;frame.bytes[19]=2;
        const s16 xyz[][3]={{-100,-100,0},{100,0,0},{0,0,100}};
        for(unsigned i=0;i<3;++i){memcpy(frame.bytes+28+i*8,xyz[i],sizeof(xyz[i]));frame.bytes[28+i*8+6]=i%2;}
        memcpy(frame.bytes+52,(u8[]){0,1,2,0},4);memcpy(frame.bytes+56,(u8[]){255,17,129,18},4);
        frame.bytes[67]=frame.bytes[71]=33;
        player.pos_x=(s32)(((u32)(camera?50:0)<<16)|0x1234);
        player.pos_y=(s32)(((u32)(camera?-300:-25)<<16)|0x1234);
        player.pos_z=(s32)(((u32)(camera?8000:1000)<<16)|0x1234);
        actor.stateFlags=modes[mode];action.range=distances[distance];
        g_GameState.room_geometry_table=frame.bytes;g_DrawPacketOffset=0;g_ActiveDrawSlot=slot;g_BattleAttackAnimFrame=frames[step];
        for(unsigned i=0;i<2;++i){g_OtBufferTable[i]=(u8 *)ordering;g_RenderAnimBufferBase[i]=arena.bytes;}
        g_GeomScroll.state.position.screenOffsetX=camera?140:160;g_GeomScroll.state.position.screenOffsetY=camera?100:112;
        memset(arena.bytes,0xA5,sizeof(arena.bytes));
        memset(g_WireScratch.screenXY,0xA5,sizeof(saved_xy));memset(g_WireScratch.depth,0xA5,sizeof(saved_depth));
        for(unsigned i=0;i<4096;++i)ordering[i]=(RenderNativeTag){0xFFFFFF,0xAB};
        Render_AnimationFrame();int used=g_DrawPacketOffset;
        u8 *cur=arena.bytes;unsigned count=0,old_used=0;RenderNativeTag *packets[16];
        if(used<0 || used>(int)sizeof(arena.bytes)){valid=0;goto wire_restore;}
        while(cur<arena.bytes+used){
            RenderNativeTag *tag=(RenderNativeTag *)cur;
            if(count>=16 || (tag->length!=0 && tag->length!=1 && tag->length!=4)){valid=0;goto wire_restore;}
            packets[count++]=tag;old_used+=(tag->length+1)*4;
            cur+=tag->length==0?sizeof(RenderNativeTag):tag->length==1?sizeof(RenderDrawModePacket):sizeof(FieldLineG2Packet);
        }
        if(cur!=arena.bytes+used){valid=0;goto wire_restore;}
        WIRE_PUT(old_used,4);WIRE_PUT(g_BattleAttackAnimFrame,4);
        WIRE_PUT(Psyz_GteCtrlRead(24),4);WIRE_PUT(Psyz_GteCtrlRead(25),4);
        for(unsigned i=0;i<3;++i){WIRE_PUT(g_WireScratch.screenXY[i],4);WIRE_PUT(g_WireScratch.depth[i],2);}
        for(unsigned i=56;i<60;++i)WIRE_PUT(frame.bytes[i],1);
        for(unsigned i=0;i<count;++i){RenderNativeTag *tag=packets[i];WIRE_PUT(tag->length,1);
            const u8 *payload=(const u8 *)tag+sizeof(*tag);for(unsigned j=0;j<tag->length*4;++j)WIRE_PUT(payload[j],1);}
        for(unsigned layer=0;layer<4096;++layer){
            uintptr_t link=ordering[layer].address;unsigned links=0;
            if(ordering[layer].length!=0xAB)valid=0;
            if(link==0xFFFFFF)continue;WIRE_PUT(layer,2);
            while(link!=0xFFFFFF){
                unsigned id=0;while(id<count && (uintptr_t)packets[id]!=link)++id;
                if(id==count || ++links>=20){valid=0;goto wire_restore;}
                WIRE_PUT(id,1);link=packets[id]->address;
            }
            WIRE_PUT(links,1);
        }
        WIRE_PUT(65535,2);++samples;
    }
wire_restore:
#undef WIRE_PUT
    g_GameState=saved_state;g_GeomScroll=saved_scroll;g_ActiveActor=saved_actor;g_PlayerEntity=saved_player;
    g_DrawPacketOffset=saved_offset;g_ActiveDrawSlot=saved_slot;g_BattleAttackAnimFrame=saved_frame;
    memcpy(g_WireScratch.screenXY,saved_xy,sizeof(saved_xy));memcpy(g_WireScratch.depth,saved_depth,sizeof(saved_depth));
    if(!valid || hash!=UINT64_C(0xabf9dfc230518165)){
        fprintf(stderr,"Original wireframe animation: samples=%u hash=%016llx valid=%d\n",samples,(unsigned long long)hash,valid);return 0;}
    puts("Original wireframe animation: 672 USA CPU/GTE-bridge frames, canonical actor range/position, both buffers, OT depth and counter wrapping match");return 1;
}


static int original_textbox_renderer(PeDisc *disc,PeDiscFile *exe)
{
    u8 authored[sizeof(g_FieldFontTextureState)]={0};u8 spacing[sizeof(g_TextboxGlyphSpacing)]={0};
    if(!pe_disc_read(disc,exe,0x91644-0x10000+0x800,authored,sizeof(authored)) ||
       memcmp(authored,&g_FieldFontTextureState,sizeof(authored)) ||
       !pe_disc_read(disc,exe,0x916A0-0x10000+0x800,spacing,sizeof(spacing)) ||
       memcmp(spacing,g_TextboxGlyphSpacing,sizeof(spacing))){fprintf(stderr,"original textbox authored tables differ (font=%d spacing=%d)\n",memcmp(authored,&g_FieldFontTextureState,sizeof(authored)),memcmp(spacing,g_TextboxGlyphSpacing,sizeof(spacing)));return 0;}
    TextboxEntry saved_entries[4];memcpy(saved_entries,g_TextboxEntries,sizeof(saved_entries));
    TextboxGlyphPacket saved_choice[2];memcpy(saved_choice,g_TextboxChoiceCursor,sizeof(saved_choice));
    RenderModeTilePacket saved_fog[2];memcpy(saved_fog,g_TextboxFogTilePackets,sizeof(saved_fog));
    RenderBufferPrefix saved_buffers=D_800B0E38;
    u8 *saved_source=g_TextboxMessageSource,*saved_cursor=g_DrawPacketCursor,*saved_base=g_DrawPacketArenaBase;
    u32 *saved_order=g_DrawOrderingTableEntry;
    int saved_slot=g_ActiveDrawSlot,saved_collision=g_ScriptCollisionFlag,saved_dim=g_DrawTextDimmed;
    u32 saved_pad=g_FieldPadPressed,saved_color=g_DrawPrimColor,saved_shade=g_DrawColorShaded;
    u32 saved_blend[]={g_DrawBlendColorPair[0],g_DrawBlendColorPair[1]};
    u8 saved_globals[]={g_DialogueChoiceCursor,(u8)g_MenuEquipSlotIndex,g_TextboxKernGlyph0F,g_PlayerMovementEnabled,
        g_TextboxGlyphColorR,g_TextboxGlyphColorG,g_TextboxGlyphColorB,
        g_TextboxHighlightColorR,g_TextboxHighlightColorG,g_TextboxHighlightColorB};
    int saved_x=g_TextCursorX,saved_y=g_TextCursorY,saved_stack[28];
    memcpy(saved_stack,g_TextCursorStackBottom,sizeof(saved_stack));u16 *saved_vertex=g_DrawVertexWritePtr;
    struct {u8 before[16];TextboxGlyphPacket packets[TEXTBOX_GLYPH_PACKET_CAPACITY];u8 after[16];} glyph_storage;
    TextboxGlyphPacket *glyphs=glyph_storage.packets;
    union {uintptr_t align;u8 bytes[0x4000];} arena;
    RenderNativeTag ordering[8];u8 source[1024];
    const u8 special[][32]={
        {250,247,250,255},{251,4,1,251,0,251,1,251,2,251,3,251,5,2,255},
        {251,8,76,251,8,255},{251,9,3,1,247,2,247,3,255},{1,248,2,255},{1,249},
        {1,251,6,2,255},{1,251,7,3,2,255}};
    const u8 extended[]={0,1,20,21,127,255},fb[]={10,19,20,127,255};
    uint64_t hash=UINT64_C(14695981039346656037);int valid=1;unsigned frames=0;
#define TEXT_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
#define TEXT_PAYLOAD(packet,kind) do { \
    RenderNativeTag *t_=(RenderNativeTag *)(packet);TEXT_PUT(kind==1?6:kind==2?5:t_->length,1); \
    const u8 *p_=(const u8 *)(packet)+sizeof(RenderNativeTag); \
    if(kind){ \
        unsigned expected_=(sizeof(RenderTexturePagePacket)+(kind==1?16:12))/4; \
        if(t_->length!=expected_)valid=0; \
        for(unsigned j_=0;j_<4;++j_)TEXT_PUT(p_[j_],1); \
        const u8 *end_=(const u8 *)(packet)+sizeof(RenderTexturePagePacket)+sizeof(RenderNativeTag); \
        for(const u8 *zero_=p_+4;zero_<end_;++zero_)if(*zero_)valid=0; \
        p_=end_; \
        for(unsigned j_=0;j_<(kind==1?16:12);++j_)TEXT_PUT(p_[j_],1); \
    }else for(unsigned j_=0;j_<t_->length*4;++j_)TEXT_PUT(p_[j_],1); \
}while(0)
    for(unsigned msg=0;msg<104;++msg)for(unsigned slot=0;slot<2;++slot)
    for(unsigned style=0;style<4;++style)for(unsigned state=1;state<=2;++state)
    for(unsigned variant=0;variant<4;++variant){
        memcpy(&g_FieldFontTextureState,authored,sizeof(authored));
        if(variant&1)for(unsigned i=0;i<4;++i){
            FieldBgTextureSlot *t=&g_FieldBgTextureSlots[i];
            t->tpage=BG_SLOT_TPAGE(t->x,t->y);t->clut=BG_SLOT_CLUT(t->clut_x,t->clut_y);
            if(g_TextboxFontPages[i].tpage!=t->tpage || g_TextboxFontPages[i].clut!=t->clut)valid=0;
        }
        memset(g_TextboxEntries,0xA5,sizeof(g_TextboxEntries));
        memset(g_TextboxChoiceCursor,0xA5,sizeof(g_TextboxChoiceCursor));
        memset(g_TextboxFogTilePackets,0xA5,sizeof(g_TextboxFogTilePackets));
        Render_SetupFogLayer(source);
        for(unsigned i=0;i<4;++i){TextboxEntry *e=&g_TextboxEntries[i];
            TEXT_PUT(e->state,1);TEXT_PUT(e->style,1);TEXT_PUT(e->background,1);
            TEXT_PUT(e->control.flags,4);TEXT_PUT(e->page_id,2);}
        for(unsigned i=0;i<2;++i){TEXT_PAYLOAD(&g_TextboxChoiceCursor[i],1);TEXT_PAYLOAD(&g_TextboxFogTilePackets[i],2);}
        memset(source,0,sizeof(source));source[0]=255;source[1]=254;source[2]=7;
        if(msg<78){source[3]=msg;source[4]=255;}
        else if(msg<86)memcpy(source+3,special[msg-78],sizeof(*special));
        else if(msg<98){source[3]=msg<92?252:253;source[4]=extended[(msg-86)%6];source[5]=255;}
        else if(msg<103){source[3]=251;source[4]=fb[msg-98];source[5]=255;}
        else memcpy(source+3,(u8[]){1,247,2,255},4);
        memset(arena.bytes,0xA5,sizeof(arena.bytes));memset(&glyph_storage,0xA5,sizeof(glyph_storage));
        for(unsigned i=0;i<8;++i)ordering[i]=(RenderNativeTag){0xFFFFFF,0xAB};
        for(unsigned i=0;i<2;++i){D_800B0E38.ordering[i]=ordering;D_800B0E38.other_buffers[1+i]=(char *)glyphs;}
        g_DrawPacketCursor=g_DrawPacketArenaBase=arena.bytes;g_DrawOrderingTableEntry=(u32 *)ordering;
        g_DrawPrimColor=0x112233;g_DrawColorShaded=0x445566;g_DrawTextDimmed=0;
        g_DrawBlendColorPair[0]=0x102030;g_DrawBlendColorPair[1]=0x405060;
        g_ActiveDrawSlot=slot;g_ScriptCollisionFlag=variant==3;
        g_FieldPadPressed=(u32[]){0,0x100,0x20,8}[variant];
        g_DialogueChoiceCursor=variant%3;g_MenuEquipSlotIndex=7;g_TextboxKernGlyph0F=variant%2;
        g_TextboxHighlightColorR=17;g_TextboxHighlightColorG=34;g_TextboxHighlightColorB=51;
        TextboxEntry *e;
        for(unsigned entry=0;entry<4;++entry){
            if(msg!=103 && entry!=variant%4)continue;
        e=&g_TextboxEntries[entry];
        e->state=state;e->message=source+3;e->style=style;e->background=1;
        e->control.flags=((variant&1)<<25)|((variant>>1)<<16)|((variant%4)<<8);e->page_id=7;
        e->x=43;e->y=57;e->width=260;e->height=32;
        memcpy(e->numbers,(u8[]){1,2,3,4,5,3,9,8,7,6,5,2},12);
        }
        Menu_DrawTextboxEntries();
        for(unsigned i=0;i<4;++i){e=&g_TextboxEntries[i];TEXT_PUT(e->state,1);
            TEXT_PUT(e->message?(u32)(e->message-source):UINT32_MAX,4);TEXT_PUT(e->control.flags,4);}
        TEXT_PUT(g_DialogueChoiceCursor,1);TEXT_PUT(g_MenuEquipSlotIndex,1);
        TEXT_PUT(g_TextboxGlyphColorR,1);TEXT_PUT(g_TextboxGlyphColorG,1);TEXT_PUT(g_TextboxGlyphColorB,1);
        for(unsigned layer=0;layer<8;++layer){
            uintptr_t link=ordering[layer].address;unsigned links=0;
            while(link!=0xFFFFFF){
                unsigned kind=0,id=0;void *packet=(void *)link;
                if(link>=(uintptr_t)glyphs && link<(uintptr_t)(glyphs+TEXTBOX_GLYPH_PACKET_CAPACITY)){
                    kind=1;id=(link-(uintptr_t)glyphs)/sizeof(*glyphs);
                }else if(link>=(uintptr_t)g_TextboxChoiceCursor && link<(uintptr_t)(g_TextboxChoiceCursor+2)){
                    kind=1;id=1000+(link-(uintptr_t)g_TextboxChoiceCursor)/sizeof(*g_TextboxChoiceCursor);
                }else if(link>=(uintptr_t)g_TextboxFogTilePackets && link<(uintptr_t)(g_TextboxFogTilePackets+2)){
                    kind=2;id=1010+(link-(uintptr_t)g_TextboxFogTilePackets)/sizeof(*g_TextboxFogTilePackets);
                }else{
                    u8 *cur=arena.bytes;unsigned old_offset=0;
                    while(cur<g_DrawPacketCursor && (uintptr_t)cur!=link){
                        RenderNativeTag *t=(RenderNativeTag *)cur;
                        if(t->length!=1 && t->length!=4){valid=0;break;}
                        old_offset+=t->length==1?8:20;
                        cur+=t->length==1?sizeof(RenderDrawModePacket):sizeof(RenderColorTrianglePacket);
                    }
                    if((uintptr_t)cur!=link || cur>=g_DrawPacketCursor){valid=0;break;}
                    id=2000+old_offset/4;
                }
                if(++links>=400){valid=0;break;}
                TEXT_PUT(id,2);TEXT_PAYLOAD(packet,kind);link=((RenderNativeTag *)packet)->address;
            }
            TEXT_PUT(links,2);if(ordering[layer].length!=0xAB)valid=0;
        }
        for(unsigned i=0;i<16;++i)if(glyph_storage.before[i]!=0xA5 || glyph_storage.after[i]!=0xA5)valid=0;
        ++frames;if(!valid)goto text_restore;
    }
    /* Preserve the original number of glyph slots after pointer widening;
     * overlong strings must stop without touching the adjacent work arena. */
    for(unsigned kind=0;kind<6;++kind){
        memset(&glyph_storage,0xA5,sizeof(glyph_storage));memset(g_TextboxEntries,0,sizeof(g_TextboxEntries));
        for(unsigned i=0;i<8;++i)ordering[i]=(RenderNativeTag){0xFFFFFF,0xAB};
        g_ActiveDrawSlot=0;g_ScriptCollisionFlag=0;g_FieldPadPressed=0;
        TextboxEntry *e=&g_TextboxEntries[0];e->state=2;e->style=2;e->message=source;
        unsigned bytes=0;
        if(kind==0)for(unsigned i=0;i<340;++i)source[bytes++]=1;
        else if(kind==1)for(unsigned i=0;i<120;++i)source[bytes++]=250;
        else if(kind==2)for(unsigned i=0;i<340;++i){source[bytes++]=252;source[bytes++]=1;}
        else if(kind==3)source[bytes++]=78;
        else{
            for(unsigned i=0;i<6;++i){source[bytes++]=251;source[bytes++]=8;}
            for(unsigned i=0;i<5;++i){e->numbers[i].digits[0]=1;e->numbers[i].count=kind==4?6:1;}
        }
        source[bytes]=255;Menu_DrawTextboxEntries();
        uintptr_t link=ordering[1].address;unsigned count=0,expected=kind<3?TEXTBOX_GLYPH_PACKET_CAPACITY:kind==5?5:0;
        while(link!=0xFFFFFF){
            if(count>=expected || link!=(uintptr_t)&glyphs[expected-count-1]){valid=0;break;}
            link=((RenderNativeTag *)link)->address;++count;
        }
        if(count!=expected)valid=0;
        for(unsigned i=0;i<16;++i)if(glyph_storage.before[i]!=0xA5 || glyph_storage.after[i]!=0xA5)valid=0;
    }
text_restore:
    memcpy(&g_FieldFontTextureState,authored,sizeof(authored));
#undef TEXT_PAYLOAD
#undef TEXT_PUT
    memcpy(g_TextboxEntries,saved_entries,sizeof(saved_entries));memcpy(g_TextboxChoiceCursor,saved_choice,sizeof(saved_choice));
    memcpy(g_TextboxFogTilePackets,saved_fog,sizeof(saved_fog));D_800B0E38=saved_buffers;
    g_TextboxMessageSource=saved_source;g_DrawPacketCursor=saved_cursor;g_DrawPacketArenaBase=saved_base;g_DrawOrderingTableEntry=saved_order;
    g_ActiveDrawSlot=saved_slot;g_ScriptCollisionFlag=saved_collision;g_DrawTextDimmed=saved_dim;
    g_FieldPadPressed=saved_pad;g_DrawPrimColor=saved_color;g_DrawColorShaded=saved_shade;
    memcpy(g_DrawBlendColorPair,saved_blend,sizeof(saved_blend));g_DialogueChoiceCursor=saved_globals[0];g_MenuEquipSlotIndex=saved_globals[1];
    g_TextboxKernGlyph0F=saved_globals[2];g_PlayerMovementEnabled=saved_globals[3];
    g_TextboxGlyphColorR=saved_globals[4];g_TextboxGlyphColorG=saved_globals[5];g_TextboxGlyphColorB=saved_globals[6];
    g_TextboxHighlightColorR=saved_globals[7];g_TextboxHighlightColorG=saved_globals[8];g_TextboxHighlightColorB=saved_globals[9];
    g_TextCursorX=saved_x;g_TextCursorY=saved_y;memcpy(g_TextCursorStackBottom,saved_stack,sizeof(saved_stack));g_DrawVertexWritePtr=saved_vertex;
    if(!valid || hash!=UINT64_C(0xa279a1e503166b41)){
        fprintf(stderr,"original textbox renderer mismatch: frames=%u hash=%016llx valid=%d\n",frames,(unsigned long long)hash,valid);return 0;}
    puts("Original textbox renderer: 6656 USA MIPS frames, all 78 base glyphs, extended pages, names, numbers, colors, choices, sections, delays, fog and both buffers match");
    return 1;
}

static int original_scan_overlay(PeDisc *disc, PeDiscFile *exe)
{
    u8 custom[]={1,255};
    struct {u8 *data;u32 address;size_t size;} texts[]={
        {g_AnalyzeHpUnknownText,0x91464,16},{g_AnalyzeHpFormatText,0x91474,12},
        {(u8 *)g_TextSusceptibleToPoison,0x91480,44},
        {(u8 *)g_TextSusceptibleToAcid,0x914AC,40},
        {(u8 *)g_AnalyzeTranquilizerWeakTexts,0x914D4,40},
        {(u8 *)g_AnalyzeHeatWeakTexts,0x914FC,36},
        {(u8 *)g_AnalyzeColdWeakTexts,0x91520,36},
        {(u8 *)g_AnalyzeStealWeakTexts,0x91544,44},
        {(u8 *)g_AnalyzeConfusionWeakTexts,0x91570,42}};
    u8 *messages[]={g_AnalyzeHpUnknownText,g_AnalyzeHpFormatText,
        g_TextSusceptibleToPoison[0],g_TextSusceptibleToPoison[1],
        g_TextSusceptibleToAcid[0],g_TextSusceptibleToAcid[1],
        g_AnalyzeTranquilizerWeakTexts[0],g_AnalyzeTranquilizerWeakTexts[1],
        g_AnalyzeHeatWeakTexts[0],g_AnalyzeHeatWeakTexts[1],
        g_AnalyzeColdWeakTexts[0],g_AnalyzeColdWeakTexts[1],
        g_AnalyzeStealWeakTexts[0],g_AnalyzeStealWeakTexts[1],
        g_AnalyzeConfusionWeakTexts[0],g_AnalyzeConfusionWeakTexts[1],custom};
    for(unsigned i=0;i<sizeof(texts)/sizeof(*texts);++i){
        u8 raw[44];
        if(!pe_disc_read(disc,exe,texts[i].address-0x10000+0x800,raw,texts[i].size) ||
           memcmp(raw,texts[i].data,texts[i].size))return 0;
    }
    TextboxEntry saved_entries[4];memcpy(saved_entries,g_TextboxEntries,sizeof(saved_entries));
    TextboxRect saved_rect=g_TextboxPendingRect;
    SavePromptBlock saved_scan=g_BattleScanPanel;
    u8 saved_active=g_BattleSaveOverlayActive,saved_timer=g_SaveOverlayTimer;
    u8 saved_choice=g_DialogueChoiceCursor,saved_mode=g_MenuActiveMode;
    s8 saved_equip=g_MenuEquipSlotIndex;
    u8 *saved_message=g_CurItemEffectData,*saved_cursor=g_DrawPacketCursor,*saved_base=g_DrawPacketArenaBase;
    unsigned *saved_order=g_DrawOrderingTableEntry;
    int saved_window=g_SaveMetadataWindowIndex,saved_dim=g_DrawTextDimmed;
    int saved_x=g_TextCursorX,saved_y=g_TextCursorY;
    u32 saved_color=g_DrawPrimColor,saved_shade=g_DrawColorShaded;
    u32 saved_blend[]={g_DrawBlendColorPair[0],g_DrawBlendColorPair[1]};
    int saved_stack[28];memcpy(saved_stack,g_TextCursorStackBottom,sizeof(saved_stack));
    u16 *saved_vertex=g_DrawVertexWritePtr;
    union {uintptr_t align;u8 bytes[0x4000];} arena;
    RenderNativeTag ordering;
    EnemyCombatant core={0};BattleEntity target={0};
    const u32 weaknesses[]={0,0x1000,0x2000,0x4000,0x8000,0x10000,0x20000,0x40000,0x7F000};
    const unsigned timers[]={0,1,75,255};
    uint64_t hash=UINT64_C(14695981039346656037);int valid=1;
#define SCAN_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned sample=0;sample<18560;++sample){
        unsigned n=sample<18432?sample:sample-18432;
        unsigned window=n%2;n/=2;unsigned mode=n%4;n/=4;
        memset(arena.bytes,0xA5,sizeof(arena.bytes));ordering=(RenderNativeTag){0xFFFFFF,0xAB};
        g_DrawPacketCursor=g_DrawPacketArenaBase=arena.bytes;g_DrawOrderingTableEntry=(unsigned *)&ordering;
        g_DrawPrimColor=0x112233;g_DrawColorShaded=0x445566;g_DrawTextDimmed=0;
        g_DrawBlendColorPair[0]=0x102030;g_DrawBlendColorPair[1]=0x405060;
        g_SaveMetadataWindowIndex=window;g_MenuActiveMode=mode;
        memset(&g_TextboxPendingRect,0xA5,sizeof(g_TextboxPendingRect));
        g_DialogueChoiceCursor=3;g_MenuEquipSlotIndex=7;
        memset(g_TextboxEntries,0xA5,sizeof(g_TextboxEntries));
        for(unsigned i=0;i<4;++i){g_TextboxEntries[i].message=custom;g_TextboxEntries[i].control.flags=-1;}
        if(sample<18432){
            unsigned which=n%4;n/=4;unsigned timer=timers[n%4];n/=4;
            u32 weak=weaknesses[n%9];n/=9;unsigned prompt=n%4;unsigned phase=n/4;
            g_BattleSaveOverlayActive=0;g_SaveOverlayTimer=7;
            g_BattleScanPanel.prompt.word=0x48000000|weak|(prompt<<10)|(phase<<8)|timer;
            core.hpAlive=which==2?-1:123;core.hpAliveCap=456;
            target.core=which==1?NULL:&core;g_BattleScanPanel.target=which?&target:NULL;
            Save_DrawSlotMetadata();
        }else{
            g_SaveOverlayTimer=timers[n%4];g_BattleSaveOverlayActive=n/4;
            g_BattleScanPanel.prompt.word=0x12345678;g_CurItemEffectData=custom;
            Menu_SaveOverlayDraw();
        }
        SCAN_PUT(g_BattleScanPanel.prompt.word,4);SCAN_PUT(g_BattleSaveOverlayActive,1);
        SCAN_PUT(g_SaveOverlayTimer,1);SCAN_PUT(g_DialogueChoiceCursor,1);SCAN_PUT(g_MenuEquipSlotIndex,1);
        for(unsigned i=0;i<4;++i){
            TextboxEntry *e=&g_TextboxEntries[i];unsigned id=0;
            for(unsigned j=0;j<17;++j)if(e->message==messages[j])id=j+1;
            SCAN_PUT(e->state,1);SCAN_PUT(id,1);SCAN_PUT(e->style,1);SCAN_PUT(e->background,1);
            SCAN_PUT(e->control.flags,4);SCAN_PUT(e->page_id,2);SCAN_PUT(e->x,2);SCAN_PUT(e->y,2);
            SCAN_PUT(e->width,2);SCAN_PUT(e->height,2);
            const u8 *numbers=(const u8 *)e->numbers;for(unsigned j=0;j<12;++j)SCAN_PUT(numbers[j],1);
        }
        SCAN_PUT(g_TextboxPendingRect.x,2);SCAN_PUT(g_TextboxPendingRect.y,2);
        SCAN_PUT(g_TextboxPendingRect.width,2);SCAN_PUT(g_TextboxPendingRect.height,2);
        SCAN_PUT(g_TextCursorX,4);SCAN_PUT(g_TextCursorY,4);
        RenderNativeTag *packets[16];unsigned count=0;u8 *cur=arena.bytes;
        while(cur<g_DrawPacketCursor){
            RenderNativeTag *tag=(RenderNativeTag *)cur;
            if(count>=16 || (tag->length!=1 && tag->length!=4)){valid=0;break;}
            packets[count++]=tag;SCAN_PUT(tag->length,1);
            const u8 *payload=cur+sizeof(*tag);
            for(unsigned j=0;j<tag->length*4;++j)SCAN_PUT(payload[j],1);
            cur+=tag->length==1?sizeof(RenderDrawModePacket):sizeof(RenderColorTrianglePacket);
        }
        if(cur!=g_DrawPacketCursor || ordering.length!=0xAB)valid=0;
        for(u8 *guard=g_DrawPacketCursor;guard<arena.bytes+sizeof(arena.bytes);++guard)if(*guard!=0xA5)valid=0;
        SCAN_PUT(count,1);uintptr_t link=ordering.address;unsigned links=0;
        while(link!=0xFFFFFF){
            unsigned i=0;while(i<count && link!=(uintptr_t)packets[i])++i;
            if(i==count || links>=count){valid=0;break;}
            SCAN_PUT(i,1);link=packets[i]->address;++links;
        }
        SCAN_PUT(links,1);
        if(!valid)break;
    }
#undef SCAN_PUT
    /* Allocation pressure at every native alignment near the arena end. */
    for(unsigned remaining=0;remaining<=512;remaining+=sizeof(uintptr_t))
    for(unsigned kind=0;kind<4;++kind){
        memset(arena.bytes,0xA5,sizeof(arena.bytes));ordering=(RenderNativeTag){0xFFFFFF,0xAB};
        g_DrawPacketCursor=arena.bytes+sizeof(arena.bytes)-remaining;
        g_DrawPacketArenaBase=arena.bytes;g_DrawOrderingTableEntry=(unsigned *)&ordering;
        u8 *start=g_DrawPacketCursor;
        if(kind<2)Draw_AllocColorGradient(320,20,NULL,kind);
        else if(kind==2)Draw_AllocColorTri(320,20,0);
        else Draw_EmitWipeBarRect(320,20,0);
        if(g_DrawPacketCursor<start || g_DrawPacketCursor>arena.bytes+sizeof(arena.bytes) || ordering.length!=0xAB)valid=0;
        for(u8 *guard=arena.bytes;guard<start;++guard)if(*guard!=0xA5)valid=0;
        uintptr_t link=ordering.address;unsigned links=0;
        while(link!=0xFFFFFF){
            if(link<(uintptr_t)start || link+sizeof(RenderNativeTag)>(uintptr_t)g_DrawPacketCursor || ++links>20){valid=0;break;}
            RenderNativeTag *tag=(RenderNativeTag *)link;
            if(tag->length<1 || tag->length>4 || link+sizeof(*tag)+tag->length*4>(uintptr_t)g_DrawPacketCursor){valid=0;break;}
            link=tag->address;
        }
    }
    memcpy(g_TextboxEntries,saved_entries,sizeof(saved_entries));g_TextboxPendingRect=saved_rect;
    g_BattleScanPanel=saved_scan;g_BattleSaveOverlayActive=saved_active;g_SaveOverlayTimer=saved_timer;
    g_DialogueChoiceCursor=saved_choice;g_MenuEquipSlotIndex=saved_equip;g_MenuActiveMode=saved_mode;
    g_CurItemEffectData=saved_message;g_DrawPacketCursor=saved_cursor;g_DrawPacketArenaBase=saved_base;
    g_DrawOrderingTableEntry=saved_order;g_SaveMetadataWindowIndex=saved_window;g_DrawTextDimmed=saved_dim;
    g_TextCursorX=saved_x;g_TextCursorY=saved_y;g_DrawPrimColor=saved_color;g_DrawColorShaded=saved_shade;
    memcpy(g_DrawBlendColorPair,saved_blend,sizeof(saved_blend));
    memcpy(g_TextCursorStackBottom,saved_stack,sizeof(saved_stack));g_DrawVertexWritePtr=saved_vertex;
    if(!valid || hash!=UINT64_C(0x2c8a283ef8f19f95)){
        fprintf(stderr,"original Scan/message overlay mismatch: %016llx valid=%d\n",(unsigned long long)hash,valid);return 0;
    }
    puts("original Scan/message overlays: 18560 USA MIPS cases, native wipe payloads and OT chains");
    return 1;
}

static int original_hud_packet_updates(void)
{
    extern void Battle_DrawSpellName(void);
    extern void Battle_DrawStatusSymbol(s8);
    struct {void *data;size_t bytes,stride;unsigned id,kind;} pools[]={
        {g_BattleStatusValueDigits,sizeof(g_BattleStatusValueDigits),sizeof(RenderPagedSpritePacket),1,0},
        {g_BattleRemainingActionSprites,sizeof(g_BattleRemainingActionSprites),sizeof(RenderPagedSpritePacket),17,0},
        {g_BattleStatusValueMarkers,sizeof(g_BattleStatusValueMarkers),sizeof(RenderPagedSpritePacket),18,0},
        {g_BattleStatusSymbolSprites,sizeof(g_BattleStatusSymbolSprites),sizeof(RenderPagedSpritePacket),19,0},
        {g_BattlePanelDigits,sizeof(g_BattlePanelDigits),sizeof(RenderPagedSpritePacket),20,0},
        {g_BattleStatusBarTiles,sizeof(g_BattleStatusBarTiles),sizeof(RenderColorTilePacket),0,1},
        {g_StatusMarkerPackets,sizeof(g_StatusMarkerPackets),sizeof(RenderPagedSpritePacket),25,0},
    };
    Combatant actor={0};BattleAction action={0};actor.action=&action;
    Combatant *old_actor=g_ActiveActor;int old_slot=g_ActiveDrawSlot,old_panel=g_BattleStatusPanelSlot;
    u8 old_remaining=g_BattleRemainingActions,old_countdown=g_BattleSpritePrimCountdown;
    RenderNativeTag *old_ordering[]={D_800B0E38.ordering[0],D_800B0E38.ordering[1]};
    RenderNativeTag ordering[2][8];
    for(unsigned i=0;i<7;++i)memset(pools[i].data,0xA5,pools[i].bytes);
    memset(g_BattleStatusPointerSprites,0xA5,sizeof(g_BattleStatusPointerSprites));Battle_DrawHPBar();
    for(unsigned slot=0;slot<2;++slot)D_800B0E38.ordering[slot]=ordering[slot];
    g_ActiveActor=&actor;uint64_t hash=UINT64_C(14695981039346656037);int valid=1;
    const s16 anchors[][2]={{0,0},{221,177},{-32768,32767}};
    const u8 reds[]={0,128,255};
#define HUD_SETUP(slot,anchor) do { \
    for(unsigned layer=0;layer<8;++layer)ordering[slot][layer]=(RenderNativeTag){0xFFFFFF,0xAB}; \
    g_ActiveDrawSlot=slot;g_BattleStatusPanelSlot=0; \
    g_BattleStatusBarTiles[slot*3].x0=anchors[anchor][0];g_BattleStatusBarTiles[slot*3].y0=anchors[anchor][1]; \
    g_BattleStatusPointerSprites[slot].sprite.color.bytes.r=reds[anchor]; \
    g_BattleStatusPointerSprites[slot].sprite.color.bytes.g=17;g_BattleStatusPointerSprites[slot].sprite.color.bytes.b=231; \
} while(0)
#define HUD_BYTE(value) (hash=(hash^(u8)(value))*UINT64_C(1099511628211))
#define HUD_COLLECT(slot) do { \
    for(unsigned layer=0;layer<8;++layer) { \
        struct {unsigned pool,index;} nodes[16];unsigned count=0;uintptr_t address=ordering[slot][layer].address; \
        while(address!=0xFFFFFF) { \
            unsigned pool=0;for(;pool<7;++pool) { \
                uintptr_t base=(uintptr_t)pools[pool].data; \
                if(address>=base && address<base+pools[pool].bytes && !((address-base)%pools[pool].stride))break; \
            } \
            if(pool==7 || count>=16) {valid=0;break;} \
            nodes[count].pool=pool;nodes[count].index=(address-(uintptr_t)pools[pool].data)/pools[pool].stride;++count; \
            memcpy(&address,(const void *)address,sizeof(address)); \
        } \
        if(ordering[slot][layer].length!=0xAB)valid=0;HUD_BYTE(count); \
        for(unsigned i=0;i<count;++i) { \
            unsigned pool=nodes[i].pool;u8 *packet=(u8 *)pools[pool].data+nodes[i].index*pools[pool].stride; \
            HUD_BYTE(pools[pool].id);HUD_BYTE(nodes[i].index); \
            if(pools[pool].kind) { \
                for(unsigned byte=0;byte<12;++byte)HUD_BYTE(packet[sizeof(RenderNativeTag)+byte]); \
            } else { \
                for(unsigned byte=0;byte<4;++byte)HUD_BYTE(packet[sizeof(RenderNativeTag)+byte]); \
                for(unsigned byte=0;byte<16;++byte)HUD_BYTE(packet[sizeof(RenderTexturePagePacket)+sizeof(RenderNativeTag)+byte]); \
            } \
        } \
    } \
    for(unsigned byte=0;byte<4;++byte)HUD_BYTE((u32)g_BattleStatusPanelSlot>>(byte*8)); \
} while(0)
    for(unsigned slot=0;slot<2;++slot)for(unsigned anchor=0;anchor<3;++anchor)
    for(unsigned category=0;category<4;++category)for(unsigned mode=0;mode<4;++mode)for(unsigned left=0;left<11;++left) {
        HUD_SETUP(slot,anchor);action.attackWord=category<<20;action.turnWord=mode<<6;g_BattleRemainingActions=left;
        Battle_DrawSpellName();HUD_COLLECT(slot);
    }
    for(unsigned slot=0;slot<2;++slot)for(unsigned anchor=0;anchor<3;++anchor)for(unsigned which=0;which<9;++which) {
        HUD_SETUP(slot,anchor);Battle_DrawStatusSymbol(which);HUD_COLLECT(slot);
    }
    const s16 offsets[]={0,30,-32768,32767};
    for(unsigned slot=0;slot<2;++slot)for(unsigned anchor=0;anchor<3;++anchor)
    for(unsigned value=0;value<100;++value)for(unsigned offset=0;offset<4;++offset) {
        HUD_SETUP(slot,anchor);Battle_DrawStatusValue(value,offsets[offset]);HUD_COLLECT(slot);
    }
    const s16 values[]={-32768,-1,0,1,9,10,99,100,999,1000,9999,32767};
    const u8 timers[]={0,1,15,29,30,255};const u16 positions[][2]={{0,0},{123,234},{65535,32768}};
    for(unsigned slot=0;slot<2;++slot)for(unsigned panel=0;panel<10;++panel)for(unsigned mode=0;mode<2;++mode)
    for(unsigned style=0;style<4;++style)for(unsigned value=0;value<12;++value)
    for(unsigned timer=0;timer<6;++timer)for(unsigned position=0;position<3;++position) {
        HUD_SETUP(slot,position);g_BattleStatusPanelSlot=panel;
        BattleStatusPanel input={values[value],positions[position][0],positions[position][1],timers[timer],style};
        Battle_DrawStatusPanel(mode,&input);HUD_COLLECT(slot);
    }
    const int bottoms[]={0,24,28,32,40,127,128,255};
    for(unsigned slot=0;slot<2;++slot)for(unsigned anchor=0;anchor<3;++anchor)for(unsigned bottom=0;bottom<8;++bottom) {
        HUD_SETUP(slot,anchor);Battle_LayoutStatusPrimRow(bottoms[bottom]);HUD_COLLECT(slot);
    }
    const u8 countdowns[]={0,1,30,255};
    for(unsigned slot=0;slot<2;++slot)for(unsigned position=0;position<3;++position)for(unsigned timer=0;timer<4;++timer) {
        HUD_SETUP(slot,position);g_StatusMarkerPackets[slot].sprite.x=positions[position][0];
        g_StatusMarkerPackets[slot].sprite.y=positions[position][1];g_BattleSpritePrimCountdown=countdowns[timer];
        Draw_QueuePrimitive();HUD_COLLECT(slot);HUD_BYTE(g_BattleSpritePrimCountdown);
    }
#undef HUD_COLLECT
#undef HUD_BYTE
#undef HUD_SETUP
    g_ActiveActor=old_actor;g_ActiveDrawSlot=old_slot;g_BattleStatusPanelSlot=old_panel;g_BattleRemainingActions=old_remaining;g_BattleSpritePrimCountdown=old_countdown;
    D_800B0E38.ordering[0]=old_ordering[0];D_800B0E38.ordering[1]=old_ordering[1];
    if(!valid || hash!=UINT64_C(0xfa7672b3016c55b0)) {
        fprintf(stderr,"Original HUD update hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original HUD updates: 1,056 action lists, 54 symbols, 2,400 values and 34,560 floating/MISS panels, 48 layouts and 24 markers match unhooked USA MIPS");
    return 1;
}

static int original_hud_packet_initialization(void)
{
    struct {void *data;size_t bytes,stride;unsigned kind,count;} pools[]={
        {g_BattleStatusBarTiles,sizeof(g_BattleStatusBarTiles),sizeof(RenderColorTilePacket),3,6},
        {g_BattleStatusValueDigits,sizeof(g_BattleStatusValueDigits),sizeof(RenderPagedSpritePacket),0,4},
        {g_BattleTargetHighlightSprites,sizeof(g_BattleTargetHighlightSprites),sizeof(RenderPagedSpritePacket),0,6},
        {g_BattleStatusPointerSprites,sizeof(g_BattleStatusPointerSprites),sizeof(RenderPagedSpritePacket),0,2},
        {g_BattleStatusValueLabelSprites,sizeof(g_BattleStatusValueLabelSprites),sizeof(RenderPagedSpritePacket),0,2},
        {g_BattleAmmoLabelSprites,sizeof(g_BattleAmmoLabelSprites),sizeof(RenderPagedSpritePacket),0,2},
        {g_BattleAmmoDigitSprites,sizeof(g_BattleAmmoDigitSprites),sizeof(RenderPagedSpritePacket),0,8},
        {g_BattleStatusBackgrounds,sizeof(g_BattleStatusBackgrounds),sizeof(BattleModeTilePacket),1,2},
        {g_BattleAtbTracks,sizeof(g_BattleAtbTracks),sizeof(RenderColorTilePacket),3,2},
        {g_BattleAtbLabels,sizeof(g_BattleAtbLabels),sizeof(RenderPagedSpritePacket),0,2},
        {g_BattlePeLabels,sizeof(g_BattlePeLabels),sizeof(RenderPagedSpritePacket),0,2},
        {g_BattleAtbGauges,sizeof(g_BattleAtbGauges),sizeof(RenderGouraudQuad),2,2},
        {g_BattlePeGauges,sizeof(g_BattlePeGauges),sizeof(RenderGouraudQuad),2,4},
        {g_BattleAtbMarkers,sizeof(g_BattleAtbMarkers),sizeof(RenderPagedSpritePacket),0,2},
        {g_BattleHpDigits,sizeof(g_BattleHpDigits),sizeof(RenderPagedSpritePacket),0,8},
        {g_BattleMaxHpDigits,sizeof(g_BattleMaxHpDigits),sizeof(RenderPagedSpritePacket),0,10},
        {g_BattleHpLabels,sizeof(g_BattleHpLabels),sizeof(RenderPagedSpritePacket),0,2},
        {g_BattleRemainingActionSprites,sizeof(g_BattleRemainingActionSprites),sizeof(RenderPagedSpritePacket),0,20},
        {g_BattleStatusValueMarkers,sizeof(g_BattleStatusValueMarkers),sizeof(RenderPagedSpritePacket),0,2},
        {g_BattleStatusSymbolSprites,sizeof(g_BattleStatusSymbolSprites),sizeof(RenderPagedSpritePacket),0,2},
        {g_BattlePanelDigits,sizeof(g_BattlePanelDigits),sizeof(RenderPagedSpritePacket),0,100},
        {g_BattleEndPanels,sizeof(g_BattleEndPanels),sizeof(RenderTexturedQuad),4,2},
        {g_BattleStatusPointerLines,sizeof(g_BattleStatusPointerLines),sizeof(RenderLinePacket),5,4},
        {g_BattleStatusPointerTriangles,sizeof(g_BattleStatusPointerTriangles),sizeof(BattleStatusPointerPrim),6,2},
        {g_StatusIconPackets,sizeof(g_StatusIconPackets),sizeof(RenderPagedSpritePacket),0,26},
        {g_StatusMarkerPackets,sizeof(g_StatusMarkerPackets),sizeof(RenderPagedSpritePacket),0,2},
    };
    for(unsigned p=0;p<sizeof(pools)/sizeof(*pools);++p)memset(pools[p].data,0xA5,pools[p].bytes);
    Battle_DrawHPBar();
    uint64_t hash=UINT64_C(14695981039346656037);
    for(unsigned p=0;p<sizeof(pools)/sizeof(*pools);++p)for(unsigned i=0;i<pools[p].count;++i) {
        u8 *packet=(u8 *)pools[p].data+i*pools[p].stride;
        if(pools[p].kind<2) {
            RenderTexturePagePacket *page=(void *)packet;
            u8 *primitive=packet+sizeof(RenderTexturePagePacket);
            for(unsigned byte=0;byte<4;++byte)hash=(hash^((u8 *)&page->command)[byte])*UINT64_C(1099511628211);
            unsigned size=pools[p].kind==0?16:12;
            for(unsigned byte=0;byte<size;++byte)hash=(hash^primitive[sizeof(RenderNativeTag)+byte])*UINT64_C(1099511628211);
        } else {
            unsigned size=pools[p].kind==2?32:pools[p].kind==4?36:pools[p].kind==6?16:12;
            for(unsigned byte=0;byte<size;++byte)hash=(hash^packet[sizeof(RenderNativeTag)+byte])*UINT64_C(1099511628211);
        }
    }
    if(hash!=UINT64_C(0xfbe92ec193830469)) {
        fprintf(stderr,"Original HUD init hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    if(g_BattleAtbGauges[0].tag.length!=8 || g_BattleAtbTracks[1].tag.length!=3 ||
       g_BattleHpDigits[1][3].page.length!=(sizeof(RenderTexturePagePacket)+16)/4 || g_BattleStatusBackgrounds[1].mode.tag.length!=(sizeof(RenderDrawModePacket)+12)/4 ||
       g_BattleStatusPointerLines[1][1].tag.length!=3 || g_BattleStatusPointerTriangles[1].tag.length!=4)return 0;
    BattleHudOrigin old_origin=g_HudOrigin;unsigned old_mode=Menu_GetActiveMode();
    for(int mode=-512;mode<512;++mode) {
        BattleHudOrigin before=g_HudOrigin;unsigned before_mode=Menu_GetActiveMode();
        Battle_SetWindowBoundsByMode(mode);unsigned index=(u8)mode;
        if(index<4) {
            if(g_HudOrigin.x!=g_MenuWindowModeBounds.values[index*2] || g_HudOrigin.y!=g_MenuWindowModeBounds.values[index*2+1] || Menu_GetActiveMode()!=index)return 0;
        } else if(memcmp(&before,&g_HudOrigin,sizeof(before)) || Menu_GetActiveMode()!=before_mode)return 0;
    }
    Combatant actor={0};Combatant *old_actor=g_ActiveActor;int old_slot=g_ActiveDrawSlot;
    RenderNativeTag *old_ordering[2]={D_800B0E38.ordering[0],D_800B0E38.ordering[1]};
    RenderNativeTag ordering[2][8]={0};g_ActiveActor=&actor;
    for(unsigned slot=0;slot<2;++slot)D_800B0E38.ordering[slot]=ordering[slot];
    hash=UINT64_C(14695981039346656037);int valid=1;
    for(unsigned slot=0;slot<2;++slot)for(unsigned mode=0;mode<4;++mode)for(unsigned flags=0;flags<0x2000;++flags) {
        g_ActiveDrawSlot=slot;Battle_SetWindowBoundsByMode(mode);actor.stateFlags=flags;
        ordering[slot][7].address=0xFFFFFF;ordering[slot][7].length=0xAB;
        Draw_StatusIcons();
        unsigned icons[13],count=0;uintptr_t address=ordering[slot][7].address;
        uintptr_t start=(uintptr_t)&g_StatusIconPackets[slot][0];
        while(address!=0xFFFFFF) {
            if(count>=13 || address<start || address>=start+sizeof(g_StatusIconPackets[slot]) ||
               (address-start)%sizeof(RenderPagedSpritePacket)) {valid=0;break;}
            unsigned index=(address-start)/sizeof(RenderPagedSpritePacket);icons[count++]=index;
            address=g_StatusIconPackets[slot][index].page.address;
        }
        if(ordering[slot][7].length!=0xAB)valid=0;
        hash=(hash^count)*UINT64_C(1099511628211);
        for(unsigned i=0;i<count;++i) {
            hash=(hash^icons[i])*UINT64_C(1099511628211);
            const u8 *payload=(const u8 *)&g_StatusIconPackets[slot][icons[i]].sprite+sizeof(RenderNativeTag);
            for(unsigned byte=0;byte<16;++byte)hash=(hash^payload[byte])*UINT64_C(1099511628211);
        }
    }
    uint64_t icon_hash=hash;hash=UINT64_C(14695981039346656037);
    BattleEntity entity={0};entity.core=&actor;BattleEntity *old_entity=D_8009D254;g_PlayerEntity=(FieldActor *)(&entity);
    u32 old_flags=g_GameStateFlags;int old_context=g_MenuItemContextFlag,old_frame=g_BattleFrameCounter;
    const u16 hp_cases[][2]={{0,0},{1,0},{9,9},{10,9},{99,10},{100,99},{999,100},{1000,999},{9999,0},{9999,9999}};
    const u32 states[]={0,0x400,0x800,0xC00,0x1FFF};const int frames[]={0,16,17,31};
    const int energy[]={-1,50<<16,101<<16};const u16 charge[]={0,4500,10000};
    for(unsigned slot=0;slot<2;++slot)for(unsigned battle=0;battle<2;++battle)for(unsigned context=0;context<2;++context)
    for(unsigned mode=0;mode<4;++mode)for(unsigned gauge=0;gauge<3;++gauge)for(unsigned hp=0;hp<10;++hp)
    for(unsigned state=0;state<5;++state)for(unsigned frame=0;frame<4;++frame) {
        for(unsigned layer=0;layer<8;++layer)ordering[slot][layer]=(RenderNativeTag){0xFFFFFF,0xAB};
        g_ActiveDrawSlot=slot;g_GameStateFlags=battle*2;g_MenuItemContextFlag=context*2;g_BattleFrameCounter=frames[frame];
        actor.exp_or_acc=energy[gauge];actor.hpAlive=charge[gauge];actor.maxAtk=100<<16;
        actor.maxHP=hp_cases[hp][0];actor.curHP=hp_cases[hp][1];actor.stateFlags=states[state];
        Battle_SetWindowBoundsByMode(mode);Battle_DrawActiveStatus();
        for(unsigned layer=0;layer<8;++layer) {
            struct {unsigned pool,index;} nodes[40];unsigned count=0;uintptr_t address=ordering[slot][layer].address;
            while(address!=0xFFFFFF) {
                unsigned pool=0;
                for(;pool<sizeof(pools)/sizeof(*pools);++pool) {
                    uintptr_t base=(uintptr_t)pools[pool].data;
                    if(address>=base && address<base+pools[pool].bytes && !( (address-base)%pools[pool].stride))break;
                }
                if(pool==sizeof(pools)/sizeof(*pools) || count>=40) {valid=0;break;}
                nodes[count].pool=pool;nodes[count].index=(address-(uintptr_t)pools[pool].data)/pools[pool].stride;++count;
                memcpy(&address,(const void *)address,sizeof(address));
            }
            if(ordering[slot][layer].length!=0xAB)valid=0;
            hash=(hash^count)*UINT64_C(1099511628211);
            for(unsigned i=0;i<count;++i) {
                unsigned pool=nodes[i].pool;u8 *packet=(u8 *)pools[pool].data+nodes[i].index*pools[pool].stride;
                hash=(hash^pool)*UINT64_C(1099511628211);hash=(hash^nodes[i].index)*UINT64_C(1099511628211);
                if(pools[pool].kind<2) {
                    for(unsigned byte=0;byte<4;++byte)hash=(hash^packet[sizeof(RenderNativeTag)+byte])*UINT64_C(1099511628211);
                    unsigned size=pools[pool].kind==0?16:12;
                    for(unsigned byte=0;byte<size;++byte)hash=(hash^packet[sizeof(RenderTexturePagePacket)+sizeof(RenderNativeTag)+byte])*UINT64_C(1099511628211);
                } else {
                    unsigned size=pools[pool].kind==2?32:pools[pool].kind==4?36:pools[pool].kind==6?16:12;
                    for(unsigned byte=0;byte<size;++byte)hash=(hash^packet[sizeof(RenderNativeTag)+byte])*UINT64_C(1099511628211);
                }
            }
        }
        for(unsigned byte=0;byte<10;++byte)hash=(hash^((u8 *)&actor.exp_or_acc)[byte])*UINT64_C(1099511628211);
    }
    g_PlayerEntity=(FieldActor *)(old_entity);g_GameStateFlags=old_flags;g_MenuItemContextFlag=old_context;g_BattleFrameCounter=old_frame;
    g_ActiveActor=old_actor;g_ActiveDrawSlot=old_slot;
    D_800B0E38.ordering[0]=old_ordering[0];D_800B0E38.ordering[1]=old_ordering[1];
    Battle_SetWindowBoundsByMode(old_mode);g_HudOrigin=old_origin;
    /* Complete icon order and sprite payloads from 65,536 actual executions
     * of USA Draw_StatusIcons 800334AC, with every 13-bit status combination. */
    if(!valid || icon_hash!=UINT64_C(0x34a3ba0320ab74ad) || hash!=UINT64_C(0xc21dbda617e554d5)) {
        fprintf(stderr,"Original HUD icon/active hashes %016llx/%016llx differ from USA MIPS\n",(unsigned long long)icon_hash,(unsigned long long)hash);return 0;
    }
    puts("Original HUD: 226 initialized packets, 65,536 icon lists and 19,200 active HUD frames match USA MIPS");
    return 1;
}

static int original_enemy_death(void)
{
    extern void Battle_StepEntityAnimState(BattleEntity *);
    Combatant actor={0};BattleEntity entity,child;RenderObjectHeader header={0};
    Combatant *old_actor=g_ActiveActor;FieldActor *old_player=g_PlayerEntity,*old_head=g_FieldActorListHead;
    BattleEnemySlot old_slot=g_BattleEnemySlots[0];s8 old_live=g_BattleLiveEnemyCount;
    u32 old_exp=g_BattleExpReward;u16 old_pe=g_BattlePeBonusReward;int old_draw=g_ActiveDrawSlot;
    BattleRewardSlot old_rewards[10];memcpy(old_rewards,g_BattleRewardSlots,sizeof(old_rewards));
    int queued=g_AkaoMessageQueueCount;AkaoQueueEntry old_message=g_AkaoMessageQueue[0];AkaoStagedCommand staged=g_AkaoCmd;
    g_ActiveActor=&actor;g_PlayerEntity=NULL;g_FieldActorListHead=(FieldActor *)&entity;
    const u8 kinds[]={0,2},frames[]={0,7,8},fades[]={0,1,7,8,9};
    const u16 rewards[][3]={{9,2,3},{0,65535,65535},{65535,1,1}};
    uint64_t hash=UINT64_C(14695981039346656037);
#define HASH_DEATH(value,bytes) do { \
    u32 word=(u32)(value); \
    for(unsigned byte=0;byte<(bytes);++byte)hash=(hash^((word>>(byte*8))&255))*UINT64_C(1099511628211); \
} while(0)
    for(unsigned phase=0;phase<4;++phase)for(unsigned persist=0;persist<2;++persist)
    for(unsigned visible=0;visible<2;++visible)for(unsigned kind=0;kind<2;++kind)
    for(unsigned frame=0;frame<3;++frame)for(unsigned fade=0;fade<5;++fade)
    for(unsigned reward=0;reward<3;++reward)for(unsigned full=0;full<2;++full)
    for(unsigned aux=0;aux<2;++aux)for(unsigned slot=0;slot<2;++slot) {
        EnemyCombatant *core=&g_BattleEnemySlots[0].combatant;
        memset(core,0,sizeof(*core));memset(&entity,0,sizeof(entity));memset(&child,0,sizeof(child));
        entity.core=core;entity.next=&child;entity.entityFlags=0x240;entity.animFrame=frames[frame]<<16;
        entity.motionX=100;entity.motionY=101;entity.motionZ=102;child.parent=&entity;child.entityFlags=0x20;
        entity.renderObject.header=child.renderObject.header=&header;
        entity.renderObject.draw_count=child.renderObject.draw_count=1;entity.renderObject.variant_visible=visible;
        core->field04.bytes.kind=kinds[kind];core->slotBit=4;core->hpMirror=65535;core->deathAnimFrame=8;
        core->deathAnimPhase=phase;core->deathFadeStep=fades[fade];core->deathPersist=persist;
        core->rewardBase=rewards[reward][0];core->rewardFactorA=rewards[reward][1];core->rewardFactorB=rewards[reward][2];
        core->rewardSlotId=0x44;core->lootItemId=300;core->lootItemAux=aux?77:-1;
        for(unsigned i=0;i<10;++i)g_BattleRewardSlots[i]=(BattleRewardSlot){full?55:0,-1};
        g_BattleEnemySlots[0].active=1;g_BattleLiveEnemyCount=1;g_ActiveDrawSlot=slot;
        g_BattleExpReward=0xfffffff0;g_BattlePeBonusReward=65530;g_AkaoMessageQueueCount=0;
        memset(&g_AkaoMessageQueue[0],0,sizeof(g_AkaoMessageQueue[0]));
        Battle_StepEntityAnimState(&entity);
        HASH_DEATH(core->deathAnimPhase,1);HASH_DEATH(core->deathFadeStep,1);
        HASH_DEATH(entity.core==NULL,1);HASH_DEATH(g_BattleEnemySlots[0].active,4);
        HASH_DEATH((u8)g_BattleLiveEnemyCount,1);HASH_DEATH(g_BattleExpReward,4);HASH_DEATH(g_BattlePeBonusReward,2);
        BattleEntity *entities[]={&entity,&child};
        for(unsigned i=0;i<2;++i) {
            BattleEntity *e=entities[i];RenderObjectEntity *o=&e->renderObject;
            HASH_DEATH(e->entityFlags,4);HASH_DEATH(e->motionX,4);HASH_DEATH(e->motionY,4);HASH_DEATH(e->motionZ,4);
            HASH_DEATH((u8)o->fade_duration,1);HASH_DEATH(o->fade_red_step,1);HASH_DEATH(o->fade_green_step,1);
            HASH_DEATH(o->fade_blue_step,1);HASH_DEATH(o->primitive_red,1);HASH_DEATH(o->primitive_green,1);
            HASH_DEATH(o->primitive_blue,1);HASH_DEATH(o->flags_9C,2);
        }
        for(unsigned i=0;i<10;++i) {HASH_DEATH((u16)g_BattleRewardSlots[i].id,2);HASH_DEATH((u16)g_BattleRewardSlots[i].extra,2);}
        HASH_DEATH(g_AkaoMessageQueueCount,4);
        if(g_AkaoMessageQueueCount) {HASH_DEATH(g_AkaoMessageQueue[0].opcode.word,4);HASH_DEATH(g_AkaoMessageQueue[0].arg0.value,4);HASH_DEATH(g_AkaoMessageQueue[0].arg1,4);}
    }
#undef HASH_DEATH
    g_ActiveActor=old_actor;g_PlayerEntity=old_player;g_FieldActorListHead=old_head;g_BattleEnemySlots[0]=old_slot;
    g_BattleLiveEnemyCount=old_live;g_BattleExpReward=old_exp;g_BattlePeBonusReward=old_pe;g_ActiveDrawSlot=old_draw;
    memcpy(g_BattleRewardSlots,old_rewards,sizeof(old_rewards));g_AkaoMessageQueueCount=queued;g_AkaoMessageQueue[0]=old_message;g_AkaoCmd=staged;
    if(hash!=UINT64_C(0x85c5db394010ef9d)) {
        fprintf(stderr,"Original enemy death hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original enemy death: 11,520 phase/fade/child/visibility/reward cases match unhooked USA MIPS");
    return 1;
}

static int original_enemy_damage(void)
{
    Combatant actor={0};EnemyCombatant enemy;BattleEntity entity;BattleAction action={0};
    Combatant *old_actor=g_ActiveActor;u32 old_random=Psyz_RandState();s8 old_count=g_BattleTargetCount;
    void *old_animation=g_GameState.bank_rows[0][2];
    RenderAnimationDataHeader animation={0};animation.frame_count=9;
    g_GameState.bank_rows[0][2]=&animation;g_ActiveActor=&actor;g_BattleTargetCount=4;
    const u32 abilities[]={0,0x80000,0x100000},statuses[]={0,0x155555,0xaaaaaa,0xffffff};
    const u16 stats[]={25,100,9999,65535},shots[]={1,2,3,5,7,10};
    const u16 elements[]={0,0x100,0x200,0x300,0x400,0x800,0x1000,0x1f00},defenses[]={0,10,200,65535};
    const u32 flags[]={0,0x8000,0x18000,0x20000,0x80000,0xc0000,0x90000,0xd8000};
    const u32 seeds[]={0x12345678,0x87654321};
    actor.field04.fieldId04=99;actor.exp_or_acc=100<<16;actor.maxAtk=200<<16;actor.action=&action;
    action.powerBonus=75;action.actionCode.actionId=6;
    uint64_t hash=UINT64_C(14695981039346656037);
#define HASH_DAMAGE(value,bytes) do { \
    u32 word=(u32)(value); \
    for(unsigned byte=0;byte<(bytes);++byte)hash=(hash^((word>>(byte*8))&255))*UINT64_C(1099511628211); \
} while(0)
    for(unsigned ability=0;ability<3;++ability)for(unsigned stat=0;stat<4;++stat)
    for(unsigned shot=0;shot<6;++shot)for(unsigned element=0;element<8;++element)
    for(unsigned status=0;status<4;++status)for(unsigned flag=0;flag<8;++flag)
    for(unsigned seed=0;seed<2;++seed)for(unsigned defense=0;defense<4;++defense) {
        memset(&enemy,0,sizeof(enemy));memset(&entity,0,sizeof(entity));
        actor.stateFlags=abilities[ability];actor.stat1E=stats[stat];action.turnWord=shots[shot]|elements[element];
        entity.core=&enemy;entity.entityFlags=0x240;entity.actionMode=9;entity.animLastFrame=5;
        entity.animFrame=7<<16;entity.animPrev.fixed=3<<16;
        enemy.coreFlags=flags[flag];enemy.field04.bytes.kind=1;enemy.lowAndEntityId.bytes.low=2;
        enemy.hpAlive=200000;enemy.defense=defenses[defense];enemy.statusFlags2=statuses[status];
        Psyz_Srand(seeds[seed]);Battle_StepEnemyMovement(&entity);
        HASH_DAMAGE(enemy.coreFlags,4);HASH_DAMAGE(enemy.statusFlags2,4);HASH_DAMAGE(enemy.hpAlive,4);
        HASH_DAMAGE(entity.entityFlags,4);HASH_DAMAGE(entity.actionMode,1);HASH_DAMAGE(entity.animLastFrame,1);
        HASH_DAMAGE(entity.animFrame,4);HASH_DAMAGE(entity.animPrev.fixed,4);
        HASH_DAMAGE(entity.actionData==&animation,1);HASH_DAMAGE(Psyz_RandState(),4);
    }
#undef HASH_DAMAGE
    g_ActiveActor=old_actor;g_GameState.bank_rows[0][2]=old_animation;g_BattleTargetCount=old_count;Psyz_Srand(old_random);
    if(hash!=UINT64_C(0x19758ebd45a2608d)) {
        fprintf(stderr,"Original enemy damage hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original enemy damage: 147,456 stat/PE/shot/resistance/RNG cases match USA MIPS including real soft-float callees");
    return 1;
}

static int original_enemy_facing(void)
{
    Combatant actor={0};EnemyCombatant enemy;BattleEntity entity,player;
    Combatant *old_actor=g_ActiveActor;FieldActor *old_player=g_PlayerEntity;
    void *old_animations[]={g_GameState.bank_rows[0][0],g_GameState.bank_rows[0][1]};
    RenderAnimationDataHeader animation={0};animation.frame_count=9;
    g_GameState.bank_rows[0][0]=g_GameState.bank_rows[0][1]=&animation;
    g_ActiveActor=&actor;g_PlayerEntity=(FieldActor *)&player;
    const s16 positions[][2]={{100,100},{-100,100},{100,-100},{-100,-100},{0,0}};
    const s16 yaws[]={-32768,-1,0,1024,2048,4095,32767};
    uint64_t hash=UINT64_C(14695981039346656037);
#define HASH_FACING(value,bytes) do { \
    u32 word=(u32)(value); \
    for(unsigned byte=0;byte<(bytes);++byte)hash=(hash^((word>>(byte*8))&255))*UINT64_C(1099511628211); \
} while(0)
    for(unsigned kind=0;kind<2;++kind)for(unsigned resume=0;resume<3;++resume)
    for(unsigned motion=0;motion<4;++motion)for(unsigned ability=0;ability<2;++ability)
    for(unsigned event=0;event<2;++event)for(unsigned selected=0;selected<6;++selected)
    for(unsigned knockback=0;knockback<2;++knockback)for(unsigned position=0;position<5;++position)
    for(unsigned yaw=0;yaw<7;++yaw) {
        memset(&enemy,0,sizeof(enemy));memset(&entity,0,sizeof(entity));memset(&player,0,sizeof(player));
        actor.stateFlags=ability*0x80000;entity.core=&enemy;entity.entityFlags=0x240;
        entity.actionMode=9;entity.animLastFrame=5;entity.animFrame=7<<16;entity.animPrev.fixed=3<<16;entity.animStep=0x18000;
        enemy.coreFlags=(selected<<21)|(knockback*0x100000);enemy.field04.bytes.kind=kind;
        enemy.records[selected].state=event;enemy.resumePhase=resume;
        enemy.motionPhase=motion;enemy.motionFrames=7;enemy.motionAmplitude=123;enemy.motionAngle=-100;
        player.posX.fixed=(s32)positions[position][0]*65536;player.posZ.fixed=(s32)positions[position][1]*65536;
        player.facingAngle=yaws[yaw];Battle_UpdateEntityFacing(&entity);
        HASH_FACING(enemy.resumePhase,1);HASH_FACING(enemy.savedActionMode,1);HASH_FACING(enemy.savedInterpolation,1);
        HASH_FACING(enemy.savedAnimFrame,4);HASH_FACING(enemy.savedAnimPrev,4);HASH_FACING(enemy.savedAnimStep,4);
        HASH_FACING(enemy.motionPhase,1);HASH_FACING(enemy.motionFrames,1);
        HASH_FACING(enemy.motionAmplitude,2);HASH_FACING((u16)enemy.motionAngle,2);HASH_FACING(entity.entityFlags,4);
        HASH_FACING(entity.actionMode,1);HASH_FACING(entity.animLastFrame,1);HASH_FACING(entity.animFrame,4);
        HASH_FACING(entity.animPrev.fixed,4);HASH_FACING(entity.animStep,4);
        HASH_FACING(entity.posX.fixed,4);HASH_FACING(entity.posZ.fixed,4);HASH_FACING(entity.actionData==&animation,1);
    }
#undef HASH_FACING
    g_ActiveActor=old_actor;g_PlayerEntity=old_player;
    g_GameState.bank_rows[0][0]=old_animations[0];g_GameState.bank_rows[0][1]=old_animations[1];
    if(hash!=UINT64_C(0x6496fbe76c786105)) {
        fprintf(stderr,"Original enemy facing hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original enemy hit reaction: 40,320 kind/record/phase/position/facing cases match unhooked USA MIPS");
    return 1;
}

static int original_enemy_slots(PeDisc *disc,PeDiscFile *exe)
{
    extern void Battle_SlotFree(BattleEntity *);
    u8 original[0xD8],native[0xD8];
    if(!pe_disc_read(disc,exe,0x109b0-0x10000+0x800,original,sizeof(original)))return 0;
    memcpy(native,&g_EnemyCombatantInitial,0x18);memset(native+0x18,0,4);
    memcpy(native+0x1c,(const u8 *)&g_EnemyCombatantInitial+PE1_OFFSETOF(EnemyCombatant,records),0xD8-0x1c);
    if(memcmp(native,original,sizeof(native)) || g_EnemyCombatantInitial.effect ||
       g_EnemyCombatantInitial.action || g_EnemyCombatantInitial.attributes || g_EnemyCombatantInitial.field70)return 0;
    BattleEnemySlot old_slots[7];memcpy(old_slots,g_BattleEnemySlots,sizeof(old_slots));
    u8 old_counter=g_EntityInstanceCounter;s8 old_live=g_BattleLiveEnemyCount;
    void *old_animation=g_GameState.bank_rows[0][2];
    RenderAnimationDataHeader animation={0};animation.frame_count=9;
    g_GameState.bank_rows[0][2]=&animation;
    BattleEntity actors[8];const u8 starts[]={0,127,254,255};
    uint64_t hash=UINT64_C(14695981039346656037);
#define HASH_SLOT(value,bytes) do { \
    u32 word=(u32)(value); \
    for(unsigned byte=0;byte<(bytes);++byte)hash=(hash^((word>>(byte*8))&255))*UINT64_C(1099511628211); \
} while(0)
#define HASH_SLOT_ACTOR(index) do { \
    BattleEntity *actor=&actors[index];EnemyCombatant *core=actor->core; \
    if(core) { \
        memcpy(native,core,0x18);u32 pointer=(core->effect==&core->records[0]);memcpy(native+0x18,&pointer,4); \
        memcpy(native+0x1c,(const u8 *)core+PE1_OFFSETOF(EnemyCombatant,records),0xD8-0x1c); \
        for(unsigned byte=0;byte<sizeof(native);++byte)HASH_SLOT(native[byte],1); \
    } \
    HASH_SLOT(core!=NULL,1);HASH_SLOT(actor->entityFlags,4);HASH_SLOT(actor->actionMode,1); \
    HASH_SLOT(actor->animLastFrame,1);HASH_SLOT(actor->animFrame,4);HASH_SLOT(actor->animPrev.fixed,4); \
    HASH_SLOT(actor->actionData==&animation,1);HASH_SLOT(g_EntityInstanceCounter,1);HASH_SLOT((u8)g_BattleLiveEnemyCount,1); \
} while(0)
    for(unsigned mask=0;mask<128;++mask)for(unsigned start=0;start<4;++start) {
        Battle_ClearMotionTable();g_EntityInstanceCounter=starts[start];g_BattleLiveEnemyCount=0;
        memset(actors,0,sizeof(actors));
        for(unsigned i=0;i<8;++i) {
            actors[i].entityFlags=0x240|(((mask>>i)&1)*0x2000);actors[i].actionMode=9;actors[i].animLastFrame=5;
            actors[i].animFrame=7<<16;actors[i].animPrev.fixed=3<<16;
            Entity_AllocSlot(&actors[i]);HASH_SLOT_ACTOR(i);
        }
        for(unsigned i=1;i<7;i+=2)Battle_SlotFree(&actors[i]);
        for(unsigned i=0;i<7;++i) {HASH_SLOT(g_BattleEnemySlots[i].active,4);HASH_SLOT(actors[i].core!=NULL,1);}
        for(unsigned i=1;i<7;i+=2) {
            actors[i].entityFlags^=0x2000;Entity_AllocSlot(&actors[i]);HASH_SLOT_ACTOR(i);
        }
        Battle_ClearMotionTable();
        for(unsigned i=0;i<7;++i) {HASH_SLOT(g_BattleEnemySlots[i].active,4);HASH_SLOT(actors[i].core!=NULL,1);}
    }
#undef HASH_SLOT_ACTOR
#undef HASH_SLOT
    memcpy(g_BattleEnemySlots,old_slots,sizeof(old_slots));g_EntityInstanceCounter=old_counter;g_BattleLiveEnemyCount=old_live;
    g_GameState.bank_rows[0][2]=old_animation;
    if(hash!=UINT64_C(0x81c2f5af25011535)) {
        fprintf(stderr,"Original enemy-slot hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original enemy slots: complete template and 5,632 allocations, exhaustion, reuse, ID wrapping and clears match unhooked USA MIPS");
    return 1;
}

static int original_player_turn(void)
{
    extern void Battle_UpdatePlayerTurn(void);
    const u8 modes[]={0,4,6,7,8,9,10,11,12,13,15,17};
    const s16 heights[]={-900,-1,0,1,900};
    Combatant actor={0};EnemyCombatant enemy={0},dead_enemy={0};BattleEntity player,target,dead_target={0};
    dead_target.core=&dead_enemy;
    const u8 frames[]={0,7,8},indices[]={0,3,11,42};
    BattleAction action={0};RenderAnimationDataHeader animation={0};animation.frame_count=9;
    void *old_animations[48];memcpy(old_animations,g_GameState.bank_rows[0],sizeof(old_animations));
    BattleInitSlot old_queue[45];memcpy(old_queue,g_BattleActionQueue,sizeof(old_queue));
    Combatant *old_actor=g_ActiveActor;FieldActor *old_player=g_PlayerEntity;
    u32 old_flags=g_GameStateFlags;u8 old_count=g_BattleActionQueueCount,old_index=g_BattleActionIndex;
    u8 old_remaining=g_BattleRemainingActions,old_hit_timer=g_BattleHitTimer;
    u8 old_modes[4];memcpy(old_modes,g_BattleActionAnimModes,sizeof(old_modes));
    int old_attack=g_BattleAttackPmSlot,old_impact=g_BattleImpactPmSlot;
    s16 old_height=g_BattlePlayerAimHeight;
    for(unsigned i=0;i<48;++i)g_GameState.bank_rows[0][i]=&animation;
    g_ActiveActor=&actor;g_PlayerEntity=(FieldActor *)&player;
    action.actionCode.actionId=8;action.turnWord=1;actor.action=&action;
    actor.actionMode12=4;actor.actionMode13=5;
    actor.targetAngleActionModes[0]=6;actor.targetAngleActionModes[1]=8;actor.targetAngleActionModes[2]=10;
    enemy.hpAlive=100;
    uint64_t hash=UINT64_C(14695981039346656037);
#define HASH_TURN(value,bytes) do { \
    u32 word=(u32)(value); \
    for(unsigned byte=0;byte<(bytes);++byte)hash=(hash^((word>>(byte*8))&255))*UINT64_C(1099511628211); \
} while(0)
    for(unsigned mode=0;mode<12;++mode)for(unsigned phase=0;phase<3;++phase)
    for(unsigned height=0;height<5;++height)for(unsigned committed=0;committed<2;++committed)
    for(unsigned position=0;position<4;++position)for(unsigned next_kind=0;next_kind<2;++next_kind) {
        unsigned index=indices[position];
        memset(&player,0,sizeof(player));memset(&target,0,sizeof(target));
        actor.stateFlags=committed*0x200000;player.core=&actor;target.core=&enemy;
        player.actionMode=modes[mode];player.animLastFrame=8;player.animFrame=frames[phase]<<16;
        player.animPrev.fixed=(modes[mode]==12?8:frames[phase])<<16;player.entityFlags=0x240;
        player.posX.fixed=100<<16;player.posZ.fixed=-(20<<16);
        target.renderObject.target_x=200;target.renderObject.target_y=heights[height];target.renderObject.target_z=-200;
        memset(g_BattleActionQueue,0,sizeof(g_BattleActionQueue));
        g_BattleActionQueue[index]=(BattleInitSlot){&target,1,0};
        g_BattleActionQueue[index+1]=(BattleInitSlot){next_kind?&dead_target:NULL,next_kind?1:3,0};
        g_BattleActionIndex=index;g_BattleActionQueueCount=index+1+next_kind;
        for(unsigned i=0;i<4;++i)g_BattleActionAnimModes[i]=4+i;
        g_GameStateFlags=2;g_BattleAttackPmSlot=g_BattleImpactPmSlot=-1;g_BattlePlayerAimHeight=17;
        Battle_UpdatePlayerTurn();
        HASH_TURN(player.actionMode,1);HASH_TURN(player.animLastFrame,1);
        HASH_TURN(player.animFrame,4);HASH_TURN(player.animPrev.fixed,4);
        HASH_TURN((u16)player.facingAngle,2);HASH_TURN(player.entityFlags,4);
        HASH_TURN(actor.stateFlags,4);HASH_TURN(g_BattleActionIndex,1);
        for(unsigned i=0;i<4;++i)HASH_TURN(g_BattleActionAnimModes[i],1);
        HASH_TURN(player.actionData==&animation,1);
    }
#undef HASH_TURN
    memcpy(g_GameState.bank_rows[0],old_animations,sizeof(old_animations));
    memcpy(g_BattleActionQueue,old_queue,sizeof(old_queue));
    g_ActiveActor=old_actor;g_PlayerEntity=old_player;g_GameStateFlags=old_flags;
    g_BattleActionQueueCount=old_count;g_BattleActionIndex=old_index;
    g_BattleRemainingActions=old_remaining;g_BattleHitTimer=old_hit_timer;
    memcpy(g_BattleActionAnimModes,old_modes,sizeof(old_modes));
    g_BattleAttackPmSlot=old_attack;g_BattleImpactPmSlot=old_impact;g_BattlePlayerAimHeight=old_height;
    if(hash!=UINT64_C(0x782dd423afe0eac1)) {
        fprintf(stderr,"Original player turn hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original player turn: 2,880 animation/aim/committed-state/queue cases match unhooked USA MIPS");
    return 1;
}

static int original_combat_angles(void)
{
    extern int Battle_CalcRelativeAngle(BattleEntity *,BattleEntity *);
    const s32 coords[]={0,1,-1,65536,-65536,0x3fffffff,-0x3fffffff,0x12345678,-0x12345678};
    const s16 yaws[]={-32768,-8193,-4096,-1,0,1,1023,2048,4095,4096,8193,32767};
    BattleEntity target={0},viewer={0};uint64_t hash=UINT64_C(14695981039346656037);
    for(unsigned x=0;x<9;++x)for(unsigned z=0;z<9;++z)for(unsigned offset=0;offset<9;++offset)
    for(unsigned yaw=0;yaw<12;++yaw) {
        target.posX.fixed=coords[x];target.posZ.fixed=coords[z];
        viewer.posX.fixed=coords[offset];viewer.posZ.fixed=-coords[offset];viewer.facingAngle=yaws[yaw];
        u16 result=Battle_CalcRelativeAngle(&target,&viewer);
        for(unsigned byte=0;byte<2;++byte)hash=(hash^((result>>(byte*8))&255))*UINT64_C(1099511628211);
    }
    /* Unhooked USA 800305C8 and its real SDK ratan2 callee. */
    if(hash!=UINT64_C(0x37f5f1d4d13308e7)) {
        fprintf(stderr,"Original battle angle hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original relative battle angle: 8,748 fixed-position/facing cases match USA MIPS");
    return 1;
}

/* Actual USA Entity_UpdateTimers 8001F9C4, including its real poison and
 * animation callees, supplies this oracle. Auto medicine/cure are excluded here. */
static int original_combat_timers(void)
{
    extern void Entity_UpdateTimers(void);
    extern unsigned char g_AyaStatusEffectTimer;
    const u16 times[]={0,1,32767,32768,65535,3,50,9000};
    const u16 steps[]={0,1,2,32767,32768,65535,50,9001};
    const u16 hp[]={0,1,50,200,65535,32767,32768,199};
    const u32 energy[]={0,65536,65537,200000,5000000,0x7fffffff,0x80000000,0xffffffff};
    const u8 transient[]={0,1,127,128,255,3,50,90};
    Combatant actor;BattleEntity player;BattleAttributes attributes={0};
    RenderAnimationDataHeader animation={0};animation.frame_count=9;
    void *old_animation=g_GameState.bank_rows[0][4];
    Combatant *old_actor=g_ActiveActor;FieldActor *old_player=g_PlayerEntity;
    u32 old_flags=g_GameStateFlags;int old_slot=g_ActiveDrawSlot,old_lock=D_8009D2E8;
    u16 old_haste=g_ActorEffectFlag100Timer;u8 old_transient=g_AyaStatusEffectTimer;
    uint64_t hash=UINT64_C(14695981039346656037);
    g_GameState.bank_rows[0][4]=&animation;g_ActiveActor=&actor;
    g_PlayerEntity=(FieldActor *)&player;
#define HASH_TIMER(value,bytes) do { \
    u32 word=(u32)(value); \
    for(unsigned byte=0;byte<(bytes);++byte)hash=(hash^((word>>(byte*8))&255))*UINT64_C(1099511628211); \
} while(0)
    for(unsigned low=0;low<256;++low)for(unsigned buffs=0;buffs<16;++buffs)
    for(unsigned slot=0;slot<2;++slot)for(unsigned phase=0;phase<8;++phase) {
        memset(&actor,0,sizeof(actor));memset(&player,0,sizeof(player));
        actor.attributes=&attributes;actor.field04.fieldId04=2;
        actor.exp_or_acc=(s32)energy[phase];actor.curHP=hp[phase];actor.hpMirror=198;
        actor.maxHP=200;actor.maxAtk=100<<16;
        actor.stateFlags=low|((buffs&7)<<8)|((buffs>>3)<<24);
        actor.actionMode12=4;actor.subActionStep=3;actor.subActionCounter=(phase&1)?3:2;
        actor.subActionPeriod=3;actor.statusStep3C=actor.statusStep3E=steps[phase];
        actor.statusTimer40=actor.statusTimer42=actor.statusTimer44=actor.statusTimer46=times[phase];
        player.core=&actor;player.actionMode=(phase&1)?8:17;player.animLastFrame=5;
        player.animFrame=7<<16;player.animPrev.fixed=3<<16;player.entityFlags=0x240;
        player.motionX=100;player.motionY=-200;player.motionZ=300;
        player.renderObject.projected_x=123;player.renderObject.projected_y=234;
        g_GameStateFlags=2;D_8009D2E8=0x1234;g_ActorEffectFlag100Timer=times[phase];
        g_AyaStatusEffectTimer=transient[phase];g_ActiveDrawSlot=slot;
        Entity_UpdateTimers();
        HASH_TIMER(actor.stateFlags,4);HASH_TIMER(actor.curHP,2);HASH_TIMER(actor.hpMirror,2);
        HASH_TIMER(actor.exp_or_acc,4);HASH_TIMER(actor.subActionCounter,1);
        HASH_TIMER(actor.statusTimer40,2);HASH_TIMER(actor.statusTimer42,2);
        HASH_TIMER(actor.statusTimer44,2);HASH_TIMER(actor.statusTimer46,2);
        HASH_TIMER(g_ActorEffectFlag100Timer,2);HASH_TIMER(g_AyaStatusEffectTimer,1);HASH_TIMER(D_8009D2E8,4);
        HASH_TIMER(player.motionX,4);HASH_TIMER(player.motionY,4);HASH_TIMER(player.motionZ,4);
        HASH_TIMER(player.actionMode,1);HASH_TIMER(player.animLastFrame,1);
        HASH_TIMER(player.animFrame,4);HASH_TIMER(player.animPrev.fixed,4);
        HASH_TIMER(player.actionData==&animation,1);
        HASH_TIMER(actor.panelAux_val,2);HASH_TIMER(actor.panelAux_x,2);HASH_TIMER(actor.panelAux_y,2);
        HASH_TIMER(actor.panelAux_timer,1);HASH_TIMER(actor.panelAux_mode,1);
    }
#undef HASH_TIMER
    g_GameState.bank_rows[0][4]=old_animation;g_ActiveActor=old_actor;g_PlayerEntity=old_player;
    g_GameStateFlags=old_flags;g_ActiveDrawSlot=old_slot;D_8009D2E8=old_lock;
    g_ActorEffectFlag100Timer=old_haste;g_AyaStatusEffectTimer=old_transient;
    if(hash!=UINT64_C(0xa1f411039c73e485)) {
        fprintf(stderr,"Original combat timer hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original combat timers: 65,536 status/buff/phase cases match USA MIPS");
    return 1;
}

static int original_spell_effects(PeDisc *disc,PeDiscFile *executable)
{
    u8 effect_ids[20];
    if(!pe_disc_read(disc,executable,0x1074c-0x10000+0x800,effect_ids,sizeof(effect_ids)) ||
       memcmp(effect_ids,g_BattleActionEffectIds.bytes,sizeof(effect_ids)))return 0;
    u8 sound_ids[40];
    if(!pe_disc_read(disc,executable,0x10760-0x10000+0x800,sound_ids,sizeof(sound_ids)) ||
       memcmp(sound_ids,&g_BattleActionSoundIds,sizeof(sound_ids)))return 0;
    Combatant actor={0};EnemyCombatant target={0};BattleEntity entity={0};BattleAttributes attributes={0};
    RenderAnimationDataHeader animation={0};animation.frame_count=9;
    void *old_animation=g_GameState.bank_rows[0][4];g_GameState.bank_rows[0][4]=&animation;
    Combatant *old_actor=g_ActiveActor;g_ActiveActor=&actor;
    u32 old_flags=g_GameStateFlags,old_random=Psyz_RandState();int old_lock=D_8009D2E8;
    u16 old_timer=g_ActorEffectFlag100Timer;SavePromptBlock old_scan=g_BattleScanPanel;
    const u16 hp[][2]={{45,0},{45,44},{9999,9999},{45,65530}};
    const s32 energy[]={0,100<<16,INT32_MIN};
    const u32 pairs[]={0,0xAAAAA,0x55555,0xFFFFF},seeds[]={0x12345678,0x87654321};
    uint64_t hash=UINT64_C(14695981039346656037);
#define HASH_EFFECT(value,bytes) do { \
    u32 word=(u32)(value); \
    for(unsigned byte=0;byte<(bytes);++byte)hash=(hash^((word>>(byte*8))&255))*UINT64_C(1099511628211); \
} while(0)
    for(unsigned spell=0;spell<20;++spell)for(unsigned status=0;status<256;++status)
    for(unsigned reduced=0;reduced<2;++reduced)for(unsigned pair=0;pair<4;++pair)for(unsigned seed=0;seed<2;++seed) {
        memset(&actor,0,sizeof(actor));memset(&target,0,sizeof(target));memset(&entity,0,sizeof(entity));
        attributes.effectFlags=reduced*0x200;actor.attributes=&attributes;
        actor.maxHP=hp[status/64][0];actor.curHP=hp[status/64][1];actor.exp_or_acc=energy[status%3];
        actor.maxAtk=100<<16;actor.stateFlags=0x12341000|status;
        target.coreFlags=0xA5A55A5A;target.statusFlags2=pairs[pair];entity.core=&target;
        entity.entityFlags=0x240;entity.actionMode=1;entity.animFrame=7<<16;entity.animPrev.fixed=3<<16;
        g_GameStateFlags=2;g_BattleResultFlags=0xA5A55A5A;D_8009D2E8=0x1234;
        g_ActorEffectFlag100Timer=77;g_BattleActiveEntity=NULL;Psyz_Srand(seeds[seed]);
        Battle_ApplySpellEffect(spell,&entity);
        HASH_EFFECT(actor.curHP,2);HASH_EFFECT(actor.exp_or_acc,4);HASH_EFFECT(actor.stateFlags,4);
        HASH_EFFECT(target.coreFlags,4);HASH_EFFECT(g_BattleResultFlags,4);HASH_EFFECT(D_8009D2E8,4);
        HASH_EFFECT(g_ActorEffectFlag100Timer,2);HASH_EFFECT(g_BattleActiveEntity==&entity,1);
        HASH_EFFECT(entity.entityFlags,4);HASH_EFFECT(entity.actionMode,1);HASH_EFFECT(entity.animLastFrame,1);
        HASH_EFFECT(entity.animFrame,4);HASH_EFFECT(entity.animPrev.fixed,4);HASH_EFFECT(entity.actionData==&animation,1);
        HASH_EFFECT(Psyz_RandState(),4);
    }
#undef HASH_EFFECT
    g_GameState.bank_rows[0][4]=old_animation;g_ActiveActor=old_actor;g_GameStateFlags=old_flags;
    D_8009D2E8=old_lock;g_ActorEffectFlag100Timer=old_timer;g_BattleScanPanel=old_scan;Psyz_Srand(old_random);
    /* Actual USA 80024250 and Entity_SetActionMode 8001A680, with only the
     * PS1 BIOS rand service emulated. Animation input is a valid 9-frame header. */
    if(hash!=UINT64_C(0x8ac4e547f4235e4d)) {
        fprintf(stderr,"Original PE effects hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original PE effects: 81,920 cases for all 20 commands match USA MIPS, including Scan, Confuse animation and PE cost");
    return 1;
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

static unsigned unload_log[32], unload_count;
#define UNLOAD_PROBE(id) static void unload_probe_##id(void) { unload_log[unload_count++]=id; }
UNLOAD_PROBE(0) UNLOAD_PROBE(1) UNLOAD_PROBE(2) UNLOAD_PROBE(3)
UNLOAD_PROBE(4) UNLOAD_PROBE(5) UNLOAD_PROBE(6) UNLOAD_PROBE(7)
UNLOAD_PROBE(8) UNLOAD_PROBE(84) UNLOAD_PROBE(85)
#undef UNLOAD_PROBE

static int original_scene_teardown(void)
{
    struct Archive { SceneAssetBlob header;SceneAssetDirectory directory;SceneAssetRecord entries[8]; } archive={0};
    archive.header.directoryOffset=offsetof(struct Archive,directory);
    archive.directory.entries=offsetof(struct Archive,entries) | 8u<<22;
    const u8 ids[]={0,7,8,84,85,191,255,8};
    for(unsigned i=0;i<8;++i)archive.entries[i].handlerId=ids[i];
    PmCommand handlers[86]={{0}};PmCommand *table[86];
    void (*probes[])(void)={unload_probe_0,unload_probe_1,unload_probe_2,unload_probe_3,
        unload_probe_4,unload_probe_5,unload_probe_6,unload_probe_7};
    for(unsigned i=0;i<8;++i)handlers[i].unload=probes[i];
    handlers[8].unload=unload_probe_8;handlers[84].unload=unload_probe_84;
    handlers[85].unload=unload_probe_85;
    PmTables saved=g_PmTables;u32 flags=g_GameStateFlags;
    SceneAssetView *loaded=g_GameState.loaded_scene_assets;
    void *programs[104];memcpy(programs,g_FieldFxTaskPrograms,sizeof(programs));
    int valid=1;
    for(unsigned mode=0;mode<4;++mode) {
        for(unsigned i=0;i<86;++i)table[i]=&handlers[i];
        for(unsigned i=0;i<104;++i)g_FieldFxTaskPrograms[i]=&table[i%86];
        g_PmTables.handlers=mode==3?NULL:table;
        g_GameStateFlags=0x31415900u | (mode?0x80:0);
        g_GameState.loaded_scene_assets=mode==2?NULL:(SceneAssetView *)&archive;
        unload_count=0;
        int result=Asset_UnloadTableEntries();
        if(mode>=2) {
            if(result!=-1 || unload_count || g_GameStateFlags!=0x31415980u)valid=0;
        } else {
            const unsigned expected[]={0,1,2,3,4,5,6,7,85,8,84,8};
            if(result || g_GameStateFlags!=0x31415900u ||
               unload_count!=(mode?12u:0u) ||
               (mode && memcmp(unload_log,expected,sizeof(expected))))valid=0;
        }
        for(unsigned i=0;i<86;++i)
            if(table[i]!=(mode<2 && i>=8 && i<85?NULL:&handlers[i]))valid=0;
        for(unsigned i=0;i<104;++i)
            if(g_FieldFxTaskPrograms[i]!=(mode<2 && i>=30?NULL:&table[i%86]))valid=0;
    }
    g_PmTables=saved;g_GameStateFlags=flags;g_GameState.loaded_scene_assets=loaded;
    memcpy(g_FieldFxTaskPrograms,programs,sizeof(programs));
    if(!valid)fprintf(stderr,"original scene teardown regression\n");
    else puts("Original scene teardown: native callback order, duplicate/boundary IDs, retained resident handlers, task ranges and missing-state guards passed");
    return valid;
}

static int original_boot_display_data(PeDisc *disc)
{
    Pe1GameState saved=g_GameState;PeDisc *old_image=g_CdImage;
    u8 *workspace=g_CdLoadAreaBase;u16 first=g_PeImageSectorTable[71],last=g_PeImageSectorTable[72];
    int queued=g_AkaoMessageQueueCount,valid=1;PeDiscFile archive;u8 expected[2048];
    g_CdImage=disc;
    if(Asset_OpenPeImage() || !pe_disc_find(disc,"PE.IMG",&archive))return 0;
    g_GameState.flags&=~0x1000000u;
    memset(workspace,0xA5,BOOT_DISPLAY_WORK_SIZE);
    glyph_pending=glyph_poll_retry=0;
    if(CdRom_LoadBootAudio())valid=0;
    for(unsigned sector=first;sector<last;++sector) {
        if(!pe_disc_read(disc,&archive,(uint64_t)sector*2048,expected,sizeof(expected)) ||
           memcmp(workspace+(sector-first)*2048,expected,sizeof(expected)))valid=0;
    }
    for(unsigned byte=(last-first)*2048;byte<BOOT_DISPLAY_WORK_SIZE;++byte)
        if(workspace[byte]!=0xA5)valid=0;
    if(g_GameState.scene_audio.tracks.keys[0][0]!=-1 || g_GameState.scene_audio.tracks.keys[0][1]!=-1 ||
       g_GameState.scene_audio.tracks.keys[1][0]!=-1 || g_GameState.scene_audio.tracks.keys[1][1]!=-1 ||
       g_GameState.scene_audio.tracks.banks[0]!=-1 || g_GameState.scene_audio.tracks.banks[1]!=-1)valid=0;
    g_GameState.flags&=~0x1000000u;glyph_poll_retry=1;
    if(CdRom_LoadBootAudio()!=-2)valid=0;
    glyph_poll_retry=0;g_CdImage=NULL;g_GameState.flags&=~0x1000000u;
    if(CdRom_LoadBootAudio()!=-1)valid=0;
    g_CdImage=disc;
    g_CdLoadAreaBase=NULL;if(CdRom_LoadBootAudio()!=-3)valid=0;g_CdLoadAreaBase=workspace;
    g_PeImageSectorTable[72]=first;if(CdRom_LoadBootAudio()!=-3)valid=0;
    g_PeImageSectorTable[72]=first-1;if(CdRom_LoadBootAudio()!=-3)valid=0;
    g_PeImageSectorTable[72]=first+BOOT_DISPLAY_WORK_SIZE/2048+1;if(CdRom_LoadBootAudio()!=-3)valid=0;
    g_PeImageSectorTable[72]=last;g_GameState.pe_image_base_lba=UINT32_MAX;
    if(CdRom_LoadBootAudio()!=-3)valid=0;
    g_GameState=saved;g_CdImage=old_image;g_AkaoMessageQueueCount=queued;
    glyph_polls=0;glyph_pending=glyph_poll_retry=0;
    if(valid)puts("Original boot display data: full archive range and scratch guards match Disc 1; I/O errors and invalid extents terminate without loading code");
    return valid;
}

static int original_image_display_switch(PeDisc *disc,PeDiscFile *exe)
{
    PeDisc *previous=g_CdImage;Pe1GameState saved=g_GameState;
    static u16 pixels[1024*512],after[1024*512];RECT area={0,0,1024,512};
    for(unsigned i=0;i<1024*512;++i)pixels[i]=(u16)(i*13+0x1234);
    LoadImage(&area,pixels);DrawSync(0);g_CdImage=disc;
    if(Render_InitDisplayLists(0)!=-4 || Render_InitDisplayLists(3)!=-4 ||
       Render_InitDisplayLists(2)!=-4)return 0;
    StoreImage(&area,after);DrawSync(0);
    if(memcmp(after,pixels,sizeof(pixels)))return 0;
    g_CdImage=NULL;
    if(Render_InitDisplayLists(1)!=-1)return 0;
    g_CdImage=disc;
    if(Render_InitDisplayLists(1) || g_PeImageMountFlags!=1)return 0;
    StoreImage(&area,after);DrawSync(0);
    for(unsigned y=0;y<512;++y)for(unsigned x=0;x<1024;++x)
        if(after[y*1024+x]!=(x<320 && y<448?0:pixels[y*1024+x]))return 0;
    static const struct { AkaoMessageHandler function;u32 address; } bindings[]={
        {Akao_MessageNoop,0x8008ca7cu},
        {Seq_SelectPlaybackBank,0x8008ae94u},
        {Akao_InitPrimarySecondaryVoicesWithMode,0x8008c3e4u},
        {Seq_SelectPlaybackBankWithCountdown,0x8008b040u},
        {Seq_RestoreSecondaryStateAndSelect,0x8008afb8u},
        {Seq_StartIndexedNestedStream,0x8008b124u},
        {Spu_ManageVoicesCmd,0x8008b1d0u},
        {Seq_StartRelativeNestedStream,0x8008b168u},
        {Seq_StartIndexedNestedStreamWithDefaults,0x8008b0c8u},
        {Seq_StartDefaultNestedStream,0x8008b084u},
        {Seq_RestorePrimaryState,0x8008af08u},
        {Seq_SetPlaybackMode1AndRefreshVoices,0x8008c55cu},
        {Seq_SetPlaybackMode2AndRefreshVoices,0x8008c654u},
        {Seq_SetPlaybackMode4AndRefreshVoices,0x8008c5d8u},
        {Seq_SetGlobalD2B8AndDirtyAllTracks,0x8008c6d0u},
        {Seq_SetCurrentTrackField56,0x8008c70cu},
        {Seq_RestorePendingTracks,0x8008c814u},
        {Seq_DeactivatePendingTracks,0x8008c720u},
        {Spu_RestoreStoppedVoices,0x8008c9dcu},
        {Spu_VoiceStopAll,0x8008c8b8u},
        {Spu_SetVoiceVolumeImmediateMasked,0x8008b698u},
        {Spu_ParameterSlide,0x8008b780u},
        {Spu_SetVoicePanImmediateMasked,0x8008ba3cu},
        {Spu_SlideVoicePanMasked,0x8008bb1cu},
        {Spu_SetVoicePitchImmediateMasked,0x8008bdc8u},
        {Spu_SlideVoicePitchMasked,0x8008bea8u},
        {Spu_SetAllVoiceVolumeImmediate,0x8008b900u},
        {Spu_SlideAllVoiceVolume,0x8008b978u},
        {Spu_SetAllVoicePanImmediate,0x8008bc94u},
        {Spu_SlideAllVoicePan,0x8008bd08u},
        {Spu_SetAllVoicePitchImmediate,0x8008c030u},
        {Spu_SlideAllVoicePitch,0x8008c0a4u},
        {Seq_SetTrackPitchImmediate,0x8008b1fcu},
        {Seq_SlideTrackPitch,0x8008b2ccu},
        {Seq_TrackPitchSetup,0x8008b410u},
        {Seq_SetGlobalPitchImmediate,0x8008b580u},
        {Seq_SlideGlobalPitchToTarget,0x8008b5b8u},
        {Seq_SlideGlobalPitchFromStartToTarget,0x8008b624u},
        {Akao_SetGlobalD2D0Immediate,0x8008c16cu},
        {Akao_SlideGlobalD2D0ToTarget,0x8008c18cu},
        {Akao_SlideGlobalD2D0FromStartToTarget,0x8008c1f8u},
        {Akao_SetGlobalD2CCImmediate,0x8008c270u},
        {Akao_SlideGlobalD2CCToTarget,0x8008c290u},
        {Akao_SlideGlobalD2CCFromStartToTarget,0x8008c2fcu},
        {Akao_InitPrimarySecondaryVoices,0x8008c374u},
        {Spu_StopActiveVoices,0x8008c46cu},
    };
    u32 original[256];
    if(!pe_disc_read(disc,exe,0x8009c0c0u-0x80010000u+0x800,original,sizeof(original)))return 0;
    for(unsigned i=0;i<256;++i) {
        unsigned found=0;
        for(unsigned j=0;j<sizeof(bindings)/sizeof(bindings[0]);++j)
            if(Akao_MessageHandlers[i]==bindings[j].function && original[i]==bindings[j].address)++found;
        if(found!=1)return 0;
    }
    u8 silence[96];
    if(!pe_disc_read(disc,exe,0x8009b7fcu-0x80010000u+0x800,silence,sizeof(silence)) ||
       memcmp(silence,g_AkaoSilentLoopSample,32) || memcmp(silence+32,g_AkaoSilentEndSample,64))return 0;
    g_CdImage=previous;g_GameState=saved;
    puts("Original image display selection: valid Disc 1, missing Disc 2/image, invalid mode, full VRAM clear footprint and authored silence records passed");
    return 1;
}

static int original_boot_audio_lifecycle(void)
{
    g_CdImage=NULL; /* Restart must not depend on physical drive readiness. */
    for(unsigned cycle=0;cycle<2;++cycle) {
    Boot_SyncShutdown();
    if(!StopRCnt(0xf2000002u)){fprintf(stderr,"audio root counter stop failed\n");return 0;}
    g_AkaoTimerDeltaHist0=g_AkaoTimerDeltaHist1=g_AkaoTimerDeltaHist2=g_AkaoTimerDeltaHist3=0;
    g_AkaoTimerDeltaSum=0;
    for(unsigned i=0;i<4;++i) {
        DeliverEvent(0xf2000002u,2);
        if(g_AkaoTimerDeltaHist0<=0 || g_AkaoTimerDeltaSum!=g_AkaoTimerDeltaHist0+
           g_AkaoTimerDeltaHist1+g_AkaoTimerDeltaHist2+g_AkaoTimerDeltaHist3){fprintf(stderr,"audio timer history failed %u: %d %d %d %d sum %d\n",i,g_AkaoTimerDeltaHist0,g_AkaoTimerDeltaHist1,g_AkaoTimerDeltaHist2,g_AkaoTimerDeltaHist3,g_AkaoTimerDeltaSum);return 0;}
    }
    u8 uploaded[32];Psyz_SpuMemRead(0x1010,uploaded,sizeof(uploaded));
    if(memcmp(uploaded,g_AkaoSilentLoopSample,32)){fprintf(stderr,"audio silence upload mismatch\n");return 0;}
    /* Queue commands 20/30/34 must retain native script pointers and defaults. */
    u16 offsets[2048];u8 scripts[16];
    memset(offsets,0xff,sizeof(offsets));memset(scripts,0xa0,sizeof(scripts));
    offsets[6]=0;offsets[7]=8;
    unsigned short *saved_offsets=g_AkaoSampleOffsetTable;
    char *saved_samples=g_AkaoSampleDataBase;
    g_AkaoSampleOffsetTable=offsets;g_AkaoSampleDataBase=(char *)scripts;
    const u8 commands[]={0x20,0x30,0x34};
    for(unsigned i=0;i<3;++i) {
        AkaoQueueEntry command={0};command.opcode.id=commands[i];
        command.arg0.value=3;command.arg1=0x12;command.arg2=0x32;command.arg3=0x23;
        if(i==2) {command.arg0.sequence=scripts;command.second_sequence=scripts+8;}
        g_SpuActiveVoiceMask=0;g_AkaoSeqPendingFlags=0;
        g_AkaoMessageQueue[0]=command;g_AkaoMessageQueueCount=1;Akao_ProcessMessageQueue();
        if(g_AkaoMessageQueueCount || g_AkaoVoiceChannelTable[10].pc!=scripts ||
           g_AkaoVoiceChannelTable[11].pc!=scripts+8 ||
           g_AkaoVoiceChannelTable[10].key_off_mask!=(i==2?0x400u:3u) ||
           g_AkaoVoiceChannelTable[10].key_on_mask!=(i==0?0x12u:i==1?0x2000000u:0x1000000u))return 0;
    }
    g_AkaoSampleOffsetTable=saved_offsets;g_AkaoSampleDataBase=saved_samples;
    long event=g_AkaoTimerEventDesc;
    Menu_ResetEquipSlotState();g_SceneDispatchToken=0xa80651c8;
    Boot_Shutdown();
    if(g_SceneDispatchToken!=(int)0xa9400048u ||
       g_ScriptVariableBank[SCRIPT_VAR_ROOM]!=(s32)0xa80651c8u ||
       !(g_GameState.flags&0x100))return 0;
    /* EnableEvent accepts a valid slot even when unused; it must stay closed. */
    g_AkaoTimerDeltaHist0=-1234;
    EnableEvent(event);DeliverEvent(0xf2000002u,2);
    if(g_AkaoTimerDeltaHist0!=-1234)return 0;
    }
    puts("Original audio restart/reset: two SPU start/stop cycles, native event handles, indexed/direct stream commands and four dispatched AKAO timer ticks and closed-event soft reset passed without a drive");
    return 1;
}

static int original_movie_uploads(PeDisc *disc)
{
    _Alignas(16) static u8 arena[0x55e00], whole_frame[320*240*3], actual[320*240*3];
    PeDisc *old_image=g_CdImage;Pe1SceneAudioState audio=g_GameState.scene_audio;
    CdlATV mix=g_AkaoCdVolume;u32 flags=g_GameStateFlagsWord;
    int old_slot=g_ActiveDrawSlot,background=g_MenuSaveBgFadeState;
    u8 wide=g_MenuCardScreenVideoTable[0].wide;
    u8 *buffers[]={arena};int valid=1;
    g_CdImage=disc;g_DrawEnabled.bits=0;g_ActiveDrawSlot=0;
    g_GameStateFlagsWord|=0x200;g_MenuSaveBgFadeState=0;
    g_MenuCardScreenVideoTable[0].wide=1;g_MenuCardScreenVideoDepthFlip=0;
    if(!MenuCard_InitVideoBuffers(1,buffers) || !MenuCard_OpenVideo(0))return 0;
    unsigned frames=0;int tick=VSync(-1);
    for(;;) {
        unsigned decoded=g_MenuCardScreenVideoDisplay.decodeIndex;
        unsigned region=g_MenuCardScreenVideoDisplay.region;
        RECT area={0,region?0:240,480,240};
        int running=MenuCard_StepVideo();
        StoreImage(&area,actual);DrawSync(0);
        /* A whole transfer must agree with all twenty original slice callbacks. */
        DecDCToutCallback(NULL);
        DecDCTin(g_MenuCardScreenVideoDisplay.decode[decoded],3);
        DecDCTout(whole_frame,sizeof(whole_frame)/4);
        if(DecDCToutSync(0))valid=0;
        DecDCToutCallback(MenuCard_UploadVideoSlice);
        for(unsigned x=0;x<20;++x)for(unsigned y=0;y<15;++y)for(unsigned row=0;row<16;++row)
            if(memcmp(actual+((y*16+row)*320+x*16)*3,
                      whole_frame+((x*15+y)*256+row*16)*3,48))valid=0;
        if(!frames)for(unsigned i=0;i<sizeof(actual);++i)if(actual[i])valid=0;
        ++frames;
        Draw_RenderFrame();
        if(!running || frames>150)break;
    }
    if((unsigned)(VSync(-1)-tick)!=149*4 || frames!=149 || g_SeqElapsed!=150 || g_MenuCardScreenVideoLastTime!=150 ||
       g_MenuCardScreenVideoFinished!=1 || g_DrawEnabled.bits!=1)valid=0;
    DecDCToutCallback(NULL);MenuCard_ResetDisplayState();
    g_MenuCardScreenVideoTable[0].wide=wide;g_MenuSaveBgFadeState=background;
    g_GameStateFlagsWord=flags;g_GameState.scene_audio=audio;g_ActiveDrawSlot=old_slot;
    g_AkaoCdVolume=mix;CdMix(&mix);g_CdImage=old_image;
    if(!valid)fprintf(stderr,"original movie MDEC/slice/VRAM regression\n");
    else puts("Original movie: 149 RGB24 frames through real OpenVideo/StepVideo, 2980 slice uploads, full VRAM regions and 15fps render path passed headlessly");
    return valid;
}

static int original_video_stream_errors(void)
{
    static u8 image[2352*2];
    _Alignas(16) static u8 arena[0x20000];
    PeDisc *mounted=g_CdImage;
    PeDisc disc={0};disc.stream=tmpfile();disc.sector_size=2352;
    disc.image_size=sizeof(image);
    if(!disc.stream)return 0;
    g_CdImage=&disc;g_MenuCardScreenVideoStreamRing=arena;
    VideoEntry entry={"test",0,1,300,0,0,{0}};
    g_MenuCardScreenVideoEntry=&entry;
    g_MenuCardScreenVideoSize[0]=320;g_MenuCardScreenVideoSize[1]=240;
    VideoDisplay display={0};int valid=1;
    for(unsigned test=0;test<15;++test) {
        memset(image,0,sizeof(image));
        VideoFrame first={.id=0x160,.type=0x8001,.nSectors=2,.frameCount=1,
                          .frameSize=4032,.width=320,.height=240};
        VideoFrame second=first;second.secCount=1;
        switch(test) {
        case 0:first.nSectors=0;break;
        case 1:first.nSectors=66;break;
        case 2:first.frameSize=8;break;
        case 3:first.frameSize=4033;break;
        case 4:first.width=0;break;
        case 5:first.width=321;break;
        case 6:first.height=241;break;
        case 7:first.secCount=1;break;
        case 8:second.secCount=0;break;
        case 9:second.frameCount=2;break;
        case 10:second.width=304;break;
        case 11:second.frameSize=4000;break;
        case 12:second.nSectors=3;break;
        case 13:break; /* Advertised MDEC output exceeds its 0xFA00 arena. */
        case 14:first.frameCount=0x7fffffff;break;
        }
        memcpy(image+24,&first,sizeof(first));memcpy(image+2352+24,&second,sizeof(second));
        u32 command=test==13?0x3800ffff:0x38000020;
        memcpy(image+56,&command,4);
        rewind(disc.stream);
        if(fwrite(image,1,sizeof(image),disc.stream)!=sizeof(image) || fflush(disc.stream))valid=0;
        g_MenuCardScreenVideoFile=(PeDiscFile){.sector=0,.size=4096};
        g_MenuCardScreenVideoSector=0;
        if(MenuCard_UpdateVideoFrame(&display))valid=0;
    }
    /* Image extent rejection, incomplete frame and a cursor already at EOF. */
    for(unsigned test=0;test<3;++test) {
        g_MenuCardScreenVideoFile=(PeDiscFile){.sector=0,.size=test==1?2048:4096};
        g_MenuCardScreenVideoSector=test==2?2:0;
        disc.image_size=test==0?100:sizeof(image);
        if(MenuCard_UpdateVideoFrame(&display))valid=0;
    }
    g_CdImage=NULL;g_DrawEnabled.bits=0;
    if(MenuCard_PlayVideo(-1) || MenuCard_PlayVideo(47) || MenuCard_OpenVideo(0) ||
       MenuCard_PlayVideo(0) || g_DrawEnabled.bits || (g_GameStateFlagsWord&0x200))valid=0;
    g_MenuCardScreenVideoStreamRing=NULL;g_MenuCardScreenVideoEntry=NULL;
    g_CdImage=mounted;fclose(disc.stream);
    if(!valid)fprintf(stderr,"original movie stream rejection regression\n");
    return valid;
}

/* Output goldens and yield counts come from the retail VLC_C procedure. */
static int original_video_decoder(PeDisc *disc)
{
    static const struct { const char *name; unsigned frames; uint64_t hash; unsigned chunks; } movies[]={
        {"FMV1/FMV000.STR",150,UINT64_C(0x30a713f43d9413a7),1711},
        {"FMV1/FMV001.STR",2,UINT64_C(0xef1b245b6adc85b3),1711},
        {"FMV1/FMV002.STR",2,UINT64_C(0x392138382a56d9a5),1420},
        {"FMV1/FMV003.STR",2,UINT64_C(0x8ef0d2fb8bf52fc),1674},
        {"FMV1/FMV004.STR",2,UINT64_C(0x6d2ff28b9e23591a),1581},
        {"FMV1/FMV005.STR",2,UINT64_C(0x7ef08eb0f7c6edad),1642},
        {"FMV1/FMV006.STR",2,UINT64_C(0x6b815afd94529e92),1929},
        {"FMV1/FMV006C.STR",2,UINT64_C(0xe97a75ac7bef8804),1939},
        {"FMV1/FMV007.STR",2,UINT64_C(0x392138382a56d9a5),1420},
        {"FMV1/FMV008.STR",2,UINT64_C(0xc5dd9de216b31929),1701},
        {"FMV1/FMV009.STR",2,UINT64_C(0xbb82491ab3017b5a),1947},
        {"FMV1/FMV010.STR",2,UINT64_C(0xafc44a597db8d24e),1739},
        {"FMV1/FMV011.STR",2,UINT64_C(0xb7a90cc9ea87b03d),1614},
        {"FMV1/FMV012.STR",2,UINT64_C(0x946efb22d923a428),1592},
        {"FMV1/FMV013.STR",2,UINT64_C(0x557e4491d4ccc846),2034},
        {"FMV1/FMV014.STR",2,UINT64_C(0x49efb4fe4b2d2329),1622},
        {"FMV1/FMV015.STR",2,UINT64_C(0x4cbf8a26f125337c),2055},
        {"FMV1/FMV016A.STR",2,UINT64_C(0x9de5f375d2863e0d),2130},
        {"FMV1/FMV016B.STR",2,UINT64_C(0x8c4308a3c6cda742),1905},
        {"FMV1/FMV017A.STR",2,UINT64_C(0xd7107e60165c5819),1615},
        {"FMV1/FMV017B.STR",2,UINT64_C(0xbea326aefe8b8cfa),1729},
    };
    static u8 frame[0x20000], output[0x40010], partial[0x40010];
    static u16 table[0x8800];
    _Alignas(16) static u8 stream[0x20000];
    PeDisc *old_image=g_CdImage;
    Pe1SceneAudioState old_audio=g_GameState.scene_audio;
    CdlATV old_mix=g_AkaoCdVolume;
    g_CdImage=disc;
    g_MenuCardScreenVideoStreamRing=stream;
    VideoDisplay display={0};
    MdecDiag_ExpandDecoderTable(table);
    DecDCTvlcSize2(0);
    if(DecDCTvlcSize2(2)!=0xffffff || DecDCTvlcSize2(17)!=4 ||
       DecDCTvlcSize2(256)!=34 || DecDCTvlcSize2(1)!=512 ||
       DecDCTvlcSize2(-1)!=0xffffff)return 0;
    for(unsigned movie=0;movie<sizeof(movies)/sizeof(movies[0]);++movie) {
        PeDiscFile file; u8 raw[2352]; unsigned frames=0,chunks=0,part=0,parts=0;
        u32 frame_number=0,frame_size=0; size_t stored=0;
        uint64_t hash=UINT64_C(14695981039346656037);
        if(!pe_disc_find(disc,movies[movie].name,&file))return 0;
        g_MenuCardScreenVideoFile=file;g_MenuCardScreenVideoSector=0;
        g_MenuCardScreenVideoEntry=&g_MenuCardScreenVideoTable[movie];
        g_MenuCardScreenVideoLastTime=0;g_MenuCardScreenVideoFinished=0;
        g_MenuCardScreenVideoSize[0]=320;g_MenuCardScreenVideoSize[1]=240;
        for(u32 sector=0;sector<(file.size+2047u)/2048u && frames<movies[movie].frames;++sector) {
            if(!pe_disc_read_raw(disc,&file,sector,raw))return 0;
            u32 magic;memcpy(&magic,raw+24,4);
            if(magic!=0x80010160)continue; /* XA audio is interleaved. */
            u16 index,count;u32 number,size;
            memcpy(&index,raw+28,2);memcpy(&count,raw+30,2);
            memcpy(&number,raw+32,4);memcpy(&size,raw+36,4);
            if(!index) {part=0;parts=count;frame_number=number;frame_size=size;stored=0;}
            if(index!=part || count!=parts || number!=frame_number || size!=frame_size ||
               stored+2016>sizeof(frame) || frame_size>sizeof(frame)-64)return 0;
            memcpy(frame+stored,raw+56,2016);stored+=2016;++part;
            if(part!=parts)continue;
            if(frame_size>stored || frame_size<12)return 0;
            g_MovieWide=frames&1;
            void *native_frame=MenuCard_UpdateVideoFrame(&display);
            if(native_frame!=stream+sizeof(VideoFrame) || memcmp(native_frame,frame,frame_size) ||
               g_MenuCardScreenVideoLastTime!=(s16)frame_number ||
               g_MenuCardScreenVideoFinished!=(frame_number>=(u32)g_MenuCardScreenVideoEntry->end) ||
               display.regions[0].w!=(g_MovieWide?480:320) ||
               display.regions[1].w!=display.regions[0].w || display.rect.h!=240 ||
               display.regions[0].h!=240 || display.regions[1].h!=240)return 0;
            u16 words;memcpy(&words,frame,2);size_t bytes=4+(size_t)words*4;
            if(bytes+16>sizeof(output))return 0;
            memset(output,0xa5,sizeof(output));DecDCTvlcSize2(0);
            if(DecDCTvlc2(frame,output,table)!=0)return 0;
            for(size_t i=0;i<bytes;++i)hash=(hash^output[i])*UINT64_C(1099511628211);
            for(size_t i=bytes;i<sizeof(output);++i)if(output[i]!=0xa5)return 0;
            if(frame_number<=2)for(unsigned mode=0;mode<3;++mode) {
                const int limits[]={2,17,256};unsigned calls=1;
                memset(partial,0xa5,sizeof(partial));DecDCTvlcSize2(limits[mode]);
                int result=DecDCTvlc2(frame,partial,table);
                while(result==1 && calls<10000) {result=DecDCTvlc2(NULL,NULL,table);++calls;}
                if(result || memcmp(output,partial,sizeof(output)))return 0;
                chunks+=calls;
            }
            ++frames;
        }
        if(frames!=movies[movie].frames || hash!=movies[movie].hash || chunks!=movies[movie].chunks) {
            fprintf(stderr,"VLC mismatch %s frames=%u chunks=%u hash=%llx\n",movies[movie].name,
                    frames,chunks,(unsigned long long)hash);return 0;
        }
    }
    g_MenuCardScreenVideoSector=(g_MenuCardScreenVideoFile.size+2047u)/2048;
    if(MenuCard_UpdateVideoFrame(&display))return 0;
    g_CdImage=NULL;if(MenuCard_UpdateVideoFrame(&display))return 0;
    g_CdImage=disc;g_MenuCardScreenVideoStreamRing=NULL;
    if(MenuCard_UpdateVideoFrame(&display))return 0;
    g_CdImage=old_image;g_GameState.scene_audio=old_audio;
    g_AkaoCdVolume=old_mix;CdMix(&old_mix);
    /* V3 fixture: all DC magnitude sizes/signs and three predictors, with AC escapes. */
    static const u8 version3[]={
        0x00,0x01,0x00,0x38,0x1f,0x00,0x03,0x00,0x00,0x01,0x80,0x00,0x20,0x40,0x08,0xe8,
        0x35,0x08,0x01,0x01,0xa0,0x89,0x41,0x20,0x04,0x94,0x3e,0x0a,0x81,0x40,0x4c,0x89,
        0x39,0x10,0x02,0x58,0x31,0x08,0x41,0x10,0xe0,0x26,0x28,0x08,0x41,0xf4,0xa1,0x05,
        0x10,0xa0,0x4a,0x62,0x06,0xc1,0x90,0xa7,0x72,0x10,0xc1,0xa9,0xad,0x07,0x10,0x90,
        0x09,0x83,0x08,0xc1,0xb0,0xb3,0x24,0x04,0xdc,0xda,0x9b,0x10,0x80,0x9a,0x8f,0x42,
        0xc1,0x2b,0xbf,0x0a,0x04,0xa8,0x0a,0x2d,0x10,0xbc,0x5b,0xbc,0x10,0x80,0x8b,0xc4,
        0x10,0xbc,0xbb,0xcc,0x21,0x00,0xd6,0xa9,0x43,0xf0,0x6c,0x74,0x87,0x00,0x5b,0x2a,
        0x0e,0xc1,0xbc,0xd7,0x43,0x00,0xaf,0xd6,0x10,0x7c,0xdb,0xfd,0x08,0x80,0x05,0x83,
        0x04,0xdf,0x8e,0x43,0x02,0xe0,0xcd,0x22,0xc1,0x77,0xe9,0x11,0x00,0xbe,0x26,0x11,
        0xef,0xcb,0x12,0xc1,0xbc,0xef,0x22,0x00,0x57,0x6e,0x04,0xbf,0xd6,0x4f,0x00,0xf0,
        0x3c,0x8a,0xfc,0x5e,0x4f,0x11,0xf0,0xbb,0x45,0x00,0xaf,0x5f,0xc1,0xdf,0x01,0x16,
        0x00,0xbe,0xb4,0x08,0xf7,0x25,0x45,0xf0,0xef,0xc1,0x02,0x80,0x15,0x2f,0xfc,0x7d,
        0x80,0x11,0xf8,0xdb,0x11,0x00,0x0b,0x89,0xfc,0xfb,0x91,0x11,0xf0,0x3b,0x23,0x00,
        0xd7,0x32,0xf0,0xef,0x86,0x46,0xc0,0x6f,0x8d,0x00,0x5f,0x4e,0xc1,0xbf,0x1f,0x1b,
        0x46,0x80,0xa0,0xe8,0xc2,0x11,0x02,0x5a,0x51,0x39,0x47,0x40,0xe8,0x4a,0xed,0x08,
        0x01,0x75,0x31,0x1e,0x23,0x90,0x93,0xd6,0x7c,0x04,0x00,0xde,0xdd,0x8f,0x12,0x44,
        0xd8,0x03,0x41,0x02,0x10,0x81,0x50,0x48,0x04,0xe8,0x1a,0x87,0x48,0xb0,0x64,0x92,
        0x8b,0x04,0x70,0x32,0xd3,0x48,0x04,0xe4,0x4a,0x8f,0x49,0x70,0x6c,0x15,0x24,0x01,
        0xb7,0xd8,0x95,0x04,0xa0,0x6e,0x5d,0x12,0xf0,0xea,0x98,0x49,0x01,0x6a,0xe4,0x26,
        0x04,0xaf,0x9e,0x9d,0x04,0xe0,0xaa,0x9f,0x04,0xef,0xb6,0xa1,0x09,0xc0,0x85,0x47,
        0x12,0xbc,0x3b,0x97,0x25,0x00,0xd6,0x3e,0x4a,0xf0,0x6f,0x9e,0x12,0x00,0xcb,0xaf,
        0x04,0xdf,0xfe,0xad,0x02,0xe0,0x05,0x58,0xc1,0x77,0x85,0x2c,0x00,0xb8,0x84,0x96,
        0xf0,0x5d,0x62,0x4b,0x80,0xef,0xb8,0x04,0xfb,0x3a,0x4b,0xf0,0x6f,0xa4,0x09,0x00,
        0xa5,0x78,0xc1,0xef,0x97,0x2f,0x00,0xbc,0x03,0x26,0xbf,0x57,0xc2,0x04,0xfc,0x76,
        0x13,0x00,0x0b,0x12,0xf0,0xf7,0x68,0x4c,0x80,0xef,0x64,0x02,0x7d,0x4d,0x13,0xfc,
        0x9b,0x2a,0x00,0xe0,0x96,0x99,0x7f,0x5f,0xce,0x04,0xfe,0xbe,0x04,0x00,0xca,0xd0,
        0xff,0xfe,0xd2,0x04,0xfc,0xd6,0x09,0x00,0xc5,0xa9,0xfc,0xfb,0x5b,0x13,0xf0,0xbb,
        0x26,0x00,0xd7,0xc7,0xf0,0xef,0xb0,0x4d,0xfc,0x6f,
    };
    const int limits[]={0,2,17,256};const unsigned expected_calls[]={1,55,10,1};
    for(unsigned mode=0;mode<4;++mode) {
        memcpy(frame,version3,sizeof(version3));memset(frame+sizeof(version3),0,64);
        memset(output,0xa5,sizeof(output));DecDCTvlcSize2(limits[mode]);
        int result=DecDCTvlc2(frame,output,table);unsigned calls=1;
        while(result==1 && calls<10000) {result=DecDCTvlc2(NULL,NULL,table);++calls;}
        uint64_t hash=UINT64_C(14695981039346656037);
        for(unsigned i=0;i<1028;++i)hash=(hash^output[i])*UINT64_C(1099511628211);
        if(result || calls!=expected_calls[mode] || hash!=UINT64_C(0x0fa0f433aa9048a1))return 0;
        for(unsigned i=1028;i<sizeof(output);++i)if(output[i]!=0xa5)return 0;
    }
    DecDCTvlcSize2(0);
    if(DecDCTvlc2(NULL,NULL,table)!=-1 || DecDCTvlc2(frame,NULL,table)!=-1 ||
       DecDCTvlc2(frame,output,NULL)!=-1)return 0;
    memset(frame,0,16);memset(output,0xa5,sizeof(output));
    if(DecDCTvlc2(frame,output,table)!=-1 || DecDCTvlc2(NULL,NULL,table)!=-1)return 0;
    for(unsigned i=4;i<sizeof(output);++i)if(output[i]!=0xa5)return 0;
    puts("Original VLC_C: 190 real STR frames, 21 movie hashes, incremental yield counts and V3 differential/escape fixture match USA MIPS");
    return 1;
}

static int original_video_buffers(PeDisc *disc)
{
    static u8 arena[0x55e00], second[0x16a00];
    static u16 saved[1024*512], pixels[1024*512], expected[1024*512];
    RECT whole={0,0,1024,512};
    DISPENV displays[2]; DRAWENV draws[2];
    Pe1SceneAudioState audio=g_GameState.scene_audio;
    u32 flags=g_GameStateFlagsWord;
    memcpy(displays,g_RenderDispEnvArray,sizeof(displays));
    memcpy(draws,g_RenderDrawEnvArray,sizeof(draws));
    StoreImage(&whole,saved);DrawSync(0);
    int valid=1;
    PeDiscFile archive;
    static u8 bank[133*2048];
    if(!pe_disc_find(disc,"PE.IMG",&archive) ||
       !pe_disc_read(disc,&archive,978ull*2048,bank,sizeof(bank)))return 0;
    for(unsigned i=0;i<47;++i) {
        const u8 *row=bank+0x41e10+i*20;
        u32 name; s16 speed,end; u16 x,y;
        memcpy(&name,row,4);memcpy(&speed,row+6,2);memcpy(&end,row+8,2);
        memcpy(&x,row+10,2);memcpy(&y,row+12,2);
        VideoEntry *entry=&g_MenuCardScreenVideoTable[i];
        if(name<0x8018eff0u || name-0x8018eff0u>=sizeof(bank) ||
           !memchr(bank+(name-0x8018eff0u),0,sizeof(bank)-(name-0x8018eff0u)))return 0;
        if(strcmp(entry->name,(char *)bank+(name-0x8018eff0u)) || entry->wide!=row[4] ||
           entry->speed!=speed || entry->end!=end || entry->x!=x || entry->y!=y ||
           memcmp(entry->reserved0e,row+14,6))valid=0;
    }
    /* Golden is the complete output of USA 0x8010BD4C in Unicorn MIPS. */
    static struct { u16 before[8], table[0x8800], after[8]; } vlc;
    memset(&vlc,0xa5,sizeof(vlc));
    MdecDiag_ExpandDecoderTable(vlc.table);
    uint64_t expanded_hash=UINT64_C(14695981039346656037);
    for(unsigned i=0;i<sizeof(vlc.table);++i)
        expanded_hash=(expanded_hash^((u8 *)vlc.table)[i])*UINT64_C(1099511628211);
    if(expanded_hash!=UINT64_C(0x62fd6458f3443095))valid=0;
    for(unsigned i=0;i<8;++i)if(vlc.before[i]!=0xa5a5 || vlc.after[i]!=0xa5a5)valid=0;
    for(int split=1;split<=2;++split)for(int skip=0;skip<2;++skip) {
        u8 *buffers[2]={arena,second};
        memset(arena,0xa5,sizeof(arena));memset(second,0xa5,sizeof(second));
        for(unsigned i=0;i<1024*512;++i)pixels[i]=(u16)(i*37+(i>>10)*19);
        memcpy(expected,pixels,sizeof(expected));
        LoadImage(&whole,pixels);DrawSync(0);
        g_DrawEnabled.bits=0;g_SeqElapsed=123;
        g_GameStateFlagsWord=skip?0x08000000:0;
        if(MenuCard_InitVideoBuffers(0,buffers) || MenuCard_InitVideoBuffers(3,buffers) ||
           MenuCard_InitVideoBuffers(1,NULL))valid=0;
        if(!MenuCard_InitVideoBuffers(split,buffers))valid=0;
        if(g_MenuCardScreenVideoDecodeBuffer0!=arena ||
           g_MenuCardScreenVideoDecodeBuffer1!=arena+0xfa00 ||
           g_MenuCardScreenVideoStreamRing!=arena+0x1f400 ||
           g_MenuCardScreenVideoVlcTable!=(split==1?arena+0x3f400:second) ||
           g_MenuCardScreenVideoSliceBuffer0!=(split==1?arena+0x50400:second+0x11000) ||
           g_MenuCardScreenVideoSliceBuffer1!=(split==1?arena+0x53100:second+0x13d00) ||
           g_GameState.scene_audio.tracks.movie_active!=1 || g_SeqElapsed!=0 ||
           g_MenuCardScreenVideoLastTime!=-1 ||
           g_GameState.scene_audio.tracks.movie_volume!=152 ||
           MenuCard_InitVideoBuffers(split,buffers))valid=0;
        for(unsigned y=0;y<256;++y)memcpy(expected+y*1024+512,pixels+y*1024+320,192*2);
        if(!skip)for(unsigned y=0;y<64;++y)
            memcpy(expected+(y+256)*1024+512,pixels+(y+448)*1024,320*2);
        StoreImage(&whole,pixels);DrawSync(0);
        if(memcmp(pixels,expected,sizeof(pixels)))valid=0;
        for(int index=0;index<2;++index)for(int wide=0;wide<2;++wide) {
            MenuCard_SetVideoDisplay(index,wide);
            if(g_RenderDispEnvArray[index].disp.w!=320 ||
               g_RenderDispEnvArray[index].disp.y!=(index?0:240) ||
               g_RenderDispEnvArray[index].isrgb24!=wide ||
               g_RenderDrawEnvArray[index].clip.w!=320 ||
               g_RenderDrawEnvArray[index].clip.y!=(index?240:0))valid=0;
        }
        /* The movie overwrites the graphics whose backups must survive. */
        for(unsigned y=0;y<256;++y)memset(pixels+y*1024+320,0,192*2);
        if(!skip)for(unsigned y=0;y<64;++y)memset(pixels+(y+448)*1024,0,320*2);
        LoadImage(&whole,pixels);DrawSync(0);
        MenuCard_ResetDisplayState();
        if(g_DrawEnabled.bits || g_MenuCardScreenVideoDecodeBuffer0 ||
           g_MenuCardScreenVideoDecodeBuffer1 || g_MenuCardScreenVideoStreamRing ||
           g_MenuCardScreenVideoVlcTable || g_MenuCardScreenVideoSliceBuffer0 ||
           g_MenuCardScreenVideoSliceBuffer1 ||
           memcmp(displays,g_RenderDispEnvArray,sizeof(displays)) ||
           memcmp(draws,g_RenderDrawEnvArray,sizeof(draws)))valid=0;
        StoreImage(&whole,pixels);DrawSync(0);
        if(memcmp(pixels,expected,sizeof(pixels)))valid=0;
        for(unsigned i=0;i<sizeof(arena);++i)if(arena[i]!=0xa5)valid=0;
        for(unsigned i=0;i<sizeof(second);++i)if(second[i]!=0xa5)valid=0;
    }
    LoadImage(&whole,saved);DrawSync(0);
    g_GameState.scene_audio=audio;g_GameStateFlagsWord=flags;
    if(!valid)fprintf(stderr,"original movie buffer/display regression\n");
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
    if(!original_battle_target_list() || !original_script_sprites() || !original_wireframe_renderer() || !original_wireframe_animation() || !original_textbox_renderer(disc,executable) || !original_scan_overlay(disc,executable) || !original_save_card_images(disc) || !original_hud_packet_initialization() || !original_hud_packet_updates() || !original_spell_effects(disc,executable))valid=0;
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
    if (!original_combat_timers() || !original_combat_angles() || !original_player_turn() || !original_enemy_damage() || !original_enemy_facing() || !original_enemy_death()) return 141;
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
        if (!original_video_buffers(&disc)) return 135;
        if (!original_video_decoder(&disc)) return 136;
        if (!original_video_stream_errors()) return 137;
        if (!original_movie_uploads(&disc)) return 138;
        if (!original_image_display_switch(&disc,&exe) || !original_boot_display_data(&disc) || !original_enemy_slots(&disc,&exe)) return 140;
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
    if(!original_scene_teardown())return 139;
    if(argc>1 && !original_boot_audio_lifecycle())return 141;
    puts("Original action/clock/audio trigger and digital/analog player motion passed");
    SDL_Quit();
    return 0;
}
