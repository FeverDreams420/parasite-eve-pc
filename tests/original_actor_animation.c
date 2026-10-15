#include "../src/overlays/fx_common/camera_externs.h"
#include "../src/overlays/fx_common/fx_common_model.h"
#include "../src/overlays/fx_common/fx_common_motion.h"
#include <signal.h>
#include "../src/overlays/menu_memcard/menu_memcard_link.h"
#include "../src/overlays/menu_memcard/menu_memcard_trail.h"
#include "pe1/akao/pan3d.h"
#include "pe1/akao/voice_masks.h"
#include "pe1/akao/note_step.h"
#include "pe1/akao/voice_state.h"
#include "../src/overlays/menu_memcard/menu_memcard_glint.h"
#include "../src/overlays/menu_memcard/menu_memcard_video.h"
#include "pe1/resident_tables.h"
#include "pe1/draw_area.h"
#include "pe1/textbox.h"
#include "pe1/textbox_open.h"
#include "pe1/menu_flags.h"
#include "pe1/menu_sound.h"
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
#include "pe1/boot_display.h"
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
#include "pe1/room_flare.h"
#include "pe1/room_module.h"
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
#include "pe1/room_floor.h"
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
#include <psyz/audio.h>
#include <psyz/cd.h>

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
    if(g_AkaoBgmHandle[0]!=g_GameState.bank_work_far_end)return 0;
    void *resident=g_AkaoBgmHandle[0];u8 sound_alias[4];
    g_AkaoBgmHandle[0]=sound_alias;
    g_AkaoBgmHandle[1]=sound_alias+1;
    g_GameState.scene_object_tables[1]=sound_alias+2;
    if(g_GameState.bank_work_far_end!=sound_alias || g_GameState.scene_object_tables[0]!=sound_alias+1 ||
       g_AkaoBgmHandle[2]!=sound_alias+2)return 0;
    g_AkaoBgmHandle[0]=resident;g_GameState.scene_object_tables[0]=g_GameState.scene_object_tables[1]=NULL;
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
    if(!SDL_Init(SDL_INIT_EVENTS))return 0;
    ResetGraph(0);
    int saved_present=g_DrawPresentEnabled,saved_index=g_DrawBufferIndex;
    RenderNativeTag *saved_base=g_DrawOrderingTableBase;
    u32 *saved_entry=g_DrawOrderingTableEntry;
    u8 *saved_cursor=g_DrawPacketCursor,*saved_arena=g_DrawPacketArenaBase;
    DrawFrameBuffer *saved_current=g_DrawCurrentFrameBuffer;
    extern u_long D_800B89E4[]; /* SDK's terminating NOP packet. */
    for(unsigned slot=0;slot<2;++slot){
        g_DrawPresentEnabled=1;g_DrawBufferIndex=1-slot;Draw_SelectBuffer();
        RenderNativeTag *ot=g_DrawFrameBuffers[slot].orderingTableBase;
        if(g_DrawBufferIndex!=(int)slot || g_DrawOrderingTableBase!=ot ||
           g_DrawOrderingTableEntry!=(u32 *)(ot+1) || g_DrawCurrentFrameBuffer!=g_DrawFrameBuffers+slot ||
           g_DrawPacketCursor!=g_DrawFrameBuffers[slot].frontBufferBase){fprintf(stderr,"Native menu selection mismatch slot=%u index=%d base=%p expected=%p entry=%p expected=%p\n",slot,g_DrawBufferIndex,(void *)g_DrawOrderingTableBase,(void *)ot,(void *)g_DrawOrderingTableEntry,(void *)(ot+1));return 0;}
        for(unsigned entry=0;entry<4096;++entry)
            if(ot[entry].length || ot[entry].address!=(entry?(uintptr_t)(ot+entry-1):(uintptr_t)D_800B89E4)){fprintf(stderr,"Native menu OT mismatch slot=%u entry=%u address=%llx length=%llu\n",slot,entry,(unsigned long long)ot[entry].address,(unsigned long long)ot[entry].length);return 0;}
    }
    Draw_SetPresentImage(&saved_present);
    if(g_DrawPresentImage!=&saved_present)return 0;
    Draw_SetPresentImage(NULL);
    g_DrawPresentEnabled=saved_present;g_DrawBufferIndex=saved_index;
    g_DrawOrderingTableBase=saved_base;g_DrawOrderingTableEntry=saved_entry;
    g_DrawPacketCursor=saved_cursor;g_DrawPacketArenaBase=saved_arena;g_DrawCurrentFrameBuffer=saved_current;
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
static int fx_image_io;
static unsigned boot_credit_corruption;
static unsigned texture_page_corruption;
static unsigned fx_read_failure,fx_poll_failure;
int CdRom_ImageReadSectorsFromLba(u32 lba,void *buffer,u32 count);
int CdRom_ImagePollReady(void);
int CdRom_ReadSectorsFromLba(u32 lba, void *buffer, u32 count)
{
    ++glyph_reads;
    if(fx_image_io){
        int status=glyph_reads==fx_read_failure?-1:CdRom_ImageReadSectorsFromLba(lba,buffer,count);
        if(status>=0 && texture_page_corruption){
            TimFile *tim=buffer;TimBlock *image=(TimBlock *)((u8 *)&tim->first_block+44);
            switch(texture_page_corruption){
            case 1:tim->magic=0;break;
            case 2:tim->flags|=0x1000;break;
            case 3:tim->first_block.header.length=-4;break;
            case 4:image->header.length=13;break;
            case 5:image->header.rect.h=-1;break;
            case 6:image->header.rect.w=1025;break;
            case 7:tim->first_block.header.length=count*2048-8;break;
            case 8:tim->first_block.header.rect.w=32;break;
            }
        }
        return status;
    }
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
    if(fx_image_io){
        int ready=glyph_polls==fx_poll_failure?-1:CdRom_ImagePollReady();
        if(ready || !boot_credit_corruption)return ready;
    }
    if (glyph_pending) { glyph_pending = 0; return 1; }
    if (glyph_poll_retry) { glyph_poll_retry = 0; return -1; }
    if(boot_credit_corruption){
        u32 value=0;unsigned fault=boot_credit_corruption;boot_credit_corruption=0;
        switch(fault){
        case 1:memcpy(g_CdLoadAreaBase+0x4f88,&value,4);break;
        case 2:value=0x80122fa4;memcpy(g_CdLoadAreaBase+0x4f88,&value,4);break;
        case 3:value=0x80120d1c;memcpy(g_CdLoadAreaBase+0x4f88,&value,4);break;
        case 4:memcpy(g_CdLoadAreaBase+0x4f88+744*4,g_CdLoadAreaBase+0x4f88,4);break;
        case 5:memcpy(g_BootDisplayFontTim,&value,4);break;
        case 6:memcpy(g_CdLoadAreaBase+0x4ea4,&value,4);break;
        case 7:value=0x80200000;memcpy(g_CdLoadAreaBase+0x4ea4+30*4,&value,4);break;
        case 8:memcpy(&value,g_CdLoadAreaBase+0x4ea4+4,4);value-=4;memcpy(g_CdLoadAreaBase+0x4ea4+8,&value,4);break;
        case 9:memset(g_CdLoadAreaBase+4,'A',24);break;
        case 10:memset(g_CdLoadAreaBase+0x1c,'A',0x22a4-0x1c);break;
        }
    }
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
    if(mode==-7 && value==INT32_MIN && b==0x12345678 && c==-9)return 42;
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
        PmPrimarySlot bank[11]={0};PmSecondarySlot secondary[11]={0};PmTables old_tables=g_PmTables;
        Pe1GameState old_game=g_GameState;u32 old_flags=g_GameStateFlags;
        FieldActor actor={0},*old_actor=g_CurrentEntity,*old_player=g_PlayerEntity,*old_head=g_FieldActorListHead;ScriptNode *old_node=g_ScriptNodePool;u32 *old_cursor=g_ScriptCursor;
        g_CurrentEntity=&actor;
        g_PmTables=(PmTables){g_PmCmdHandlerArray,bank,secondary};
        g_GameStateFlags=0x80;g_GameState.flags&=~8u;g_GameState.tim_load_state=0;
        if(g_PmCmdHandlerArray[85]!=&g_FieldFxTaskClass)return 0;
        program.script=script+9;task_pointer_value=0;
        u32 allocate[]={106|(2u<<13),0,117,99,32,0};
        ScriptNode dispatch={.current.script=allocate,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        if(allocate[3]!=0 || dispatch.current.script!=allocate+6 || dispatch.flags!=0x90 || g_ScriptNodePool ||
            bank[0].header.owner!=&actor || bank[0].header.command!=117 || bank[0].header.state!=1 ||
            bank[0].task.tasks.table!=&program.table)return 0;
        u32 send[]={107|(5u<<13),0,0,(u32)-7,0x80000000,0x12345678,(u32)-9,32,0};
        dispatch=(ScriptNode){.current.script=send,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        if(bank[0].task.tasks.argument!=42 || dispatch.current.script!=send+9 || dispatch.flags!=0x90 || g_ScriptNodePool)return 0;
        if(Pm_SendCmd(0,0,7,(intptr_t)&task_pointer_value,(intptr_t)&task_pointer_value+sizeof(int),-9)!=0)return 0;
        g_FieldFxTaskContext=&bank[0].task.tasks;
        if(FieldFx_SpawnTask(0)!=(int)sizeof(NativeTaskState))return 0;
        g_ViewMatrixSlot.value=&identity;
        if(Pm_Start(0)!=0 || Pm_Exec(0)!=0 || task_pointer_value!=11)return 0;
        bank[0].header.field02=0xa5;bank[0].header.field03=0xd2;bank[0].header.ticks=0x89abcdef;
        u32 query[]={108|(5u<<13),0,0,0,99,99,99,32,0};
        dispatch=(ScriptNode){.current.script=query,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        if(query[4]!=bank[0].header.field02 || query[5]!=bank[0].header.field03 || query[6]!=bank[0].header.ticks ||
           dispatch.current.script!=query+9 || dispatch.flags!=0x90 || g_ScriptNodePool)return 0;
        u32 states[]={109|(2u<<13),0,0,4,110|(2u<<13),0,0,99,32,0};
        dispatch=(ScriptNode){.current.script=states,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        if(states[7]!=4 || bank[0].header.state!=4 || dispatch.current.script!=states+10 || dispatch.flags!=0x90 || g_ScriptNodePool)return 0;
        if(Pm_Stop(0,&actor,0)!=0 || bank[0].header.state)return 0;
        FieldActor peer={.type_id=6,.sub_id=7,.flags=0x10,.next=&actor};actor.type_id=6;actor.sub_id=7;
        g_FieldActorListHead=&peer;g_PlayerEntity=&actor;
        if(Scene_LoadRoomAssets(117,&actor)!=0 || Scene_LoadRoomAssets(117,&peer)!=1)return 0;
        secondary[3].header=(PmSlotHeader){.state=1,.command=117,.owner=&actor};
        u32 selected[]={152|(2u<<13),0,6,7,32,0};
        dispatch=(ScriptNode){.current.script=selected,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        if(bank[0].header.state || bank[1].header.state!=1 || secondary[3].header.state || dispatch.current.script!=selected+6 || dispatch.flags!=0x90 || g_ScriptNodePool)return 0;
        u32 stop[]={151|(1u<<13),0,1,32,0};
        dispatch=(ScriptNode){.current.script=stop,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        if(bank[1].header.state!=1 || dispatch.current.script!=stop+5 || dispatch.flags!=0x90 || g_ScriptNodePool)return 0;
        g_CurrentEntity=&peer;
        dispatch=(ScriptNode){.current.script=stop,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        if(bank[1].header.state || dispatch.current.script!=stop+5 || dispatch.flags!=0x90 || g_ScriptNodePool)return 0;
        if(Scene_LoadRoomAssets(117,&actor)!=0 || Scene_LoadRoomAssets(117,&peer)!=1)return 0;
        secondary[4].header=(PmSlotHeader){.state=1,.command=117,.owner=&peer};
        selected[3]=99;dispatch=(ScriptNode){.current.script=selected,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        if(bank[0].header.state!=1 || bank[1].header.state!=1)return 0;
        selected[2]=0;dispatch=(ScriptNode){.current.script=selected,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        if(bank[0].header.state || bank[1].header.state!=1 || secondary[4].header.state!=1)return 0;
        u32 reset[]={153,0,32,0};
        dispatch=(ScriptNode){.current.script=reset,.active=1,.flags=0x80};g_ScriptNodePool=&dispatch;Script_RunQueue();
        for(unsigned i=0;i<11;++i)if(bank[i].header.state || secondary[i].header.state)return 0;
        if(dispatch.current.script!=reset+4 || dispatch.flags!=0x90 || g_ScriptNodePool)return 0;
        g_CurrentEntity=&actor;g_PlayerEntity=old_player;g_FieldActorListHead=old_head;
        puts("Original effect cleanup opcodes 151/152/153: current/selected/player ownership, dead/missing subtype filtering and both-bank reset passed through dispatcher");
        memset(bank,0,sizeof(bank));g_FieldFxTaskPrograms[32]=NULL;
        if(Scene_LoadRoomAssets(117,NULL)!=-8 || bank[0].header.state ||
            Scene_LoadRoomAssets(191,NULL)!=-8 || bank[0].header.state ||
            FieldFx_BindTaskProgram(owner,-1)!=-1 || FieldFx_BindTaskProgram(owner,104)!=-1)return 0;
        g_ViewMatrixSlot.value=saved_view;g_PmTables=old_tables;g_GameState=old_game;g_GameStateFlags=old_flags;
        g_CurrentEntity=old_actor;g_ScriptNodePool=old_node;g_ScriptCursor=old_cursor;
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
    /* Retail's NULL reads see the nonzero low-memory byte observed after
     * boot. Compare owner/core state; native code never signals a missing event. */
    FieldActor idle_actor={0};FieldActorState idle_state={0};idle_actor.state=&idle_state;
    void *saved_archive=g_GameState.loaded_scene_archive;int saved_palette=g_FieldFxAltPaletteFlag;
    struct {SceneAssetBlob header;SceneAssetDirectory directory;} idle_archive={0};
    idle_archive.header.directoryOffset=sizeof(SceneAssetBlob);g_GameState.loaded_scene_archive=&idle_archive.header;
    u16 idle_scripts[][6]={{4,0,1},{5,1,0,4,0,1},{0,0,0}};
    const u32 held_flags[]={0,0x1000000,0x2000000,0x3000000,0x1000002,0x200180e};
    const int health[]={-1,0,1},idle_modes[]={0,1,4};uint64_t idle_hash=UINT64_C(14695981039346656037);
    for(unsigned script_id=0;script_id<3;++script_id)for(unsigned held=0;held<6;++held)
    for(unsigned hp=0;hp<3;++hp)for(unsigned mode=0;mode<3;++mode)for(unsigned phase=0;phase<6;++phase){
        FieldFx_InitTaskContext((PmSlotHeader *)owner);owner->prefix.actor=&idle_actor;owner->prefix.asset_type=0;
        owner->prefix.reserved00=0;owner->tasks.flags=1;owner->tasks.pc.script=idle_scripts[script_id];
        idle_state=(FieldActorState){0};idle_state.core_flags=held_flags[held];idle_state.control10.command_value=health[hp];
        u8 event_phase=phase?phase-1:0;idle_state.action_state=phase?&event_phase:NULL;idle_actor.mode=idle_modes[mode];
        u32 result=FieldFx_UpdateTasks((PmSlotHeader *)owner);
        const u32 words[]={result,owner->prefix.reserved00,idle_state.core_flags,owner->tasks.flags,
            (u32)((u8 *)owner->tasks.pc.script-(u8 *)idle_scripts[script_id]),idle_state.action_state!=NULL,
            idle_state.action_state?*idle_state.action_state:0};
        for(unsigned i=0;i<7;++i)for(unsigned b=0;b<4;++b)
            idle_hash=(idle_hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
    }
    g_GameState.loaded_scene_archive=saved_archive;g_FieldFxAltPaletteFlag=saved_palette;
    if(idle_hash!=UINT64_C(0x15a8baf2abf03bd9)){fprintf(stderr,"Idle event task hash %016llx\n",(unsigned long long)idle_hash);return 0;}
    puts("Original task event lifetime: 972 USA CPU cases, absent/0..4 event phases, hold/stop/signal, HP and actor-mode gates passed without low-memory writes");
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
    const unsigned orbit_maps[]={12,13,27,30};u8 orbit_reference[0xd10-12];
    for(unsigned m=0;m<4;++m){
        unsigned map=orbit_maps[m];
        if(!pe_disc_read(&disc,&file,0x800ull+0x80093378u-base+(map-1)*8,room_range,sizeof(room_range)))return 0;
        room_sector=(u64)room_range[0]+(room_range[1]&255)+((room_range[1]>>8)&4095);
        if(!pe_disc_read(&disc,&archive,room_sector*2048,room_bytes,sizeof(room_bytes)))return 0;
        if(!m)memcpy(orbit_reference,room_bytes+12,sizeof(orbit_reference));
        else if(memcmp(orbit_reference,room_bytes+12,sizeof(orbit_reference)))return 0;
        struct {u32 callbacks[8];u16 sizes[8];u32 initialize,script;} descriptor;
        memcpy(&descriptor,room_bytes+(0x8018fcb4u-0x8018efe8u),sizeof(descriptor));
        for(unsigned i=0;i<8;++i){
            if(descriptor.callbacks[i]!=(i?0u:0x8018f20cu) || descriptor.sizes[i]!=(i?0u:20u) ||
               g_RoomOrbitFlareEffects.table.callbacks[i]!=(i?NULL:RoomEffect_OrbitFlareController) ||
               g_RoomOrbitFlareEffects.table.sizes[i]!=(i?0u:sizeof(RoomOrbitFlareState)))return 0;
        }
        if(descriptor.initialize!=0x8018fc54u || descriptor.script!=0x8018fc78u ||
           g_RoomOrbitFlareEffects.initialize!=RoomLib_SetArgs3 ||
           memcmp(room_bytes+(descriptor.script-0x8018efe8u),g_RoomOrbitFlareScript,sizeof(g_RoomOrbitFlareScript)))return 0;
    }
    pe_disc_close(&disc);
    puts("Original orbit flare descriptors: M012/M013/M027/M030 identical module bytes, callbacks, sizes, initializer and all 30 script halfwords match the supplied USA disc");
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

static int authored_orbit_flare_cycle(void)
{
    PmPrimarySlot bank[11]={0};FieldActor actor={0},player={0};FieldActorState state={0};u8 action=1;
    RenderMatrix matrices[26]={0};
    for(unsigned i=0;i<26;++i){
        for(unsigned axis=0;axis<3;++axis)matrices[i].rotation[axis][axis]=4096;
        matrices[i].translation[0]=20;matrices[i].translation[2]=1024;
    }
    actor.state=&state;actor.mode=2;actor.render_object.matrices=matrices;
    actor.render_object.target_x=20;actor.render_object.target_z=1024;
    state.action_state=&action;player.pos_x=10000<<16;player.pos_z=10000<<16;
    PolygonVertex polygon[]={{0,-1100,0,-1100},{0,1100,0,-1100},{0,1100,0,1100},{0,-1100,0,1100}};
    PmTables old_tables=g_PmTables;Pe1GameState old_game=g_GameState;u32 old_flags=g_GameStateFlags;
    FieldActor *old_player=g_PlayerEntity;PolygonVertex *old_floor=g_FloorPolygonVertices;
    u16 old_count=g_FloorPolygonVertexCount;
    RenderBufferPrefix old_buffers=D_800B0E38;GteMatrix *old_view=g_GeomScroll.words.viewMatrix;
    int old_slot=g_ActiveDrawSlot,old_offset=g_DrawPacketOffset,old_age=g_FieldFxCurrentAge;
    void *old_program=g_FieldFxTaskPrograms[30];u32 old_random=Psyz_RandState();
    int old_args[3];memcpy(old_args,g_RoomScriptArgs,sizeof(old_args));
    FieldFxTaskContext *old_context=g_FieldFxTaskContext;FieldFxObjectPrefix *old_owner=g_FieldFxOwner;
    FieldFxTaskSlot *old_task=g_FieldFxActiveTaskSlot;
    const u32 patterns[]={0,1,0x7fff,0x8000,0xffff,0x80000000,0xffffffff,0x010c0000,0xa5a55a5a,0x12345678,0x7fffffff,0x600000};
    uint64_t init_hash=UINT64_C(14695981039346656037);
    const int modes[]={0,1,-1};
    for(unsigned m=0;m<3;++m)for(unsigned p=0;p<12;++p){
        intptr_t result=RoomLib_SetArgs3(modes[m],(s32)patterns[p],(s32)patterns[(p+1)%12],(s32)patterns[(p+2)%12]);
        const u32 words[]={result==(intptr_t)g_RoomScriptArgs,(u32)g_RoomScriptArgs[0],(u32)g_RoomScriptArgs[1],(u32)g_RoomScriptArgs[2]};
        for(unsigned j=0;j<4;++j)for(unsigned b=0;b<4;++b)
            init_hash=(init_hash^((words[j]>>(b*8))&255))*UINT64_C(1099511628211);
    }
    if(init_hash!=UINT64_C(0x3c0af86f8810cdb5))return 0;
    uint64_t drift_hash=UINT64_C(14695981039346656037);
    const int drift_modes[]={0,1,3},ages[]={0,1,13,14,15,32767},positions[]={0,1,-1,32767,-32768,1234};
    const u32 seeds[]={1,0xffffffff};
    for(unsigned m=0;m<3;++m)for(unsigned a=0;a<6;++a)for(unsigned p=0;p<6;++p)for(unsigned seed=0;seed<2;++seed){
        RoomFxDriftParticle drift={.x=positions[p],.y=(s16)-positions[p],.z=(s16)(positions[p]^0x5555),
            .pad6=(s16)0xa55a,.radius=0x12345678};
        srand(seeds[seed]);g_FieldFxCurrentAge=ages[a];
        u32 result=RoomEffect_OrbitFlareDrift(drift_modes[m],&drift,0);
        for(unsigned b=0;b<4;++b)drift_hash=(drift_hash^((result>>(b*8))&255))*UINT64_C(1099511628211);
        for(unsigned b=0;b<sizeof(drift);++b)drift_hash=(drift_hash^((u8 *)&drift)[b])*UINT64_C(1099511628211);
        u32 random=Psyz_RandState();
        for(unsigned b=0;b<4;++b)drift_hash=(drift_hash^((random>>(b*8))&255))*UINT64_C(1099511628211);
    }
    if(drift_hash!=UINT64_C(0xf3429d82d5c5a69d))return 0;
    GteMatrix view={.m={{4096,0,0},{0,4096,0},{0,0,4096}}};
    static RenderGpuTag ordering[4096];
    struct {_Alignas(void *) u8 bytes[65536];u8 guard[16];} packets;
    g_PmTables=(PmTables){g_PmCmdHandlerArray,bank,NULL};g_FieldFxTaskPrograms[30]=&g_RoomOrbitFlareEffects;
    g_PlayerEntity=&player;g_FloorPolygonVertices=polygon;g_FloorPolygonVertexCount=4;
    FieldActorState player_state={0};player.state=&player_state;
    FieldFxObjectPrefix owner={.actor=&actor};FieldFxTaskContext context={0};
    g_FieldFxOwner=&owner;g_FieldFxTaskContext=&context;g_FieldFxCurrentAge=0;
    uint64_t controller_hash=UINT64_C(14695981039346656037);
    const u16 frames[]={0,1,15,16,51,52,32767,65535};
    const int flight_positions[]={0,1000,-1000,1100,-1100,3000},angles[]={0,1024,2048,3072,-1},radii[]={0,64,-64};
    for(unsigned phase=1;phase<=3;++phase)for(unsigned f=0;f<8;++f)for(unsigned p=0;p<6;++p)
    for(unsigned angle=0;angle<5;++angle)for(unsigned r=0;r<3;++r)for(unsigned near=0;near<2;++near)for(unsigned flags=0;flags<2;++flags){
        RoomOrbitFlareState flight={.x=flight_positions[p],.y=20,.z=flight_positions[p],.reserved06=-5,
            .state=phase,.frame=frames[f],.soundHandle=-1,.angle=angles[angle],.height=100,.spread=60};
        RoomOrbitFlareSpawn spawn={radii[r],512,1};
        context.flags=flags;state.core_flags=0x01000000;player_state.flags=0;
        player.pos_x=(s32)((u32)(near?flight_positions[p]:10000)<<16);player.pos_z=player.pos_x;
        u32 result=RoomEffect_OrbitFlareController(1,&flight,(intptr_t)&spawn);
        for(unsigned b=0;b<4;++b)controller_hash=(controller_hash^((result>>(b*8))&255))*UINT64_C(1099511628211);
        for(unsigned b=0;b<sizeof(flight);++b)controller_hash=(controller_hash^((u8 *)&flight)[b])*UINT64_C(1099511628211);
        const u32 words[]={state.core_flags,player_state.flags};
        for(unsigned j=0;j<2;++j)for(unsigned b=0;b<4;++b)
            controller_hash=(controller_hash^((words[j]>>(b*8))&255))*UINT64_C(1099511628211);
    }
    if(controller_hash!=UINT64_C(0xa1ad64689555871d)){fprintf(stderr,"Orbit controller hash %016llx\n",(unsigned long long)controller_hash);return 0;}
    player.pos_x=player.pos_z=10000<<16;
    g_GameStateFlags=0x80;g_GameState.flags&=~8u;g_GameState.tim_load_state=0;
    struct {SceneAssetBlob header;SceneAssetDirectory directory;} archive={0};
    archive.header.directoryOffset=sizeof(SceneAssetBlob);g_GameState.loaded_scene_archive=&archive.header;
    g_GeomScroll.words.viewMatrix=&view;
    Psyz_GteCtrlWrite(26,256);Psyz_GteCtrlWrite(27,1);Psyz_GteCtrlWrite(28,0);
    Psyz_GteCtrlWrite(29,1365);Psyz_GteCtrlWrite(30,1024);gte_SetGeomOffset(160,120);
    FieldFx_InitTexturePageTable();srand(1);
    int init=Scene_LoadRoomAssets(115,&actor),send=init?init:Pm_SendCmd(0,0,0,64,512,1);
    if(init || send){fprintf(stderr,"Orbit init=%d send=%d\n",init,send);return 0;}
    if(g_RoomScriptArgs[0]!=64 || g_RoomScriptArgs[1]!=512 || g_RoomScriptArgs[2]!=1 ||
       bank[0].task.tasks.argument!=(intptr_t)g_RoomScriptArgs){fprintf(stderr,"Orbit args %d %d %d\n",g_RoomScriptArgs[0],g_RoomScriptArgs[1],g_RoomScriptArgs[2]);return 0;}
    unsigned drawn=0,linked=0,seen=0,maximum=0;
    for(unsigned frame=0;frame<160;++frame){
        memset(&packets,0xa5,sizeof(packets));
        for(unsigned depth=0;depth<4096;++depth)ordering[depth]=(RenderGpuTag){.address=0xffffff};
        g_ActiveDrawSlot=frame&1;g_DrawPacketOffset=0;
        D_800B0E38.ordering[g_ActiveDrawSlot]=ordering;D_800B0E38.packets[g_ActiveDrawSlot]=(char *)packets.bytes;
        int start=Pm_Start(0),exec=start?start:Pm_Exec(0);
        if(start || exec){fprintf(stderr,"Orbit frame %u start=%d exec=%d\n",frame,start,exec);return 0;}
        for(unsigned i=0;i<8;++i)if(bank[0].task.tasks.slots[i].id!=0xffff){
            seen|=1u<<i;
            if(bank[0].task.tasks.slots[i].age>maximum)maximum=bank[0].task.tasks.slots[i].age;
        }
        if(g_DrawPacketOffset<0 || g_DrawPacketOffset>sizeof(packets.bytes))return 0;
        drawn+=g_DrawPacketOffset!=0;
        for(unsigned depth=0;depth<4096;++depth){
            uintptr_t address=ordering[depth].address;unsigned chain=0;
            while(address!=0xffffff){
                if(address<(uintptr_t)packets.bytes || address+sizeof(RenderGpuTag)>(uintptr_t)packets.bytes+g_DrawPacketOffset ||
                   address%_Alignof(RenderGpuTag) || ++chain>1024)return 0;
                RenderGpuTag *tag=(void *)address;++linked;address=tag->address;
            }
        }
        for(unsigned i=0;i<16;++i)if(packets.guard[i]!=0xa5)return 0;
    }
    if(!drawn || !linked || seen!=7 || maximum<52 || bank[0].task.tasks.count || action!=4){
        fprintf(stderr,"Orbit lifecycle drawn=%u linked=%u seen=%x age=%u count=%u action=%u\n",drawn,linked,seen,maximum,bank[0].task.tasks.count,action);return 0;
    }
    if(Pm_Stop(0,&actor,0)!=0 || bank[0].header.state)return 0;
    g_FieldFxTaskContext=old_context;g_FieldFxOwner=old_owner;g_FieldFxActiveTaskSlot=old_task;g_FieldFxCurrentAge=old_age;
    g_PmTables=old_tables;g_GameState=old_game;g_GameStateFlags=old_flags;g_FieldFxTaskPrograms[30]=old_program;
    g_PlayerEntity=old_player;g_FloorPolygonVertices=old_floor;g_FloorPolygonVertexCount=old_count;
    D_800B0E38=old_buffers;g_GeomScroll.words.viewMatrix=old_view;g_ActiveDrawSlot=old_slot;g_DrawPacketOffset=old_offset;
    srand(old_random);memcpy(g_RoomScriptArgs,old_args,sizeof(old_args));
    puts("Original orbit flare: 36 initializer, 216 drift and 8640 flight/end USA CPU states (BIOS rand boundary controlled) passed");
    puts("Original orbit flare: 160 real PM frames, three authored controller slots, drift list draw/update, native OT guards and complete expiry passed");
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
    const u32 table_values[]={0,1,0x7fff,0x8000,0xffff,0x10000,0x12345678,0x80000000,0xffffffff,0xffff0007,0xa5a55a5a,0x7fffffff};
    uint64_t table_hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[202]!=Script_SetEntityField1E6)return 0;
    for(unsigned i=0;i<sizeof(table_values)/sizeof(table_values[0]);++i){
        FieldActor actor={0};actor.render_object.table_value30=0x1234;actor.render_object.table_index=0xa55a;
        FieldActor before=actor,expected=actor;g_CurrentEntity=&actor;
        int value=(s32)table_values[i];int *args[]={&value};u32 result=Script_SetEntityField1E6(args);
        const u32 words[]={result,(u32)actor.render_object.table_value30|((u32)actor.render_object.table_index<<16)};
        for(unsigned w=0;w<2;++w)for(unsigned b=0;b<4;++b)
            table_hash=(table_hash^((words[w]>>(b*8))&255))*UINT64_C(1099511628211);
        expected.render_object.table_index=actor.render_object.table_index;
        if(memcmp(&actor,&expected,sizeof(actor)))return 0;
        actor=before;u32 program[]={202|(1u<<13),0,table_values[i],32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;
        Script_RunQueue();
        if(result!=1 || memcmp(&actor,&expected,sizeof(actor)) || node.current.script!=program+5 ||
           node.active || node.flags!=0x90 || g_ScriptNodePool)return 0;
    }
    if(table_hash!=UINT64_C(0x7b1afe75d03b90c1))return 0;
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
    puts("Original render table opcode 202: 12 USA CPU narrowing states, adjacent fields, whole actor and dispatcher passed");
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

static int original_tracking_angle_opcode(void)
{
    FieldActor *saved_actor=g_CurrentEntity,*saved_player=g_PlayerEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    const u32 factors[]={0,65536,327680,0x80000000,0x7fffffff};
    const u16 speeds[]={0,1,4096,65535};
    const u32 poses[][4]={{0,0,0,0},{100u<<16,200u<<16,98u<<16,203u<<16},
        {(u32)-100<<16,50u<<16,300u<<16,(u32)-200<<16},
        {0x7fff0000,0x80000000,0x80010000,0x7ffe0000},{0,0,1,0},
        {(u32)-1000<<16,300u<<16,1000u<<16,(u32)-300<<16}};
    const s16 yaws[]={-1,0,1,2048,4095,32767};
    const u32 turns[]={0,1,64,(u32)-64,0x80000000};
    uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[219]!=Render_CameraTrackEntityAngle)return 0;
    for(unsigned player=0;player<2;++player)for(unsigned resumed=0;resumed<2;++resumed)
    for(unsigned factor=0;factor<5;++factor)for(unsigned speed=0;speed<4;++speed)
    for(unsigned pose=0;pose<6;++pose)for(unsigned yaw=0;yaw<6;++yaw)for(unsigned turn=0;turn<5;++turn){
        FieldActor actor={.pos_x=(s32)poses[pose][0],.pos_z=(s32)poses[pose][1],.rot_y=yaws[yaw],
            .move_factor=(s32)factors[factor],.move_speed=speeds[speed],.motion_x=0x1234,.motion_z=0x5678,.flags=0x240};
        FieldActor before=actor;
        ScriptNode node={.flags=resumed?0xa0:0x80,.active=7,.target14=resumed?(s32)poses[pose][2]:0x1234,
            .target18.coordinate=resumed?(s32)poses[pose][3]:0x5678,.target1c=resumed?(s32)turns[turn]:0x9abc};
        ScriptNode node_before=node;u32 storage[32]={0};
        int values[]={resumed?0x11220000:(s32)poses[pose][2],resumed?0x33440000:(s32)poses[pose][3],resumed?7:(s32)turns[turn]};
        int *args[]={values,values+1,values+2};
        g_CurrentEntity=&actor;g_PlayerEntity=player?&actor:NULL;g_ScriptNodePool=&node;g_ScriptCursor=storage+16;
        u32 result=Render_CameraTrackEntityAngle(args);
        const u32 words[]={result,(u32)actor.pos_x,(u32)actor.pos_z,(u32)actor.motion_x,(u32)actor.motion_z,
            (u16)actor.rot_y,actor.flags,node.flags,(u32)node.active,(u32)node.target14,
            (u32)node.target18.coordinate,(u32)node.target1c,(u32)(g_ScriptCursorAddress-(uintptr_t)storage)};
        for(unsigned w=0;w<13;++w)for(unsigned b=0;b<4;++b)
            hash=(hash^((words[w]>>(b*8))&255))*UINT64_C(1099511628211);
        FieldActor expected=before;expected.pos_x=actor.pos_x;expected.pos_z=actor.pos_z;
        expected.motion_x=actor.motion_x;expected.motion_z=actor.motion_z;expected.rot_y=actor.rot_y;
        if(memcmp(&actor,&expected,sizeof(actor)))return 0;
        ScriptNode after=node;actor=before;node=node_before;
        u32 program[]={219|(3u<<13),0,(u32)values[0],(u32)values[1],(u32)values[2],32,0};
        node.current.script=program;node.active=1;g_ScriptNodePool=&node;Script_RunQueue();
        if(memcmp(&actor,&expected,sizeof(actor)) || node.current.script!=program+(result?7:0) ||
           node.active!=!result || node.flags!=(after.flags|(result?0x10:0)) ||
           node.target14!=after.target14 || node.target18.coordinate!=after.target18.coordinate ||
           node.target1c!=after.target1c || g_ScriptNodePool)return 0;
    }
    g_CurrentEntity=saved_actor;g_PlayerEntity=saved_player;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0xc90373fa22ae5275)){fprintf(stderr,"Tracking angle hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original tracking opcode 219: 14400 unhooked USA CPU states, player scaling, signed wrap, retained goals, turn/arrival and native dispatcher passed");
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

static int original_wait_opcode_bindings(void)
{
    const u8 enabled[]={0,1,2,127,128,255},wide[]={0,1,255};
    const s16 elapsed[]={-32768,-1,0,1,2,32767};
    const s32 targets[]={-32768,-1,0,1,2,32767,32768,INT32_MIN,INT32_MAX};
    u8 saved_enabled=g_DrawEnabled.bits,saved_wide=g_MovieWide;short saved_elapsed=g_SeqElapsed;
    uintptr_t saved_cursor=g_ScriptCursorAddress;ScriptNode *saved_node=g_ScriptNodePool;
    FieldActor *saved_actor=g_CurrentEntity;
    int (*handlers[])(int **)={Script_WaitFrameOrDraw,Entity_YieldOnDrawBusy};
    const unsigned opcodes[]={140,143};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    for(unsigned op=0;op<2;++op)for(unsigned e=0;e<6;++e)for(unsigned time=0;time<6;++time)
    for(unsigned target=0;target<9;++target)for(unsigned w=0;w<3;++w){
        u32 program[6]={0};ScriptNode node={.active=7};int value=targets[target],*args[]={&value};
        g_DrawEnabled.bits=enabled[e];g_MovieWide=wide[w];g_SeqElapsed=elapsed[time];
        g_ScriptNodePool=&node;g_ScriptCursor=program+3;
        if(g_ScriptQueueOpcodeHandlers[opcodes[op]]!=handlers[op])return 0;
        u32 result=g_ScriptQueueOpcodeHandlers[opcodes[op]](args);
        const u32 values[]={result,(u32)(g_ScriptCursorAddress-(uintptr_t)program),(u32)node.active};
        for(unsigned i=0;i<3;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((values[i]>>(b*8))&255))*UINT64_C(1099511628211);
        if(value!=targets[target])return 0;++cases;
    }
    if(cases!=1944 || hash!=UINT64_C(0x5ccf0cb9f8ead07a))return 0;
    FieldActor actor={0};g_CurrentEntity=&actor;
    u32 program[]={140|(1u<<13),0,2,143,0,32,0};
    ScriptNode node={.current.script=program,.active=1,.flags=0x80};
    g_DrawEnabled.bits=2;g_MovieWide=1;g_SeqElapsed=1;
    g_ScriptNodePool=&node;Script_RunQueue();
    if(node.current.script!=program || node.active!=1 || (node.flags&0x10))return 0;
    g_SeqElapsed=2;g_ScriptNodePool=&node;Script_RunQueue();
    if(node.current.script!=program+3 || node.active!=1 || (node.flags&0x10))return 0;
    g_DrawEnabled.bits=0;g_ScriptNodePool=&node;Script_RunQueue();
    if(node.current.script!=program+7 || !(node.flags&0x10) || g_ScriptNodePool)return 0;
    g_DrawEnabled.bits=saved_enabled;g_MovieWide=saved_wide;g_SeqElapsed=saved_elapsed;
    g_ScriptCursorAddress=saved_cursor;g_ScriptNodePool=saved_node;g_CurrentEntity=saved_actor;
    puts("Original frame/draw waits: 1944 unhooked USA MIPS states and three-tick dispatcher yield/resume/finish passed");
    return 1;
}

static int original_field_sfx_opcode_binding(void)
{
    const u32 patterns[]={0,1,127,128,255,256,0xffffffff,0x80000000};
    const u16 senders[]={0,1,65535,0x1234};
    FieldActor *saved_actor=g_CurrentEntity,*saved_head=g_FieldActorListHead;
    ScriptNode *saved_pool=g_ScriptNodePool,*saved_free=g_ScriptNodeFreeListHead;
    u32 *saved_cursor=g_ScriptCursor;u16 saved_seq=g_ScriptNodeSeqCounter[0];
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    Script_ClearSfxTable();
    if(g_ScriptQueueOpcodeHandlers[28]!=Script_PlayFieldSfx)return 0;
#define SFX_PUT(value,bytes) do {u32 v_=(u32)(value);for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned kind=0;kind<8;++kind)for(unsigned sub=0;sub<8;++sub)
    for(unsigned task=0;task<8;++task)for(unsigned sender=0;sender<4;++sender){
        u32 callback[]={32,0};
        FieldActor actor={.field_sfx_id=senders[sender]};
        FieldActor recipient={.type_id=(u8)patterns[kind],.sub_id=(u8)patterns[sub],.script_cursor_19c=callback};
        ScriptNode available[2]={{0}};available[0].next=available+1;
        g_CurrentEntity=&actor;g_FieldActorListHead=&recipient;g_ScriptNodeFreeListHead=available;
        u32 program[]={28|(3u<<13),0,patterns[kind],patterns[sub],patterns[task],32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;
        Script_RunQueue();
        if(!(node.flags&0x10) || node.current.script!=program+7 || g_ScriptNodePool || g_FieldSfxQueueCount!=1)return 0;
        SFX_PUT(1,4);SFX_PUT(g_FieldSfxQueueCount,1);
        const u8 *payload=(const u8 *)&g_FieldSfxQueue[0];
        for(unsigned byte=0;byte<sizeof(FieldSfxQueueEntry);++byte)SFX_PUT(payload[byte],1);
        Scene_UpdateEntityList();ScriptNode *event=recipient.script_node_lists[2];
        if(g_FieldSfxQueueCount)return 0;
        if((u16)patterns[kind]>255){
            if(event || g_ScriptNodeFreeListHead!=available)return 0;
        }else if(event!=available || event->current.script!=callback || event->flags!=4 || event->active!=1 ||
           event->target14!=(u8)patterns[task] || event->trigger_value!=senders[sender] ||
           g_ScriptNodeFreeListHead!=available+1)return 0;
        ++cases;
    }
#undef SFX_PUT
    g_CurrentEntity=saved_actor;g_FieldActorListHead=saved_head;g_ScriptNodePool=saved_pool;
    g_ScriptNodeFreeListHead=saved_free;g_ScriptCursor=saved_cursor;g_ScriptNodeSeqCounter[0]=saved_seq;
    if(cases!=2048 || hash!=UINT64_C(0xafaf0f79b0115df5))return 0;
    puts("Original field-event opcode: 2048 USA MIPS payloads match full dispatcher, queue and native recipient-node delivery");
    return 1;
}

static int original_flag_opcode_bindings(void)
{
    const u32 patterns[]={0,1,2,3,0x40,0x41,0x80,0x7fffffff,0x80000000,0xffffffff,0x12345678,0xfedcba98};
    int (*handlers[])(int **)={Entity_ClearGlobalFlag1,Entity_SetGlobalFlag1,Entity_SetCurrentFlag40,Entity_ClearCurrentFlag40};
    FieldActor *saved_actor=g_CurrentEntity;ScriptNode *saved_node=g_ScriptNodePool;
    u32 *saved_cursor=g_ScriptCursor;int saved_lock=g_FieldMoveLock;
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    for(unsigned op=0;op<4;++op)for(unsigned m=0;m<12;++m)for(unsigned f=0;f<12;++f){
        FieldActor actor={.flags=patterns[f]};g_CurrentEntity=&actor;g_FieldMoveLock=(s32)patterns[m];
        if(g_ScriptQueueOpcodeHandlers[63+op]!=handlers[op])return 0;
        u32 result=g_ScriptQueueOpcodeHandlers[63+op](NULL);
        const u32 values[]={result,(u32)g_FieldMoveLock,actor.flags};
        for(unsigned i=0;i<3;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((values[i]>>(b*8))&255))*UINT64_C(1099511628211);
        u32 program[]={63+op,0,32,0};ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || !(node.flags&0x10) || node.current.script!=program+4 || g_ScriptNodePool ||
           (u32)g_FieldMoveLock!=values[1] || actor.flags!=values[2])return 0;
        ++cases;
    }
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;g_FieldMoveLock=saved_lock;
    if(cases!=576 || hash!=UINT64_C(0x4c0f80b0374806b5))return 0;
    RenderTintState saved_tint;memcpy(&saved_tint,&g_ScreenTint,sizeof(saved_tint));
    hash=UINT64_C(14695981039346656037);
    FieldActor actor={0};g_CurrentEntity=&actor;
    if(g_ScriptQueueOpcodeHandlers[136]!=Script_ClearEntityFlag40)return 0;
    for(unsigned fade=0;fade<256;++fade){
        memcpy(&g_ScreenTint,&saved_tint,sizeof(saved_tint));g_ScreenTint.fadeMode=fade;
        u32 result=g_ScriptQueueOpcodeHandlers[136](NULL);
        for(unsigned b=0;b<4;++b)hash=(hash^((result>>(b*8))&255))*UINT64_C(1099511628211);
        hash=(hash^g_ScreenTint.fadeMode)*UINT64_C(1099511628211);
        RenderTintState expected;memcpy(&expected,&saved_tint,sizeof(expected));expected.fadeMode=g_ScreenTint.fadeMode;
        u32 program[]={136,0,32,0};ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || !(node.flags&0x10) || node.current.script!=program+4 || g_ScriptNodePool ||
           memcmp(&g_ScreenTint,&expected,sizeof(expected)))return 0;
    }
    int (*noops[])(int **)={Script_Noop1,Script_Noop2,Script_Noop4,Script_Noop3};
    const unsigned opcodes[]={141,142,144,145};
    for(unsigned i=0;i<4;++i){
        if(g_ScriptQueueOpcodeHandlers[opcodes[i]]!=noops[i] || noops[i](NULL)!=1)return 0;
        u32 program[]={opcodes[i],0,32,0};ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        g_ScriptNodePool=&node;Script_RunQueue();
        if(!(node.flags&0x10) || node.current.script!=program+4 || g_ScriptNodePool)return 0;
    }
    memcpy(&g_ScreenTint,&saved_tint,sizeof(saved_tint));
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x14fd97a6d6c2c325))return 0;
    u32 saved_flags=g_GameStateFlags;
    const u32 flag_values[]={0,1,2,4,0x100,0x800,0x2000,0x80000000,0xffffffff,0xa5a55a5a};
    int (*global_handlers[])(int **)={Scene_TestGlobalFlags,Scene_SetGlobalFlags,Scene_ClearGlobalFlags};
    uint64_t global_hash=UINT64_C(14695981039346656037);
    g_CurrentEntity=&actor;
    for(unsigned op=81;op<=83;++op)for(unsigned f=0;f<10;++f)for(unsigned m=0;m<10;++m){
        int mask=(s32)flag_values[m],out=(s32)0x98765432u;int *args[]={&mask,&out};
        if(g_ScriptQueueOpcodeHandlers[op]!=global_handlers[op-81])return 0;
        g_GameStateFlags=flag_values[f];u32 result=g_ScriptQueueOpcodeHandlers[op](args);
        const u32 values[]={result,g_GameStateFlags,(u32)out};
        for(unsigned i=0;i<3;++i)for(unsigned byte=0;byte<4;++byte)
            global_hash=(global_hash^((values[i]>>(byte*8))&255))*UINT64_C(1099511628211);
        g_GameStateFlags=flag_values[f];
        u32 program[]={op|((op==81?2u:1u)<<13),0,flag_values[m],op==81?0x98765432u:32u,op==81?32u:0u,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || g_GameStateFlags!=values[1] || (op==81 && program[3]!=values[2]) ||
           !(node.flags&0x10) || node.current.script!=program+(op==81?6:5) || g_ScriptNodePool)return 0;
    }
    g_GameStateFlags=saved_flags;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(global_hash!=UINT64_C(0x7d8e36c81c7f7588))return 0;
    u8 saved_escape=g_BattleEnemyEscapeFlag;
    if(g_ScriptQueueOpcodeHandlers[207]!=Script_SignalSceneExit)return 0;
    g_CurrentEntity=&actor;g_BattleEnemyEscapeFlag=0;
    u32 exit_program[]={207,0,32,0};ScriptNode exit_node={.current.script=exit_program,.active=1,.flags=0x80};
    g_ScriptNodePool=&exit_node;Script_RunQueue();
    if(g_BattleEnemyEscapeFlag!=1 || exit_node.current.script!=exit_program+4 || exit_node.flags!=0x90 ||
       exit_node.active || g_ScriptNodePool)return 0;
    g_BattleEnemyEscapeFlag=saved_escape;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    puts("Original battle sequence signal opcode 207: native escape latch and real dispatcher completion passed");
    const u32 actor_values[]={0,1,8,0x100,0x400,0x508,0x7fffffff,0x80000000,0xffffffff,0x12345678,0x20,0xa5a55a5a};
    const unsigned actor_ops[]={74,79,80,185,186,193,194,195};
    int (*actor_handlers[])(int **)={Entity_TestCurrentFlags,Entity_SetCurrentFlag100,Entity_ClearCurrentFlag100,
        Script_ClearEntityFlag1,Script_SetEntityFlag1,Script_SetEntityFlag400,Script_ClearEntityFlag400,Script_WaitEntityFlags8};
    uint64_t actor_hash=UINT64_C(14695981039346656037);g_CurrentEntity=&actor;
    for(unsigned op=0;op<8;++op)for(unsigned f=0;f<12;++f)for(unsigned m=0;m<12;++m)for(unsigned phase=0;phase<2;++phase){
        if(g_ScriptQueueOpcodeHandlers[actor_ops[op]]!=actor_handlers[op])return 0;
        int mask=(s32)actor_values[m],out=(s32)0x98765432;int *args[]={&mask,&out};u32 scratch[16]={0};
        ScriptNode node={.active=7,.flags=phase?0xa0:0x80};g_ScriptNodePool=&node;g_ScriptCursor=scratch+8;actor.flags=actor_values[f];
        u32 result=actor_handlers[op](args);const u32 words[]={result,actor.flags,(u32)out,node.flags,(u32)node.active,
            (u32)(g_ScriptCursorAddress-(uintptr_t)scratch)};
        for(unsigned i=0;i<6;++i)for(unsigned b=0;b<4;++b)actor_hash=(actor_hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
        actor.flags=actor_values[f];u32 program[6]={actor_ops[op],0,32,0};unsigned length=4;
        if(op==0){program[0]|=2u<<13;program[2]=(u32)mask;program[3]=0x98765432;program[4]=32;length=6;}
        node=(ScriptNode){.current.script=program,.active=1,.flags=phase?0xa0:0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(actor.flags!=words[1] || (op==0 && program[3]!=(u32)out) || node.current.script!=program+(result?length:0) ||
           node.active!=(result?0:1) || node.flags!=(words[3]|(result?0x10:0)) || g_ScriptNodePool)return 0;
    }
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(actor_hash!=UINT64_C(0xab04cf6d428f3bf9)){fprintf(stderr,"Actor flag hash %016llx\n",(unsigned long long)actor_hash);return 0;}
    puts("Original actor flag opcodes 74/79/80/185/186/193/194/195: 2304 unhooked USA CPU cases, all-bit mask test, native flag updates and dispatcher wait/retry passed");
    puts("Original global flag opcodes 81/82/83: 300 USA CPU states, high-bit masks and real dispatcher passed");
    puts("Original flag opcodes: 576 USA MIPS cases, native handler ABI and full dispatcher/node completion passed");
    puts("Original fade release opcode: all 256 USA MIPS values, dispatcher completion and untouched native tint state passed");
    return 1;
}

static int original_polygon_opcode(void)
{
    const s32 polygons[][8]={{-10,-10,10,-10,10,10,-10,10},{-10,10,10,10,10,-10,-10,-10},
        {-10,-10,10,-10,0,0,-10,10},{0,0,0,0,0,0,0,0},
        {INT32_MIN,INT32_MIN,INT32_MAX,INT32_MIN,INT32_MAX,INT32_MAX,INT32_MIN,INT32_MAX},
        {INT32_MAX,-1,INT32_MIN,1,0,INT32_MAX,1,INT32_MIN}};
    const u32 points[]={0x80000000,0xffffffff,0,1,65535,65536,0x7fffffff,0x100000,0xfff00000};
    FieldActor *saved_actor=g_CurrentEntity;ScriptNode *saved_node=g_ScriptNodePool;
    u32 *saved_cursor=g_ScriptCursor;u32 saved_var=g_ScriptVariableBank[9];
    FieldActor actor={0};g_CurrentEntity=&actor;uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[119]!=Script_PointInPoly)return 0;
    for(unsigned p=0;p<6;++p)for(unsigned x=0;x<9;++x)for(unsigned z=0;z<9;++z){
        int data[11];memcpy(data,polygons[p],sizeof(polygons[p]));
        data[8]=(s32)points[x];data[9]=(s32)points[z];data[10]=(s32)0xa5a5a5a5;
        int before[11];memcpy(before,data,sizeof(data));int *args[11];
        for(unsigned i=0;i<11;++i)args[i]=data+i;
        u32 result=Script_PointInPoly(args);u32 output[]={result,(u32)data[10]};
        for(unsigned i=0;i<2;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((output[i]>>(b*8))&255))*UINT64_C(1099511628211);
        if(memcmp(data,before,10*sizeof(int))){fprintf(stderr,"Polygon input changed %u %u %u\n",p,x,z);return 0;}
        u32 program[15]={119|(11u<<13),2u<<15};
        for(unsigned i=0;i<10;++i)program[i+2]=(u32)before[i];
        program[12]=9;program[13]=32;g_ScriptVariableBank[9]=0xa5a5a5a5;
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || g_ScriptVariableBank[9]!=output[1] || node.current.script!=program+15 ||
           !(node.flags&0x10) || g_ScriptNodePool){fprintf(stderr,"Polygon dispatch %u %u %u result=%u var=%u want=%u cursor=%llu flags=%x\n",p,x,z,result,g_ScriptVariableBank[9],output[1],(unsigned long long)(node.current.script-program),node.flags);return 0;}
    }
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;g_ScriptVariableBank[9]=saved_var;
    if(hash!=UINT64_C(0xd928a71a360ec985)){fprintf(stderr,"Polygon hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original polygon opcode: 486 USA MIPS boundary, winding, degenerate and overflow states, eleven operands and native dispatcher passed");
    return 1;
}

static int original_battle_entry_opcode(void)
{
    Pe1GameState saved_state=g_GameState;FieldActor actor={0},*saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    const u8 phases[]={0,59,60,255},stories[]={0,1,2,3,4,128,252,255};
    const u32 flags[]={0,1,0x400000,0x800000,0xa5a55a5a,0xffffffff};
    uint64_t hash=UINT64_C(14695981039346656037);g_CurrentEntity=&actor;
    if(g_ScriptQueueOpcodeHandlers[85]!=Boot_MemCardStepInit)return 0;
    for(unsigned p=0;p<4;++p)for(unsigned s=0;s<8;++s)for(unsigned f=0;f<6;++f){
        int mode=0x12340045;int *args[]={&mode};u32 scratch[16]={0};
        ScriptNode node={.active=7,.flags=0x80};g_ScriptNodePool=&node;g_ScriptCursor=scratch+8;
        g_GameState.flags=flags[f];g_GameState.memcard_init_phase=phases[p];g_GameState.story_day_flags=stories[s];
        u32 result=g_ScriptQueueOpcodeHandlers[85](args);
        const u32 words[]={result,g_GameState.flags,g_GameState.memcard_init_phase,(u32)node.active,
            (u32)(g_ScriptCursorAddress-(uintptr_t)scratch)};
        for(unsigned i=0;i<5;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
        if(node.flags!=0x80)return 0;
        u32 program[]={85|(1u<<13),0,(u32)mode,32,0};
        node=(ScriptNode){.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;
        g_GameState.flags=flags[f];g_GameState.memcard_init_phase=phases[p];g_GameState.story_day_flags=stories[s];
        Script_RunQueue();
        if(g_GameState.flags!=words[1] || g_GameState.memcard_init_phase!=words[2] || g_ScriptNodePool ||
           node.current.script!=program+(result?5:0) || node.active!=(result?0:1) ||
           node.flags!=(result?0x90:0x80))return 0;
    }
    ScriptNode node={.active=7};u32 scratch[16]={0};int mode=0;int *args[]={&mode};
    g_ScriptNodePool=&node;g_ScriptCursor=scratch+8;g_GameState.memcard_init_phase=0x39;
    g_GameState.tim_load_state=0x34;g_GameState.scene_load_scratch=NULL;
    if(Boot_MemCardStepInit(args)!=-1 || g_GameState.memcard_init_phase!=0x39 ||
       g_GameState.tim_load_state!=0x34 || node.active!=7 || g_ScriptCursor!=scratch+8)return 0;
    g_GameState=saved_state;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0xdcc852ebc349be75))return 0;
    int saved_mode=g_BattleModeState;
    const unsigned mode_ops[]={137,138,149,150};const int modes[]={6,5,0,8};
    int (*mode_handlers[])(int **)={Menu_SetModeState6,Menu_SetModeState5,Menu_ClearModeState,Menu_SetModeState8};
    for(unsigned m=0;m<4;++m){
        unsigned op=mode_ops[m];int (*handler)(int **)=mode_handlers[m];
        if(g_ScriptQueueOpcodeHandlers[op]!=handler)return 0;
        for(unsigned i=0;i<6;++i){
            g_BattleModeState=(s32)flags[i];
            u32 program[]={op,0,32,0};ScriptNode dispatched={.current.script=program,.active=1,.flags=0x80};
            g_CurrentEntity=&actor;g_ScriptNodePool=&dispatched;Script_RunQueue();
            if(g_BattleModeState!=modes[m] || g_ScriptNodePool || dispatched.active ||
               dispatched.flags!=0x90 || dispatched.current.script!=program+4)return 0;
        }
    }
    g_BattleModeState=saved_mode;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    puts("Original battle mode opcodes 137/138/149/150: four native handlers and dispatcher completion passed");
    puts("Original battle entry opcode 85: 192 USA CPU phase/flag/yield states, real dispatcher and texture-load error propagation passed");return 1;
}

static int original_conditional_branch_opcode(void)
{
    FieldActor actor={0},*saved_actor=g_CurrentEntity;ScriptNode *saved_node=g_ScriptNodePool;
    u32 *saved_cursor=g_ScriptCursor;u32 program[10]={0};actor.script_base=(u8 *)program;g_CurrentEntity=&actor;
    const u32 offsets[]={0,1,16,0x7fffffff,0x80000000,0xffffffff};
    if(g_ScriptQueueOpcodeHandlers[29]!=Script_CondBranch)return 0;
    for(unsigned equal=0;equal<2;++equal)for(unsigned i=0;i<6;++i){
        int a=1,b=equal?1:2,offset=(s32)offsets[i];int *args[]={&a,&b,&offset};
        g_ScriptCursor=program+3;
        uintptr_t expected=equal?(uintptr_t)(program+3):(uintptr_t)program+(intptr_t)(s32)(offsets[i]<<1);
        if(g_ScriptQueueOpcodeHandlers[29](args)!=1 || g_ScriptCursorAddress!=expected)return 0;
    }
    for(unsigned equal=0;equal<2;++equal){
        u32 script[]={29|(3u<<13),0,1,equal?1u:2u,16,32,0,0,32,0};
        actor.script_base=(u8 *)script;ScriptNode node={.current.script=script,.active=1,.flags=0x80};g_ScriptNodePool=&node;
        Script_RunQueue();if(node.current.script!=script+(equal?7:10) || !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    puts("Original opcode 29: native conditional target, 32-bit offset wrapping, both paths and real dispatcher passed");return 1;
}

static int original_context_getter_opcodes(void)
{
    FieldActor *saved_list=g_FieldActorListHead,*saved_player=g_PlayerEntity,*saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    const u32 patterns[]={0,1,0x7fff,0x8000,0xffff,0x80000000,0xffffffff,0x010c0000,0xa5a55a5a,0x12345678,0x7fffffff,0x600000};
    FieldActor actor={0};Combatant player;EnemyCombatant enemy;
    uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[89]!=Entity_GetField || g_ScriptQueueOpcodeHandlers[139]!=Entity_FindByTypeId)return 0;
    for(unsigned kind=0;kind<2;++kind)for(unsigned p=0;p<12;++p)for(unsigned cmd=0;cmd<512;++cmd){
        u32 raw[0xd8/4];for(unsigned i=0;i<0xd8/4;++i)raw[i]=patterns[p];
        raw[0]=(raw[0]&~0xe00000u)|((p%6)<<21);
        memset(&player,0,sizeof(player));memcpy(&player,raw,0x1c);
        memcpy(&player.maxHP,(u8 *)raw+0x1c,0xd8-0x1c);
        memset(&enemy,0,sizeof(enemy));memcpy(&enemy,raw,0x18);
        memcpy(enemy.records,(u8 *)raw+0x1c,0xd8-0x1c);
        actor.state=(FieldActorState *)(kind?(void *)&enemy:(void *)&player);
        actor.type_id=kind?6:0;actor.sub_id=3;actor.flags=0;actor.next=NULL;
        g_PlayerEntity=&actor;g_CurrentEntity=&actor;g_FieldActorListHead=&actor;
        int command=cmd,out=(s32)0x98765432;int *args[]={&command,&out};
        u32 result=Entity_GetField(args),post_flags=kind?enemy.statusFlags2:player.statusFlags2;
        const u32 words[]={result,(u32)out,post_flags};
        for(unsigned i=0;i<3;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
        for(unsigned typed=0;typed<2;++typed){
            player.statusFlags2=patterns[p];enemy.statusFlags2=patterns[p];
            u32 program[8];unsigned length;
            if(typed){u32 code[]={139|(4u<<13),0,actor.type_id,3,cmd,0x98765432,32,0};memcpy(program,code,sizeof(code));length=8;}
            else{u32 code[]={89|(2u<<13),0,cmd,0x98765432,32,0};memcpy(program,code,sizeof(code));length=6;}
            ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
            if(result!=1 || program[typed?5:3]!=(u32)out || (kind?enemy.statusFlags2:player.statusFlags2)!=post_flags ||
               node.current.script!=program+length || node.flags!=0x90 || node.active || g_ScriptNodePool){
                fprintf(stderr,"Context getter kind=%u pattern=%u cmd=%u typed=%u\n",kind,p,cmd,typed);return 0;}
        }
    }
    FieldActor missing[3]={{0}};
    for(unsigned i=0;i<3;++i){missing[i].type_id=6;missing[i].sub_id=i?3:2;missing[i].next=i<2?&missing[i+1]:NULL;missing[i].flags=i?0x10:0;}
    for(unsigned mode=0;mode<3;++mode){
        g_FieldActorListHead=mode==0?NULL:missing;u32 code[]={139|(4u<<13),0,mode==2?7:6,3,43,0x98765432,32,0};
        ScriptNode node={.current.script=code,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(code[5]!=0x98765432 || node.current.script!=code+8 || node.flags!=0x90 || g_ScriptNodePool)return 0;
    }
    g_FieldActorListHead=saved_list;g_PlayerEntity=saved_player;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0xbc5b5bf975c75f8d)){fprintf(stderr,"Context getter hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original context getters 89/139: 12288 unhooked USA CPU states, all command bytes, signed HP, event records, status consumption and dispatcher selection passed");return 1;
}

static int original_context_setter_opcodes(void)
{
    FieldActor *saved_list=g_FieldActorListHead,*saved_player=g_PlayerEntity,*saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;s16 saved_floor=g_RoomFloorY->y;
    const u32 patterns[]={0,1,0x7fff,0x8000,0xffff,0x80000000,0xffffffff,0x010c0000,0xa5a55a5a,0x12345678,0x7fffffff,0x600000};
    FieldActor actor={0};Combatant player;FieldActorState enemy;
    uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[90]!=Entity_SetField)return 0;
    for(unsigned kind=0;kind<2;++kind)for(unsigned p=0;p<12;++p)for(unsigned cmd=0;cmd<512;++cmd){
        u32 raw[0xd8/4];for(unsigned i=0;i<0xd8/4;++i)raw[i]=patterns[p];
        memset(&player,0,sizeof(player));memcpy(&player,raw,0x1c);memcpy(&player.maxHP,(u8 *)raw+0x1c,0xd8-0x1c);
        memset(&enemy,0,sizeof(enemy));memcpy(&enemy,raw,0x18);memcpy(&enemy.count_limit,(u8 *)raw+0x1c,0xd8-0x1c);
        actor.state=kind?&enemy:(FieldActorState *)&player;actor.type_id=kind?6:0;actor.sub_id=3;actor.flags=0;actor.next=NULL;
        g_PlayerEntity=&actor;g_CurrentEntity=&actor;g_FieldActorListHead=&actor;g_RoomFloorY->y=0x1234;
        Combatant before_player=player;FieldActorState before_enemy=enemy;
        int command=cmd,value=(s32)patterns[(p+cmd)%12];int *args[]={&command,&value};u32 result=Entity_SetField(args);
        u8 after[0xd8];
        if(kind){memcpy(after,&enemy,0x18);memcpy(after+0x18,(u8 *)raw+0x18,4);memcpy(after+0x1c,&enemy.count_limit,0xd8-0x1c);}
        else{memcpy(after,&player,0x1c);memcpy(after+0x1c,&player.maxHP,0xd8-0x1c);}
        for(unsigned b=0;b<4;++b)hash=(hash^((result>>(b*8))&255))*UINT64_C(1099511628211);
        for(unsigned b=0;b<sizeof(after);++b)hash=(hash^after[b])*UINT64_C(1099511628211);
        u16 floor=g_RoomFloorY->y;for(unsigned b=0;b<2;++b)hash=(hash^((floor>>(b*8))&255))*UINT64_C(1099511628211);
        Combatant after_player=player;FieldActorState after_enemy=enemy;
        for(unsigned typed=0;typed<2;++typed){
            player=before_player;enemy=before_enemy;g_RoomFloorY->y=0x1234;
            u32 program[8];unsigned length;
            if(typed){u32 code[]={206|(4u<<13),0,actor.type_id,3,cmd,(u32)value,32,0};memcpy(program,code,sizeof(code));length=8;}
            else{u32 code[]={90|(2u<<13),0,cmd,(u32)value,32,0};memcpy(program,code,sizeof(code));length=6;}
            ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
            if(result!=1 || memcmp(&player,&after_player,sizeof(player)) || memcmp(&enemy,&after_enemy,sizeof(enemy)) ||
               (u16)g_RoomFloorY->y!=floor || node.current.script!=program+length || node.flags!=0x90 || node.active || g_ScriptNodePool)return 0;
        }
    }
    g_RoomFloorY->y=saved_floor;g_FieldActorListHead=saved_list;g_PlayerEntity=saved_player;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x2249f25ad3a196bd)){fprintf(stderr,"Context setter hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original context setters 90/206: 12288 unhooked USA CPU states, all command bytes, complete actor/floor writes and both dispatcher forms passed");return 1;
}

static int original_script_event_entries(void)
{
    FieldActor actor={0},*saved_actor=g_CurrentEntity;ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    EntityAnimEventCore core;actor.state=(FieldActorState *)&core;g_CurrentEntity=&actor;
    const u32 patterns[]={0,1,0x7fff,0x8000,0xffff,0x80000000,0xffffffff,0x010c0000,0xa5a55a5a,0x12345678,0x7fffffff,0x600000};
    const unsigned ops[]={112,183,176},counts[]={11,5,3};
    int (*handlers[])(int **)={Script_SetEntityAnim12Args,Script_SetEntityAnim5Args,Script_SetBattleEntryCoords};
    uint64_t hash=UINT64_C(14695981039346656037);
    for(unsigned op=0;op<3;++op)for(unsigned slot=0;slot<6;++slot)for(unsigned p=0;p<12;++p){
        memset(&core,0,sizeof(core));memset(&core,0xa5,0x18);memset(core.records,0xa5,0xb4-0x1c);
        EntityAnimEventCore before=core;int values[11];int *args[11];
        for(unsigned i=0;i<11;++i){values[i]=i?(s32)patterns[(p+i)%12]:(int)(slot|((p%2)<<8));args[i]=values+i;}
        if(g_ScriptQueueOpcodeHandlers[ops[op]]!=handlers[op])return 0;
        u32 result=handlers[op](args);EntityAnimEventCore after=core;
        u8 raw[0xb4];memcpy(raw,&core,0x18);memset(raw+0x18,0xa5,4);memcpy(raw+0x1c,core.records,0xb4-0x1c);
        for(unsigned b=0;b<4;++b)hash=(hash^((result>>(b*8))&255))*UINT64_C(1099511628211);
        for(unsigned b=0;b<sizeof(raw);++b)hash=(hash^raw[b])*UINT64_C(1099511628211);
        core=before;u32 program[15]={ops[op]|(counts[op]<<13),0};
        for(unsigned i=0;i<counts[op];++i)program[2+i]=(u32)values[i];
        program[2+counts[op]]=32;
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || memcmp(&core,&after,sizeof(core)) || node.current.script!=program+4+counts[op] ||
           node.flags!=0x90 || node.active || g_ScriptNodePool)return 0;
    }
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0xf942d785949a4441)){fprintf(stderr,"Script event setup hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original event setup opcodes 112/183/176: 216 unhooked USA CPU states, six records, narrowed operands, all parameters and full dispatcher passed");return 1;
}

static int original_animation_event_opcode(void)
{
    FieldActor *old_actor=g_CurrentEntity,*old_player=g_PlayerEntity,*old_head=g_FieldActorListHead;
    ScriptNode *old_node=g_ScriptNodePool;u32 *old_cursor=g_ScriptCursor;u32 old_flags=g_GameStateFlags;
    unsigned old_sounds=sounds;void *old_rows[48];memcpy(old_rows,g_GameState.bank_rows[0],sizeof(old_rows));
    RenderAnimationDataHeader animation={0};animation.frame_count=9;
    for(unsigned i=0;i<48;++i)g_GameState.bank_rows[0][i]=&animation;
    g_PlayerEntity=NULL;g_FieldActorListHead=NULL;g_GameStateFlags=2;
    const unsigned modes[]={0,1,2,6,7,8,9,14,15},flags[]={0,0x6000,0x40000000,0x80000000};
    const unsigned frames[][2]={{2,4},{8,9},{8,1}},kinds[]={0,2,2,4};
    uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[100]!=Script_RunEntityCommand)return 0;
    for(unsigned slot=0;slot<6;++slot)for(unsigned variant=0;variant<4;++variant)for(unsigned m=0;m<9;++m)
    for(unsigned state=0;state<5;++state)for(unsigned f=0;f<3;++f)for(unsigned mask=0;mask<4;++mask){
        EntityAnimEventCore core={0};BattleEntity actors[2]={{0}};
        core.flags.raw=(slot<<21)|flags[mask];core.kind=kinds[variant];core.baseMode=4;core.assetId=1000;
        core.active=core.records+slot;
        for(unsigned i=0;i<6;++i){core.records[i].state=i==slot?state:0;core.records[i].enterMode=6;core.records[i].exitMode=7;
            core.records[i].enterStep=0x18000;core.records[i].exitStep=0x24000;core.records[i].category=slot%2?3:1;core.records[i].frame=3;}
        for(unsigned i=0;i<2;++i){actors[i].core=&core;actors[i].entityFlags=0x240;actors[i].animLastFrame=8;actors[i].animStep=0x12000;}
        if(variant>=2)actors[0].parent=actors+1;
        BattleEntity *selected=actors+(variant>=2);selected->actionMode=modes[m];
        selected->animPrev.fixed=frames[f][0]<<16;selected->animFrame=frames[f][1]<<16;
        EntityAnimEventCore before_core=core;BattleEntity before_actors[2];memcpy(before_actors,actors,sizeof(actors));
        g_CurrentEntity=(FieldActor *)actors;ScriptNode node={.active=7};u32 scratch[16]={0};
        g_ScriptNodePool=&node;g_ScriptCursor=scratch+8;sounds=0;
        int command=slot|((mask%2)<<8),out=(s32)0x98765432;int *args[]={&command,&out};
        u32 result=Script_RunEntityCommand(args);u32 words[27]={result,(u32)out,(u32)(g_ScriptCursorAddress-(uintptr_t)scratch),
            (u32)node.active,core.flags.raw,core.active?1+(u32)(core.active-core.records):0};
        for(unsigned i=0;i<6;++i)words[6+i]=core.records[i].state;
        for(unsigned i=0;i<2;++i){u32 values[]={actors[i].entityFlags,actors[i].actionMode,actors[i].animLastFrame,
            actors[i].animFrame,(u32)actors[i].animPrev.fixed,(u32)actors[i].animStep,actors[i].actionData==&animation};
            memcpy(words+12+i*7,values,sizeof(values));}
        words[26]=sounds;
        for(unsigned i=0;i<27;++i)for(unsigned b=0;b<4;++b)hash=(hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
        EntityAnimEventCore after_core=core;BattleEntity after_actors[2];memcpy(after_actors,actors,sizeof(actors));
        core=before_core;memcpy(actors,before_actors,sizeof(actors));sounds=0;
        u32 code[]={100|(2u<<13),0,(u32)command,0x98765432,32,0};
        node=(ScriptNode){.current.script=code,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(code[3]!=(u32)out || memcmp(&core,&after_core,sizeof(core)) || memcmp(actors,after_actors,sizeof(actors)) ||
           sounds!=words[26] || node.current.script!=code+(result?6:0) || node.active!=(result?0:1) ||
           node.flags!=(result?0x90:0x80) || g_ScriptNodePool)return 0;
    }
    g_CurrentEntity=old_actor;g_PlayerEntity=old_player;g_FieldActorListHead=old_head;g_ScriptNodePool=old_node;g_ScriptCursor=old_cursor;
    g_GameStateFlags=old_flags;sounds=old_sounds;memcpy(g_GameState.bank_rows[0],old_rows,sizeof(old_rows));
    if(hash!=UINT64_C(0xaa6e5b4256b2bf25)){fprintf(stderr,"Animation event hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original animation event opcode 100: 12960 USA CPU cases with recorded audio boundary, parent selection, frames/wrap, six records, all state changes and dispatcher retry passed");return 1;
}

static int original_action_wait_opcode(void)
{
    FieldActor actor={0},*old_actor=g_CurrentEntity,*old_head=g_FieldActorListHead,*old_player=g_PlayerEntity;
    ScriptNode *old_node=g_ScriptNodePool;u32 *old_cursor=g_ScriptCursor,old_flags=g_GameStateFlags;
    void *old_rows[48];memcpy(old_rows,g_GameState.bank_rows[0],sizeof(old_rows));
    RenderAnimationDataHeader animation={0};animation.frame_count=9;
    for(unsigned i=0;i<48;++i)g_GameState.bank_rows[0][i]=&animation;
    g_CurrentEntity=&actor;g_FieldActorListHead=NULL;g_PlayerEntity=NULL;g_GameStateFlags=0;
    const u32 node_flags[]={0x80,0xa0},modes[]={0,1,4,0x10006,0xffff0007},flags[]={0,0x100,0x100000,0xffffffff};
    const u32 steps[]={0,65536,0x80000000,0xffffffff},frames[][2]={{0,0},{1,0},{0,1},{0x80000000,0x7fffffff},{0xffffffff,0},{0,0xffffffff}};
    const u8 lasts[]={0,1,8};uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[198]!=Script_SetEntityActionAndWait)return 0;
    for(unsigned n=0;n<2;++n)for(unsigned m=0;m<5;++m)for(unsigned f=0;f<4;++f)
    for(unsigned l=0;l<3;++l)for(unsigned step=0;step<4;++step)for(unsigned frame=0;frame<6;++frame){
        actor=(FieldActor){0};actor.mode=2;actor.action=lasts[l];actor.flags=flags[f];
        actor.anim.fixed=(s32)frames[frame][0];actor.anim_prev=(s32)frames[frame][1];actor.anim_step=(s32)steps[step];
        FieldActor before=actor;ScriptNode node={.active=7,.flags=node_flags[n]};u32 scratch[16]={0};
        g_ScriptNodePool=&node;g_ScriptCursor=scratch+8;int mode=(s32)modes[m];int *args[]={&mode};
        u32 result=Script_SetEntityActionAndWait(args);FieldActor after=actor;
        const u32 words[]={result,node.flags,(u32)node.active,(u32)(g_ScriptCursorAddress-(uintptr_t)scratch),actor.flags,
            actor.mode,actor.action,(u32)actor.anim.fixed,(u32)actor.anim_prev,(u32)actor.anim_step,actor.action_data==&animation};
        for(unsigned i=0;i<11;++i)for(unsigned b=0;b<4;++b)hash=(hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
        actor=before;u32 code[]={198|(1u<<13),0,modes[m],32,0};
        node=(ScriptNode){.current.script=code,.active=1,.flags=node_flags[n]};g_ScriptNodePool=&node;Script_RunQueue();
        if(memcmp(&actor,&after,sizeof(actor)) || node.current.script!=code+(result?5:0) || node.active!=(result?0:1) ||
           node.flags!=(words[1]|(result?0x10:0)) || g_ScriptNodePool)return 0;
    }
    g_CurrentEntity=old_actor;g_FieldActorListHead=old_head;g_PlayerEntity=old_player;g_GameStateFlags=old_flags;
    g_ScriptNodePool=old_node;g_ScriptCursor=old_cursor;memcpy(g_GameState.bank_rows[0],old_rows,sizeof(old_rows));
    if(hash!=UINT64_C(0x27a316c7f7829479)){fprintf(stderr,"Action wait hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original action wait opcode 198: 2880 unhooked USA CPU states, action initialization, 16-bit mode narrowing, signed direction, wrap and dispatcher retry/completion passed");return 1;
}

static int original_typed_field_opcode(void)
{
    FieldActor *saved_list=g_FieldActorListHead,*saved_player=g_PlayerEntity,*saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    FieldActor actors[3]={0},player={0};FieldActorState states[3]={0};Combatant core={0};
    for(unsigned i=0;i<3;++i){actors[i].state=&states[i];actors[i].type_id=6;
        actors[i].sub_id=i?2:1;actors[i].next=i<2?&actors[i+1]:NULL;}
    actors[1].flags=0x10;player.state=(FieldActorState *)&core;g_PlayerEntity=&player;g_CurrentEntity=&player;
    if(g_ScriptQueueOpcodeHandlers[206]!=Entity_SetFieldByTypeId)return 0;
    for(unsigned mode=0;mode<5;++mode)for(int value=0;value<5;++value){
        memset(states,0,sizeof(states));FieldActorState expected[3]={0};
        int operands[]={mode==4?7:6,mode==3?9:2,0x150,value};
        int *args[]={operands,operands+1,operands+2,operands+3};
        g_FieldActorListHead=mode==1?actors+1:mode==2?NULL:actors;
        if(mode<2){expected[2].behavior_mode=value;
            expected[2].behavior_timer=value==1?400:value==2?70:value==3?20:0;}
        if(g_ScriptQueueOpcodeHandlers[206](args)!=1 || memcmp(states,expected,sizeof(states)))return 0;
        memset(states,0,sizeof(states));
        u32 program[]={206|(4u<<13),0,(u32)operands[0],(u32)operands[1],(u32)operands[2],(u32)value,32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(memcmp(states,expected,sizeof(states)) || node.current.script!=program+8 ||
           !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    int operands[]={0,99,0x104,1234};int *args[]={operands,operands+1,operands+2,operands+3};
    if(g_ScriptQueueOpcodeHandlers[206](args)!=1 || core.curHP!=1234)return 0;
    core.curHP=0;Combatant expected_core={0};expected_core.curHP=1234;
    u32 program[]={206|(4u<<13),0,0,99,0x104,1234,32,0};
    ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
    if(memcmp(&core,&expected_core,sizeof(core)) || node.current.script!=program+8 || g_ScriptNodePool)return 0;
    s16 saved_floor=g_RoomFloorY->y;g_PlayerEntity=NULL;
    operands[2]=255;operands[3]=-1111;
    if(g_ScriptQueueOpcodeHandlers[206](args)!=1 || g_RoomFloorY->y!=-1111)return 0;
    g_RoomFloorY->y=saved_floor;
    g_FieldActorListHead=saved_list;g_PlayerEntity=saved_player;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    puts("Original opcode 206: native actor selection, dead/missing/subtype filtering, command-byte truncation, behavior timers, player HP and real dispatcher passed");
    return 1;
}

static int original_collision_gate_opcodes(void)
{
    CollisionDatabase *saved_db=g_CollisionDb;struct CollisionPlane *saved_planes=g_CollisionPlaneTable;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    FieldActor actor={0},*saved_actor=g_CurrentEntity;g_CurrentEntity=&actor;
    CollisionDatabase db={0};_Alignas(16) u8 triangles[116];
    db.triangles.pointer=triangles+16;g_CollisionDb=&db;
    const unsigned ops[]={128,129};uint64_t hash=UINT64_C(14695981039346656037);
    for(unsigned op=0;op<2;++op)for(unsigned mode=0;mode<2;++mode){
        size_t stride=mode?sizeof(CollisionTriangleXYZ):sizeof(CollisionTriangleXZ);
        g_CollisionPlaneTable=mode?(struct CollisionPlane *)triangles:NULL;
        if(g_ScriptQueueOpcodeHandlers[ops[op]]!=(op?Script_ClearObjEntryFlag80:Script_SetObjEntryFlag80))return 0;
        for(int index=0;index<3;++index)for(unsigned kind=0;kind<256;++kind){
            memset(triangles,0xa5,sizeof(triangles));
            for(unsigned i=0;i<3;++i)triangles[16+i*stride]=kind;
            u8 before[sizeof(triangles)],expected[sizeof(triangles)];memcpy(before,triangles,sizeof(before));
            int *args[]={&index};u32 result=g_ScriptQueueOpcodeHandlers[ops[op]](args);
            for(unsigned b=0;b<4;++b)hash=(hash^((result>>(b*8))&255))*UINT64_C(1099511628211);
            for(unsigned b=0;b<sizeof(triangles);++b)hash=(hash^triangles[b])*UINT64_C(1099511628211);
            memcpy(expected,before,sizeof(expected));expected[16+index*stride]=op?kind&0x7f:kind|0x80;
            if(result!=1 || memcmp(expected,triangles,sizeof(triangles)))return 0;
            memcpy(triangles,before,sizeof(triangles));u32 program[]={ops[op]|(1u<<13),0,(u32)index,32,0};
            ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
            if(memcmp(expected,triangles,sizeof(triangles)) || node.current.script!=program+5 ||
               !(node.flags&0x10) || g_ScriptNodePool)return 0;
        }
    }
    g_CollisionDb=saved_db;g_CollisionPlaneTable=saved_planes;g_CurrentEntity=saved_actor;
    g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x12d5c65f0d2fc325))return 0;
    PolygonVertex *saved_wall=g_EntityCollisionWallSlot,*saved_floor_polygon=g_FloorPolygonVertices;
    s16 saved_count=g_EntityCollisionWallParam;u16 saved_floor_count=g_FloorPolygonVertexCount,saved_radius=g_CollisionRadius;
    FieldActor player={0},controller={0},*saved_player=g_PlayerEntity;_Alignas(8) u8 polygon_script[128]={0};
    const PolygonVertex square[]={{0,-100,0,-100},{0,100,0,-100},{0,100,0,100},{0,-100,0,100}};
    memcpy(polygon_script+64,square,sizeof(square));controller.script_base=polygon_script;
    g_CurrentEntity=&controller;g_PlayerEntity=&player;g_FloorPolygonVertices=(PolygonVertex *)polygon_script;g_FloorPolygonVertexCount=3;
    u32 setup[]={172|(2u<<13),0,32,0x10004,32,0};ScriptNode node={.current.script=setup,.active=1,.flags=0x80};
    g_ScriptNodePool=&node;Script_RunQueue();
    if(g_EntityCollisionWallSlot!=(PolygonVertex *)(polygon_script+64) || (u16)g_EntityCollisionWallParam!=4 ||
       g_FloorPolygonVertices!=(PolygonVertex *)polygon_script || g_FloorPolygonVertexCount!=3 ||
       node.current.script!=setup+6 || node.flags!=0x90 || g_ScriptNodePool)return 0;
    const int wall_points[]={-110,-105,-100,-95,-90,0,90,95,100,105,110},speeds[]={2048,4096};
    uint64_t response_hash=UINT64_C(14695981039346656037);
    for(unsigned speed=0;speed<2;++speed)for(unsigned x=0;x<11;++x)for(unsigned z=0;z<11;++z){
        memset(&player,0,sizeof(player));player.move_speed=speeds[speed];player.render_object.hit_cylinder.radius=10;
        player.pos_x=(s32)((u32)wall_points[x]<<16);player.pos_z=(s32)((u32)wall_points[z]<<16);
        Entity_ApplyCollisionResponse(NULL);
        const u32 words[]={(u32)player.pos_x,(u32)player.pos_y,(u32)player.pos_z,g_CollisionRadius};
        for(unsigned i=0;i<4;++i)for(unsigned b=0;b<(i==3?2:4);++b)
            response_hash=(response_hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
    }
    g_EntityCollisionWallSlot=saved_wall;g_EntityCollisionWallParam=saved_count;g_CollisionRadius=saved_radius;
    g_FloorPolygonVertices=saved_floor_polygon;g_FloorPolygonVertexCount=saved_floor_count;
    g_PlayerEntity=saved_player;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(response_hash!=UINT64_C(0x99967d7c2a10f007)){fprintf(stderr,"Wall response hash %016llx\n",(unsigned long long)response_hash);return 0;}
    puts("Original wall binding: opcode 172 shared native state, separate floor polygon and 242 USA CPU/GTE-bridge collision responses passed");
    puts("Original collision gate opcodes: 3072 USA MIPS states, both triangle layouts, all kind bits, neighbouring records and real dispatcher passed");
    return 1;
}

static int original_scroll_opcodes(void)
{
    const u32 values[]={0,1,32767,32768,65535,65536,0x80000000,0xffffffff,0x12345678};
    const u32 flags[]={0,1,0x3f,0x40,0x48,0x7f,0x80000040,0xffffffff};
    const u32 durations[]={0,1,30,65535,65536};const unsigned ops[]={3,70};
    GeomScrollRecord saved=g_GeomScroll;ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    FieldActor actor={0},*saved_actor=g_CurrentEntity;g_CurrentEntity=&actor;
    uint64_t hash=UINT64_C(14695981039346656037);
    for(unsigned op=0;op<2;++op)for(unsigned f=0;f<8;++f)for(unsigned i=0;i<9;++i)for(unsigned d=0;d<5;++d){
        if(g_ScriptQueueOpcodeHandlers[ops[op]]!=(op?Script_SetFogParamsAlt:Script_SetFogParams))return 0;
        memset(&g_GeomScroll,0xa5,sizeof(g_GeomScroll));g_GeomScroll.state.flags=(s32)flags[f];
        GeomScrollRecord before=g_GeomScroll;
        int data[]={(s32)values[i],(s32)values[(i+3)%9],(s32)durations[d]};int *args[]={data,data+1,data+2};
        u32 result=g_ScriptQueueOpcodeHandlers[ops[op]](args);u16 halves[12];
        memcpy(halves,&g_GeomScroll.state.position,sizeof(halves));
        const u32 words[]={result,(u32)g_GeomScroll.state.flags};
        for(unsigned j=0;j<14;++j){u32 value=j<2?words[j]:halves[j-2];
            for(unsigned b=0;b<4;++b)hash=(hash^((value>>(b*8))&255))*UINT64_C(1099511628211);}
        GeomScrollRecord expected=before;
        if(flags[f]&0x40){
            expected.state.flags=(flags[f]&~15u)|(op?9:1);
            expected.words.start=before.words.position;
            expected.state.position.targetX=(u16)data[0];expected.state.position.targetY=(u16)data[1];
            expected.state.position.elapsed=1;expected.state.position.duration=(u16)data[2];
        }
        if(result!=1 || memcmp(&g_GeomScroll,&expected,sizeof(expected)))return 0;
        g_GeomScroll=before;u32 program[]={ops[op]|(3u<<13),0,values[i],values[(i+3)%9],durations[d],32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(memcmp(&g_GeomScroll,&expected,sizeof(expected)) || node.current.script!=program+7 ||
           !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    g_GeomScroll=saved;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;g_CurrentEntity=saved_actor;
    if(hash!=UINT64_C(0x93501ab9832625cd)){fprintf(stderr,"Scroll opcode hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original scroll opcodes: 720 USA MIPS states, signed coordinates, unsigned duration, disabled camera and full dispatcher passed");
    return 1;
}

static int original_motion_opcodes(void)
{
    const u32 factors[]={0,1,65536,327680,0x80000000,0x7fffffff};
    const u16 speeds[]={0,1,4096,65535};
    const s16 angles[]={-32768,-1,0,1,1024,2048,3072,4095,32767};
    const u32 patterns[]={0,1,2,3,0x40,0x41,0x80,0x7fffffff,0x80000000,0xffffffff,0x12345678,0xfedcba98};
    FieldActor *saved_actor=g_CurrentEntity,*saved_player=g_PlayerEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    uint64_t velocity=UINT64_C(14695981039346656037),reset=velocity;
    const unsigned scalar_ops[]={77,93};
    int (*scalar_handlers[])(int **)={Script_SetEntityField1C,Script_SetCurrentEntityField20};
    for(unsigned op=0;op<2;++op){
        uint64_t factor_hash=velocity;
        if(g_ScriptQueueOpcodeHandlers[scalar_ops[op]]!=scalar_handlers[op])return 0;
        for(unsigned f=0;f<6;++f)for(unsigned p=0;p<12;++p){
            FieldActor actor={.anim_step=(s32)0x98765432,.move_factor=(s32)0x98765432,.flags=patterns[p]};
            FieldActor before=actor,expected=actor;
            g_CurrentEntity=&actor;int factor=(s32)factors[f];int *args[]={&factor};
            u32 result=scalar_handlers[op](args);
            const u32 data[]={result,(u32)(op?actor.move_factor:actor.anim_step),actor.flags};
            for(unsigned i=0;i<3;++i)for(unsigned b=0;b<4;++b)
                factor_hash=(factor_hash^((data[i]>>(b*8))&255))*UINT64_C(1099511628211);
            if(op)expected.move_factor=actor.move_factor;else expected.anim_step=actor.anim_step;
            if(memcmp(&expected,&actor,sizeof(actor)))return 0;
            actor=before;u32 program[]={scalar_ops[op]|(1u<<13),0,factors[f],32,0};
            ScriptNode node={.current.script=program,.active=1,.flags=0x80};
            g_ScriptNodePool=&node;Script_RunQueue();
            if(result!=1 || memcmp(&expected,&actor,sizeof(actor)) ||
               node.current.script!=program+5 || node.active || node.flags!=0x90 || g_ScriptNodePool)return 0;
        }
        if(factor_hash!=UINT64_C(0x3bf7cb6b9837996d))return 0;
    }
    if(g_ScriptQueueOpcodeHandlers[101]!=Entity_ClearMotionVectors ||
       g_ScriptQueueOpcodeHandlers[102]!=Entity_ComputeVelocity)return 0;
    for(unsigned player=0;player<2;++player)for(unsigned f=0;f<6;++f)
    for(unsigned speed=0;speed<4;++speed)for(unsigned angle=0;angle<9;++angle){
        FieldActor actor={.move_factor=(s32)factors[f],.move_speed=speeds[speed],.rot_y=angles[angle],
            .motion_x=0x1234,.motion_z=0x5678};
        FieldActor before=actor;g_CurrentEntity=&actor;g_PlayerEntity=player?&actor:NULL;
        u32 result=Entity_ComputeVelocity(NULL);const u32 data[]={result,(u32)actor.motion_x,(u32)actor.motion_z};
        for(unsigned i=0;i<3;++i)for(unsigned b=0;b<4;++b)
            velocity=(velocity^((data[i]>>(b*8))&255))*UINT64_C(1099511628211);
        FieldActor expected=before;expected.motion_x=actor.motion_x;expected.motion_z=actor.motion_z;
        if(memcmp(&expected,&actor,sizeof(actor)))return 0;
        actor=before;u32 program[]={102,0,32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || memcmp(&expected,&actor,sizeof(actor)) || node.current.script!=program+4 ||
           !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    for(unsigned p=0;p<12;++p)for(unsigned shift=0;shift<8;++shift){
        FieldActor actor={.flags=patterns[p]},expected;g_CurrentEntity=&actor;g_PlayerEntity=NULL;
        int *vectors[]={&actor.motion_x,&actor.motion_y,&actor.motion_z,&actor.accel_x,&actor.accel_y,&actor.accel_z};
        for(unsigned i=0;i<6;++i)*vectors[i]=(s32)patterns[(p+shift+i)%12];
        FieldActor before=actor;u32 result=Entity_ClearMotionVectors(NULL);
        const u32 data[]={result,(u32)actor.motion_x,(u32)actor.motion_y,(u32)actor.motion_z,
            (u32)actor.accel_x,(u32)actor.accel_y,(u32)actor.accel_z,actor.flags};
        for(unsigned i=0;i<8;++i)for(unsigned b=0;b<4;++b)
            reset=(reset^((data[i]>>(b*8))&255))*UINT64_C(1099511628211);
        expected=before;expected.motion_x=expected.motion_y=expected.motion_z=0;
        expected.accel_x=expected.accel_y=expected.accel_z=0;
        if(memcmp(&expected,&actor,sizeof(actor)))return 0;
        actor=before;u32 program[]={101,0,32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || memcmp(&expected,&actor,sizeof(actor)) || node.current.script!=program+4 ||
           !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    uint64_t flags_hash=UINT64_C(14695981039346656037);
    for(unsigned op=120;op<=121;++op)for(unsigned bit=0;bit<32;++bit)for(unsigned inverse=0;inverse<2;++inverse){
        FieldActor actor={.flags=(1u<<bit)^(inverse?UINT32_MAX:0)},before=actor,expected=actor;
        if(op==120)expected.flags&=~0x20u;else expected.flags|=0x20;
        g_CurrentEntity=&actor;
        if(g_ScriptQueueOpcodeHandlers[op]!=(op==120?Entity_ClearCurrentFlag20:Entity_SetCurrentFlag20))return 0;
        u32 result=g_ScriptQueueOpcodeHandlers[op](NULL);const u32 data[]={result,actor.flags};
        for(unsigned i=0;i<2;++i)for(unsigned b=0;b<4;++b)
            flags_hash=(flags_hash^((data[i]>>(b*8))&255))*UINT64_C(1099511628211);
        if(result!=1 || memcmp(&actor,&expected,sizeof(actor)))return 0;
        actor=before;u32 program[]={op,0,32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(memcmp(&actor,&expected,sizeof(actor)) || node.current.script!=program+4 ||
           !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    g_CurrentEntity=saved_actor;g_PlayerEntity=saved_player;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(flags_hash!=UINT64_C(0xf3302e2482610125))return 0;
    if(velocity!=UINT64_C(0xd151bdfd099e675a) || reset!=UINT64_C(0x137482f2419b4165)){
        fprintf(stderr,"Motion opcode hashes %016llx %016llx\n",(unsigned long long)velocity,(unsigned long long)reset);return 0;}
    puts("Original motion opcodes: 432 velocity, 96 reset and 128 actor flag20 USA MIPS states, native player scaling, wrap and full dispatcher passed");
    puts("Original animation step/movement factor opcodes 77/93: 144 USA CPU states, whole-actor guards and native dispatcher passed");
    return 1;
}

static int original_dialogue_opcodes(void)
{
    const unsigned ops[]={13,34,35};
    int (*handlers[])(int **)={Script_StopEntityAnim,Script_YieldWhileTableEntrySet,Script_ClearTableEntry};
    const u32 open_pages[]={0,1,32767,32768,65535,65536,0x80000000,0xffffffff,0x12345678};
    const u32 wait_pages[]={5,6,7,8,0xffffffff,0,65541,0x80000000,0x7fffffff};
    const u8 movements[]={0,1,255};
    const s16 pages[4][4]={{5,6,7,8},{5,5,5,5},{-1,5,-1,5},{-1,-1,-1,-1}};
    const u8 states[8][4]={{0,0,0,0},{1,2,3,4},{0,3,2,1},{128,255,127,1},
        {255,0,3,0},{0,0,255,0},{0,0,0,255},{2,2,2,2}};
    TextboxEntry saved[4];memcpy(saved,g_TextboxEntries,sizeof(saved));
    u8 saved_movement=g_PlayerMovementEnabled,saved_choice=g_DialogueChoiceCursor;
    s8 saved_menu=g_MenuEquipSlotIndex;FieldActor *saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    u8 markers[4];FieldActor actor={0};g_CurrentEntity=&actor;
    uint64_t hashes[2]={UINT64_C(14695981039346656037),UINT64_C(14695981039346656037)};
    for(unsigned op=0;op<3;++op)for(unsigned layout=0;layout<(op?4u:16u);++layout)
    for(unsigned middle=0;middle<(op?8u:9u);++middle)for(unsigned last=0;last<(op?9u:3u);++last){
        if(g_ScriptQueueOpcodeHandlers[ops[op]]!=handlers[op])return 0;
        memset(g_TextboxEntries,0xa5,sizeof(saved));
        for(unsigned i=0;i<4;++i){
            g_TextboxEntries[i].message=markers+i;
            g_TextboxEntries[i].state=op?states[middle][i]:(layout&(1u<<i))?3:0;
            if(op)g_TextboxEntries[i].page_id=pages[layout][i];
        }
        g_DialogueChoiceCursor=0x12;g_MenuEquipSlotIndex=-7;
        if(!op)g_PlayerMovementEnabled=movements[last];
        TextboxEntry before[4];memcpy(before,g_TextboxEntries,sizeof(before));
        int value=(s32)(op?wait_pages[last]:open_pages[middle]);int *args[]={&value};
        u32 storage[4]={0};ScriptNode node={.active=7};g_ScriptNodePool=&node;g_ScriptCursor=storage+3;
        u32 result=handlers[op](args);uint64_t hash=hashes[op?1:0];
#define DIALOGUE_PUT(value,bytes) do {u32 put_=(u32)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)put_)*UINT64_C(1099511628211);put_>>=8;}}while(0)
        DIALOGUE_PUT(result,4);
        if(op){DIALOGUE_PUT((uintptr_t)g_ScriptCursor-(uintptr_t)storage,4);DIALOGUE_PUT(node.active,4);}
        for(unsigned i=0;i<4;++i){
            TextboxEntry *entry=&g_TextboxEntries[i];
            if(entry->message!=markers+i)return 0;
            for(unsigned b=0;b<4;++b)DIALOGUE_PUT(((u8 *)entry)[b],1);
            DIALOGUE_PUT(0x80180000u+i*256,4);
            for(unsigned b=0;b<48;++b)DIALOGUE_PUT(((u8 *)&entry->style)[b],1);
        }
        if(!op){DIALOGUE_PUT(g_DialogueChoiceCursor,1);DIALOGUE_PUT(g_MenuEquipSlotIndex,1);}
#undef DIALOGUE_PUT
        hashes[op?1:0]=hash;
        TextboxEntry expected[4];memcpy(expected,g_TextboxEntries,sizeof(expected));
        u8 choice=g_DialogueChoiceCursor;s8 menu=g_MenuEquipSlotIndex;
        memcpy(g_TextboxEntries,before,sizeof(before));g_DialogueChoiceCursor=0x12;g_MenuEquipSlotIndex=-7;
        u32 program[]={ops[op]|(1u<<13),0,(u32)value,32,0};
        node=(ScriptNode){.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(g_ScriptNodePool || node.current.script!=(result?program+5:program) || node.active!=!result ||
           !!(node.flags&0x10)!=!!result || memcmp(expected,g_TextboxEntries,sizeof(expected)) ||
           choice!=g_DialogueChoiceCursor || menu!=g_MenuEquipSlotIndex)return 0;
    }
    /* USA 80017DE4 executes the real signed-byte menu getter, including
     * cancellation (-1); the opcode writes a complete script word. */
    if(g_ScriptQueueOpcodeHandlers[67]!=Script_GetMenuChoice)return 0;
    uint64_t choice_hash=UINT64_C(14695981039346656037);
    u32 saved_variable=g_ScriptVariableBank[9];
    for(unsigned choice=0;choice<256;++choice){
        g_MenuEquipSlotIndex=(s8)choice;
        int output=(s32)0x98765432;int *args[]={&output};
        u32 result=Script_GetMenuChoice(args);
        const u32 words[]={result,(u32)output,(u8)g_MenuEquipSlotIndex};
        for(unsigned i=0;i<3;++i)for(unsigned b=0;b<4;++b)
            choice_hash=(choice_hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
        u32 program[]={67|(1u<<13)|(2u<<17),0,9,32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        g_ScriptVariableBank[9]=0x98765432;g_ScriptNodePool=&node;
        Script_RunQueue();
        if(g_ScriptVariableBank[9]!=(u32)output || node.current.script!=program+5 ||
           node.active || node.flags!=0x90 || g_ScriptNodePool ||
           (u8)g_MenuEquipSlotIndex!=choice)return 0;
    }
    g_ScriptVariableBank[9]=saved_variable;
    if(choice_hash!=UINT64_C(0xffdfd872a5725525))return 0;
    memcpy(g_TextboxEntries,saved,sizeof(saved));g_PlayerMovementEnabled=saved_movement;
    g_DialogueChoiceCursor=saved_choice;g_MenuEquipSlotIndex=saved_menu;g_CurrentEntity=saved_actor;
    g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hashes[0]!=UINT64_C(0x76d0cc627a0cae51) || hashes[1]!=UINT64_C(0x1506f2e400f8d88f)){
        fprintf(stderr,"Dialogue opcode hashes %016llx %016llx\n",(unsigned long long)hashes[0],(unsigned long long)hashes[1]);return 0;}
    puts("Original dialogue opcodes: 432 open and 576 wait/close USA MIPS states, native pointers, duplicate pages and full dispatcher passed");
    puts("Original menu choice opcode 67: all 256 signed-byte USA CPU states and native dispatcher outputs passed");
    return 1;
}

static int original_turn_to_actor_opcode(void)
{
    const s32 positions[][2]={{0,0},{65536,0},{0,-65536},{INT32_MIN,INT32_MAX}};
    const s16 angles[]={0,1,1023,1024,2047,2048,3072,4095,-1};
    const int speeds[]={0,1,64,4096,INT32_MAX};
    FieldActor *saved_actor=g_CurrentEntity,*saved_player=g_PlayerEntity,*saved_head=g_FieldActorListHead;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[75]!=Render_CameraSnapToEntity)return 0;
    for(unsigned kind=0;kind<5;++kind)for(unsigned p=0;p<4;++p)
    for(unsigned a=0;a<9;++a)for(unsigned speed=0;speed<5;++speed){
        FieldActor actor={0},target={.type_id=7,.sub_id=11},dead={.type_id=7,.sub_id=11,.flags=0x10};
        if(kind==4)dead.next=&target;
        g_CurrentEntity=&actor;g_PlayerEntity=kind==0?&target:NULL;
        g_FieldActorListHead=kind==2?&target:kind>=3?&dead:NULL;
        actor.pos_x=p==3?INT32_MAX:0;actor.pos_z=p==3?INT32_MIN:0;actor.rot_y=angles[a];
        target.pos_x=positions[p][0];target.pos_z=positions[p][1];
        u32 storage[8]={0};ScriptNode node={.flags=0x80,.active=7};
        g_ScriptNodePool=&node;g_ScriptCursor=storage+5;
        int data[]={kind<2?0:7,11,speeds[speed]};int *args[]={data,data+1,data+2};
        u32 result=Render_CameraSnapToEntity(args);
        const u32 output[]={result,(u16)actor.rot_y,node.flags,(u32)node.active,
            (u32)((uintptr_t)g_ScriptCursor-(uintptr_t)storage),(u32)node.target14,node.target18.actor!=NULL};
        for(unsigned i=0;i<7;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((output[i]>>(b*8))&255))*UINT64_C(1099511628211);
        if(node.target18.actor && node.target18.actor!=&target)return 0;
        u32 program[]={75|(3u<<13),0,(u32)data[0],11,(u32)data[2],32,0};
        node=(ScriptNode){.current.script=program,.active=1,.flags=0x80};actor.rot_y=angles[a];
        g_ScriptNodePool=&node;Script_RunQueue();
        if(g_ScriptNodePool || (u16)actor.rot_y!=output[1] || node.target14!=(s32)output[5] ||
           (node.target18.actor!=NULL)!=output[6] || (node.target18.actor && node.target18.actor!=&target) ||
           node.current.script!=(result?program+7:program) || node.active!=!result ||
           !!(node.flags&0x10)!=!!result)return 0;
    }
    g_CurrentEntity=saved_actor;g_PlayerEntity=saved_player;g_FieldActorListHead=saved_head;
    g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x34c38eb49612044f)){fprintf(stderr,"Turn opcode hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original turn-to-actor opcode: 900 USA MIPS states, live/missing/dead lookup, native target pointers and dispatcher passed");
    return 1;
}

static int original_screen_fade_wait_opcode(void)
{
    RenderTintState saved=g_ScreenTint;FieldActor *saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    FieldActor actor={0};g_CurrentEntity=&actor;uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[156]!=Entity_YieldOnLoading)return 0;
    for(unsigned fade=0;fade<256;++fade){
        u32 storage[4]={0};ScriptNode node={.active=7};
        g_ScriptNodePool=&node;g_ScriptCursor=storage+3;g_ScreenTint.fadeMode=fade;
        u32 result=Entity_YieldOnLoading(NULL);
        const u32 data[]={result,(u32)((uintptr_t)g_ScriptCursor-(uintptr_t)storage),(u32)node.active};
        for(unsigned i=0;i<3;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((data[i]>>(b*8))&255))*UINT64_C(1099511628211);
        u32 program[]={156,0,32,0};node=(ScriptNode){.current.script=program,.active=1,.flags=0x80};
        g_ScriptNodePool=&node;Script_RunQueue();
        int busy=(fade&3)>=2;
        if(g_ScriptNodePool || node.current.script!=(busy?program:program+4) ||
           node.active!=busy || !!(node.flags&0x10)==busy)return 0;
        RenderTintState expected=saved;expected.fadeMode=fade;
        if(memcmp(&g_ScreenTint,&expected,sizeof(expected)))return 0;
    }
    g_ScreenTint=saved;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x3ae97f7ab9537f25))return 0;
    puts("Original fade wait opcode: all 256 USA MIPS modes, cursor rewinds and full dispatcher yield/completion passed");
    return 1;
}

static int original_screen_fade_opcodes(void)
{
    const unsigned ops[]={133,134,135};
    int (*handlers[])(int **)={Script_SetCdRomSeekPos,Script_SetFogColor,Script_SetCdRomScreenPos};
    const u32 values[]={0,1,3,4,65535,0x80000000,0xffffffff,0x12345678};
    RenderTintState saved=g_ScreenTint;FieldActor *saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    FieldActor actor={0};g_CurrentEntity=&actor;uint64_t hash=UINT64_C(14695981039346656037);
    for(unsigned op=0;op<3;++op)for(unsigned seed=0;seed<8;++seed)for(unsigned v=0;v<8;++v){
        if(g_ScriptQueueOpcodeHandlers[ops[op]]!=handlers[op])return 0;
        g_ScreenTint=saved;
        for(unsigned i=0;i<20;++i)((u8 *)&g_ScreenTint.targetR)[i]=(u8)(i*17+seed*31);
        int data[5];int *args[5];for(unsigned i=0;i<5;++i){data[i]=(s32)values[(v+i)%8];args[i]=data+i;}
        u32 result=handlers[op](args);
        for(unsigned b=0;b<4;++b)hash=(hash^((result>>(b*8))&255))*UINT64_C(1099511628211);
        for(unsigned i=0;i<20;++i)hash=(hash^((u8 *)&g_ScreenTint.targetR)[i])*UINT64_C(1099511628211);
        RenderTintState expected=g_ScreenTint;
        if(memcmp(&g_ScreenTint,&saved,PE1_OFFSETOF(RenderTintState,targetR)))return 0;
        g_ScreenTint=saved;
        for(unsigned i=0;i<20;++i)((u8 *)&g_ScreenTint.targetR)[i]=(u8)(i*17+seed*31);
        u32 program[]={ops[op]|(5u<<13),0,(u32)data[0],(u32)data[1],(u32)data[2],(u32)data[3],(u32)data[4],32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || node.current.script!=program+9 || !(node.flags&0x10) || g_ScriptNodePool ||
           memcmp(&expected,&g_ScreenTint,sizeof(expected)))return 0;
    }
    g_ScreenTint=saved;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x0ddf13ae684d5bb5))return 0;
    puts("Original screen fade opcodes: 192 USA MIPS states, full tint state, native packets retained and dispatcher passed");
    return 1;
}

static int original_geometry_control_opcodes(void)
{
    const unsigned ops[]={116,117,123};
    int (*handlers[])(int **)={Script_SetObjEntryLimit,Script_SetFlagSlot,Script_SetObjEntryField8};
    const u32 values[]={0,1,255,256,65535,0x80000000,0xffffffff,0x12345678};
    const u8 flags[]={0,1,2,4,0x20,255};
    GeomState *saved_geom=g_GeomState;FieldActor *saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    FieldActor actor={0};g_CurrentEntity=&actor;uint64_t hash=UINT64_C(14695981039346656037);
    struct {GeomState header;GeomCtrlEntry controls[2];} geometry={0};
    geometry.header.ctrl_offset=(u8 *)geometry.controls-(u8 *)&geometry.header;g_GeomState=&geometry.header;
    for(unsigned op=0;op<3;++op)for(unsigned index=0;index<2;++index)
    for(unsigned v=0;v<8;++v)for(unsigned f=0;f<6;++f){
        if(g_ScriptQueueOpcodeHandlers[ops[op]]!=handlers[op])return 0;
        memset(geometry.controls,0xa5,sizeof(geometry.controls));geometry.controls[index].head.b.flags=flags[f];
        int data[]={index,(s32)values[v]};int *args[]={data,data+1};u32 result=handlers[op](args);
        for(unsigned b=0;b<4;++b)hash=(hash^((result>>(b*8))&255))*UINT64_C(1099511628211);
        for(unsigned i=0;i<sizeof(geometry.controls);++i)
            hash=(hash^((u8 *)geometry.controls)[i])*UINT64_C(1099511628211);
        GeomCtrlEntry expected[2];memcpy(expected,geometry.controls,sizeof(expected));
        memset(geometry.controls,0xa5,sizeof(geometry.controls));geometry.controls[index].head.b.flags=flags[f];
        u32 program[]={ops[op]|(2u<<13),0,index,values[v],32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || node.current.script!=program+6 || !(node.flags&0x10) || g_ScriptNodePool ||
           memcmp(expected,geometry.controls,sizeof(expected)))return 0;
    }
    g_GeomState=saved_geom;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x18383904d1c6e665))return 0;
    puts("Original geometry control opcodes: 288 USA MIPS states, untouched adjacent controls and full dispatcher passed");
    return 1;
}

static int original_shadow_radius_opcode(void)
{
    const s16 radii[]={-32768,-32767,-1,0,1,123,32766,32767};
    const u32 values[]={0,1,0xffffffff,32767,0xffff8000,65535,65536,0x7fffffff,0x80000000,0x12345678,0xfedcba98};
    FieldActor *saved_actor=g_CurrentEntity;ScriptNode *saved_node=g_ScriptNodePool;
    u32 *saved_cursor=g_ScriptCursor;uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[157]!=Script_ScaleAnimValue)return 0;
    for(unsigned r=0;r<8;++r)for(unsigned v=0;v<11;++v){
        RenderObjectHeader header={0},expected={0};header.shadow_radius=0x7654;
        FieldActor actor={0};actor.render_object.header=&header;
        actor.render_object.hit_cylinder.radius=radii[r];g_CurrentEntity=&actor;
        int value=(s32)values[v];int *args[]={&value};u32 result=g_ScriptQueueOpcodeHandlers[157](args);
        for(unsigned b=0;b<4;++b)hash=(hash^((result>>(b*8))&255))*UINT64_C(1099511628211);
        u16 radius=header.shadow_radius;
        for(unsigned b=0;b<2;++b)hash=(hash^((radius>>(b*8))&255))*UINT64_C(1099511628211);
        expected.shadow_radius=radius;
        if(memcmp(&header,&expected,sizeof(header)))return 0;
        u32 program[]={157|(1u<<13),0,values[v],32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        header.shadow_radius=0x7654;g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || memcmp(&header,&expected,sizeof(header)) || node.current.script!=program+5 ||
           !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x0cb313b7d38c9570))return 0;
    puts("Original shadow-radius opcode: 88 USA MIPS signed/overflow states and native dispatcher completion passed");
    return 1;
}

static int original_camera_flags_opcodes(void)
{
    const u32 patterns[]={0,1,2,3,0x40,0x41,0x80,0x7fffffff,0x80000000,0xffffffff,0x12345678,0xfedcba98};
    const unsigned ops[]={91,92,132};
    int (*handlers[])(int **)={Script_ClearBattleFlags,Script_SetBattleFlags,Script_SetScreenScrollPos};
    GeomScrollRecord saved=g_GeomScroll;FieldActor *saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    uint64_t hash=UINT64_C(14695981039346656037);FieldActor actor={0};g_CurrentEntity=&actor;
    for(unsigned op=0;op<3;++op)for(unsigned x=0;x<12;++x)for(unsigned y=0;y<12;++y){
        if(g_ScriptQueueOpcodeHandlers[ops[op]]!=handlers[op])return 0;
        g_GeomScroll.state.flags=(s32)patterns[x];
        g_GeomScroll.state.position.baseAngleDigital=0x1234;
        g_GeomScroll.state.position.baseAngleAnalog=(s16)0xfedc;
        int data[]={(s32)patterns[x],(s32)patterns[y]};int *args[]={data,data+1};
        u32 result=handlers[op](args);
        u32 words[]={result,(u32)g_GeomScroll.state.flags};
        for(unsigned i=0;i<2;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
        const u16 angles[]={(u16)g_GeomScroll.state.position.baseAngleDigital,(u16)g_GeomScroll.state.position.baseAngleAnalog};
        for(unsigned i=0;i<2;++i)for(unsigned b=0;b<2;++b)
            hash=(hash^((angles[i]>>(b*8))&255))*UINT64_C(1099511628211);
        u32 program[]={ops[op]|(2u<<13),0,patterns[x],patterns[y],32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || node.current.script!=program+6 || !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    g_GeomScroll=saved;g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0xc4b5a5fd1165f305)){fprintf(stderr,"Camera flags opcode hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original camera/scroll flag opcodes: 432 USA MIPS states and native dispatcher completion passed");
    return 1;
}

static int original_animation_frame_opcode(void)
{
    const u32 values[]={0,1,255,256,65535,65536,0x80000000,0xffffffff,0x12345678};
    FieldActor *saved_actor=g_CurrentEntity;ScriptNode *saved_node=g_ScriptNodePool;
    u32 *saved_cursor=g_ScriptCursor;uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[78]!=Script_SetEntityAnimSpeed)return 0;
    for(unsigned action=0;action<256;++action)for(unsigned v=0;v<9;++v){
        FieldActor actor={.action=action};actor.anim.fixed=0x12345678;g_CurrentEntity=&actor;
        int value=(s32)values[v];int *args[]={&value};u32 result=g_ScriptQueueOpcodeHandlers[78](args);
        u32 data[]={result,(u32)actor.anim.fixed};
        for(unsigned i=0;i<2;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((data[i]>>(b*8))&255))*UINT64_C(1099511628211);
        u32 program[]={78|(1u<<13),0,values[v],32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        actor.anim.fixed=0x12345678;g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || (u32)actor.anim.fixed!=data[1] || node.current.script!=program+5 ||
           !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0xfb78ff1167f295fc))return 0;
    puts("Original animation-frame opcode: 2304 USA MIPS states and native dispatcher completion passed");
    return 1;
}

static int original_script_command_binding(void)
{
    const int ops[]={0,2100,2101,2201,2300,2401,2600,2601,2700,2900,3000,3200};
    const u32 values[]={0,1,255,256,65535,0x80000000,0xffffffff,0x12345678};
    u32 saved_flags=g_GameState.flags;int saved_collision=g_ScriptCollisionFlag;
    GeomScrollRecord saved_scroll=g_GeomScroll;u8 saved_camera[3];memcpy(saved_camera,g_ScriptCameraBytes,3);
    FieldActor *saved_actor=g_CurrentEntity,*saved_head=g_FieldActorListHead;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[237]!=Script_DispatchScriptCmd)return 0;
    for(unsigned op=0;op<12;++op)for(unsigned v=0;v<8;++v){
        FieldActor actor={0};g_CurrentEntity=&actor;g_FieldActorListHead=NULL;
        int data[]={ops[op],(s32)values[v],(s32)(values[v]+1u),(s32)(values[v]-1u),0,0};
        int *args[]={data,data+1,data+2,data+3,data+4,data+5};
        g_GameState.flags=0x12345678;g_GeomScroll.state.position.originX=0x1234;
        g_GeomScroll.state.position.originY=(s16)0xfedc;
        memcpy(g_ScriptCameraBytes,(u8[]){1,2,3},3);
        actor.shadow_matrix_index=4;actor.shadow_intensity=5;
        g_RoomLightTint.r=7;g_RoomLightTint.g=8;g_RoomLightTint.b=9;
        g_ScriptCollisionFlag=0x76543210;
        u32 result=g_ScriptQueueOpcodeHandlers[237](args);
#define COMMAND_PUT(value,bytes) do {u32 put_=(u32)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)put_)*UINT64_C(1099511628211);put_>>=8;}}while(0)
        COMMAND_PUT(result,4);COMMAND_PUT(g_GameState.flags,4);
        COMMAND_PUT(g_GeomScroll.state.position.originX,2);COMMAND_PUT(g_GeomScroll.state.position.originY,2);
        for(unsigned i=0;i<3;++i)COMMAND_PUT(g_ScriptCameraBytes[i],1);
        COMMAND_PUT(actor.shadow_matrix_index,1);COMMAND_PUT(actor.shadow_intensity,1);
        COMMAND_PUT(g_RoomLightTint.r,1);COMMAND_PUT(g_RoomLightTint.g,1);COMMAND_PUT(g_RoomLightTint.b,1);
        COMMAND_PUT(g_ScriptCollisionFlag,4);
#undef COMMAND_PUT
        u32 program[]={237|(6u<<13),0,(u32)data[0],(u32)data[1],(u32)data[2],(u32)data[3],0,0,32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || node.current.script!=program+10 || !(node.flags&0x10) || g_ScriptNodePool)return 0;
    }
    g_GameState.flags=saved_flags;g_ScriptCollisionFlag=saved_collision;g_GeomScroll=saved_scroll;
    memcpy(g_ScriptCameraBytes,saved_camera,3);g_CurrentEntity=saved_actor;g_FieldActorListHead=saved_head;
    g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x0dd78fa3cadbad1b)){fprintf(stderr,"Original command dispatcher hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original script command dispatcher: 96 USA MIPS states and native opcode completion passed");
    return 1;
}

static int original_clamp_opcode_binding(void)
{
    const u32 high[]={0,0xfffffe00,0x80000000,0x7ffffe00};
    int saved=g_MenuItemContextFlag;FieldActor *saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    FieldActor actor={0};g_CurrentEntity=&actor;
    uint64_t hash=UINT64_C(14695981039346656037);
    if(g_ScriptQueueOpcodeHandlers[166]!=Script_ClampMenuRange)return 0;
    for(unsigned h=0;h<4;++h)for(unsigned low=0;low<512;++low){
        int value=(s32)(high[h]|low);int *args[]={&value};g_MenuItemContextFlag=0x1234;
        u32 result=g_ScriptQueueOpcodeHandlers[166](args);
        u32 values[]={result,(u32)g_MenuItemContextFlag};
        for(unsigned i=0;i<2;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((values[i]>>(b*8))&255))*UINT64_C(1099511628211);
        u32 program[]={166|(1u<<13),0,(u32)value,32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        g_MenuItemContextFlag=0x1234;g_ScriptNodePool=&node;Script_RunQueue();
        if(result!=1 || node.current.script!=program+5 || !(node.flags&0x10) ||
           g_ScriptNodePool || (u32)g_MenuItemContextFlag!=values[1])return 0;
    }
    g_MenuItemContextFlag=saved;g_CurrentEntity=saved_actor;
    g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(hash!=UINT64_C(0x9ba3598d49695b25))return 0;
    puts("Original menu range opcode: 2048 USA MIPS states and native dispatcher completion passed");
    return 1;
}

static int original_scene_transition_opcode(void)
{
    const u32 tokens[]={0,1,0xA80830C8,0xA80810C8,0xA9400048,0xffffffff,0x80000000,0x7fffffff};
    const u32 flags[]={0,1,0x2000,0x4000,0xffffffff,0x12345678};
    const u32 counters[]={0,2000,2001,0xffffffff};
    u32 saved_flags=g_GameStateFlags,saved_counter=g_PlayTimeFrameCounter;
    int saved_token=g_SceneDispatchToken;FieldActor *saved_actor=g_CurrentEntity;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    FieldActor actor={0};g_CurrentEntity=&actor;
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    if(g_ScriptQueueOpcodeHandlers[49]!=Script_EncodeBarcode)return 0;
    for(unsigned t=0;t<8;++t)for(unsigned f=0;f<6;++f)for(unsigned c=0;c<4;++c){
        if(tokens[t]==0xA9400048 && c!=1)continue;
        int value=(s32)tokens[t];int *args[]={&value};
        g_GameStateFlags=flags[f];g_SceneDispatchToken=0x1234;g_PlayTimeFrameCounter=counters[c];
        u32 result=g_ScriptQueueOpcodeHandlers[49](args);
        u32 values[]={result,g_GameStateFlags,(u32)g_SceneDispatchToken,g_PlayTimeFrameCounter};
        for(unsigned i=0;i<4;++i)for(unsigned b=0;b<4;++b)
            hash=(hash^((values[i]>>(b*8))&255))*UINT64_C(1099511628211);
        u32 program[]={49|(1u<<13),0,tokens[t],32,0};
        ScriptNode node={.current.script=program,.active=1,.flags=0x80};
        g_GameStateFlags=flags[f];g_SceneDispatchToken=0x1234;g_PlayTimeFrameCounter=counters[c];
        g_ScriptNodePool=&node;Script_RunQueue();
        if(result || node.current.script!=program+3 || node.flags!=0x80 || node.active!=0 ||
           g_ScriptNodePool || g_GameStateFlags!=values[1] || (u32)g_SceneDispatchToken!=values[2] ||
           g_PlayTimeFrameCounter!=values[3])return 0;
        ++cases;
    }
    g_GameStateFlags=saved_flags;g_PlayTimeFrameCounter=saved_counter;g_SceneDispatchToken=saved_token;
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(cases!=174 || hash!=UINT64_C(0x8ae1600ce89fd57c))return 0;
    puts("Original scene transition opcode: 174 USA MIPS states and native dispatcher yield/cursor passed");
    return 1;
}

static int original_native_voice_release(void)
{
    AkaoSequencerBank *saved_bank=g_AkaoCurTrack;uint64_t hash=UINT64_C(14695981039346656037);
    const u32 masks[]={0,0xffffff,0x555555,0xffffffff};u8 programs[24][8];
    for(unsigned function=0;function<2;++function)for(unsigned request=0;request<27;++request)
    for(unsigned layout=0;layout<6;++layout)for(unsigned m=0;m<4;++m){
        int requested=request<25?(int)request:request==25?-1:INT32_MAX;
        AkaoTrack tracks[24],before[24];memset(tracks,0xa5,sizeof(tracks));
        for(unsigned i=0;i<24;++i){
            int id=layout==0?(int)i:layout==1?7:layout==2?(i&1)?24:(int)i:
                layout==3?(i&1)?-1:0:layout==4?(int)(i%3):(int)((i*17+layout)%28)-2;
            tracks[i].assigned_voice_index=id;tracks[i].pc=programs[i];
            for(unsigned j=0;j<4;++j)tracks[i].call_stack[j]=programs[i]+j;
        }
        memcpy(before,tracks,sizeof(before));
        AkaoSequencerBank bank={.allocated_voice_mask=masks[m]},expected_bank=bank;g_AkaoCurTrack=&bank;
        if(function)Seq_ClearTrackVoiceId(tracks,(u32)requested);else Akao_RemoveVoice(tracks,requested);
        for(unsigned i=0;i<24;++i){
            u32 id=(u32)tracks[i].assigned_voice_index;
            for(unsigned b=0;b<4;++b)hash=(hash^((id>>(b*8))&255))*UINT64_C(1099511628211);
            before[i].assigned_voice_index=tracks[i].assigned_voice_index;
        }
        for(unsigned b=0;b<4;++b)hash=(hash^((bank.allocated_voice_mask>>(b*8))&255))*UINT64_C(1099511628211);
        expected_bank.allocated_voice_mask=bank.allocated_voice_mask;
        if(memcmp(tracks,before,sizeof(tracks)) || memcmp(&bank,&expected_bank,sizeof(bank)))return 0;
    }
    g_AkaoCurTrack=saved_bank;
    if(hash!=UINT64_C(0xe2e44b8a1b490a5a)){fprintf(stderr,"Native voice release hash %016llx\n",(unsigned long long)hash);return 0;}
    puts("Original voice release: 1296 USA MIPS cases, all 24 native records and full-width program/return pointers retained");
    return 1;
}

static int original_actor_audio_state(void)
{
    if(!original_battle_entry_opcode() || !original_conditional_branch_opcode() || !original_context_getter_opcodes() || !original_context_setter_opcodes() || !original_script_event_entries() || !original_animation_event_opcode() || !original_action_wait_opcode() || !original_typed_field_opcode() || !original_collision_gate_opcodes() || !original_scroll_opcodes() || !original_native_voice_release() || !original_polygon_opcode() || !original_motion_opcodes() || !original_dialogue_opcodes() || !original_turn_to_actor_opcode() || !original_screen_fade_wait_opcode() || !original_screen_fade_opcodes() || !original_geometry_control_opcodes() || !original_shadow_radius_opcode() || !original_camera_flags_opcodes() || !original_animation_frame_opcode() || !original_script_command_binding() || !original_clamp_opcode_binding() || !original_scene_transition_opcode() || !original_wait_opcode_bindings() || !original_field_sfx_opcode_binding() || !original_flag_opcode_bindings())return 0;
    Pe1GameState previous=g_GameState;
    FieldActor actor={.type_id=7,.sub_id=11};
    FieldActor *previous_actor=g_CurrentEntity;
    int op=0x15e,a=0x12345,b=0x187,c=-2,d=-129,result=123;
    AkaoCommandArgs args={&op,&a,&b,&c,&d,&result};
    g_CurrentEntity=&actor;
    if(Akao_ProcessCommand(args)!=1 || g_GameState.bank_value_f8!=(u16)a ||
       g_GameState.bank_value_f6!=(u8)b || g_GameState.bank_value_fa!=(u16)c ||
       g_GameState.bank_value_f7!=(u8)d || result!=123) return 0;
    op=0x195; g_GameState.bank_state_12=0;
    if(Akao_ProcessCommand(args)!=1 || g_GameState.bank_state_12!=1) return 0;
    g_GameState.bank_state_11=0;
    for(unsigned i=0;i<18;++i) {
        op=i&1?0x197:0x196; a=0x180+i;b=-129;c=0x12345;d=-2;
        if(Akao_ProcessCommand(args)!=1 || g_GameState.bank_state_11!=(i<16?i+1:16)) return 0;
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
        if(Akao_ProcessCommand(args)!=1 || result!=42 || g_AkaoMessageQueueCount!=2 ||
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
    if(Akao_ProcessCommand(args)!=1 || (u8)g_GameState.scene_audio.tracks.pending_key!=0x34 ||
       (u8)g_GameState.scene_audio.tracks.pending_bank!=0xff ||
       g_GameState.bank_asset_table!=(u8 *)sound_archive+96) return 0;
    op=0x199;a=0x198;
    if(Akao_ProcessCommand(args)!=1 || g_GameState.scene_audio.tracks.movie_volume!=0x98) return 0;
    if(g_ScriptQueueOpcodeHandlers[234]!=Akao_ProcessCommand)return 0;
    ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor,saved_variable=g_ScriptVariableBank[9];
    u32 program[]={234|(6u<<13),2,0x15e,0x23456,0x189,(u32)-3,(u32)-130,9,
                   234|(2u<<13),0,0x199,0x145,32,0};
    ScriptNode node={.current.script=program,.active=1,.flags=0x80};
    g_ScriptVariableBank[9]=0xa5a5a5a5;g_ScriptNodePool=&node;Script_RunQueue();
    if(!(node.flags&0x10) || node.current.script!=program+14 || g_ScriptNodePool ||
       g_GameState.bank_value_f8!=0x3456 || g_GameState.bank_value_f6!=0x89 ||
       g_GameState.bank_value_fa!=65533 || g_GameState.bank_value_f7!=126 ||
       g_GameState.scene_audio.tracks.movie_volume!=0x45 || g_ScriptVariableBank[9]!=0xa5a5a5a5)return 0;
    g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;g_ScriptVariableBank[9]=saved_variable;
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

static int process_static_task_programs(void)
{
    Pe1GameState saved=g_GameState;PmTables tables=g_PmTables;u32 flags=g_GameStateFlags;
    void *programs[104];memcpy(programs,g_FieldFxTaskPrograms,sizeof(programs));
    FieldFxTaskProgram *native[]={&g_FieldFxTaskProgramCmd108,&g_FieldFxTaskProgramCmd109,
        &g_FieldFxTaskProgramCmd110,&g_FieldFxTaskProgramCmd111,&g_FieldFxTaskProgramCmd112,
        &g_FieldFxTaskProgramCmd113,&g_FieldFxTaskProgramCmd114};
    PmCommand *handlers[86]={0};PmPrimarySlot bank[11]={0};
    g_PmTables=(PmTables){handlers,bank,NULL};g_GameStateFlags=0x80;
    g_GameState.flags&=~8u;g_GameState.tim_load_state=0;
    for(unsigned command=108;command<=114;++command){
        g_GameState.flags&=~0x10000u;glyph_reads=glyph_polls=0;
        glyph_read_retry=glyph_poll_retry=glyph_pending=1;
        g_MenuCardCrossFlashRaised=g_BattleCrossFlashStop=77;
        g_MemcardSineColor.r=77;g_MemcardCosineColor.g=88;g_MemcardGlintOrigin.x=99;
        /* No handler is installed: binding succeeds without any image I/O,
         * then the original PM dispatch reports the missing handler. */
        if(Scene_LoadRoomAssets(command,NULL)!=-8 || glyph_reads || glyph_polls || !(g_GameState.flags&0x10000))return 0;
        for(unsigned i=0;i<7;++i)if(g_FieldFxTaskPrograms[23+i]!=native[i])return 0;
        if(g_MenuCardCrossFlashRaised || g_BattleCrossFlashStop || g_MemcardSineColor.r ||
           g_MemcardCosineColor.g || g_MemcardGlintOrigin.x)return 0;
        FieldFxTaskOwner owner={0};if(FieldFx_BindTaskProgram(&owner,command-85) ||
            owner.tasks.table!=&native[command-108]->table || owner.tasks.pc.script!=native[command-108]->script)return 0;
    }
    g_MemcardSineColor.r=77;
    if(Scene_LoadRoomAssets(108,NULL)!=-8 || g_MemcardSineColor.r!=77)return 0;
    bank[0].header.command=114;bank[0].header.state=1;
    if(Pm_FreeSlot(0) || (g_GameState.flags&0x10000))return 0;
    for(unsigned i=23;i<30;++i)if(g_FieldFxTaskPrograms[i])return 0;
    if(Scene_LoadRoomAssets(108,NULL)!=-8 || g_MemcardSineColor.r)return 0;
    memcpy(g_FieldFxTaskPrograms,programs,sizeof(programs));g_GameState=saved;g_PmTables=tables;g_GameStateFlags=flags;
    glyph_reads=glyph_polls=0;glyph_read_retry=glyph_poll_retry=glyph_pending=0;
    puts("Original static task programs: seven real bindings, cold/reset versus cached state, 114 release/rebind and no executable-bank I/O passed");return 1;
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
static int original_slot_compatibility(void)
{
    MemCardPortState saved[2];memcpy(saved,g_MemCardPortStates,sizeof(saved));
    MenuWidgetNode widget={0},*saved_widget=g_MenuActiveWidget;
    int saved_mode=g_McDialogMode,saved_style=g_SaveTitleStyleFlag;
    const int modes[]={0,1,2,-1},styles[]={0,1,255},states[]={0,1,2,3,255};
    const int counts[]={0,1,15},indices[]={-1,0,1,14,15};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    g_MenuActiveWidget=&widget;
    for(int port=0;port<3;++port)for(int mode=0;mode<4;++mode)
    for(int style=0;style<3;++style)for(int state=0;state<5;++state)
    for(int ready=0;ready<2;++ready)for(int count=0;count<3;++count)
    for(int slot=0;slot<5;++slot) {
        memset(g_MemCardPortStates,0,sizeof(saved));
        widget.selected_base=port+0x25;g_McDialogMode=modes[mode];g_SaveTitleStyleFlag=styles[style];
        int index=indices[slot];
        if(port<2) {
            g_MemCardPortStates[port].fileCount=counts[count];
            if(index>=0 && index<15) {
                MemCardSaveSlot *entry=&g_MemCardPortStates[port].slots[index];
                entry->state=states[state];entry->metadataReady=ready;entry->titleStyleFlag=1;
            }
        }
        u32 result=Menu_IsMemCardSlotCompatible(index);
        for(int byte=0;byte<4;++byte)hash=(hash^((result>>(byte*8))&255))*UINT64_C(1099511628211);
        ++cases;
    }
    memcpy(g_MemCardPortStates,saved,sizeof(saved));g_MenuActiveWidget=saved_widget;
    g_McDialogMode=saved_mode;g_SaveTitleStyleFlag=saved_style;
    if(cases!=5400 || hash!=UINT64_C(0x6d2dc156bb7a40e5))return 0;
    puts("Original save-slot compatibility: 5400 USA CPU cases including pending metadata passed");
    return 1;
}

static int original_card_file_services(const u8 *image)
{
    if(!original_slot_compatibility())return 0;
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



static int original_escape_chance(PeDisc *disc,PeDiscFile *exe)
{
    u8 messages[62];
    if(!pe_disc_read(disc,exe,0x9159C-0x10000+0x800,messages,34) || memcmp(messages,g_EscapeFailedTexts,34) ||
       !pe_disc_read(disc,exe,0x915C0-0x10000+0x800,messages,28) || memcmp(messages,g_EscapeBlockedTexts,28))return 0;
    Combatant active={0};BattleEntity entities[4]={{0}};EnemyCombatant enemies[3]={{0}};
    Combatant *saved_actor=g_ActiveActor;FieldActor *saved_player=g_PlayerEntity,*saved_head=g_FieldActorListHead;
    u8 saved_overlay=g_BattleSaveOverlayActive,*saved_message=g_CurItemEffectData;int saved_window=g_SaveMetadataWindowIndex;
    u8 custom=255;u8 *texts[]={&custom,g_EscapeFailedTexts,g_EscapeFailedTexts+17,g_EscapeBlockedTexts,g_EscapeBlockedTexts+14};
    const u8 player_ranks[]={0,1,2,127,128,254,255},enemy_ranks[]={0,1,2,126,127,128,255};
    const u16 health[][2]={{0,100},{9,100},{10,100},{65535,65535},{32767,32768},{100,100}};
    const u32 seeds[]={0x12345678,0x87654321};
    g_ActiveActor=&active;g_PlayerEntity=g_FieldActorListHead=(FieldActor *)&entities[0];
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;int valid=1;
#define ESCAPE_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned rank=0;rank<7;++rank)for(unsigned enemy=0;enemy<7;++enemy)
    for(unsigned hp=0;hp<6;++hp)for(unsigned attempts=0;attempts<8;++attempts)
    for(unsigned scenario=0;scenario<6;++scenario)for(unsigned window=0;window<2;++window)for(unsigned seed=0;seed<2;++seed){
        memset(entities,0,sizeof(entities));memset(enemies,0,sizeof(enemies));
        entities[0].core=(EnemyCombatant *)&active;entities[0].next=scenario==0?NULL:&entities[1];
        entities[1].next=&entities[2];entities[2].next=&entities[3];
        for(unsigned i=0;i<3;++i){entities[i+1].core=&enemies[i];enemies[i].hpAlive=1;}
        enemies[0].field04.bytes.rank=enemy_ranks[enemy];enemies[1].field04.bytes.rank=127;enemies[2].field04.bytes.rank=128;
        if(scenario==1){enemies[1].hpAlive=0;enemies[1].statusFlags2=0x40000;entities[3].core=NULL;}
        if(scenario==3){enemies[0].hpAlive=0;enemies[0].statusFlags2=0x40000;enemies[1].field04.bytes.rank=2;entities[3].core=NULL;}
        if(scenario==4)enemies[0].statusFlags2=0x40000;
        if(scenario==5){entities[1].core=NULL;enemies[1].hpAlive=-1;enemies[1].statusFlags2=0x40000;
            enemies[2].field04.bytes.rank=enemy_ranks[enemy];enemies[2].statusFlags2=0x40000;}
        active.field04.bytes.rank=player_ranks[rank];active.curHP=health[hp][0];active.maxHP=health[hp][1];
        active.stateFlags=(s32)((0xA13579BDu&0xF1FFFFFFu)|(attempts<<25));
        g_SaveMetadataWindowIndex=window;g_BattleSaveOverlayActive=7;g_CurItemEffectData=&custom;
        srand(seeds[seed]);int result=Battle_RollEscapeChance();int next_random=rand();
        unsigned message=5;for(unsigned i=0;i<5;++i)if(g_CurItemEffectData==texts[i])message=i;
        if(message==5)valid=0;
        ESCAPE_PUT(result,4);ESCAPE_PUT(active.stateFlags,4);ESCAPE_PUT(g_BattleSaveOverlayActive,1);ESCAPE_PUT(message,1);ESCAPE_PUT(next_random,2);++cases;
    }
#undef ESCAPE_PUT
    g_ActiveActor=saved_actor;g_PlayerEntity=saved_player;g_FieldActorListHead=saved_head;
    g_BattleSaveOverlayActive=saved_overlay;g_CurItemEffectData=saved_message;g_SaveMetadataWindowIndex=saved_window;
    if(!valid || hash!=UINT64_C(0x02675eb74a42ca49)){
        fprintf(stderr,"Original escape chance: cases=%u hash=%016llx valid=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    puts("Original escape chance: 56448 USA cases, rank/HP boundaries, signed chance narrowing, attempts, blocked/dead/null cores, both text variants and RNG consumption match");return 1;
}

static int original_action_slot(void)
{
    Combatant active={0};BattleAction action={0};active.action=&action;
    BattleEntity entities[4]={{0}};EnemyCombatant state={0},peer_core={0},player_core={0};RenderObjectHeader header={0};
    Combatant *saved_active=g_ActiveActor;FieldActor *saved_player=g_PlayerEntity,*saved_head=g_FieldActorListHead;
    BattleEntity *saved_actors[7];memcpy(saved_actors,g_BattleActionActors,sizeof(saved_actors));
    BattleInitSlot saved_queue=g_BattleActionQueue[0];
    u8 saved_count=g_BattleActionActorCount,saved_fade=g_BattleFadeLevel,saved_index=g_BattleActionIndex;
    int saved_slot=g_ActiveDrawSlot;g_ActiveActor=&active;
    g_FieldActorListHead=g_PlayerEntity=(FieldActor *)&entities[3];
    const u32 phases[]={0,0x2000,0x4000,0x6000};const int kinds[]={0,1,2,4,-1},health[]={-1,0,1};
    const unsigned fades[]={0,1,7,8,127,128,129,135,136,255},commands[]={0,0x189,0x196};
    const unsigned arrays[][2]={{0,0},{0,3},{0,7},{1,1},{6,6},{6,7}};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;int valid=1;
#define SLOT_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned phase=0;phase<4;++phase)for(unsigned kind=0;kind<5;++kind)for(unsigned alive=0;alive<3;++alive)
    for(unsigned fade=0;fade<10;++fade)for(unsigned command=0;command<3;++command)for(unsigned a=0;a<2;++a)
    for(unsigned array=0;array<6;++array)for(unsigned peer=0;peer<3;++peer)for(unsigned slot=0;slot<2;++slot){
        memset(entities,0,sizeof(entities));memset(&state,0,sizeof(state));memset(&peer_core,0,sizeof(peer_core));memset(&player_core,0,sizeof(player_core));
        entities[3].next=&entities[0];entities[0].next=&entities[1];entities[1].next=&entities[2];
        entities[0].core=&state;entities[0].parent=&entities[1];entities[2].core=peer?&peer_core:NULL;
        peer_core.field04.bytes.kind=peer==1?4:2;entities[3].core=&player_core;player_core.field04.bytes.kind=4;
        state.coreFlags=0xA5000003|phases[phase];state.hpAlive=health[alive];state.field04.bytes.kind=kinds[kind];
        for(unsigned i=0;i<4;++i){RenderObjectEntity *o=&entities[i].renderObject;
            o->header=&header;o->draw_count=1;o->flags_9C=0x8002;o->primitive_red=17;o->primitive_green=34;o->primitive_blue=51;}
        for(unsigned i=0;i<7;++i)g_BattleActionActors[i]=i<arrays[array][1]?&entities[i&1?1:2]:NULL;
        g_BattleActionActorCount=arrays[array][0];g_BattleFadeLevel=fades[fade];g_BattleActionIndex=0;
        g_BattleActionQueue[0].actor=&entities[1];g_BattleActionQueue[0].command=commands[command];g_ActiveDrawSlot=slot;
        action.actionCode.actionId=a?6:5;int result=Battle_ProcessActionSlot(&entities[0]);
        SLOT_PUT(result,4);SLOT_PUT(state.coreFlags,4);SLOT_PUT(g_BattleActionActorCount,1);SLOT_PUT(g_BattleFadeLevel,1);
        for(unsigned i=0;i<7;++i){unsigned id=0;for(unsigned j=0;j<4;++j)if(g_BattleActionActors[i]==&entities[j])id=j+1;
            if(g_BattleActionActors[i] && !id)valid=0;SLOT_PUT(id,1);}
        for(unsigned i=0;i<4;++i){RenderObjectEntity *o=&entities[i].renderObject;SLOT_PUT(o->flags_9C,2);
            SLOT_PUT(o->primitive_red,1);SLOT_PUT(o->primitive_green,1);SLOT_PUT(o->primitive_blue,1);}
        ++cases;
    }
    state.coreFlags=0x2000;g_BattleActionActorCount=7;g_BattleActionQueue[0].command=0;action.actionCode.actionId=5;
    for(unsigned i=0;i<7;++i)g_BattleActionActors[i]=&entities[1];
    Battle_ProcessActionSlot(&entities[0]);
    if(g_BattleActionActorCount!=7)valid=0;
    for(unsigned i=0;i<7;++i)if(g_BattleActionActors[i]!=&entities[1])valid=0;
#undef SLOT_PUT
    g_ActiveActor=saved_active;g_PlayerEntity=saved_player;g_FieldActorListHead=saved_head;
    memcpy(g_BattleActionActors,saved_actors,sizeof(saved_actors));g_BattleActionQueue[0]=saved_queue;
    g_BattleActionActorCount=saved_count;g_BattleFadeLevel=saved_fade;g_BattleActionIndex=saved_index;g_ActiveDrawSlot=saved_slot;
    if(!valid || hash!=UINT64_C(0x049bef90c93d53e1)){
        fprintf(stderr,"Original action slot: cases=%u hash=%016llx valid=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    puts("Original action slot: 129600 USA cases, seven-actor list clear/append, commands, all phases, kinds/HP/fades, parent/peer/player exclusion and both buffers match");return 1;
}

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

static unsigned fx_node_id(FxCommonNode *node)
{
    if(!node)return 0;if(node==&g_FxCommonDefaultNode)return 1;
    for(unsigned i=0;i<3;++i){if(node==&g_FxCommonListHeadNodes[i])return 202+i;
        if(node==&g_FxCommonListTailNodes[i])return 205+i;}
    for(unsigned i=0;i<200;++i)if(node==&g_FxCommonRecords[i])return i+2;
    return 0xffff;
}

static int original_gte_register_transfers(void)
{
    const u32 values[]={0,1,0x7fff,0x8000,0xffff,0x10000,0x7fffffff,0x80000000,0xffffffff,0x12345678};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    for(unsigned f=0;f<15;++f)for(unsigned v=0;v<10;++v)for(unsigned seed=0;seed<3;++seed){
        u32 value=values[v],buffer[12];
        for(unsigned j=0;j<32;++j){Psyz_GteDataWrite(j,value^(j*0x01010101u)^(seed*0x03456789u));
            Psyz_GteCtrlWrite(j,value+j*0x10203u+seed*0x07654321u);}
        for(unsigned j=0;j<12;++j)buffer[j]=value^(j*0x01020304u)^(seed*0x00112233u);
        s32 a=(s32)value,b=(s32)(value^0x80000000u),c=(s32)~value,d=(s32)(seed*0xfedcba98u);
        switch(f){
        case 0:SetVertex0(buffer);break;case 1:SetVertex1(buffer);break;case 2:SetVertex2(buffer);break;
        case 3:SetVertexTri(buffer,buffer+4,buffer+8);break;case 4:SetRGBfifo(buffer,buffer+4,buffer+8);break;
        case 5:SetIR123(a,b,c);break;case 6:SetIR0(a);break;case 7:SetSZfifo3(a,b,c);break;
        case 8:SetSZfifo4(a,b,c,d);break;case 9:SetSXSYfifo(a,b,c);break;case 10:SetRii(a,b,c);break;
        case 11:SetMAC123(a,b,c);break;case 12:SetData32(a);break;case 13:SetDQA(a);break;case 14:SetDQB(a);break;
        }
        for(unsigned bank=0;bank<2;++bank)for(unsigned j=0;j<32;++j){
            u32 word=bank?Psyz_GteCtrlRead(j):Psyz_GteDataRead(j);
            for(unsigned byte=0;byte<4;++byte)hash=(hash^(u8)(word>>(byte*8)))*UINT64_C(1099511628211);
        }
        ++cases;
    }
    /* Leave the GTE in a clean state before checking display initialization. */
    for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
    if(cases!=450 || hash!=UINT64_C(0x3bcbb387746aa626)){
        fprintf(stderr,"Original GTE REG03: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    puts("Original GTE REG03: all 15 original entry points match USA CPU/COP2 transfers across 450 scenarios and all 64 readable data/control registers");return 1;
}

static int original_fx_fog(void)
{
    const s32 near_values[]={-32768,-1000,-1,0,1,100,1000,6500,32767,65535};
    const s32 far_values[]={-1000,-1,0,100,199,200,1000,6500,11500,32767,65535};
    const s32 distances[]={-32768,-1,1,160,320,768,32767};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    for(unsigned a=0;a<10;++a)for(unsigned b=0;b<11;++b)for(unsigned c=0;c<7;++c){
        SetDQA(0x1234);SetDQB(0x12345678);
        SetFogNearFar(near_values[a],far_values[b],distances[c]);
        const u32 values[]={Psyz_GteCtrlRead(27),Psyz_GteCtrlRead(28)};
        for(unsigned i=0;i<2;++i)for(unsigned byte=0;byte<4;++byte)
            hash=(hash^(u8)(values[i]>>(byte*8)))*UINT64_C(1099511628211);
        ++cases;
    }
    SetDQA(0x1234);SetDQB(0x12345678);SetFogNearFar(6500,11500,0);
    if(cases!=770 || hash!=UINT64_C(0x2b60b43c2716c8c8) ||
       Psyz_GteCtrlRead(27)!=0x1234 || Psyz_GteCtrlRead(28)!=0x12345678){
        fprintf(stderr,"Original FX fog: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    puts("Original FX fog: 770 USA CPU/GTE cases match DQA/DQB, range rejection, signed scales and clamps; zero projection guard preserves registers");return 1;
}

DISPENV *GetDispEnv(DISPENV *env);
static int original_fx_display(void)
{
    Pe1GameState saved=g_GameState;FxCommonFrame frames[2];
    FxCommonBuffer *active=g_FxCommonActiveBuffer;
    memcpy(frames,g_FxCommonFrames,sizeof(frames));
    Boot_InitMemoryLayout();
    int arena_fits=g_RenderScratchBufferBase[0]+2*sizeof(FxCommonBufferRegion)<=g_RenderFrontBufferBase[0];
    static struct {u64 before;RenderNativeTag entries[4096];u64 after;} ordering[2];
    static struct {u64 before;FxCommonBufferRegion regions[2];u64 after;} packets;
    static u16 old_pixels[1024*512],pixels[1024*512];
    DRAWENV old_draw;DISPENV old_display;RECT area={0,0,1024,512};
    GetDrawEnv(&old_draw);GetDispEnv(&old_display);StoreImage(&area,old_pixels);DrawSync(0);
    for(unsigned i=0;i<1024*512;++i)pixels[i]=0x1357;
    LoadImage(&area,pixels);DrawSync(0);
    memset(ordering,0xa5,sizeof(ordering));memset(&packets,0xa5,sizeof(packets));
    memset(g_FxCommonFrames,0xa5,sizeof(g_FxCommonFrames));
    g_OtBufferTable[0]=(u8 *)ordering[0].entries;g_RenderOtBufferBaseAlt[0]=(u8 *)ordering[1].entries;
    g_RenderScratchBufferBase[0]=(u8 *)packets.regions;
    RenderNativeTag terminal;ClearOTagR((u_int *)&terminal,1);
    FxCommon_InitDisplay();int valid=arena_fits && g_FxCommonActiveBuffer==&g_FxCommonFrames[0].buffer;
    for(unsigned i=0;i<2;++i){
        FxCommonFrame *frame=&g_FxCommonFrames[i];
        if(frame->buffer.data!=(u8 *)&packets.regions[i] ||
           frame->buffer.allocation!=(void *)ordering[i].entries ||
           frame->drawEnv.clip.x || frame->drawEnv.clip.y!=i*240 ||
           frame->drawEnv.clip.w!=320 || frame->drawEnv.clip.h!=240 ||
           frame->drawEnv.ofs[0] || frame->drawEnv.ofs[1]!=i*240 ||
           frame->drawEnv.isbg || frame->drawEnv.r0 || frame->drawEnv.g0 || frame->drawEnv.b0 ||
           frame->dispEnv.disp.x || frame->dispEnv.disp.y!=(1-i)*240 ||
           frame->dispEnv.disp.w!=320 || frame->dispEnv.disp.h!=240 || frame->dispEnv.isrgb24)valid=0;
        for(unsigned j=0;j<4096;++j)if(ordering[i].entries[j].length ||
            ordering[i].entries[j].address!=(j?(uintptr_t)&ordering[i].entries[j-1]:terminal.address))valid=0;
        if(ordering[i].before!=UINT64_C(0xa5a5a5a5a5a5a5a5) || ordering[i].after!=UINT64_C(0xa5a5a5a5a5a5a5a5))valid=0;
    }
    if(Psyz_GteCtrlRead(24)!=(160u<<16) || Psyz_GteCtrlRead(25)!=(120u<<16) || Psyz_GteCtrlRead(26)!=768)valid=0;
    StoreImage(&area,pixels);DrawSync(0);
    for(unsigned y=0;y<512;++y)for(unsigned x=0;x<1024;++x)
        if(pixels[y*1024+x]!=(x<320 && y<480?0:0x1357))valid=0;
    for(unsigned i=0;i<2;++i){
        memset(ordering[i].entries,0xa5,sizeof(ordering[i].entries));
        g_FxCommonActiveBuffer=&g_FxCommonFrames[i].buffer;
        g_FxCommonActiveBuffer->data+=sizeof(FxCommonBufferRegion)-16;
        FxCommon_ClearActiveOrdering();
        if(g_FxCommonActiveBuffer->data!=(u8 *)&packets.regions[i])valid=0;
        for(unsigned j=0;j<4096;++j)if(ordering[i].entries[j].length ||
            ordering[i].entries[j].address!=(j?(uintptr_t)&ordering[i].entries[j-1]:terminal.address))valid=0;
    }
    for(unsigned i=0;i<sizeof(packets);++i)if(((u8 *)&packets)[i]!=0xa5)valid=0;
    LoadImage(&area,old_pixels);PutDrawEnv(&old_draw);PutDispEnv(&old_display);DrawSync(0);
    g_GameState=saved;memcpy(g_FxCommonFrames,frames,sizeof(frames));g_FxCommonActiveBuffer=active;
    if(!valid){fprintf(stderr,"Original FX display: frame/env/OT/VRAM regression\n");return 0;}
    puts("Original FX display: typed double buffers, native 4096-entry OTs, complete VRAM clear footprint, GTE setup, both arena resets and guards pass headlessly");return 1;
}

static int original_fx_node_pool(void)
{
    const unsigned counts[]={0,1,2,7,63,199,200};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;int valid=1;
    _Alignas(16) u8 table[1024]={0};
    for(unsigned i=0;i<8;++i){u32 offset=32+i*64,value=100+i*7;memcpy(table+i*4,&offset,4);memcpy(table+offset+48,&value,4);}
#define FX_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned count=0;count<7;++count)for(unsigned pattern=0;pattern<5;++pattern)for(unsigned offset=0;offset<3;++offset){
        memset(&g_FxCommonRuntime,0,sizeof(g_FxCommonRuntime));memset(&g_FxCommonDefaultNode,0,sizeof(g_FxCommonDefaultNode));
        memset(g_FxCommonListHeadNodes,0,3*sizeof(FxCommonNode));memset(g_FxCommonListTailNodes,0,3*sizeof(FxCommonNode));
        memset(g_FxCommonIndexFreeNext,0,200*sizeof(s16));g_FxCommonIndexFreeHead=g_FxCommonIndexLast=g_FxCommonRecordCursor=0;
        FxCommon_InitNodePools();FxCommonNode *live[200];unsigned kinds[200]={0},n=0,removed=0;
        for(unsigned stage=0;stage<2;++stage){
            unsigned number=stage?removed/2:counts[count];
            for(unsigned i=0;i<number;++i){unsigned serial=stage?counts[count]+i:i,kind=(serial+offset)%3;
                FxCommonNode *parent=&g_FxCommonDefaultNode;if(serial%4==3 && n && live[0]->bucket==1)parent=live[0];
                FxCommonNode *node=kind==0?FxCommon_CreateBareNode(parent,table):
                    kind==1?FxCommon_CreateTypedNode(parent,serial%8,table):
                    FxCommon_CreateConfiguredNode(parent,serial%8,(serial+1)%8,(serial+2)%8,1000+serial,500+serial,table);
                if(!node || n>=200 || (unsigned)node->id>=200){valid=0;break;}
                kinds[node->id]=kind;live[n++]=node;
            }
            if(!stage){unsigned initial=n;FxCommonNode *original[200];memcpy(original,live,n*sizeof(*live));
                for(unsigned i=0;i<initial;++i){FxCommonNode *node=original[i];unsigned id=node->id;
                    int release=pattern==4 || (pattern==1 && !(id%2)) || (pattern==2 && id%2) || (pattern==3 && !(id%3));
                    for(unsigned j=0;j<n;++j)if(live[j]!=node && live[j]->parent==node)release=0;
                    if(release){FxCommon_ReleaseNode(node);unsigned j=0;while(j<n && live[j]!=node)++j;
                        memmove(live+j,live+j+1,(n-j-1)*sizeof(*live));--n;++removed;}
                }
            }
        }
        FX_PUT(g_FxCommonIndexFreeHead,2);FX_PUT(g_FxCommonIndexLast,2);
        for(unsigned i=0;i<200;++i)FX_PUT(g_FxCommonIndexFreeNext[i],2);
        for(unsigned i=0;i<202;++i){FX_PUT(g_FxCommonIndexLinks[i].previous,2);FX_PUT(g_FxCommonIndexLinks[i].next,2);}
        FxCommonNode *ordered[200],*node;unsigned length=0;FxCommon_BeginRecordIteration();
        while((node=FxCommon_NextRecord())){if(length>=200){valid=0;break;}ordered[length++]=node;}
        FX_PUT(length,2);if(length!=n)valid=0;
        for(unsigned i=0;i<length;++i){node=ordered[i];unsigned kind=kinds[node->id];
            FX_PUT(node->id,2);FX_PUT(kind,1);FX_PUT(node->type,2);FX_PUT(node->resource==table,1);
            if(kind==2){FX_PUT(node->middleKind,2);FX_PUT(node->nearKind,2);FX_PUT(node->lod.margin,2);FX_PUT(node->lod.farDepth,4);FX_PUT(node->lod.nearDepth,4);}
            else if(kind==1)FX_PUT(node->margin,2);
            for(unsigned j=0;j<3;++j)FX_PUT(kind==2?node->lod.matrix.t[j]:node->matrix.t[j],4);
            FX_PUT(node->bucket,2);FX_PUT(fx_node_id(node->parent),2);FX_PUT(fx_node_id(node->previous),2);FX_PUT(fx_node_id(node->next),2);
        }
        for(unsigned i=0;i<3;++i){FX_PUT(fx_node_id(g_FxCommonListHeadNodes[i].next),2);FX_PUT(fx_node_id(g_FxCommonListTailNodes[i].previous),2);}
        ++cases;
    }
#undef FX_PUT
    if(cases!=105 || hash!=UINT64_C(0x405afb1f82d27f95))valid=0;
    /* Exhaustion and malformed ownership cannot consume a slot or a sentinel. */
    FxCommon_InitNodePools();for(unsigned i=0;i<200;++i)if(!FxCommon_CreateBareNode(&g_FxCommonDefaultNode,NULL))valid=0;
    FxCommonRuntime full=g_FxCommonRuntime;
    if(FxCommon_AllocIndexLink()!=-1 || FxCommon_CreateBareNode(&g_FxCommonDefaultNode,NULL) || memcmp(&full,&g_FxCommonRuntime,sizeof(full)))valid=0;
    FxCommon_ReleaseNode(&g_FxCommonRecords[199]);FxCommonRuntime freed=g_FxCommonRuntime;
    FxCommon_ReleaseNode(&g_FxCommonRecords[199]);FxCommon_FreeIndexLink(-1);FxCommon_FreeIndexLink(200);
    if(memcmp(&freed,&g_FxCommonRuntime,sizeof(freed)))valid=0;
    FxCommonNode bad={.bucket=2};u16 head=g_FxCommonIndexFreeHead;
    if(FxCommon_CreateBareNode(NULL,NULL) || FxCommon_CreateBareNode(&bad,NULL) ||
       FxCommon_CreateTypedNode(&g_FxCommonDefaultNode,0,NULL) ||
       FxCommon_CreateConfiguredNode(&g_FxCommonDefaultNode,0,0,0,1,2,NULL) || g_FxCommonIndexFreeHead!=head)valid=0;
    FxCommon_InitNodePools();FxCommon_BeginRecordIteration();for(unsigned i=0;i<4;++i)if(FxCommon_NextRecord())valid=0;
    if(!valid){fprintf(stderr,"Original FX node pool: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    puts("Original FX node pool: 105 unhooked USA scenarios, all 200 slots, three constructors, parent/bucket links, frees/reuse and record iteration match; native exhaustion/double-free guards pass");return 1;
}

static int original_fx_culling(void)
{
    const s32 scales[]={1,8,4096,32767},depths[]={0,1,-1,INT32_MAX,INT32_MIN};
    const s32 points[][3]={{0,0,0},{1,-1,1},{-1,1,-1},{512,-512,500},
        {32767,-32768,32760},{-32768,32767,-32760},{INT32_MAX,INT32_MIN,1},
        {INT32_MIN,INT32_MAX,-1},{1234,-5678,9012}};
    const s32 sides[][4]={{1,1,1,1},{-1,-1,-1,-1},{1,-1,1,-1},{0,0,0,0},{0,1,-1,0}};
    const s32 margins[]={-32768,-1,0,1,12,4095,0x10000000,INT32_MAX},lengths[]={1,64,65535};
    GteVector *normals[]={&g_FxCommonFrustumNormal0,&g_FxCommonFrustumNormal1,&g_FxCommonFrustumNormal2,&g_FxCommonFrustumNormal3};
    s32 *distance[]={&g_FxCommonFrustumDistance0,&g_FxCommonFrustumDistance1,&g_FxCommonFrustumDistance2,&g_FxCommonFrustumDistance3};
    s32 *side[]={&g_FxCommonFrustumSide0,&g_FxCommonFrustumSide1,&g_FxCommonFrustumSide2,&g_FxCommonFrustumSide3};
    s32 *length[]={&g_FxCommonFrustumLength0,&g_FxCommonFrustumLength1,&g_FxCommonFrustumLength2,&g_FxCommonFrustumLength3};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    for(unsigned a=0;a<4;++a)for(unsigned b=0;b<9;++b)for(unsigned c=0;c<5;++c)
    for(unsigned d=0;d<5;++d)for(unsigned e=0;e<8;++e)for(unsigned f=0;f<3;++f){
        s32 scale=scales[a],n[][3]={{scale,1,-scale},{-scale,scale,1},{1,-scale,scale},{scale,-scale,scale}};
        s32 point[3];s16 shortpoint[3];
        for(unsigned i=0;i<4;++i){normals[i]->x=n[i][0];normals[i]->y=n[i][1];normals[i]->z=n[i][2];
            *distance[i]=(s32)((u32)depths[c]+i*17u);*side[i]=sides[d][i];*length[i]=lengths[f]+i;}
        for(unsigned i=0;i<3;++i){point[i]=points[b][i];shortpoint[i]=(s16)point[i];}
        s32 result[]={FxCommon_IsPointInView(point),FxCommon_IsShortPointInView(shortpoint),
            FxCommon_CheckBoundsWithMargin(point,margins[e]),FxCommon_CheckFourBoundsWithMargin(point,margins[e])};
        for(unsigned i=0;i<4;++i)for(unsigned byte=0;byte<4;++byte)
            hash=(hash^(u8)((u32)result[i]>>(byte*8)))*UINT64_C(1099511628211);
        ++cases;
    }
    int valid=cases==21600 && hash==UINT64_C(0xa642cfccbdbba705);
    s32 point[3]={0};
    for(unsigned i=0;i<4;++i){*length[i]=0;
        if(FxCommon_CheckFourBoundsWithMargin(point,1))valid=0;*length[i]=1;}
    g_FxCommonFrustumLength0=0;
    if(FxCommon_CheckBoundsWithMargin(point,1) || FxCommon_IsPointInView(NULL) ||
       FxCommon_IsShortPointInView(NULL) || FxCommon_CheckBoundsWithMargin(NULL,1) ||
       FxCommon_CheckFourBoundsWithMargin(NULL,1))valid=0;
    if(!valid){fprintf(stderr,"Original FX culling: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    puts("Original FX culling: 21600 unhooked USA scenarios match all four checks, including signed boundaries and 32-bit overflow; null/zero-length guards pass");return 1;
}

static int original_fx_camera_matrices(void)
{
    const s32 poses[][2][3]={{{0,0,0},{0,0,1000}},{{100,-200,300},{-300,500,2000}},
        {{-1000,2000,-3000},{4000,-5000,6000}},{{0,0,0},{0,1000,0}},
        {{0,0,0},{0,-1000,0}},{{1,1,1},{1,1,1}}};
    GteVector *normals[]={&g_FxCommonFrustumNormal0,&g_FxCommonFrustumNormal1,&g_FxCommonFrustumNormal2,&g_FxCommonFrustumNormal3};
    s32 *distance[]={&g_FxCommonFrustumDistance0,&g_FxCommonFrustumDistance1,&g_FxCommonFrustumDistance2,&g_FxCommonFrustumDistance3};
    s32 *side[]={&g_FxCommonFrustumSide0,&g_FxCommonFrustumSide1,&g_FxCommonFrustumSide2,&g_FxCommonFrustumSide3};
    s32 *length[]={&g_FxCommonFrustumLength0,&g_FxCommonFrustumLength1,&g_FxCommonFrustumLength2,&g_FxCommonFrustumLength3};
    uint64_t hash=UINT64_C(14695981039346656037);
#define FX_HASH_BYTES(ptr,count) do {const u8 *bytes_=(const u8 *)(ptr); \
    for(unsigned byte_=0;byte_<(count);++byte_)hash=(hash^bytes_[byte_])*UINT64_C(1099511628211);}while(0)
    for(unsigned i=0;i<6;++i){
        for(unsigned j=0;j<32;++j){Psyz_GteDataWrite(j,0);Psyz_GteCtrlWrite(j,0);}
        memcpy(&g_FxCommonCameraPosition,poses[i][0],12);memcpy(&g_FxCommonCameraTarget,poses[i][1],12);
        memset(g_FxCommonCameraAngles,0,sizeof(GteShortVector));
        FxCommon_UpdateCameraMatrices();FxCommon_UpdateFrustumPlanes(&g_FxCommonCameraPosition);
        /* MATRIX has two implicit alignment bytes, which carry no game state. */
        FX_HASH_BYTES(g_FxCommonViewMatrix.m,18);FX_HASH_BYTES(g_FxCommonViewMatrix.t,12);
        FX_HASH_BYTES(g_FxCommonCameraMatrix.m,18);FX_HASH_BYTES(g_FxCommonCameraMatrix.t,12);
        FX_HASH_BYTES(g_FxCommonCameraAngles,8);
        for(unsigned j=0;j<4;++j){FX_HASH_BYTES(normals[j],12);FX_HASH_BYTES(distance[j],4);
            FX_HASH_BYTES(side[j],4);FX_HASH_BYTES(length[j],4);}
    }
#undef FX_HASH_BYTES
    if(hash!=UINT64_C(0xab55eca183324865)){
        fprintf(stderr,"Original FX camera matrices: hash=%016llx\n",(unsigned long long)hash);return 0;}
    puts("Original FX camera: six USA CPU/GTE scenarios match both matrices, angles and all four frustum planes, including vertical and coincident positions");return 1;
}

static int original_fx_poly_render_assets(PeDisc *disc)
{
    PeDiscFile archive;if(!pe_disc_find(disc,"PE.IMG",&archive))return 0;
    static _Alignas(16) u8 bank[100*2048],synthetic[2048];
    static struct {u64 before;_Alignas(16) u8 bytes[0x40000];u64 after;} arena;
    static struct {u64 before;RenderNativeTag entries[4096];u64 after;} ot;
    const s32 z_values[]={-1000,1,768,7980,7990,8000,8016,8144,8160,8176,11200,11216,11232,20000},biases[]={0,500,1000};
    const u8 codes[]={0x20,0x28,0x30,0x38,0x24,0x2c,0x34,0x3c};
    const unsigned strides[]={sizeof(FxCommonFlatTriangle),sizeof(FxCommonFlatQuad),sizeof(FxCommonColoredTriangle),sizeof(FxCommonColoredQuad),
        sizeof(FxCommonTexturedTriangle),sizeof(FxCommonTexturedQuad),sizeof(FxCommonColoredTexturedTriangle),sizeof(FxCommonColoredTexturedQuad)};
    FxCommonPolyResource *header=(FxCommonPolyResource *)synthetic;memset(synthetic,0,sizeof(synthetic));unsigned cursor=48;
    for(unsigned kind=0;kind<8;++kind){header->counts[kind]=2;header->offsets[kind]=cursor;
        for(unsigned reverse=0;reverse<2;++reverse){u8 *primitive=synthetic+cursor;unsigned quad=kind%2,colors=kind<2 || (kind>=4 && kind<6)?1:quad?4:3,at=colors*4;
            for(unsigned i=0;i<colors;++i){primitive[i*4]=31+i*47;primitive[i*4+1]=67+i*23;primitive[i*4+2]=101+i*31;primitive[i*4+3]=i?0xcd:codes[kind];}
            if(kind>=4){for(unsigned texture=0;texture<(kind<6?3u:1u);++texture){
                    const u16 uv[]={(11+texture*3)|((23+texture*5)<<8),0x7100+texture,(101+texture*2)|((203+texture*7)<<8),0x101+texture,(77+texture)|((99+texture*9)<<8),231|((17+texture*11)<<8)};
                    memcpy(primitive+at,uv,sizeof(uv));at+=sizeof(uv);}
                if(kind>=6){unsigned unknown=quad?16:12;memset(primitive+at,0x5a,unknown);at+=unknown;}}
            GteShortVector vertices[]={{-100,-80,0,0},{100,-80,0,0},{-100,80,0,0},{100,80,0,0}};
            if(reverse){GteShortVector swap=vertices[1];vertices[1]=vertices[2];vertices[2]=swap;}
            memcpy(primitive+at,vertices,(quad?4:3)*sizeof(*vertices));cursor+=strides[kind];
        }
    }
    FxCommonBuffer *old_buffer=g_FxCommonActiveBuffer; s32 old_base=g_FxCommonDepthBase;
    FxCommonRenderScratchpad old_work=g_FxCommonRenderWorkspace;
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0,packets=0;int valid=1;
#define POLY_PUT(value,count) do {uint64_t word_=(uint64_t)(value); \
    for(unsigned byte_=0;byte_<(count);++byte_){hash=(hash^(u8)word_)*UINT64_C(1099511628211);word_>>=8;}}while(0)
    for(unsigned variant=0;variant<3 && valid;++variant){
        FxCommonPolyResource *resource=(FxCommonPolyResource *)synthetic;
        if(variant<2){unsigned sector=variant?2105:2016,size=variant?100:89;u32 offset;
            if(!pe_disc_read(disc,&archive,(uint64_t)sector*2048,bank,size*2048)){valid=0;break;}
            memcpy(&offset,bank+4,4);if(offset>size*2048-48){valid=0;break;}resource=(FxCommonPolyResource *)(bank+offset);}
        for(unsigned z=0;z<14 && valid;++z)for(unsigned bias=0;bias<3 && valid;++bias)for(unsigned mirror=0;mirror<2 && valid;++mirror){
            for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
            const u32 controls[]={4096,0,mirror?0xf000:4096,0,4096,0,0,(u32)z_values[z]};
            for(unsigned i=0;i<8;++i)Psyz_GteCtrlWrite(i,controls[i]);
            Psyz_GteCtrlWrite(24,160u<<16);Psyz_GteCtrlWrite(25,120u<<16);Psyz_GteCtrlWrite(26,768);Psyz_GteCtrlWrite(29,341);Psyz_GteCtrlWrite(30,256);
            memset(&g_FxCommonRenderWorkspace,0xa5,sizeof(g_FxCommonRenderWorkspace));memset(&arena,0xa5,sizeof(arena));memset(&ot,0xa5,sizeof(ot));
            for(unsigned i=0;i<4096;++i)ot.entries[i]=(RenderNativeTag){0xffffff,0xab};
            FxCommonBuffer buffer={.data=arena.bytes,.allocation=(RenderNativeTag *)ot.entries,.end=arena.bytes+sizeof(arena.bytes)};
            g_FxCommonActiveBuffer=&buffer;g_FxCommonDepthBase=biases[bias];FxCommon_DrawPolyResource(NULL,resource);
            if(buffer.error || buffer.data<arena.bytes || buffer.data>arena.bytes+sizeof(FxCommonBufferRegion)){fprintf(stderr,"FX poly resource=%u z=%d bias=%d mirror=%u error=%d used=%td\n",variant,z_values[z],biases[bias],mirror,buffer.error,buffer.data-arena.bytes);valid=0;break;}
            u8 *at=arena.bytes,*addresses[8192];unsigned count=0;
            while(at<buffer.data){RenderNativeTag *tag=(RenderNativeTag *)at;
                if(count>=8192 || (tag->length!=4 && tag->length!=5 && tag->length!=6 && tag->length!=7 && tag->length!=8 && tag->length!=9 && tag->length!=12)){valid=0;break;}
                addresses[count++]=at;size_t size=(sizeof(*tag)+tag->length*4+_Alignof(RenderNativeTag)-1)&~(size_t)(_Alignof(RenderNativeTag)-1);
                if(size>(size_t)(buffer.data-at)){valid=0;break;}at+=size;
            }
            if(!valid || at!=buffer.data){valid=0;break;}POLY_PUT(count,4);
            for(unsigned i=0;i<count;++i){RenderNativeTag *tag=(RenderNativeTag *)addresses[i];POLY_PUT(tag->length,1);
                for(unsigned byte=0;byte<tag->length*4;++byte)POLY_PUT(addresses[i][sizeof(*tag)+byte],1);}
            for(unsigned depth=0;depth<4096 && valid;++depth){if(ot.entries[depth].length!=0xab){valid=0;break;}
                uintptr_t link=ot.entries[depth].address;if(link==0xffffff)continue;POLY_PUT(depth,2);unsigned steps=0;
                while(link!=0xffffff){unsigned id=0;while(id<count && (uintptr_t)addresses[id]!=link)++id;
                    if(id==count || ++steps>count){valid=0;break;}POLY_PUT(id,2);link=((RenderNativeTag *)link)->address;}
                POLY_PUT(65535,2);
            }
            POLY_PUT(65535,2);packets+=count;++cases;
            if(arena.before!=UINT64_C(0xa5a5a5a5a5a5a5a5) || arena.after!=UINT64_C(0xa5a5a5a5a5a5a5a5) ||
               ot.before!=UINT64_C(0xa5a5a5a5a5a5a5a5) || ot.after!=UINT64_C(0xa5a5a5a5a5a5a5a5))valid=0;
            for(u8 *p=buffer.data;p<buffer.end;++p)if(*p!=0xa5){valid=0;break;}
        }
    }
#undef POLY_PUT
    const unsigned packet_sizes[]={sizeof(FxCommonF3Packet),sizeof(FxCommonF4Packet),sizeof(FxCommonG3Packet),sizeof(FxCommonG4Packet),
        sizeof(FxCommonFt3Packet),sizeof(FxCommonFt4Packet),sizeof(FxCommonGt3Packet),sizeof(FxCommonGt4Packet)};
    unsigned guards=0;
    for(unsigned kind=0;kind<8;++kind)for(unsigned fault=0;fault<4;++fault){
        /* Both single packets and all four subdivision packets must fit. */
        if(fault==1 && kind!=4 && kind!=5)continue;
        _Alignas(16) u8 minimal[128]={0};FxCommonPolyResource *one=(FxCommonPolyResource *)minimal;
        one->counts[kind]=1;for(unsigned i=0;i<8;++i)one->offsets[i]=48;
        memcpy(minimal+48,synthetic+header->offsets[kind],strides[kind]);
        for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
        const u32 controls[]={4096,0,4096,0,4096,0,0,fault==1?20000:1000};
        for(unsigned i=0;i<8;++i)Psyz_GteCtrlWrite(i,controls[i]);
        Psyz_GteCtrlWrite(26,768);Psyz_GteCtrlWrite(29,341);Psyz_GteCtrlWrite(30,256);
        memset(&g_FxCommonRenderWorkspace,0xa5,sizeof(g_FxCommonRenderWorkspace));memset(&arena,0xa5,sizeof(arena));memset(&ot,0xa5,sizeof(ot));
        FxCommonBuffer buffer={.data=arena.bytes,.allocation=(RenderNativeTag *)ot.entries,.end=arena.bytes+sizeof(arena.bytes)};
        if(fault<2)buffer.end=arena.bytes+packet_sizes[kind]*((kind==4 || kind==5) && !fault?4:1)-1;
        g_FxCommonActiveBuffer=&buffer;g_FxCommonDepthBase=fault==2?-1000:fault==3?100000:0;
        FxCommon_DrawPolyResource(NULL,one);
        int expected=fault<2?-1:kind<6?-2:0;
        if(buffer.error!=expected || buffer.data!=arena.bytes)valid=0;
        for(unsigned i=0;i<sizeof(arena);++i)if(((u8 *)&arena)[i]!=0xa5)valid=0;
        for(unsigned i=0;i<sizeof(ot);++i)if(((u8 *)&ot)[i]!=0xa5)valid=0;
        ++guards;
    }
    if(guards!=26)valid=0;
    g_FxCommonActiveBuffer=old_buffer;g_FxCommonDepthBase=old_base;g_FxCommonRenderWorkspace=old_work;
    if(!valid || cases!=252 || packets!=6322 || hash!=UINT64_C(0x8389f407efdeab28)){fprintf(stderr,"Original FX polygon resources: cases=%u packets=%u hash=%016llx valid=%d\n",cases,packets,(unsigned long long)hash,valid);return 0;}
    puts("Original FX polygon resources: both real banks and all eight synthetic streams match USA CPU/GTE across subdivision and texture-distance boundaries; native payloads and complete OT chains pass");return 1;
}

static int original_fx_shader_packets(void)
{
    FxCommonGradientQuad *quads[]={g_FxCommonSkyQuads,g_FxCommonShadeQuads,g_FxCommonFloorFadeQuads,g_FxCommonTopFadeQuads};
    FxCommonDrawModePacket *modes[]={g_FxCommonDrawModes0,g_FxCommonDrawModes1,g_FxCommonDrawModes2};
    for(unsigned i=0;i<4;++i)memset(quads[i],0xa5,2*sizeof(*quads[i]));
    for(unsigned i=0;i<3;++i)memset(modes[i],0xa5,2*sizeof(*modes[i]));
    FxCommon_InitFadeQuads();uint64_t hash=UINT64_C(14695981039346656037);int valid=1;
#define SHADER_PUT(value,count) do {uint64_t word_=(uint64_t)(value); \
    for(unsigned byte_=0;byte_<(count);++byte_){hash=(hash^(u8)word_)*UINT64_C(1099511628211);word_>>=8;}}while(0)
    for(unsigned group=0;group<7;++group)for(unsigned slot=0;slot<2;++slot){
        RenderNativeTag *tag=group<4?&quads[group][slot].tag:&modes[group-4][slot].tag;
        if(tag->length!=(group<4?8:1))valid=0;SHADER_PUT(tag->length,1);
        for(unsigned byte=0;byte<tag->length*4;++byte)SHADER_PUT(((u8 *)(tag+1))[byte],1);
        if(group>=4 && modes[group-4][slot].command>UINT32_MAX)valid=0;
    }
    if(hash!=UINT64_C(0x353e0cebf2681c3d))valid=0;
    static struct {u64 before;_Alignas(16) u8 data[4096];u64 after;} arena;
    static struct {u64 before;RenderNativeTag entries[4096];u64 after;} ot;
    s32 fades[605],full[262];for(unsigned i=0;i<601;++i)fades[i]=(s32)i-300;
    memcpy(fades+601,(s32[]){-32768,-32767,32766,32767},16);
    for(unsigned i=0;i<256;++i)full[i]=i;memcpy(full+256,(s32[]){-1,256,32767,65535,INT32_MAX,INT32_MIN},24);
    const s32 markers[]={-32768,-255,-254,-8,-1,0,1,8,247,254,255,32767},night[]={-16,0,16,255};
    const s32 *values[]={fades,full,markers,night};const unsigned numbers[]={605,262,12,4};
    unsigned cases=0,packets=0;
    for(unsigned kind=0;kind<4 && valid;++kind)for(unsigned v=0;v<numbers[kind] && valid;++v)
    for(unsigned slot=0;slot<2 && valid;++slot)for(unsigned pattern=0;pattern<(kind<2?1:4) && valid;++pattern)
    for(unsigned view=0;view<(kind<2?1:3) && valid;++view){
        for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
        const u32 controls[]={4096,0,4096,0,4096,0,0,(u32)(view==0?1000:view==1?8000:-1000)};
        for(unsigned i=0;i<8;++i)Psyz_GteCtrlWrite(i,controls[i]);
        Psyz_GteCtrlWrite(24,160u<<16);Psyz_GteCtrlWrite(25,120u<<16);Psyz_GteCtrlWrite(26,768);Psyz_GteCtrlWrite(29,341);Psyz_GteCtrlWrite(30,256);
        memset(&arena,0xa5,sizeof(arena));memset(&ot,0xa5,sizeof(ot));
        for(unsigned i=0;i<4096;++i)ot.entries[i]=(RenderNativeTag){0xffffff,0xab};
        FxCommonBuffer *buffer=&g_FxCommonFrames[slot].buffer;
        *buffer=(FxCommonBuffer){.data=arena.data,.allocation=ot.entries,.end=arena.data+sizeof(arena.data)};g_FxCommonActiveBuffer=buffer;
        for(unsigned i=0;i<10;++i){g_FxCommonMotionPositions[i].vector.x=(s32)(((u32)(i*37-150)&65535)<<16|i);
            g_FxCommonMotionPositions[i].vector.y=(s32)(((u32)(80-i*19)&65535)<<16|i*3);
            g_FxCommonMotionPositions[i].vector.z=(s32)(((u32)(i*131-300)&65535)<<16|i*7);g_FxCommonMotionPositions[i].pad0C=(s32)0xa5a5a5a5;
            memset(&g_FxCommonEffectSetup[i],0,sizeof(g_FxCommonEffectSetup[i]));
            g_FxCommonEffectSetup[i].position[0]=i*51-200;g_FxCommonEffectSetup[i].position[1]=i*29-100;g_FxCommonEffectSetup[i].position[2]=i*113-400;
            g_FxCommonEffectSetup[i].enabled=pattern==0?0:pattern==1?1:pattern==2?i%2:(i==0 || i==3 || i==9);}
        g_FxCommonRoomVariant=pattern*3;g_FxCommonMarkerFade=(s16)values[kind][v];g_FxCommonNight=pattern%2;s16 result=0;
        if(kind==0)result=FxCommon_StepFadeIntensity((s16)values[kind][v]);
        if(kind==1)FxCommon_DrawFullScreenFade(values[kind][v]);
        if(kind==2)result=FxCommon_DrawEffectMarkers((s16)values[kind][v]);
        if(kind==3){FxCommon_DrawNightShadeAndMarkers();result=g_FxCommonMarkerFade;}
        if(buffer->error || buffer->data<arena.data || buffer->data>buffer->end){valid=0;break;}
        u8 *cursor=arena.data,*addresses[64];unsigned count=0;
        while(cursor<buffer->data){RenderNativeTag *tag=(RenderNativeTag *)cursor;
            if(count>=64 || (tag->length!=1 && tag->length!=3 && tag->length!=8 && tag->length!=12)){valid=0;break;}
            addresses[count++]=cursor;size_t size=(sizeof(*tag)+tag->length*4+_Alignof(RenderNativeTag)-1)&~(size_t)(_Alignof(RenderNativeTag)-1);
            if(size>(size_t)(buffer->data-cursor)){valid=0;break;}cursor+=size;
        }
        if(!valid || cursor!=buffer->data){valid=0;break;}
        SHADER_PUT((u16)result,2);SHADER_PUT(count,2);
        for(unsigned i=0;i<count;++i){RenderNativeTag *tag=(RenderNativeTag *)addresses[i];SHADER_PUT(tag->length,1);
            for(unsigned byte=0;byte<tag->length*4;++byte)SHADER_PUT(addresses[i][sizeof(*tag)+byte],1);
            if(tag->length==1 && ((FxCommonDrawModePacket *)tag)->command>UINT32_MAX)valid=0;}
        const unsigned depths[]={0,9,10,4095};
        for(unsigned d=0;d<4 && valid;++d){unsigned depth=depths[d];SHADER_PUT(depth,2);uintptr_t link=ot.entries[depth].address;unsigned steps=0;
            while(link!=0xffffff){unsigned id=0;while(id<count && link!=(uintptr_t)addresses[id])++id;
                if(id==count){if(link==(uintptr_t)&g_FxCommonShadeQuads[0])id=0x8000;else if(link==(uintptr_t)&g_FxCommonShadeQuads[1])id=0x8001;
                    else if(link==(uintptr_t)&g_FxCommonDrawModes1[0])id=0x8002;else if(link==(uintptr_t)&g_FxCommonDrawModes1[1])id=0x8003;else{valid=0;break;}}
                SHADER_PUT(id,2);RenderNativeTag *tag=(RenderNativeTag *)link;
                if(id>=0x8000){SHADER_PUT(tag->length,1);for(unsigned byte=0;byte<tag->length*4;++byte)SHADER_PUT(((u8 *)(tag+1))[byte],1);}
                link=tag->address;if(++steps>count+4){valid=0;break;}
            }
            SHADER_PUT(65535,2);
        }
        for(unsigned i=0;i<4096;++i)if(ot.entries[i].length!=0xab)valid=0;
        for(u8 *at=buffer->data;at<buffer->end;++at)if(*at!=0xa5)valid=0;
        if(arena.before!=UINT64_C(0xa5a5a5a5a5a5a5a5) || arena.after!=UINT64_C(0xa5a5a5a5a5a5a5a5) ||
           ot.before!=UINT64_C(0xa5a5a5a5a5a5a5a5) || ot.after!=UINT64_C(0xa5a5a5a5a5a5a5a5))valid=0;
        ++cases;packets+=count;
    }
#undef SHADER_PUT
    if(!valid || cases!=2118 || packets!=7028 || hash!=UINT64_C(0xe11d5d9a899124c5)){
        fprintf(stderr,"Original FX shaders: cases=%u packets=%u hash=%016llx valid=%d\n",cases,packets,(unsigned long long)hash,valid);return 0;}
    puts("Original FX shaders: eight initialized gradients/six modes and 2118 USA scenarios match fades, marker labels, selection palettes, night shade, both buffers, native payloads and full OT chains");return 1;
}

static int original_fx_shader_pixels(void)
{
    static u16 saved_pixels[1024*512],pixels[1024*512];RECT whole={0,0,1024,512};
    Pe1GameState saved=g_GameState;FxCommonFrame frames[2];memcpy(frames,g_FxCommonFrames,sizeof(frames));
    FxCommonBuffer *active=g_FxCommonActiveBuffer;int slot_before=g_ActiveDrawSlot;
    DRAWENV draw_before;DISPENV display_before;GetDrawEnv(&draw_before);GetDispEnv(&display_before);
    StoreImage(&whole,saved_pixels);DrawSync(0);Boot_InitMemoryLayout();FxCommon_InitDisplay();
    const unsigned intensities[]={0,32,127,255};int valid=1;
    for(unsigned slot=0;slot<2 && valid;++slot)for(unsigned value=0;value<4 && valid;++value){
        for(unsigned i=0;i<1024*512;++i)pixels[i]=0x1357;LoadImage(&whole,pixels);DrawSync(0);
        RECT area={0,(s16)(slot*240),320,240};ClearImage(&area,160,160,160);DrawSync(0);
        g_FxCommonActiveBuffer=&g_FxCommonFrames[slot].buffer;g_ActiveDrawSlot=slot;FxCommon_ClearActiveOrdering();
        g_FxCommonFrames[slot].drawEnv.dtd=0;PutDrawEnv(&g_FxCommonFrames[slot].drawEnv);PutDispEnv(&g_FxCommonFrames[slot].dispEnv);
        FxCommon_DrawFullScreenFade(intensities[value]);if(g_FxCommonActiveBuffer->error){valid=0;break;}
        DrawOTag((u_int *)&g_FxCommonActiveBuffer->allocation[4095]);DrawSync(0);StoreImage(&whole,pixels);DrawSync(0);
        int channel=20-(int)(intensities[value]>>3);if(channel<0)channel=0;u16 expected=(u16)(channel|(channel<<5)|(channel<<10));
        for(unsigned y=0;y<512 && valid;++y)for(unsigned x=0;x<1024;++x){u16 wanted=x<320 && y>=slot*240 && y<(slot+1)*240?expected:0x1357;
            if(pixels[y*1024+x]!=wanted){fprintf(stderr,"FX fade pixels slot=%u intensity=%u x=%u y=%u got=%04x expected=%04x\n",slot,intensities[value],x,y,pixels[y*1024+x],wanted);valid=0;break;}}
    }
    LoadImage(&whole,saved_pixels);PutDrawEnv(&draw_before);PutDispEnv(&display_before);DrawSync(0);
    g_GameState=saved;memcpy(g_FxCommonFrames,frames,sizeof(frames));g_FxCommonActiveBuffer=active;g_ActiveDrawSlot=slot_before;
    if(valid)puts("Original FX fade pixels: original boot memory/display/arena path, both frame buffers, four subtractive intensities and all 1024x512 VRAM pixels per frame pass headlessly");return valid;
}

static int original_fx_uncompressed_order(void)
{
    static RenderNativeTag ordering[2][4096];static FxCommonDrawModePacket packets[4096];
    const unsigned counts[]={0,1,2,10,127,4096};uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    for(unsigned slot=0;slot<2;++slot)for(unsigned pattern=0;pattern<128;++pattern){
        unsigned count=counts[pattern%6];ClearOTagR((u_int *)ordering[slot],4096);
        uintptr_t terminal=ordering[slot][0].address;
        for(unsigned j=0;j<count;++j){unsigned depth=pattern%4==0?0:pattern%4==1?4095:pattern%4==2?j%4096:(j*1103515245u+pattern*12345u)%4096;
            packets[j].tag=(RenderNativeTag){ordering[slot][depth].address,1};packets[j].command=j;
            ordering[slot][depth].address=(uintptr_t)&packets[j];}
        for(unsigned byte=0;byte<4;++byte)hash=(hash^(u8)(count>>(byte*8)))*UINT64_C(1099511628211);
        uintptr_t link=(uintptr_t)&ordering[slot][4095];unsigned steps=0,seen=0;
        while(link!=0xffffff){RenderNativeTag *tag=(RenderNativeTag *)link;
            if(link==terminal){if(tag->address!=0xffffff || tag->length!=4)return 0;break;}
            if(tag->length){uintptr_t offset=link-(uintptr_t)packets;
                if(offset>=count*sizeof(*packets) || offset%sizeof(*packets))return 0;
                unsigned id=offset/sizeof(*packets);for(unsigned byte=0;byte<2;++byte)hash=(hash^(u8)(id>>(byte*8)))*UINT64_C(1099511628211);++seen;}
            link=tag->address;if(++steps>4096+count+1)return 0;
        }
        if(seen!=count)return 0;++cases;
    }
    if(cases!=256 || hash!=UINT64_C(0x8eff7149a4f01905)){fprintf(stderr,"Original FX OT order: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    puts("Original FX OT order: 256 empty/sparse/dense patterns preserve the same packet order as USA DMA compaction with native OT entries left intact");return 1;
}

static int original_fx_shader_guards(void)
{
    FxCommonBuffer *old_buffer=g_FxCommonActiveBuffer;
    static _Alignas(16) u8 arena[4096];static RenderNativeTag ordering[4096];
    const size_t fade_size=sizeof(FxCommonGradientQuad)+sizeof(FxCommonDrawModePacket);
    unsigned cases=0;int valid=1;
    for(size_t remaining=0;remaining<=fade_size;++remaining){
        memset(arena,0xa5,sizeof(arena));memset(ordering,0xa5,sizeof(ordering));
        FxCommonBuffer buffer={.data=arena,.allocation=ordering,.end=arena+remaining};g_FxCommonActiveBuffer=&buffer;
        FxCommon_DrawFullScreenFade(127);
        if(remaining<fade_size){if(buffer.error!=-1 || buffer.data!=arena)valid=0;
            for(unsigned i=0;i<sizeof(arena);++i)if(arena[i]!=0xa5)valid=0;
            for(unsigned i=0;i<sizeof(ordering);++i)if(((u8 *)ordering)[i]!=0xa5)valid=0;
        }else if(buffer.error || buffer.data!=arena+fade_size)valid=0;
        ++cases;
    }
    const unsigned counts[]={1,4,10};
    for(unsigned slot=0;slot<2;++slot)for(unsigned c=0;c<3;++c)for(unsigned space=0;space<3;++space){
        size_t required=counts[c]*(sizeof(FxCommonLinePacket)+2*sizeof(FxCommonMarkerQuad));
        size_t remaining=space==0?0:space==1?required-1:required;
        memset(arena,0xa5,sizeof(arena));memset(ordering,0xa5,sizeof(ordering));
        for(unsigned i=0;i<10;++i)g_FxCommonEffectSetup[i].enabled=i<counts[c];
        FxCommonBuffer *buffer=&g_FxCommonFrames[slot].buffer;*buffer=(FxCommonBuffer){.data=arena,.allocation=ordering,.end=arena+remaining};g_FxCommonActiveBuffer=buffer;
        if(FxCommon_DrawEffectMarkers(127)!=135)valid=0;
        if(space<2){if(buffer->error!=-1 || buffer->data!=arena)valid=0;
            for(unsigned i=0;i<sizeof(arena);++i)if(arena[i]!=0xa5)valid=0;
            for(unsigned i=0;i<sizeof(ordering);++i)if(((u8 *)ordering)[i]!=0xa5)valid=0;
        }else if(buffer->error || buffer->data!=arena+required)valid=0;
        ++cases;
    }
    FxCommonBuffer buffer={.data=arena,.allocation=ordering,.end=arena};g_FxCommonActiveBuffer=&buffer;
    if(FxCommon_DrawEffectMarkers(0) || buffer.error || buffer.data!=arena)valid=0;++cases;
    g_FxCommonActiveBuffer=old_buffer;
    if(!valid){fprintf(stderr,"Original FX shader guards: cases=%u\n",cases);return 0;}
    printf("Original FX shader guards: %u exact-capacity/short-arena cases, atomic errors, both marker buffers and zero-level mode-only drawing pass\n",cases);return 1;
}

static int original_fx_blend(void)
{
    const s16 counters[]={-32768,-1,0,1,2,31,32,32767};uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    for(unsigned c=0;c<8;++c)for(unsigned pattern=0;pattern<16;++pattern){
        u32 positions[40],deltas[40];for(unsigned j=0;j<40;++j){positions[j]=j*0x01234567u+pattern*0x9e3779b9u;deltas[j]=~(j*0x07654321u)+pattern*0x13579bdfu;}
        memcpy(g_FxCommonMotionPositions,positions,sizeof(positions));memcpy(g_FxCommonMotionDeltas,deltas,sizeof(deltas));g_FxCommonMotionBlendFrames=counters[c];
        for(unsigned step=0;step<34;++step){FxCommon_StepMotionBlend();
            const u8 *parts[]={(const u8 *)&g_FxCommonMotionBlendFrames,(const u8 *)g_FxCommonMotionPositions};const unsigned sizes[]={2,160};
            for(unsigned p=0;p<2;++p)for(unsigned b=0;b<sizes[p];++b)hash=(hash^parts[p][b])*UINT64_C(1099511628211);
            if(memcmp(g_FxCommonMotionDeltas,deltas,sizeof(deltas)))return 0;++cases;
        }
    }
    if(cases!=4352 || hash!=UINT64_C(0xfd879abe57c21976)){fprintf(stderr,"Original FX blend: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    puts("Original FX blend: 4352 USA updates match all ten shared motion records, signed/wrapped additions, counter expiry and untouched padding/deltas");return 1;
}

static uint64_t fx_camera_motion_hash(uint64_t hash,int path)
{
    const void *fields[]={&g_FxCommonCameraPosition,&g_FxCommonCameraTarget,
        &g_FxCommonMotionStep0,&g_FxCommonMotionAccum0,&g_FxCommonMotionOrigin0,&g_FxCommonMotionTarget0,&g_FxCommonMotionShift0,&g_FxCommonMotionRemaining0,
        &g_FxCommonMotionStep1,&g_FxCommonMotionAccum1,&g_FxCommonMotionOrigin1,&g_FxCommonMotionTarget1,&g_FxCommonMotionShift1,&g_FxCommonMotionRemaining1,&g_FxCommonPathWrapped};
    const unsigned sizes[]={12,12,12,12,6,6,2,2,12,12,6,6,2,2,1};
    const void *path_fields[]={&g_FxCommonCameraPosition,&g_FxCommonCameraTarget,&g_FxCommonMotionRemaining0,&g_FxCommonMotionRemaining1,
        &g_FxCommonMarkerFade,&g_FxCommonCameraPathRemaining,&g_FxCommonCameraPathStep,&g_FxCommonCameraPathIndex,&g_FxCommonPathWrapped};
    const unsigned path_sizes[]={12,12,2,2,2,1,1,1,1};
    for(unsigned i=0;i<(path?9:15);++i){const u8 *bytes=path?(const u8 *)path_fields[i]:(const u8 *)fields[i];
        for(unsigned j=0;j<(path?path_sizes[i]:sizes[i]);++j){hash=(hash^bytes[j])*UINT64_C(1099511628211);}}
    return hash;
}

static void fx_seed_camera_motion(unsigned pose,FxCommonShortVec3 **shorts)
{
    const FxCommonVec3 positions[]={{0,0,0},{100,-200,300},{INT32_MAX,INT32_MIN,-1},{-32768,32767,0x12345678}};
    const FxCommonVec3 targets[]={{1000,-1000,500},{32767,-32768,100},{-1000000,2000000,-3000000},{INT32_MIN,INT32_MAX,-0x1234567}};
    g_FxCommonCameraPosition=positions[pose];g_FxCommonCameraTarget=targets[pose];g_FxCommonPathWrapped=pose%2;
    for(unsigned j=0;j<6;++j)*shorts[j]=(FxCommonShortVec3){(s16)(-30000+(int)j*701),(s16)(29000-(int)j*503),(s16)(-12345+(int)j*997)};
    memset(&g_FxCommonMotionStep0,0xa5,sizeof(g_FxCommonMotionStep0));memset(&g_FxCommonMotionStep1,0xa5,sizeof(g_FxCommonMotionStep1));
    memset(&g_FxCommonMotionAccum0,0xa5,sizeof(g_FxCommonMotionAccum0));memset(&g_FxCommonMotionAccum1,0xa5,sizeof(g_FxCommonMotionAccum1));
    g_FxCommonMotionShift0=g_FxCommonMotionShift1=g_FxCommonMotionRemaining0=g_FxCommonMotionRemaining1=(s16)0xa5a5;
}

static int original_fx_scene_init(PeDisc *disc)
{
    PeDiscFile archive,exe;
    if(!pe_disc_find(disc,"PE.IMG",&archive) || !pe_disc_find(disc,"SLUS_006.62",&exe) ||
       !pe_disc_read(disc,&exe,0x930d8-0x10000+0x800+76*2,g_PeImageSectorTable+76,10))return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
    if(!SDL_Init(SDL_INIT_EVENTS))return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;ResetGraph(0);Boot_InitMemoryLayout();
    CdRom_BindImage(disc,&archive);g_GameState.pe_image_base_lba=archive.sector;
    fx_image_io=1;glyph_reads=glyph_polls=fx_read_failure=fx_poll_failure=0;
    const unsigned rooms[]={0xb7,0xa1,0x5d,0x104,0x7d,0x122,0x179,0x2e,0x3a,0xbf};
    const unsigned areas[]={0,0x80,0x208,0xb8,0xc0,0xd0,0xd8,0xe0,0xe4,0x148,0x160,0x178,0x180,0x1c0,0x1c8};
    const unsigned flags[]={0,0xffffff,0x555555};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
#define SCENE_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned variant=0;variant<10;++variant)for(unsigned night=0;night<2;++night)
    for(unsigned flag=0;flag<3;++flag)for(unsigned area=0;area<15;++area){
        g_ScriptVariableBank[SCRIPT_VAR_ROOM]=rooms[variant];g_ScriptVariableBank[SCRIPT_VAR_AREA]=areas[area];
        g_ScriptVariableBank[SCRIPT_VAR_FLAGS]=(flags[flag]&~0x2000u)|(night?0x2000u:0);
        /* Re-entry must not inherit animation or reserved fields from the
         * previous visit; the original would have reloaded the overlay. */
        memset(&g_FxCommonRuntime,0xa5,sizeof(g_FxCommonRuntime));
        memset(g_FxCommonEffectSetup,0xa5,sizeof(g_FxCommonEffectSetup));
        memset(g_FxCommonMotionPositions,0xa5,sizeof(g_FxCommonMotionPositions));
        int result=FxCommon_InitScene();
        if(result || g_FxCommonRouteNodeCount!=132){fprintf(stderr,"FX scene init case %u result=%d routes=%d\n",cases,result,g_FxCommonRouteNodeCount);return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;}
        for(unsigned i=0;i<10;++i)if(g_FxCommonRuntime.pairedNodes[i]!=&g_FxCommonRecords[2*i] ||
           g_FxCommonRuntime.pairedNodes[10+i]!=&g_FxCommonRecords[2*i+1])return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
        for(unsigned i=0;i<11;++i)if(g_FxCommonSceneNodes[i]!=&g_FxCommonRecords[22+i])return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
        for(unsigned i=0;i<132;++i)if(g_FxCommonRouteNodes[i]!=&g_FxCommonRecords[55+i])return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
        if(g_FxCommonRuntime.node0!=&g_FxCommonRecords[20] || g_FxCommonType51Node!=&g_FxCommonRecords[21] ||
           g_FxCommonRuntime.node25!=&g_FxCommonRecords[33] || g_FxCommonType54Node!=&g_FxCommonRecords[34])return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
        const unsigned path_ids[]={37,39,38,40,35,36},echo_ids[]={41,43,42,44};
        for(unsigned i=0;i<6;++i)if(g_FxCommonPathNodes[i]!=&g_FxCommonRecords[path_ids[i]])return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
        for(unsigned i=0;i<4;++i)if(g_FxCommonEchoNodes[i]!=&g_FxCommonRecords[echo_ids[i]] ||
           g_FxCommonType26Nodes[i]!=&g_FxCommonRecords[47+i])return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
        for(unsigned i=0;i<2;++i)if(g_FxCommonType21Nodes[i]!=&g_FxCommonRecords[45+i] ||
           g_FxCommonLodNodes[i]!=&g_FxCommonRecords[51+i] || g_FxCommonPairedNodes[i]!=&g_FxCommonRecords[53+i])return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
        for(unsigned i=0;i<sizeof(g_FxCommonEffectSetup);++i)SCENE_PUT(((u8 *)g_FxCommonEffectSetup)[i],1);
        SCENE_PUT(g_FxCommonRouteNodeCount,4);
        FxCommonNode *ordered[200],*node;unsigned count=0;FxCommon_BeginRecordIteration();
        while((node=FxCommon_NextRecord())){if(count>=200)return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;ordered[count++]=node;}
        SCENE_PUT(count,2);if(count!=187)return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
        for(unsigned i=0;i<count;++i){node=ordered[i];unsigned ident=node->id,kind=ident<20 || ident>=51?2:ident==20?0:1;
            SCENE_PUT(ident,2);SCENE_PUT(kind,1);SCENE_PUT(node->type,2);
            if(kind==2){SCENE_PUT(node->middleKind,2);SCENE_PUT(node->nearKind,2);SCENE_PUT(node->lod.margin,2);
                SCENE_PUT(node->lod.farDepth,4);SCENE_PUT(node->lod.nearDepth,4);}
            else if(kind==1)SCENE_PUT(node->margin,2);
            SCENE_PUT((u8 *)node->resource-(u8 *)g_FxCommonModelTable,4);
            for(unsigned j=0;j<3;++j)SCENE_PUT(kind==2?node->lod.matrix.t[j]:node->matrix.t[j],4);
            for(unsigned j=0;j<3;++j)SCENE_PUT(kind==2?node->lod.seed.component[j]:node->seed.component[j],2);
            SCENE_PUT(node->bucket,2);SCENE_PUT(fx_node_id(node->parent),2);
            SCENE_PUT(fx_node_id(node->previous),2);SCENE_PUT(fx_node_id(node->next),2);
        }
        SCENE_PUT(g_FxCommonRoomVariant,2);SCENE_PUT(g_FxCommonViewMode,2);
        SCENE_PUT(g_FxCommonPlaying,1);SCENE_PUT(g_FxCommonPlayAdvance,1);SCENE_PUT(g_FxCommonReplayCamera,1);
        SCENE_PUT(g_FxCommonPlayTicks,4);SCENE_PUT(g_FxCommonPlayFrame,4);SCENE_PUT(D_8019C00D,1);SCENE_PUT(D_8019C02E,2);
        s32 *positions[]={&g_FxCommonCameraPosition.x,&g_FxCommonCameraTarget.x};
        for(unsigned i=0;i<2;++i)for(unsigned j=0;j<3;++j)SCENE_PUT(positions[i][j],4);
        for(unsigned i=0;i<sizeof(g_FxCommonMotionPositions);++i)SCENE_PUT(((u8 *)g_FxCommonMotionPositions)[i],1);
        ++cases;
    }
#undef SCENE_PUT
    for(unsigned failure=0;failure<4;++failure){
        CdRom_BindImage(disc,&archive);
        glyph_reads=glyph_polls=0;fx_read_failure=failure<2?failure+1:0;fx_poll_failure=failure>=2?failure-1:0;
        g_FxCommonRouteNodeCount=123;
        if(FxCommon_InitScene()!=-1 || g_FxCommonRouteNodeCount!=123 ||
           glyph_reads!=failure%2+1 || glyph_polls!=(failure<2?failure:failure-1))return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
    }
    CdRom_BindImage(disc,&archive);
    glyph_reads=glyph_polls=fx_read_failure=fx_poll_failure=0;
    if(FxCommon_InitScene()!=0 || g_FxCommonRouteNodeCount!=132)return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;
    fx_image_io=0;
    if(cases!=900 || hash!=UINT64_C(0x8a8774df804ba8b5)){fprintf(stderr,"Original FX InitScene: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return fprintf(stderr,"FX scene failure at line %d\n",__LINE__),0;}
    puts("Original FX InitScene: 900 USA scene graphs, all ten room variants, both day/night model banks, three flag patterns and 15 entry areas, all node slots, four I/O failures and recovery pass with real image I/O");return 1;
}

static int original_fx_controller(void)
{
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    const unsigned rooms[]={0xb7,0xa1,0x5d,0x104,0x7d,0x122,0x179,0x2e,0x3a,0xbf};
    for(unsigned scenario=0;scenario<44;++scenario){
        unsigned variant=scenario<40?scenario/4:0,night=scenario<40?(scenario/2)%2:(scenario-40)/2;
        unsigned script=scenario<40?scenario%2:2+(scenario%2);
        g_ScriptVariableBank[SCRIPT_VAR_ROOM]=rooms[variant];
        g_ScriptVariableBank[SCRIPT_VAR_AREA]=script==2?0x80:script==3?0x208:0;
        g_ScriptVariableBank[SCRIPT_VAR_FLAGS]=0xffdfff|(night?0x2000:0);g_GameState.flags=0;
        fx_image_io=1;glyph_reads=glyph_polls=fx_read_failure=fx_poll_failure=0;
        if(FxCommon_InitScene()!=0)return 0;
        for(unsigned frame=0;frame<(script==2?1230:512);++frame){
            u32 pressed=0;
            if(script==0){switch(frame%80){case 0:pressed=0x40;break;case 20:pressed=0x22;break;case 40:pressed=0x10;break;case 60:pressed=0x50;break;}}
            else if(script==1 || script==3){if(frame==0 || frame==300 || frame==400)pressed=0x20000000;else if(frame==240)pressed=0x40000000;}
            g_FieldPadPressed=g_FieldPadBits=pressed;
            FxCommon_StepEffects();FxCommon_StepPathNodes();FxCommon_StepMotionBlend();
            if(g_FxCommonActiveBuffer->error)return 0;
            const void *fields[]={&g_FxCommonPageChanged,&g_FxCommonPageOpen,&g_FxCommonExitRequested,&g_FxCommonExitCountdown,
                &g_FxCommonPageId,&g_FxCommonScreenFade,&g_FxCommonDemoFrame,&g_FxCommonDrawTintedCopies,
                &g_FxCommonTailPathPosition,&g_FxCommonFlyerPathPosition,&g_FxCommonCameraPathPosition,&g_FxCommonDemoTicks,
                &g_FxCommonDemoExiting,&g_FxCommonDemoResource,&g_FxCommonDemoResourceTicks,&g_FxCommonDemoShade,&g_FxCommonDemoFadePhase,
                &g_FxCommonInputLock,&g_FxCommonTailViewEntered,&g_FxCommonExitVariant,&g_FxCommonResetRequested,D_8019C0D0,&g_FxCommonResourceState,
                &g_FxCommonRoomVariant,&g_FxCommonViewMode,&g_FxCommonPlaying,&g_FxCommonPlayAdvance,&g_FxCommonReplayCamera,
                &g_FxCommonPlayTicks,&g_FxCommonPlayFrame,&D_8019C00D,&D_8019C02E};
            const unsigned sizes[]={4,4,2,2,2,2,2,2,4,4,4,4,4,2,4,1,1,2,2,4,1,4,8,2,2,1,1,1,4,4,1,2};
            for(unsigned i=0;i<sizeof(sizes)/sizeof(*sizes);++i)for(unsigned j=0;j<sizes[i];++j){hash=(hash^((const u8 *)fields[i])[j])*UINT64_C(1099511628211);}
            hash=fx_camera_motion_hash(hash,0);
            const void *path_fields[]={&g_FxCommonMarkerFade,&g_FxCommonCameraPathRemaining,&g_FxCommonCameraPathStep,&g_FxCommonCameraPathIndex,&g_FxCommonMotionBlendFrames,g_FxCommonMotionPositions,g_FxCommonMotionDeltas};
            const unsigned path_sizes[]={2,1,1,1,2,160,160};
            for(unsigned i=0;i<7;++i)for(unsigned j=0;j<path_sizes[i];++j){hash=(hash^((const u8 *)path_fields[i])[j])*UINT64_C(1099511628211);}
            for(unsigned i=0;i<6;++i){const void *parts[]={g_FxCommonPathNodes[i]->matrix.t,g_FxCommonPathNodes[i]->seed.component};
                for(unsigned part=0;part<2;++part)for(unsigned byte=0;byte<(part?6:12);++byte){hash=(hash^((const u8 *)parts[part])[byte])*UINT64_C(1099511628211);}}
            ++cases;if(g_FxCommonExitRequested)break;
        }
    }
    fx_image_io=0;
    if(cases!=21624 || hash!=UINT64_C(0xe9ce596e8e1fa6c1)){fprintf(stderr,"FX controller: %u states hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    fx_image_io=1;
    for(unsigned fault=0;fault<4;++fault){
        g_ScriptVariableBank[SCRIPT_VAR_AREA]=0;
        if(FxCommon_InitScene()!=0)return 0;
        g_FieldPadPressed=g_FieldPadBits=fault%2?0x10:0x40;
        if(fault<2){for(unsigned i=0;i<10;++i)g_FxCommonEffectSetup[i].enabled=0;}
        else if(fault%2)g_FxCommonEffectSetup[g_FxCommonRoomVariant].value23=254;
        else g_FxCommonEffectSetup[g_FxCommonRoomVariant].value22=254;
        FxCommon_StepEffects();if(g_FxCommonActiveBuffer->error!=-3)return 0;
    }
    fx_image_io=0;
    puts("Original FX controller: 21624 USA states across 44 browsing/play/cancel/demo/path sequences, all playback/motion/resource fields and six animated nodes match; four invalid selection graphs terminate");return 1;
}

static unsigned fx_loop_vblanks,fx_loop_frames,fx_loop_packets,fx_loop_pixels,fx_loop_failures,fx_loop_demo;
static u16 fx_loop_image[320*480];
static void fx_loop_observe(void)
{
    Boot_VsyncCallback();++fx_loop_vblanks;
    if(fx_loop_vblanks>3000){++fx_loop_failures;g_FxCommonExitRequested=1;return;}
    if(!(fx_loop_vblanks&1))return;
    FxCommonBuffer *buffer=g_FxCommonActiveBuffer;
    u8 *base=g_RenderScratchBufferBase[0]+g_ActiveDrawSlot*sizeof(FxCommonBufferRegion);
    if(buffer->data==base)return; /* EndSequence's final 30 blanks. */
    ++fx_loop_frames;
    if(buffer->data<base || buffer->data>buffer->end || buffer->error){++fx_loop_failures;g_FxCommonExitRequested=1;return;}
    uintptr_t link=(uintptr_t)&buffer->allocation[4095];unsigned steps=0,packets=0;
    while(link!=0xffffff){
        RenderNativeTag *tag=(RenderNativeTag *)link;
        if(link<0x10000 || link%_Alignof(RenderNativeTag) || ++steps>65536 || tag->length>16){++fx_loop_failures;break;}
        if(link>=(uintptr_t)base && link<(uintptr_t)buffer->end &&
           link+sizeof(*tag)+tag->length*4>(uintptr_t)buffer->data){++fx_loop_failures;break;}
        if(tag->length)++packets;link=tag->address;
    }
    if(packets<10)++fx_loop_failures;fx_loop_packets+=packets;
    if(fx_loop_frames%20==0){
        RECT area={0,0,320,480};StoreImage(&area,fx_loop_image);DrawSync(0);
        unsigned visible=0;for(unsigned i=0;i<320*480;++i)if(fx_loop_image[i]&0x7fff)++visible;
        if(visible>10000)++fx_loop_pixels;
    }
    if(!fx_loop_demo){
        /* Exercise the original SDL->pad packet->field input path. Two
         * confirmations start the playback and then request a normal exit. */
        if(fx_loop_frames==20 || fx_loop_frames==100)host_event(SDL_EVENT_KEY_DOWN,SDL_SCANCODE_X);
        if(fx_loop_frames==21 || fx_loop_frames==101)host_event(SDL_EVENT_KEY_UP,SDL_SCANCODE_X);
        if(fx_loop_frames>200){++fx_loop_failures;g_FxCommonExitRequested=1;}
    }
}

static int original_fx_frame_loop(PeDisc *disc)
{
    PeDiscFile archive,exe;u16 sectors[5];
    if(!pe_disc_find(disc,"PE.IMG",&archive) || !pe_disc_find(disc,"SLUS_006.62",&exe) ||
       !pe_disc_read(disc,&exe,0x930d8-0x10000+0x800+72*2,sectors,sizeof(sectors)))return 0;
    u8 *textures=malloc(0x80000);if(!textures)return 0;
    unsigned total_frames=0,total_packets=0;
    for(unsigned night=0;night<2;++night)for(unsigned demo=0;demo<2;++demo){
        for(unsigned bank=0;bank<2;++bank){unsigned index=bank?(night?1:2):0;
            if(!pe_disc_read(disc,&archive,(uint64_t)sectors[index]*2048,textures,(sectors[index+1]-sectors[index])*2048u)){free(textures);return 0;}
            Draw_LoadTimTable((TimRelativeTable *)textures,bank?262:3);DrawSync(0);
        }
        Boot_BuildRenderFlagTable();Boot_ResetPadInput();g_FieldPadBits=g_FieldPadPressed=0;
        g_GameState.flags=0;
        g_ScriptVariableBank[SCRIPT_VAR_AREA]=demo?0x80:0;
        g_ScriptVariableBank[SCRIPT_VAR_ROOM]=0xb7;
        g_ScriptVariableBank[SCRIPT_VAR_FLAGS]=0xffdfff|(night?0x2000:0);
        CdRom_BindImage(disc,&archive);
        fx_image_io=1;glyph_reads=glyph_polls=fx_read_failure=fx_poll_failure=0;
        fx_loop_vblanks=fx_loop_frames=fx_loop_packets=fx_loop_pixels=fx_loop_failures=0;fx_loop_demo=demo;
        VSyncCallback(fx_loop_observe);int result=FxCommon_RunFrameLoop();VSyncCallback(NULL);fx_image_io=0;
        fprintf(stderr,"FX full loop night=%u demo=%u result=%d frames=%u packets=%u visible=%u errors=%u\n",night,demo,result,fx_loop_frames,fx_loop_packets,fx_loop_pixels,fx_loop_failures);
        if(result || fx_loop_failures || fx_loop_frames!=(demo?1216u:116u) || !fx_loop_pixels ||
           g_FxCommonResetRequested || !g_FxCommonExitRequested){free(textures);return 0;}
        total_frames+=fx_loop_frames;total_packets+=fx_loop_packets;
    }
    free(textures);
    printf("Original FX frame loop: %u frames, %u linked packets, day/night original texture banks, original SDL input/playback and complete demo exits, both buffers and headless pixels pass\n",total_frames,total_packets);return 1;
}

static int original_fx_camera_motion(PeDisc *disc)
{
    PeDiscFile archive;if(!pe_disc_find(disc,"PE.IMG",&archive) ||
       !pe_disc_read(disc,&archive,1975u*2048,g_FxCommonTimTable,41*2048))return 0;
    /* Compare authored data beyond the old scanner's 26-sector code slice.
     * The real InitScene loads these addresses, not identity matrices. */
    u8 sceneData[0x68];
    if(!pe_disc_read(disc,&archive,1792u*2048+(0x8019c0d4u-0x8018eff0u),sceneData,sizeof(sceneData)) ||
       memcmp(sceneData,g_FxCommonRouteFarDepths,20) ||
       memcmp(sceneData+20,g_FxCommonRouteNearDepths,20) ||
       memcmp(sceneData+40,&g_FxCommonColorMatrix,32) ||
       memcmp(sceneData+72,&g_FxCommonLightMatrix,32))return 0;
    g_FxCommonPathTailNodes[0]=(FxCommonTransformNode *)&g_FxCommonDefaultNode;
    g_FxCommonPathNodes[5]=(FxCommonTransformNode *)&g_FxCommonDefaultNode;
    if(g_FxCommonPathNodes[4]!=(FxCommonTransformNode *)&g_FxCommonDefaultNode ||
       g_FxCommonPathTailNodes[1]!=(FxCommonTransformNode *)&g_FxCommonDefaultNode)return 0;
    g_FxCommonPathNodes[4]=g_FxCommonPathNodes[5]=NULL;
    FxCommonShortVec3 external[2];FxCommonShortVec3 *shorts[]={external,external+1,&g_FxCommonMotionOrigin0,&g_FxCommonMotionTarget0,&g_FxCommonMotionOrigin1,&g_FxCommonMotionTarget1};
    const s32 shifts[]={0,1,2,14,15,16,31,32,63,255,256,-1};
    const u32 sources[]={0,1,255,256,0xffffff80};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;int valid=1;
    for(unsigned pose=0;pose<4;++pose)for(unsigned a=0;a<6;++a)for(unsigned b=0;b<6;++b)for(unsigned d=0;d<12;++d){
        fx_seed_camera_motion(pose,shorts);FxCommon_SetMotionEndpoints(shorts[a],shorts[b],shifts[d],shifts[(d*5+3)%12]);
        hash=fx_camera_motion_hash(hash,0);++cases;
    }
    if(cases!=1728 || hash!=UINT64_C(0x8ad0fc610fb630f5)){fprintf(stderr,"Original camera endpoints: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    for(unsigned i=0;i<10;++i){g_FxCommonEffectSetup[i].motionIndices[0]=(i*7)%80;g_FxCommonEffectSetup[i].motionIndices[1]=(i*7+1)%80;}
    hash=UINT64_C(14695981039346656037);cases=0;
    for(unsigned pose=0;pose<4;++pose)for(unsigned index=0;index<10;++index)for(unsigned source=0;source<5;++source)for(unsigned d=0;d<12;++d){
        fx_seed_camera_motion(pose,shorts);FxCommon_StartCameraMove(index,shifts[d],shifts[(d*5+3)%12],sources[source]);
        hash=fx_camera_motion_hash(hash,0);++cases;
    }
    if(cases!=2400 || hash!=UINT64_C(0xf71e670bec43b4dd)){fprintf(stderr,"Original sampled camera endpoints: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    hash=UINT64_C(14695981039346656037);cases=0;
    for(unsigned index=0;index<79;++index)for(unsigned source=0;source<5;++source){
        g_FxCommonMotionRemaining0=123;g_FxCommonMotionRemaining1=-234;g_FxCommonMarkerFade=321;g_FxCommonPathWrapped=index%2;
        g_FxCommonCameraPathRemaining=g_FxCommonCameraPathStep=g_FxCommonCameraPathIndex=0xa5;
        if(FxCommon_InitCameraPath(index+(index%2?0x10000:0),111,-222,(s32)sources[source]))valid=0;
        hash=fx_camera_motion_hash(hash,1);++cases;
        unsigned count=g_FxCommonCameraPathRemaining;
        for(unsigned step=0;step<count+2;++step){int expected=g_FxCommonCameraPathRemaining>1;
            if(FxCommon_StepCameraPath()!=expected)valid=0;hash=fx_camera_motion_hash(hash,1);++cases;}
    }
    for(unsigned index=0;index<79;++index){
        g_FxCommonMotionRemaining0=g_FxCommonMotionRemaining1=g_FxCommonMarkerFade=0;g_FxCommonPathWrapped=0;
        g_FxCommonCameraPathRemaining=3;g_FxCommonCameraPathStep=254;g_FxCommonCameraPathIndex=index;
        for(unsigned step=0;step<4;++step){int expected=g_FxCommonCameraPathRemaining>1;
            if(FxCommon_StepCameraPath()!=expected)valid=0;hash=fx_camera_motion_hash(hash,1);++cases;}
    }
    if(!valid || cases!=47836 || hash!=UINT64_C(0xc29a21ef98b25f06)){fprintf(stderr,"Original camera paths: cases=%u hash=%016llx valid=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    /* Bad path selectors and rejected point counts leave all observable state intact. */
    const int bad[]={-1,79,255,65535,0x1004f};unsigned errors=0;
    for(unsigned i=0;i<5;++i){uint64_t before=fx_camera_motion_hash(UINT64_C(14695981039346656037),1);
        if(FxCommon_InitCameraPath(bad[i],0,0,0)!=-1 || fx_camera_motion_hash(UINT64_C(14695981039346656037),1)!=before)valid=0;++errors;}
    for(unsigned index=79;index<=255;index+=176){g_FxCommonCameraPathIndex=index;g_FxCommonCameraPathRemaining=2;
        uint64_t before=fx_camera_motion_hash(UINT64_C(14695981039346656037),1);
        if(FxCommon_StepCameraPath()!=-1 || fx_camera_motion_hash(UINT64_C(14695981039346656037),1)!=before)valid=0;++errors;}
    u8 *table=Draw_GetTimTableEntry(g_FxCommonTimTable,2);
    for(unsigned index=0;index<2;++index){u32 offset;memcpy(&offset,table+index*4,4);s16 count;memcpy(&count,table+offset+6,2);
        s16 bad_count=2;memcpy(table+offset+6,&bad_count,2);
        g_FxCommonCameraPathIndex=0;g_FxCommonCameraPathStep=255;g_FxCommonCameraPathRemaining=2;g_FxCommonPathWrapped=0;
        uint64_t before=fx_camera_motion_hash(UINT64_C(14695981039346656037),1);
        if(FxCommon_InitCameraPath(0,0,0,-128)!=-1 || fx_camera_motion_hash(UINT64_C(14695981039346656037),1)!=before)valid=0;++errors;
        if(FxCommon_StepCameraPath()!=-1 || fx_camera_motion_hash(UINT64_C(14695981039346656037),1)!=before)valid=0;++errors;
        memcpy(table+offset+6,&count,2);
    }
    if(!valid || errors!=11){fprintf(stderr,"Original camera path guards: errors=%u valid=%d\n",errors,valid);return 0;}
    puts("Original camera motion: 1728 USA aliased-endpoint scenarios, 2400 sampled constructors, all 79 real path pairs through 47836 init/update/hold states, shift/counter wrapping and 11 atomic error cases pass");return 1;
}

static int original_fx_model_render_assets(PeDisc *disc)
{
    PeDiscFile archive;if(!pe_disc_find(disc,"PE.IMG",&archive))return 0;
    static _Alignas(16) u8 bank[100*2048];
    static struct {u64 before;_Alignas(16) u8 bytes[0x40000];u64 after;} arena;
    static struct {u64 before;RenderNativeTag entries[4096];u64 after;} ot;
    const unsigned sectors[]={2016,2105},sizes[]={89,100};
    const s32 poses[][3]={{1000,0,0},{8000,800,0},{-1000,1000,0},{16000,4095,1}};
    void (*draws[])(void *,FxCommonPolyModel *,int)={FxCommon_DrawModel,FxCommon_DrawModelTinted,FxCommon_DrawModelFlat};
    FxCommonBuffer *old_buffer=g_FxCommonActiveBuffer; s32 old_bias=g_FxCommonDepthBias;
    u8 old_color[4];memcpy(old_color,g_FxCommonFlatColor,4);
    FxCommonRenderScratchpad old_work=g_FxCommonRenderWorkspace;
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0,packets=0,coverage[8]={0};int valid=1;
#define MODEL_PUT(value,count) do {uint64_t word_=(uint64_t)(value); \
    for(unsigned byte_=0;byte_<(count);++byte_){hash=(hash^(u8)word_)*UINT64_C(1099511628211);word_>>=8;}}while(0)
    for(unsigned variant=0;variant<2 && valid;++variant){
        if(!pe_disc_read(disc,&archive,(uint64_t)sectors[variant]*2048,bank,sizes[variant]*2048)){valid=0;break;}
        FxCommonPolyModel *models=(FxCommonPolyModel *)(bank+8);
        if(models->bankOffsets[0]!=85*4){valid=0;break;}
        for(unsigned index=0;index<85 && valid;++index)for(unsigned renderer=0;renderer<3 && valid;++renderer)
        for(unsigned pose=0;pose<4 && valid;++pose){
            for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
            const s32 controls[]={4096,0,poses[pose][2]?-4096:4096,0,4096,0,0,poses[pose][0]};
            for(unsigned i=0;i<8;++i)Psyz_GteCtrlWrite(i,(u32)controls[i]);
            Psyz_GteCtrlWrite(24,160u<<16);Psyz_GteCtrlWrite(25,120u<<16);Psyz_GteCtrlWrite(26,768);
            Psyz_GteCtrlWrite(29,341);Psyz_GteCtrlWrite(30,256);
            memset(&g_FxCommonRenderWorkspace,0xa5,sizeof(g_FxCommonRenderWorkspace));
            memset(&arena,0xa5,sizeof(arena));memset(&ot,0xa5,sizeof(ot));
            for(unsigned i=0;i<4096;++i)ot.entries[i]=(RenderNativeTag){0xffffff,0xab};
            FxCommonBuffer buffer={.data=arena.bytes,.allocation=(RenderNativeTag *)ot.entries,.end=arena.bytes+sizeof(arena.bytes)};
            g_FxCommonActiveBuffer=&buffer;g_FxCommonDepthBias=poses[pose][1];
            memcpy(g_FxCommonFlatColor,(u8[]){40,80,120,0},4);
            draws[renderer](NULL,models,index);
            if(buffer.error || buffer.data<arena.bytes || buffer.data>buffer.end){valid=0;break;}
            u8 *cursor=arena.bytes,*addresses[8192];unsigned count=0;
            while(cursor<buffer.data){RenderNativeTag *tag=(RenderNativeTag *)cursor;
                if(count>=8192 || (tag->length!=4 && tag->length!=5 && tag->length!=6 && tag->length!=7 && tag->length!=8 && tag->length!=9 && tag->length!=12)){valid=0;break;}
                addresses[count++]=cursor;
                size_t size=(sizeof(*tag)+tag->length*4+_Alignof(RenderNativeTag)-1)&~(size_t)(_Alignof(RenderNativeTag)-1);
                if(size>(size_t)(buffer.data-cursor)){valid=0;break;}cursor+=size;
            }
            if(!valid || cursor!=buffer.data){valid=0;break;}
            MODEL_PUT(count,4);
            for(unsigned i=0;i<count;++i){RenderNativeTag *tag=(RenderNativeTag *)addresses[i];MODEL_PUT(tag->length,1);
                const u8 codes[]={0x20,0x28,0x30,0x38,0x24,0x2c,0x34,0x3c};
                unsigned code=addresses[i][sizeof(*tag)+3]&0xfc;
                for(unsigned kind=0;kind<8;++kind)if(code==codes[kind])++coverage[kind];
                for(unsigned byte=0;byte<tag->length*4;++byte)MODEL_PUT(addresses[i][sizeof(*tag)+byte],1);}
            for(unsigned depth=0;depth<4096 && valid;++depth){
                if(ot.entries[depth].length!=0xab){valid=0;break;}
                uintptr_t link=ot.entries[depth].address;if(link==0xffffff)continue;MODEL_PUT(depth,2);unsigned steps=0;
                while(link!=0xffffff){unsigned id=0;while(id<count && (uintptr_t)addresses[id]!=link)++id;
                    if(id==count || ++steps>count){valid=0;break;}MODEL_PUT(id,2);link=((RenderNativeTag *)link)->address;}
                MODEL_PUT(65535,2);
            }
            MODEL_PUT(65535,2);packets+=count;++cases;
            if(arena.before!=UINT64_C(0xa5a5a5a5a5a5a5a5) || arena.after!=UINT64_C(0xa5a5a5a5a5a5a5a5) ||
               ot.before!=UINT64_C(0xa5a5a5a5a5a5a5a5) || ot.after!=UINT64_C(0xa5a5a5a5a5a5a5a5))valid=0;
            for(u8 *p=buffer.data;p<buffer.end;++p)if(*p!=0xa5){valid=0;break;}
        }
    }
#undef MODEL_PUT
    for(unsigned i=0;i<8;++i)if(!coverage[i])valid=0;
    /* A real native packet must fit completely; a bad depth cannot address an OT sentinel. */
    _Alignas(16) u8 small_bank[128]={0};s32 offset=4;memcpy(small_bank,&offset,4);
    FxCommonPolyResource *header=(FxCommonPolyResource *)(small_bank+4);header->counts[0]=1;
    for(unsigned i=0;i<8;++i)header->offsets[i]=52;
    FxCommonModelF3 *triangle=(FxCommonModelF3 *)(small_bank+56);
    triangle->color.word=0x20ffffff;
    triangle->vertices[0]=(GteShortVector){-10,-10,0,0};triangle->vertices[1]=(GteShortVector){10,-10,0,0};triangle->vertices[2]=(GteShortVector){-10,10,0,0};
    for(unsigned renderer=0;renderer<3;++renderer){
        memset(&arena,0xa5,sizeof(arena));memset(&ot,0xa5,sizeof(ot));
        FxCommonBuffer buffer={.data=arena.bytes,.allocation=(RenderNativeTag *)ot.entries,
            .end=arena.bytes+sizeof(FxCommonModelF3Packet)-1};g_FxCommonActiveBuffer=&buffer;
        draws[renderer](NULL,(FxCommonPolyModel *)small_bank,0);
        if(buffer.error!=-1 || buffer.data!=arena.bytes)valid=0;
        for(unsigned i=0;i<sizeof(arena);++i)if(((u8 *)&arena)[i]!=0xa5)valid=0;
        for(unsigned i=0;i<sizeof(ot);++i)if(((u8 *)&ot)[i]!=0xa5)valid=0;
    }
    for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
    const u32 controls[]={4096,0,4096,0,4096,0,0,1000};
    for(unsigned i=0;i<8;++i)Psyz_GteCtrlWrite(i,controls[i]);Psyz_GteCtrlWrite(26,768);
    Psyz_GteCtrlWrite(29,341);Psyz_GteCtrlWrite(30,256);
    for(unsigned i=0;i<2;++i){memset(&ot,0xa5,sizeof(ot));
        FxCommonBuffer buffer={.data=arena.bytes,.allocation=(RenderNativeTag *)ot.entries,.end=arena.bytes+sizeof(arena.bytes)};
        g_FxCommonActiveBuffer=&buffer;g_FxCommonDepthBias=i?-1:4096;
        FxCommon_DrawModelFlat(NULL,(FxCommonPolyModel *)small_bank,0);
        if(buffer.error!=-2 || buffer.data!=arena.bytes)valid=0;
        for(unsigned j=0;j<sizeof(ot);++j)if(((u8 *)&ot)[j]!=0xa5)valid=0;
    }
    g_FxCommonActiveBuffer=old_buffer;g_FxCommonDepthBias=old_bias;memcpy(g_FxCommonFlatColor,old_color,4);g_FxCommonRenderWorkspace=old_work;
    if(!valid || cases!=2040 || hash!=UINT64_C(0x22489a443c2917c3)){fprintf(stderr,"Original FX models: cases=%u packets=%u hash=%016llx valid=%d\n",cases,packets,(unsigned long long)hash,valid);return 0;}
    puts("Original FX models: all 170 day/night models, three original drawers, all eight polygon streams, four views, native packet payloads and complete ordering chains match USA CPU/GTE; arena/depth guards pass");return 1;
}

static int original_fx_resources(PeDisc *disc,PeDiscFile *exe)
{
    Pe1GameState saved=g_GameState;PeDisc *old_image=g_CdImage;PeDiscFile archive,old_archive;
    u16 saved_sectors[5],sectors[5];u8 old_night=g_FxCommonNight;
    memcpy(saved_sectors,g_PeImageSectorTable+76,sizeof(saved_sectors));
    if(!pe_disc_find(disc,"PE.IMG",&archive) ||
       !pe_disc_read(disc,exe,0x930d8-0x10000+0x800+76*2,sectors,sizeof(sectors)))return 0;
    if(old_image && !pe_disc_find(old_image,"PE.IMG",&old_archive))return 0;
    u8 *expected=malloc(FX_COMMON_RESOURCE_BYTES);if(!expected)return 0;
    int valid=1;unsigned cases=0;uint64_t hash=UINT64_C(14695981039346656037);
    memcpy(g_PeImageSectorTable+76,sectors,sizeof(sectors));fx_image_io=1;
    for(unsigned night=0;night<2 && valid;++night){
        CdRom_BindImage(disc,&archive);g_GameState.pe_image_base_lba=archive.sector;g_FxCommonNight=night;
        glyph_reads=glyph_polls=fx_read_failure=fx_poll_failure=0;
        memset(g_FxCommonTimTable,0xa5,FX_COMMON_RESOURCE_BYTES);memset(g_FxCommonModelTable,0xa5,FX_COMMON_RESOURCE_BYTES);
        if(FxCommon_LoadEffectSectors()!=0 || glyph_reads!=2 || glyph_polls!=2){valid=0;break;}
        for(unsigned bank=0;bank<2;++bank){unsigned index=bank?(night?1:2):0;
            size_t size=(sectors[index+1]-sectors[index])*2048u;
            const u8 *actual=(const u8 *)(bank?g_FxCommonModelTable:g_FxCommonTimTable);
            if(!pe_disc_read(disc,&archive,(uint64_t)sectors[index]*2048,expected,size) || memcmp(actual,expected,size)){valid=0;break;}
            for(size_t i=size;i<FX_COMMON_RESOURCE_BYTES;++i)if(actual[i]!=0xa5){valid=0;break;}
        }
    }
    if(valid){
        /* All 80 authored curves from the actual texture resource bank. */
        const u8 *bank=(const u8 *)g_FxCommonTimTable;u32 start,end;memcpy(&start,bank+8,4);memcpy(&end,bank+12,4);
        if(start>=end || end>41*2048u){valid=0;goto done;}
        const u8 *paths=bank+start;u32 first;memcpy(&first,paths,4);
        if(first!=80*4u){valid=0;goto done;}
        for(unsigned index=0;index<80 && valid;++index){u32 offset;s16 count;
            memcpy(&offset,paths+index*4,4);
            if(offset>end-start-8){valid=0;break;}memcpy(&count,paths+offset+6,2);
            if(count<=2 || offset+8+(u32)count*8>end-start){valid=0;break;}
            unsigned span=count-2;const unsigned fractions[]={0,1,127,255};
            for(unsigned seg=0;seg<span+3;++seg)for(unsigned f=0;f<4;++f)for(unsigned initial=0;initial<2;++initial){
                unsigned segment=seg==span+2?0xffffff:seg;FxCommonVec3 out;GteShortVector angles;
                memset(&out,0xa5,sizeof(out));memset(&angles,0xa5,sizeof(angles));g_FxCommonPathWrapped=initial;
                s32 result=FxCommon_SamplePath((segment<<8)|fractions[f],index,(void *)paths,&out,&angles);
                const void *parts[]={&out,&angles,&g_FxCommonPathWrapped,&result};unsigned sizes[]={12,8,1,4};
                for(unsigned part=0;part<4;++part)for(unsigned byte=0;byte<sizes[part];++byte)
                    hash=(hash^((const u8 *)parts[part])[byte])*UINT64_C(1099511628211);
                ++cases;
            }
        }
        if(cases!=80784 || hash!=UINT64_C(0x8fe143d65b03392d))valid=0;
    }
    for(unsigned fault=1;fault<=4;++fault){
        CdRom_BindImage(disc,&archive);glyph_reads=glyph_polls=0;
        fx_read_failure=fault==1?1:fault==3?2:0;fx_poll_failure=fault==2?1:fault==4?2:0;
        if(FxCommon_LoadEffectSectors()!=-(int)fault)valid=0;
    }
    fx_read_failure=fx_poll_failure=0;
    for(unsigned fault=0;fault<3;++fault){
        memcpy(g_PeImageSectorTable+76,sectors,sizeof(sectors));g_GameState.pe_image_base_lba=archive.sector;
        if(fault==0)g_PeImageSectorTable[77]=sectors[0];
        if(fault==1)g_PeImageSectorTable[77]=sectors[0]+129;
        if(fault==2)g_GameState.pe_image_base_lba=UINT32_MAX;
        glyph_reads=glyph_polls=0;
        if(FxCommon_LoadEffectSectors()!=-5 || glyph_reads || glyph_polls)valid=0;
    }
done:
    fx_image_io=0;fx_read_failure=fx_poll_failure=0;
    CdRom_BindImage(old_image,old_image?&old_archive:NULL);g_GameState=saved;g_FxCommonNight=old_night;
    memcpy(g_PeImageSectorTable+76,saved_sectors,sizeof(saved_sectors));free(expected);
    if(!valid){fprintf(stderr,"Original FX resources: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);return 0;}
    puts("Original FX resources: CUE/BIN day/night banks match every byte with intact buffer tails; four I/O failures and invalid extents propagate; all 80 paths match 80784 unhooked USA samples");return 1;
}

static int original_positional_audio_projection(void)
{
    Pe1GameState state=g_GameState;GeomScrollRecord scroll=g_GeomScroll;
    GteMatrix views[3]={{{{4096,0,0},{0,4096,0},{0,0,4096}}},
        {{{0,0,4096},{0,4096,0},{-4096,0,0}}},{{{4096,0,0},{0,4096,0},{0,0,4096}}}};
    views[2].t[0]=23;views[2].t[1]=-17;views[2].t[2]=250;
    static const s16 positions[9][3]={{0,0,0},{0,0,1},{0,0,6000},{-1000,-500,2000},{1000,500,2000},
        {-32768,32767,32767},{32767,-32768,-32768},{-1,-1,-1},{200,300,30000}};
    static const s16 offsets[3][2]={{160,112},{-512,256},{0,0}};
    static const u16 ranges[4][2]={{0,10000},{128,2048},{2048,4096},{0,65535}};
    static const u8 limits[3][2]={{0,127},{48,127},{127,0}};
    /* Exactly six addressable bytes: the GTE must receive a padded local copy. */
    AkaoPackedRect3 *position=malloc(sizeof(*position));if(!position)return 0;
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;int valid=1;
    for(unsigned view=0;view<3;++view)for(int projection=160;projection<=640;projection*=2)
    for(unsigned point=0;point<9;++point)for(unsigned offset=0;offset<3;++offset)
    for(unsigned extent=0;extent<4;++extent)for(unsigned limit=0;limit<3;++limit){
        for(unsigned i=0;i<32;++i){Psyz_GteDataWrite(i,0);Psyz_GteCtrlWrite(i,0);}
        g_ViewMatrixSlot.value=&views[view];g_GeomScroll.state.position.projectionDistance=&projection;
        g_GeomScroll.state.position.screenOffsetX=offsets[offset][0];g_GeomScroll.state.position.screenOffsetY=offsets[offset][1];
        g_GameState.bank_value_f8=ranges[extent][0];g_GameState.bank_value_fa=ranges[extent][1];
        g_GameState.bank_value_f6=limits[limit][0];g_GameState.bank_value_f7=limits[limit][1];
        position->x=positions[point][0];position->y=positions[point][1];position->z=positions[point][2];
        int pan=-1,volume=-1,result=Akao_Calc3DPan(position,&pan,&volume);
        u32 values[]={(u32)result,(u32)pan,(u32)volume,Psyz_GteCtrlRead(24),Psyz_GteCtrlRead(25)};
        for(unsigned i=0;i<5;++i)for(unsigned byte=0;byte<4;++byte)
            hash=(hash^(u8)(values[i]>>(byte*8)))*UINT64_C(1099511628211);
        if(pan<0 || pan>255 || volume<0 || volume>127)valid=0;++cases;
    }
    free(position);g_GameState=state;g_GeomScroll=scroll;
    if(!valid || hash!=UINT64_C(0xdd3f0ddcc154e8eb)){
        fprintf(stderr,"Original positional audio: cases=%u hash=%016llx valid=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    puts("Original positional audio: 2916 USA CPU/GTE-bridge cases, views/projection/clipping, negative pan, distance-volume wrapping and restored screen offsets match");return 1;
}

static int original_glint_lifecycle(void)
{
    Pe1GameState saved_state=g_GameState;GeomScrollRecord saved_scroll=g_GeomScroll;
    RenderEffectParameters saved_params=g_FieldFxSpriteParams;
    FieldFxObjectPrefix *saved_owner=g_FieldFxOwner;FieldFxTaskSlot *saved_task=g_FieldFxActiveTaskSlot;
    FieldFxTaskContext context={0},*saved_context=g_FieldFxTaskContext;g_FieldFxTaskContext=&context;
    BattleEntity *saved_entity=D_8009D254;GteShortVector saved_origin=g_MemcardGlintOrigin;
    u8 *saved_model=g_FieldFxGlintModel;void *saved_sound=g_AkaoBgmHandle[0];
    int saved_slot=g_ActiveDrawSlot,saved_offset=g_DrawPacketOffset,saved_age=g_FieldFxCurrentAge;
    int saved_alt=g_FieldFxAltPaletteFlag; s16 saved_page=g_FieldFxLoadedTexturePage;
    u16 saved_stop=g_BattleCrossFlashStop;s16 saved_raised=g_MenuCardCrossFlashRaised;
    u32 saved_colors[256];memcpy(saved_colors,g_FieldModelColorBackup,sizeof(saved_colors));
    u16 model_page=g_FieldFxModelTpage,model_clut=g_FieldFxModelClut,model_semi=g_FieldFxModelSemiTrans;
    s16 model_depth=g_FieldFxModelDepthBias;
    u8 tracks[168];memcpy(tracks,g_MenuCardDriftGlowColorTrack,40);memcpy(tracks+40,g_MenuCardRisingEmberColorTrack,48);
    memcpy(tracks+88,g_MenuCardGlintBurstColorTrack,40);memcpy(tracks+128,g_MenuCardSpinRingColorTrack,40);
    _Alignas(16) static u8 packets[0x40000+128];RenderNativeTag ordering[4096];
    _Alignas(void *) u8 pool[4096];FieldFxTaskSlot task={.end=(char *)pool};
    FieldFxObjectPrefix owner={0};BattleEntity actor={0};RenderMatrix matrices[27]={0};
    for(unsigned i=0;i<27;++i){matrices[i].rotation[0][0]=matrices[i].rotation[1][1]=matrices[i].rotation[2][2]=4096;
        matrices[i].translation[0]=(int)i*10-100;matrices[i].translation[2]=6000;}
    actor.renderObject.matrices=matrices;actor.renderObject.target_z=6000;owner.actor=(FieldActor *)&actor;
    g_FieldFxOwner=&owner;g_FieldFxActiveTaskSlot=&task;g_PlayerEntity=(FieldActor *)&actor;
    GteMatrix view={.m={{4096,0,0},{0,4096,0},{0,0,4096}}};int projection=320;
    g_ViewMatrixSlot.value=&view;g_GeomScroll.state.position.projectionDistance=&projection;
    g_GameState.bank_value_f8=0;g_GameState.bank_value_fa=10000;g_GameState.bank_value_f6=0;g_GameState.bank_value_f7=127;
    u32 sound[32]={0};g_AkaoBgmHandle[0]=sound;
    _Alignas(16) u8 model_bytes[64]={0};FieldTexturedModel *model=(void *)model_bytes;
    model->header.flatTris=1;model->header.colorOffset=52;model->header.colorCount=3;
    for(unsigned i=0;i<3;++i){model->header.tris[0].vertex[i]=28+i*8;model->header.tris[0].uv[i]=0x1000+i;
        FieldModelVertex *v=(void *)(model_bytes+28+i*8);v->x=i==0?-20:i==1?20:0;v->y=i==2?20:-20;v->color=52+i*4;
        u32 color=0x00302010+i;memcpy(model_bytes+52+i*4,&color,4);}
    g_FieldFxGlintModel=model_bytes;
    FieldFxCallbackListCallback controllers[]={MenuCard_RingBurstController,MenuCard_CrossFlashController,
        MenuCard_GlintBurstController,MenuCard_SpinBurstController};
    const int counts[]={50,36,22,40},ends[]={72,48,8,8};
    unsigned draws=0;int valid=1;
    for(unsigned controller=0;controller<4;++controller)for(unsigned slot=0;slot<2;++slot)for(unsigned alt=0;alt<2;++alt){
        union {MemcardRingBurst ring;MemcardCrossFlash cross;MemcardGlintBurst glint;MemcardSpinBurst spin;} state={0};
        memset(pool,0xa5,sizeof(pool));g_ActiveDrawSlot=slot;g_FieldFxAltPaletteFlag=alt;g_FieldFxLoadedTexturePage=1;
        g_FieldFxSpriteParams=(RenderEffectParameters){32,2,1,0,0,0,16,32,32};g_FieldFxCurrentAge=0;
        g_BattleCrossFlashStop=55;g_MenuCardCrossFlashRaised=0;
        int used=controllers[controller](0,&state,0);FieldFxCallbackList *list=(void *)pool;
        if(used<=0 || used>sizeof(pool) || list->count!=counts[controller]){valid=0;break;}
        for(unsigned i=used;i<sizeof(pool);++i)if(pool[i]!=0xa5)valid=0;
        if(controller==1 && (state.cross.attachA!=6 || state.cross.attachB!=19 || g_BattleCrossFlashStop))valid=0;
        int ended=0;unsigned peak=0;
        for(int frame=0;frame<=90;++frame){
            g_FieldFxCurrentAge=frame;g_DrawPacketOffset=0;memset(packets,0xa5,sizeof(packets));
            for(unsigned i=0;i<4096;++i)ordering[i]=(RenderNativeTag){.address=0xffffff};
            D_800B0E38.ordering[slot]=ordering;D_800B0E38.packets[slot]=(char *)packets;
            Psyz_GteCtrlWrite(26,320);Psyz_GteCtrlWrite(29,1365);Psyz_GteCtrlWrite(30,1024);gte_SetGeomOffset(160,120);
            if(frame && !ended){if(controller==1 && frame==48)g_BattleCrossFlashStop=1;
                int result=controllers[controller](1,&state,0);
                if(result){ended=frame;if(result!=(controller==0?1:2))valid=0;}}
            if(!ended || frame==ended){controllers[controller](2,&state,0);++draws;}
            int active=FieldFx_DrawCallbackList((char *)pool);if(active>peak)peak=active;
            if(g_FieldFxCurrentAge!=frame || g_DrawPacketOffset<0 || g_DrawPacketOffset>0x40000){valid=0;break;}
            /* The original model renderer writes tentative UVs before culling.
             * Its final scratch packet need not advance the committed cursor. */
            for(unsigned i=g_DrawPacketOffset+sizeof(FieldModelGt4Packet);i<sizeof(packets);++i)
                if(packets[i]!=0xa5){valid=0;break;}
            for(unsigned i=0;i<4096;++i){uintptr_t link=ordering[i].address;unsigned count=0;
                while(link!=0xffffff){
                    if(link<(uintptr_t)packets || link>(uintptr_t)packets+g_DrawPacketOffset-sizeof(RenderNativeTag) || ++count>4096){valid=0;break;}
                    RenderNativeTag *tag=(void *)link;
                    if(tag->length>16 || link+sizeof(*tag)+tag->length*4>(uintptr_t)packets+g_DrawPacketOffset){valid=0;break;}
                    link=tag->address;
                }
            }
            FieldFx_UpdateCallbackList(pool);
            if(frame==90 && active)valid=0;
        }
        if(ended!=ends[controller] || !peak)valid=0;
        if(!valid)fprintf(stderr,"glint lifecycle controller=%u slot=%u alt=%u end=%d peak=%u offset=%d\n",controller,slot,alt,ended,peak,g_DrawPacketOffset);
        if(controller==1 && g_MenuCardCrossFlashRaised!=1)valid=0;
    }
    {
    PmTables saved_pm=g_PmTables;u32 saved_flags=g_GameStateFlags;
    void *saved_programs[104];memcpy(saved_programs,g_FieldFxTaskPrograms,sizeof(saved_programs));
    static PmPrimarySlot bank[11];
    g_PmTables=(PmTables){g_PmCmdHandlerArray,bank,NULL};g_GameStateFlags=0x80;
    g_GameState.flags&=~8u;g_GameState.tim_load_state=0;
    const unsigned masks[]={1,1,3,7,7,7,1};unsigned programs_run=0,combined_frames=0;
    for(unsigned command=108;command<=114;++command){
        memset(bank,0,sizeof(bank));
        for(unsigned i=0;i<11;++i)memset(bank[i].task.tasks.arena,0xa5,sizeof(bank[i].task.tasks.arena));
        g_GameState.flags&=~0x10000u;g_FieldFxAltPaletteFlag=0;g_FieldFxLoadedTexturePage=1;
        g_FieldFxSpriteParams=(RenderEffectParameters){32,2,1,0,0,0,16,32,32};
        if(Scene_LoadRoomAssets(command,&actor)!=0 || Pm_SendCmd(0,0,0,0,0,0)){valid=0;break;}
        if(command==114)g_MenuCardCrossFlashRaised=1;
        unsigned seen=0,max_used=0,finished=0;int companion=-1;
        for(unsigned frame=0;frame<150;++frame){
            g_ActiveDrawSlot=frame&1;g_DrawPacketOffset=0;memset(packets,0xa5,sizeof(packets));
            for(unsigned i=0;i<4096;++i)ordering[i]=(RenderNativeTag){.address=0xffffff};
            D_800B0E38.ordering[g_ActiveDrawSlot]=ordering;D_800B0E38.packets[g_ActiveDrawSlot]=(char *)packets;
            Psyz_GteCtrlWrite(26,320);Psyz_GteCtrlWrite(29,1365);Psyz_GteCtrlWrite(30,1024);gte_SetGeomOffset(160,120);
            if(command==109 && frame==4){
                companion=Scene_LoadRoomAssets(114,&actor);
                if(companion!=1 || Pm_SendCmd(companion,0,0,0,0,0))valid=0;
            }
            if(command==109 && frame==48)g_BattleCrossFlashStop=1;
            if(Pm_Start(0) || (companion>=0 && Pm_Start(companion)) || Pm_Exec(0) ||
               (companion>=0 && Pm_Exec(companion))){valid=0;break;}
            FieldFxTaskContext *ctx=&bank[0].task.tasks;
            if(ctx->used>sizeof(ctx->arena) || g_DrawPacketOffset<0 || g_DrawPacketOffset>0x40000){valid=0;break;}
            if(ctx->used>max_used)max_used=ctx->used;
            for(unsigned i=0;i<8;++i)if(ctx->slots[i].id!=0xffff){
                FieldFxTaskSlot *slot=&ctx->slots[i];if(slot->id>2){valid=0;break;}
                seen|=1u<<slot->id;
                if(slot->start<ctx->arena || slot->start>=ctx->arena+sizeof(ctx->arena) ||
                    (slot->end && (slot->end<slot->start || slot->end>=ctx->arena+sizeof(ctx->arena))))valid=0;
            }
            for(unsigned i=max_used;i<sizeof(ctx->arena);++i)if((u8)ctx->arena[i]!=0xa5){valid=0;break;}
            for(unsigned i=g_DrawPacketOffset+sizeof(FieldModelGt4Packet);i<sizeof(packets);++i)
                if(packets[i]!=0xa5){valid=0;break;}
            for(unsigned i=0;i<4096;++i){uintptr_t link=ordering[i].address;unsigned count=0;
                while(link!=0xffffff){
                    if(link<(uintptr_t)packets || link>(uintptr_t)packets+g_DrawPacketOffset-sizeof(RenderNativeTag) || ++count>4096){valid=0;break;}
                    RenderNativeTag *tag=(void *)link;
                    if(tag->length>16 || link+sizeof(*tag)+tag->length*4>(uintptr_t)packets+g_DrawPacketOffset){valid=0;break;}
                    link=tag->address;
                }
            }
            if(command==109 && frame>=4 && frame<48){
                if(bank[1].header.state!=2 || !bank[1].task.tasks.count)valid=0;
                ++combined_frames;
            }
            if(!bank[0].header.state && (companion<0 || !bank[companion].header.state)){finished=frame;break;}
        }
        if(!finished || bank[0].task.tasks.count || seen!=masks[command-108] ||
           (command>=111 && command<=113 && max_used<=0x97c))valid=0;
        if(command==109 || command==114){
            if(g_GameState.flags&0x10000)valid=0;
            for(unsigned i=23;i<30;++i)if(g_FieldFxTaskPrograms[i])valid=0;
        }
        if(!valid)fprintf(stderr,"native task program %u: seen=%x arena=%u finish=%u count=%u state=%u\n",command,seen,max_used,finished,bank[0].task.tasks.count,bank[0].header.state);
        ++programs_run;
    }
    if(programs_run!=7 || combined_frames!=44)valid=0;
    memcpy(g_FieldFxTaskPrograms,saved_programs,sizeof(saved_programs));g_PmTables=saved_pm;g_GameStateFlags=saved_flags;
    }
    memcpy(g_MenuCardDriftGlowColorTrack,tracks,40);memcpy(g_MenuCardRisingEmberColorTrack,tracks+40,48);
    memcpy(g_MenuCardGlintBurstColorTrack,tracks+88,40);memcpy(g_MenuCardSpinRingColorTrack,tracks+128,40);
    memcpy(g_FieldModelColorBackup,saved_colors,sizeof(saved_colors));g_FieldFxGlintModel=saved_model;g_AkaoBgmHandle[0]=saved_sound;
    g_FieldFxModelTpage=model_page;g_FieldFxModelClut=model_clut;g_FieldFxModelSemiTrans=model_semi;g_FieldFxModelDepthBias=model_depth;
    g_GameState=saved_state;g_GeomScroll=saved_scroll;g_FieldFxSpriteParams=saved_params;
    g_FieldFxOwner=saved_owner;g_FieldFxActiveTaskSlot=saved_task;g_FieldFxTaskContext=saved_context;g_PlayerEntity=(FieldActor *)saved_entity;g_MemcardGlintOrigin=saved_origin;
    g_ActiveDrawSlot=saved_slot;g_DrawPacketOffset=saved_offset;g_FieldFxCurrentAge=saved_age;
    g_FieldFxAltPaletteFlag=saved_alt;g_FieldFxLoadedTexturePage=saved_page;g_BattleCrossFlashStop=saved_stop;g_MenuCardCrossFlashRaised=saved_raised;
    if(!valid){fprintf(stderr,"original glint controller/list/render lifecycle failed (%u draws)\n",draws);return 0;}
    puts("Original glint controllers/programs: all four controllers and seven real PM programs, overlapping tasks, cross-flash/114 handshake, complete lifetimes, both draw slots and native arena/OT guards passed");
    return 1;
}

static int original_task_bank_data(PeDisc *disc)
{
    PeDiscFile archive;static u8 bank[9*2048];
    if(!pe_disc_find(disc,"PE.IMG",&archive) || !pe_disc_read(disc,&archive,969ull*2048,bank,sizeof(bank)))return 0;
    FieldFxTaskProgram *programs[]={&g_FieldFxTaskProgramCmd108,&g_FieldFxTaskProgramCmd109,
        &g_FieldFxTaskProgramCmd110,&g_FieldFxTaskProgramCmd111,&g_FieldFxTaskProgramCmd112,
        &g_FieldFxTaskProgramCmd113,&g_FieldFxTaskProgramCmd114};
    const u32 addresses[]={0x801f1bd8,0x801f1c58,0x801f1d00,0x801f1d8c,0x801f1e18,0x801f1ea4,0x801f1ef0};
    static const struct {FieldFxCallbackListCallback callback;u32 address;} callbacks[]={
        {MenuCard_RingBurstController,0x801edc44},{MenuCard_CrossFlashController,0x801eebc8},
        {MenuCard_GlintBurstController,0x801ef978},{MenuCard_SpinBurstController,0x801f0690},
        {MenuCard_LinkEffect,0x801f0d30},{MenuCard_TrailEffect,0x801f11fc},
        {MenuCard_SineTrailEffect,0x801f1790},{MenuCard_ConsumeCrossFlashFlag,0x801f1b60}};
    intptr_t (*initializers[])(int,intptr_t,intptr_t,intptr_t)={MenuCard_GetGlintOriginA,MenuCard_GetGlintOriginB,
        MenuCard_GetGlintOriginC,MenuCard_GetGlintOriginD,MenuCard_GetGlintOriginE,MenuCard_GetGlintOriginF};
    const unsigned words[]={12,12,15,18,18,18,9};int valid=1;
    for(unsigned i=0;i<7;++i){
        const u8 *record=bank+(addresses[i]-0x801ed7f8u);FieldFxTaskProgram *program=programs[i];
        for(unsigned j=0;j<8;++j){u32 address=0,original;u16 size;
            if(program->table.callbacks[j]){for(unsigned k=0;k<8;++k)
                if(program->table.callbacks[j]==callbacks[k].callback)address=callbacks[k].address;
                if(!address)valid=0;}
            memcpy(&original,record+j*4,4);memcpy(&size,record+32+j*2,2);
            if(original!=address || size!=program->table.sizes[j])valid=0;
        }
        u32 initializer,script;memcpy(&initializer,record+48,4);memcpy(&script,record+52,4);
        if(initializer!=(i<6?0x801f1b00u+i*16:0) || program->initialize!=(i<6?initializers[i]:NULL))valid=0;
        if(program->initialize && program->initialize(-1,INTPTR_MAX,INTPTR_MIN,0)!=(intptr_t)&g_MemcardGlintOrigin)valid=0;
        if(script<0x801ed7f8u || script-0x801ed7f8u+words[i]*2>sizeof(bank) ||
            memcmp(program->script,bank+(script-0x801ed7f8u),words[i]*2))valid=0;
    }
    static const struct {const void *data;u32 address;size_t bytes;} authored[]={
        {&g_MenuCardLinkColor1,0x801ed850,4},{&g_MenuCardLinkColor2,0x801ed854,4},
        {g_MenuCardPulseColorTrack,0x801f1d5c,40},{g_MenuCardTrailColorTrack,0x801f1de8,48},
        {g_MenuCardSineTrailColorTrack,0x801f1e74,48},{&g_MenuCardLinkJointA,0x801f1d86,2},
        {&g_MenuCardLinkJointB,0x801f1d88,2}};
    for(unsigned i=0;i<sizeof(authored)/sizeof(*authored);++i)
        if(memcmp(authored[i].data,bank+(authored[i].address-0x801ed7f8u),authored[i].bytes))valid=0;
    FieldFxCallbackListCallback particles[]={MenuCard_PulseImage,MenuCard_CosineImage,MenuCard_SineRotatingImage};
    static const int timers[]={0,1,10,11,12,14,15,16,17,22,23,24,31,39,43,32767,32768,65535};
    static const s16 phases[]={-32768,-1,0,1,32767};static const int modes[]={-1,0,1,3,255};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;
    for(unsigned fn=0;fn<3;++fn)for(unsigned phase=0;phase<5;++phase)
    for(unsigned timer=0;timer<18;++timer)for(unsigned mode=0;mode<5;++mode){
        u16 words[10]={32767,32768,(u16)-10,0xa55a,(u16)-100,200,(u16)-300,0x5aa5,(u16)phases[phase],(u16)timers[timer]};
        MemcardPulseState particle;memcpy(&particle,words,20);
        int result=particles[fn](modes[mode],&particle,INTPTR_MAX);const u8 *bytes=(const u8 *)&particle;
        for(unsigned i=0;i<20;++i)hash=(hash^bytes[i])*UINT64_C(1099511628211);
        for(unsigned i=0;i<4;++i)hash=(hash^(u8)((u32)result>>(8*i)))*UINT64_C(1099511628211);
        ++cases;
    }
    if(cases!=1350 || hash!=UINT64_C(0x920c3087dc472d02)){
        fprintf(stderr,"Original image particles: cases=%u hash=%016llx\n",cases,(unsigned long long)hash);valid=0;}
    if(!valid)fprintf(stderr,"original native task descriptors/scripts/data differ from supplied USA bank\n");
    else puts("Original task bank: all seven descriptors, 56 callbacks/sizes, six initializer returns, 102 script halfwords and image data match USA; 1350 image-particle CPU cases match");
    return valid;
}

static int original_glint_particles(PeDisc *disc)
{
    static const int timers[]={0,1,14,15,16,17,22,23,24,30,31,38,39,43,44,32767,32768,65535};
    static const s16 poses[6][6]={{0,0,0,0,0,0},{-500,499,300,70,-80,33},
        {32767,-32768,32760,-32768,32767,-1},{-32768,32767,-32760,32767,-32768,1},
        {35,0,-35,-1,1,127},{-1,-1,-1,-127,-128,128}};
    FieldFxCallbackListCallback functions[]={MenuCard_DriftGlowParticle,MenuCard_RisingEmberParticle,
        MenuCard_FadingGlintParticle,MenuCard_SpinRingParticle};
    static const u32 seeds[]={1,0x87654321};static const int modes[]={0,1,3};
    RoomFloorY floor=*g_RoomFloorY;unsigned saved_random=Psyz_RandState();
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;int valid=1;
#define GLINT_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    for(unsigned fn=0;fn<4;++fn)for(int state=-1;state<4;++state)
    for(unsigned timer=0;timer<sizeof(timers)/sizeof(*timers);++timer)for(unsigned pose=0;pose<6;++pose)
    for(int height=-500;height<=500;height+=500)for(unsigned seed=0;seed<2;++seed)for(unsigned mode=0;mode<3;++mode){
        u16 words[10]={(u16)poses[pose][0],(u16)poses[pose][1],(u16)poses[pose][2],0xa55a,
            (u16)poses[pose][3],(u16)poses[pose][4],(u16)poses[pose][5],0x5aa5,(u16)state,(u16)timers[timer]};
        union {RoomDampedSpark spark;MemcardSpinParticle spin;} particle;
        memcpy(&particle,words,20);g_RoomFloorY->y=height;srand(seeds[seed]);
        int result=functions[fn](modes[mode],&particle,0);
        const u8 *bytes=(const u8 *)&particle;for(unsigned i=0;i<20;++i)GLINT_PUT(bytes[i],1);
        GLINT_PUT(result,4);
        /* Match the complete RNG state, including the three jitter draws. */
        u32 rng=seeds[seed];if(fn==1 && state==1 && modes[mode]==1)
            for(unsigned i=0;i<3;++i)rng=rng*UINT32_C(1103515245)+12345;
        GLINT_PUT(rng,4);
        u32 next=rng*UINT32_C(1103515245)+12345;if(rand()!=((next>>16)&32767))valid=0;
        ++cases;
    }
    *g_RoomFloorY=floor;srand(saved_random);
#undef GLINT_PUT
    if(cases!=38880 || hash!=UINT64_C(0x445fa6d24cc84f79))valid=0;
    PeDiscFile archive;static u8 bank[22*2048];
    if(!pe_disc_find(disc,"PE.IMG",&archive) || !pe_disc_read(disc,&archive,965ull*2048,bank,sizeof(bank)))return 0;
    static const struct {const void *data;u32 address;size_t bytes;} authored[]={
        {&g_MenuCardEffectSpinRotation,0x801ed7fc,8},{&g_MenuCardEffectTiltRotation,0x801ed804,8},
        {&g_MenuCardRingBurstGlowColor,0x801ed80c,4},{&g_MenuCardRingBurstRingColor,0x801ed810,4},
        {&g_MenuCardRingBurstBandColor,0x801ed814,4},{&g_MenuCardHandOffset,0x801ed818,8},
        {&g_MenuCardCrossFlashTipAOffset,0x801ed820,8},{&g_MenuCardCrossFlashTipBOffset,0x801ed828,8},
        {&g_MenuCardCrossFlashCenterOffset,0x801ed830,8},{&g_MenuCardCrossFlashInnerColor,0x801ed838,4},
        {&g_MenuCardCrossFlashOuterColor,0x801ed83c,4},{&g_MenuCardCrossFlashEdgeColor,0x801ed840,4},
        {&g_MenuCardGlintBurstFlashColor,0x801ed844,4},{&g_MenuCardSpinRingColorA,0x801ed848,4},
        {&g_MenuCardSpinRingColorB,0x801ed84c,4},{g_MenuCardDriftGlowColorTrack,0x801f1bb0,40},
        {g_MenuCardRisingEmberColorTrack,0x801f1c28,48},{g_MenuCardGlintBurstColorTrack,0x801f1cb0,40},
        {g_MenuCardSpinRingColorTrack,0x801f1cd8,40}};
    u16 state_bytes;memcpy(&state_bytes,bank+(0x801f1c78u-0x801eb7f8u),2);
    if(state_bytes!=sizeof(MemcardCrossFlash))valid=0;
    for(unsigned i=0;i<sizeof(authored)/sizeof(*authored);++i)
        if(memcmp(authored[i].data,bank+(authored[i].address-0x801eb7f8),authored[i].bytes))valid=0;
    if(!valid){fprintf(stderr,"Original glint particles: cases=%u hash=%016llx valid=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    puts("Original glint particles: 38880 real USA cases, four callbacks, all phases, signed/wrapping coordinates and timers, damping/bounce, jitter RNG and authored data match");
    return 1;
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
        for(const u8 *zero_=p_+4;zero_<end_;++zero_){ \
            u8 expected_byte_=0; \
            if(sizeof(uintptr_t)==8 && zero_>=end_-4) \
                expected_byte_=(u8)(UINT32_C(0x005a4d50)>>(8*(zero_-(end_-4)))); \
            if(*zero_!=expected_byte_)valid=0;} \
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
    FieldActor *saved_actor=g_CurrentEntity;ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    if(g_ScriptQueueOpcodeHandlers[111]!=Script_InitEntityMoveState)return 0;
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
            if(i&1){
                u32 program[]={111,0,32,0};ScriptNode node={.current.script=program,.active=1,.flags=0x80};
                g_CurrentEntity=(FieldActor *)&actors[i];g_ScriptNodePool=&node;Script_RunQueue();
                if(node.current.script!=program+4 || node.flags!=0x90 || node.active || g_ScriptNodePool)return 0;
            }else Entity_AllocSlot(&actors[i]);
            HASH_SLOT_ACTOR(i);
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
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    g_GameState.bank_rows[0][2]=old_animation;
    if(hash!=UINT64_C(0x81c2f5af25011535)) {
        fprintf(stderr,"Original enemy-slot hash %016llx differs from USA MIPS\n",(unsigned long long)hash);return 0;
    }
    puts("Original enemy slots: complete template and 5,632 allocations, opcode 111 dispatcher, exhaustion, reuse, ID wrapping and clears match unhooked USA MIPS");
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

static struct {s16 x,y;u8 character;u16 style;} credit_glyphs[4096];
static unsigned credit_glyph_count;
static void credit_record_glyph(s16 x,s16 y,u8 character)
{
    if(credit_glyph_count<4096){credit_glyphs[credit_glyph_count].x=x;credit_glyphs[credit_glyph_count].y=y;
        credit_glyphs[credit_glyph_count].character=character;credit_glyphs[credit_glyph_count].style=g_BootDisplayFontStyle;}
    ++credit_glyph_count;
}
static int original_credit_text(PeDisc *disc)
{
    g_CdImage=disc;glyph_pending=glyph_poll_retry=0;fx_image_io=1;
    if(Asset_OpenPeImage() || CdRom_LoadBootAudio())return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
    if(memcmp(g_BootDisplayUppercaseWidths,g_CdLoadAreaBase+0x4f20,104))return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
    for(unsigned i=0;i<745;++i){u32 pointer;memcpy(&pointer,g_CdLoadAreaBase+0x4f88+4*i,4);
        if(g_BootDisplayCreditLines[i]!=g_CdLoadAreaBase+(pointer-0x80120d00u))return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;}
    for(unsigned i=0;i<31;++i){u32 pointer;memcpy(&pointer,g_CdLoadAreaBase+0x4ea4+4*i,4);
        if(g_BootDisplayCreditImages[i]!=g_CdLoadAreaBase+(pointer-0x80120d00u))return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;}
    PeDiscFile file;
    char *authored_path=g_BootDisplayPlaybackFilePath;
    if(!pe_disc_find(disc,authored_path,&file)){
        if(BootDisplay_InitPlaybackFile()!=-1)return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
        puts("Credits XA absent on supplied Disc 1: missing-file failure confirmed; testing extent setup with the real FMV000 stream");
        g_BootDisplayPlaybackFilePath="FMV1/FMV000.STR";
        if(!pe_disc_find(disc,g_BootDisplayPlaybackFilePath,&file))return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
    }
    g_BootDisplayPlaybackRingIndex=g_BootDisplayPlaybackAltCount=5;g_BootDisplayPlaybackReadCount=123;
    if(BootDisplay_InitPlaybackFile()!=0 || g_BootDisplayPlaybackStartPos!=(s32)file.sector ||
       g_BootDisplayPlaybackEndPos!=(s32)(file.sector+(file.size>>11)) || g_BootDisplayPlaybackCurrentPos!=(s32)file.sector ||
       g_BootDisplayPlaybackRingIndex || g_BootDisplayPlaybackAltCount || g_BootDisplayPlaybackReadCount ||
       g_BootDisplayPlaybackSectorBuffer!=(char *)g_CdLoadAreaBase+((g_MainAssetLbaTbl[0]-g_BootDisplayDataLbaTbl)*2048+0x34800))return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
    g_BootDisplayPlaybackFilePath=authored_path;
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0,glyphs=0;
#define CREDIT_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value); \
    for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
    const u16 styles[]={0,1,2,3,4,65535};
    for(unsigned style=0;style<6;++style){g_BootDisplayFontStyle=styles[style];
        for(unsigned character=0;character<256;++character){CREDIT_PUT(BootDisplay_GetGlyphIndex(character),1);++cases;}}
    for(unsigned style=0;style<4;++style){g_BootDisplayFontStyle=style;
        for(unsigned character=0;character<256;++character){CREDIT_PUT(BootDisplay_GetGlyphWidth(character),2);++cases;}}
    if(cases!=2560 || hash!=UINT64_C(0x6fa8e01cd1156db8)){fprintf(stderr,"Credits glyphs: %u %016llx\n",cases,(unsigned long long)hash);return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;}
    BootDisplayCallbacks callbacks={.glyph=credit_record_glyph};g_BootDisplayCallbacks=&callbacks;
    const s32 steps[]={-256,-0x123,0,256,INT32_MAX};const int first_y[]={225,224,223,0,-7,-8,-223};
    hash=UINT64_C(14695981039346656037);cases=0;
    for(unsigned style=0;style<4;++style)for(unsigned position=0;position<132;++position)
    for(unsigned fraction=0;fraction<2;++fraction)for(unsigned step=0;step<5;++step){
        int y=position<7?first_y[position]:224-(int)(position-7)*6*18;
        g_BootDisplayFontStyle=style;g_BootDisplayTextScrollX=(s32)(((u32)y<<8)+(fraction?255:0));
        g_BootDisplayTextScrollStepX=steps[step];credit_glyph_count=0;
        int result=BootDisplay_DrawTextBlock();if(credit_glyph_count>4096)return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
        CREDIT_PUT(credit_glyph_count,2);
        for(unsigned i=0;i<credit_glyph_count;++i){CREDIT_PUT(credit_glyphs[i].x,2);CREDIT_PUT(credit_glyphs[i].y,2);
            CREDIT_PUT(credit_glyphs[i].character,1);CREDIT_PUT(credit_glyphs[i].style,2);}
        CREDIT_PUT(result,1);CREDIT_PUT(g_BootDisplayTextScrollX,4);CREDIT_PUT(g_BootDisplayFontStyle,2);
        CREDIT_PUT(g_BootDisplayTextCursor-g_BootDisplayCreditLines,2);
        glyphs+=credit_glyph_count;++cases;
    }
#undef CREDIT_PUT
    g_BootDisplayCallbacks=NULL;
    if(cases!=5280 || glyphs!=656800 || hash!=UINT64_C(0x8bd818222def553d)){fprintf(stderr,"Credits layout: %u glyphs=%u %016llx\n",cases,glyphs,(unsigned long long)hash);return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;}
    for(unsigned style=4;style<8;++style){g_BootDisplayFontStyle=style;if(BootDisplay_GetGlyphWidth('A')!=0)return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;}
    for(unsigned fault=1;fault<=10;++fault){
        boot_credit_corruption=fault;
        int status=CdRom_LoadBootAudio();
        if(status!=-4 || boot_credit_corruption || g_BootDisplayPlaybackFilePath){fprintf(stderr,"Credits metadata fault=%u status=%d pending=%u\n",fault,status,boot_credit_corruption);return 0;}
        for(unsigned i=0;i<745;++i)if(g_BootDisplayCreditLines[i])return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
        for(unsigned i=0;i<31;++i)if(g_BootDisplayCreditImages[i])return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
    }
    if(CdRom_LoadBootAudio()!=0)return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
    authored_path=g_BootDisplayPlaybackFilePath;
    g_BootDisplayPlaybackFilePath="FMV1/FMV000.STR";
    if(BootDisplay_InitPlaybackFile()!=0)return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
    s32 start=g_BootDisplayPlaybackStartPos,end=g_BootDisplayPlaybackEndPos;
    char *buffer=g_BootDisplayPlaybackSectorBuffer;
    u8 *workspace=g_CdLoadAreaBase;
    for(unsigned fault=0;fault<5;++fault){
        if(fault==0)g_CdImage=NULL;
        if(fault==1)g_BootDisplayPlaybackFilePath="missing.xa";
        if(fault==2)g_BootDisplayPlaybackFilePath="FMV1";
        if(fault==3)g_CdLoadAreaBase=NULL;
        if(fault==4)g_PeImageSectorTable[72]=0;
        if(BootDisplay_InitPlaybackFile()!=(fault<2?-1:-2) || g_BootDisplayPlaybackStartPos!=start ||
           g_BootDisplayPlaybackEndPos!=end || g_BootDisplayPlaybackSectorBuffer!=buffer)return fprintf(stderr,"Credits failure at line %d\n",__LINE__),0;
        g_CdImage=disc;g_BootDisplayPlaybackFilePath="FMV1/FMV000.STR";g_CdLoadAreaBase=workspace;g_PeImageSectorTable[72]=1276;
    }
    g_BootDisplayPlaybackFilePath=authored_path;
    puts("Original credits text: 745 native line bindings, 31 image bindings, image-file extent setup, 2560 USA glyph cases and 5280 layouts/656800 positioned glyphs match");return 1;
}

static int original_credit_sprite_packets(void)
{
    static struct {u64 before;BootDisplaySpritePacket packet;u64 after;} arena;
    static RenderNativeTag ot[16];
    const s16 poses[][2]={{-32768,32767},{-1,-1},{0,0},{320,224},{32767,-32768},{123,-456}};
    const u16 uv[]={0,1,255,256,65535,0x1234};
    const s32 brightness[]={INT32_MIN,-129,-4,-1,0,1,4,124,127,128,129,255,INT32_MAX};
    const u16 timers[]={0,1,2,3,28,29,30,65535};const u32 frames[]={0,1,356,357,358,449,450,UINT32_MAX};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;int valid=1;
#define SPRITE_PUT(value,bytes) do {uint64_t v_=(uint64_t)(value);for(unsigned b_=0;b_<(bytes);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
#define SPRITE_RESET() do {memset(&arena,0xa5,sizeof(arena));for(unsigned o_=0;o_<16;++o_)ot[o_]=(RenderNativeTag){0xffffff,0}; \
    g_BootDisplayOrderingTable=ot;g_BootDisplayPrimitiveCursor=(char *)&arena.packet;g_BootDisplayPrimitiveEnd=(char *)(&arena.packet+1); \
    g_BootDisplayTPage=0x4567;g_BootDisplayClut=0x1234;g_BootDisplayState=5;g_BootDisplayDone=0;}while(0)
#define SPRITE_PACKET() do {SPRITE_PUT((u32)arena.packet.draw_mode.code,4); \
    const u8 *p_=&arena.packet.sprite.r0;for(unsigned b_=0;b_<16;++b_)SPRITE_PUT(p_[b_],1); \
    if(arena.packet.draw_mode.tag.length!=(sizeof(BootDisplaySpritePacket)-sizeof(RenderNativeTag))/4 || \
       (arena.packet.sprite.tag.address & UINT64_C(0xff000000ff000000)) || (arena.packet.sprite.tag.length & UINT64_C(0xff000000ff000000)) || arena.before!=UINT64_C(0xa5a5a5a5a5a5a5a5) || arena.after!=UINT64_C(0xa5a5a5a5a5a5a5a5))valid=0;++cases;}while(0)
#define SPRITE_LINK(depth) do {if(ot[depth].address!=(uintptr_t)&arena.packet || arena.packet.draw_mode.tag.address!=0xffffff || \
    g_BootDisplayPrimitiveCursor!=(char *)(&arena.packet+1))valid=0;for(unsigned o_=0;o_<16;++o_)if(ot[o_].length || (o_!=(depth) && ot[o_].address!=0xffffff))valid=0;}while(0)
    for(unsigned pose=0;pose<6;++pose)for(unsigned i=0;i<6;++i){SPRITE_RESET();BootDisplay_InitSprite16(&arena.packet,poses[pose][0],poses[pose][1],uv[i],uv[(i*5+1)%6]);SPRITE_PACKET();}
    for(unsigned style=0;style<4;++style)for(unsigned pose=0;pose<6;++pose)for(unsigned character=0;character<256;++character){
        SPRITE_RESET();g_BootDisplayFontStyle=style;BootDisplay_QueueTintedSprite(poses[pose][0],poses[pose][1],character);SPRITE_PACKET();SPRITE_LINK(11);}
    for(unsigned color=0;color<256;++color){SPRITE_RESET();BootDisplay_QueueFadeSprite(color);SPRITE_PACKET();SPRITE_LINK(11);}
    if(!valid || cases!=6436 || hash!=UINT64_C(0x0de15ffbec1c8bad)){fprintf(stderr,"Credits sprites: cases=%u hash=%016llx valid=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    for(unsigned state=5;state<9;++state)for(unsigned transition=0;transition<4;++transition)
    for(unsigned timer=0;timer<8;++timer)for(unsigned frame=0;frame<8;++frame)for(unsigned slot=0;slot<2;++slot)for(unsigned value=0;value<13;++value){
        SPRITE_RESET();g_BootDisplayState=state;g_BootDisplayTransition=transition;g_BootDisplayStateTimer=timers[timer];
        g_BootDisplayFrameCounter=frames[frame];g_BootDisplayBufferIndex=slot;g_BootDisplayImageBrightness=brightness[value];
        BootDisplay_StepImageSprite();SPRITE_PACKET();SPRITE_LINK(14);
        SPRITE_PUT(g_BootDisplayState,4);SPRITE_PUT(g_BootDisplayTransition,4);SPRITE_PUT(g_BootDisplayImageBrightness,4);
        SPRITE_PUT(g_BootDisplayStateTimer,2);SPRITE_PUT(g_BootDisplayBufferIndex,4);
    }
#undef SPRITE_PUT
#undef SPRITE_PACKET
#undef SPRITE_LINK
    if(!valid || cases!=33060 || hash!=UINT64_C(0xf412ed31c8597b85)){fprintf(stderr,"Credits sprite state: cases=%u hash=%016llx valid=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    for(unsigned kind=0;kind<3;++kind)for(size_t remaining=0;remaining<sizeof(BootDisplaySpritePacket);++remaining){
        SPRITE_RESET();g_BootDisplayPrimitiveEnd=g_BootDisplayPrimitiveCursor+remaining;BootDisplaySpritePacket previous=arena.packet;
        if(kind==0)BootDisplay_QueueTintedSprite(1,2,'A');else if(kind==1)BootDisplay_QueueFadeSprite(128);else BootDisplay_StepImageSprite();
        if(g_BootDisplayState!=-1 || g_BootDisplayDone!=-1 || memcmp(&previous,&arena.packet,sizeof(previous)) ||
           g_BootDisplayPrimitiveCursor!=(char *)&arena.packet)return 0;
        for(unsigned i=0;i<16;++i)if(ot[i].address!=0xffffff || ot[i].length)return 0;
    }
#undef SPRITE_RESET
    puts("Original credits sprite packets: 33060 USA constructor/glyph/fade/image states match semantic payloads, native combined packet headers, full OT links and 168 atomic arena failures");return 1;
}

static unsigned credits_vblanks,credits_pcm_frames,credits_pcm_nonzero;
static u32 credits_seen_images,credits_probe_frame;
static unsigned credits_pixel_probes,credits_visible_pixels;
static int credits_force_bad_buffer;
static void credits_drive_audio(void)
{
    s16 samples[735*2];
    ++credits_vblanks;
    /* Disc 1 has no CREDITS.XA: this transport fixture uses FMV000's
     * channel 1, including when exercising the text-only callback table. */
    if(credits_vblanks==1 && !(g_GameStateFlags&0x20000))BootDisplay_SetCdFilter(1);
    size_t count=Psyz_CdPullSamples(samples,735);
    credits_pcm_frames+=(unsigned)count;
    for(size_t i=0;i<count*2;++i)if(samples[i])++credits_pcm_nonzero;
    g_PadPackets[0].header=0x4000;
    g_PadPackets[0].buttons=g_BootDisplayPlaybackEnded?0xfff7:0xffff;
    if((g_GameStateFlags&0x20000) && (g_BootDisplayState==5 || g_BootDisplayState==7) &&
       (unsigned)g_BootDisplayStateTimer<30)credits_seen_images|=1u<<g_BootDisplayStateTimer;
    if(g_BootDisplayFrameCounter%450==225 && credits_probe_frame!=g_BootDisplayFrameCounter){
        static _Alignas(8) u16 pixels[1024*512];RECT all={0,0,1024,512};
        StoreImage(&all,pixels);DrawSync(0);++credits_pixel_probes;
        for(unsigned y=0;y<448;++y)for(unsigned x=0;x<320;++x)
            if(pixels[y*1024+x]&0x7fff)++credits_visible_pixels;
        credits_probe_frame=g_BootDisplayFrameCounter;
    }
    if(credits_force_bad_buffer && credits_vblanks==1)g_BootDisplayBufferIndex=2;
    if(credits_vblanks>40000)g_BootDisplayState=g_BootDisplayDone=-1;
}

static int original_scene_texture_pages(PeDisc *disc)
{
    PeDiscFile archive;g_CdImage=disc;fx_image_io=1;
    fx_read_failure=fx_poll_failure=0;glyph_pending=glyph_poll_retry=0;
    if(Asset_OpenPeImage() || !pe_disc_find(disc,"PE.IMG",&archive) || !SDL_Init(SDL_INIT_EVENTS))return 0;
    ResetGraph(0);Boot_InitMemoryLayout();
    static _Alignas(8) u16 expected[1024*512],actual[1024*512];
    static _Alignas(16) u8 source[0x20000];
    RECT all={0,0,1024,512};
#define PAGE_CHECK(test) do {if(!(test)){fprintf(stderr,"Texture page failure at line %d\n",__LINE__);return 0;}}while(0)
    for(unsigned page=2;page<24;++page){
        unsigned index=43+page,bytes=(g_PeImageSectorTable[index+1]-g_PeImageSectorTable[index])*2048;
        PAGE_CHECK(bytes<=sizeof(source) && pe_disc_read(disc,&archive,(u64)g_PeImageSectorTable[index]*2048,source,bytes));
        for(unsigned i=0;i<1024*512;++i)expected[i]=0x1357;
        LoadImage(&all,expected);DrawSync(0);
        unsigned flags=source[4],offset=8;
        for(unsigned block=0;block<((flags&8)?2u:1u);++block){
            u32 length;RECT rect;memcpy(&length,source+offset,4);memcpy(&rect,source+offset+4,sizeof(rect));
            PAGE_CHECK(rect.x>=0 && rect.y>=0 && rect.w>0 && rect.h>0 && rect.x+rect.w<=1024 && rect.y+rect.h<=512 &&
                offset+length<=bytes && (unsigned)rect.w*rect.h*2<=length-12);
            for(unsigned y=0;y<(unsigned)rect.h;++y)for(unsigned x=0;x<(unsigned)rect.w;++x){
                unsigned pixel=offset+12+(y*rect.w+x)*2;
                expected[(rect.y+y)*1024+rect.x+x]=source[pixel]|(u16)source[pixel+1]<<8;
            }
            offset+=length;
        }
        g_GameState.room_type=page;g_LoadedTexturePageId=-1;glyph_reads=glyph_polls=0;
        PAGE_CHECK(Scene_StreamTexturePage()==0 && g_LoadedTexturePageId==(int)page && glyph_reads==1 && glyph_polls==1);
        PAGE_CHECK(memcmp(g_GameState.scene_load_scratch,source,bytes)==0);
        DrawSync(0);StoreImage(&all,actual);DrawSync(0);PAGE_CHECK(memcmp(expected,actual,sizeof(actual))==0);
        u32 marker=0x2222;RECT corner={1023,511,1,1};LoadImage(&corner,&marker);DrawSync(0);
        expected[511*1024+1023]=0x2222;
        PAGE_CHECK(Scene_StreamTexturePage()==0 && glyph_reads==1 && glyph_polls==1);
        StoreImage(&all,actual);DrawSync(0);PAGE_CHECK(memcmp(expected,actual,sizeof(actual))==0);
    }
    u32 base=g_GameState.pe_image_base_lba;u8 *scratch=g_GameState.scene_load_scratch;
    u16 first=g_PeImageSectorTable[45],last=g_PeImageSectorTable[46];
    for(unsigned fault=0;fault<18;++fault){
        CdRom_BindImage(disc,&archive);g_GameState.room_type=2;g_LoadedTexturePageId=23;
        g_GameState.scene_load_scratch=scratch;g_GameState.pe_image_base_lba=base;
        g_PeImageSectorTable[46]=last;fx_read_failure=fx_poll_failure=texture_page_corruption=0;
        glyph_reads=glyph_polls=0;int wanted=-3;
        if(fault==0)g_GameState.room_type=127;
        if(fault==1)g_GameState.room_type=200;
        if(fault==2)g_GameState.scene_load_scratch=NULL;
        if(fault==3)g_GameState.scene_load_scratch=scratch+1;
        if(fault==4)g_PeImageSectorTable[46]=first;
        if(fault==5)g_PeImageSectorTable[46]=first+65;
        if(fault==6)g_GameState.pe_image_base_lba=UINT32_MAX;
        if(fault==7){fx_read_failure=1;wanted=-1;}
        if(fault==8){fx_poll_failure=1;wanted=-2;}
        if(fault>=9 && fault<17){texture_page_corruption=fault-8;wanted=-4;}
        if(fault==17){g_PeImageSectorTable[46]=first+1;wanted=-4;}
        for(unsigned i=0;i<1024*512;++i)expected[i]=0x1357;
        LoadImage(&all,expected);DrawSync(0);
        PAGE_CHECK(Scene_StreamTexturePage()==wanted && g_LoadedTexturePageId==23);
        StoreImage(&all,actual);DrawSync(0);PAGE_CHECK(memcmp(expected,actual,sizeof(actual))==0);
    }
    CdRom_BindImage(disc,&archive);g_GameState.scene_load_scratch=scratch;g_GameState.pe_image_base_lba=base;
    g_PeImageSectorTable[46]=last;fx_read_failure=fx_poll_failure=texture_page_corruption=0;
    PAGE_CHECK(Scene_StreamTexturePage()==0 && g_LoadedTexturePageId==2);
#undef PAGE_CHECK
    puts("Original shared texture pages: all 22 USA TIM banks, full VRAM, cache hits, 18 atomic failures and recovery passed");
    return 1;
}

static int original_credit_playback(PeDisc *disc)
{
    g_CdImage=disc;glyph_pending=glyph_poll_retry=0;fx_image_io=1;
    if(Asset_OpenPeImage() || CdRom_LoadBootAudio())return 0;
    if(!SDL_Init(SDL_INIT_EVENTS))return 0;
    Boot_InitSystem();Boot_InitMemoryLayout();
    g_GameStateFlags=0x10000;
    int status=BootDisplay_RunDisplay();
    printf("Original credits full image flow: status=%d frame=%u timer=%d state=%d done=%d flags=%08x\n",
        status,g_BootDisplayFrameCounter,g_BootDisplayStateTimer,g_BootDisplayState,g_BootDisplayDone,g_GameStateFlags);
    if(status || g_BootDisplayDone!=1 || g_BootDisplayPlaybackActive ||
       (g_GameStateFlags&0x10000) || g_BootDisplayFrameCounter!=14028 ||
       g_BootDisplayStateTimer!=31 || g_BootDisplayState!=7)return 0;
    const char *authored=g_BootDisplayPlaybackFilePath;
    g_BootDisplayPlaybackFilePath="FMV1/FMV000.STR";
    if(BootDisplay_InitPlaybackFile())return 0;
    /* Compare real XA PCM from the CUE backend with the original module's
     * mounted-image callback. Both deliberately use the same PsyZ decoder. */
    enum {frames=88200};static s16 expected[frames*2],actual[frames*2];
    if(Psyz_CdSetDiskPath(getenv("PE_TEST_DISC")))return 0;
    u8 mode=200,filter[2]={1,1};CdlLOC position;
    CdControlB(14,&mode,NULL);CdControlB(13,filter,NULL);Psyz_AudioPause();
    CdlATV mix={128,0,128,0};CdMix(&mix);
    CdIntToPos(g_BootDisplayPlaybackStartPos,&position);CdControlB(2,(u8 *)&position,NULL);CdControlB(27,NULL,NULL);
    size_t reference=Psyz_CdPullSamples(expected,frames);
    CdControlB(8,NULL,NULL);Psyz_CdSetDiskPath(NULL);
    int start_status=BootDisplay_StartPlayback();
    if(start_status){fprintf(stderr,"Credits playback start failed: %d\n",start_status);return 0;}
    if(BootDisplay_SetCdFilter(1))return 0;
    size_t observed=Psyz_CdPullSamples(actual,frames);unsigned nonzero=0;
    for(size_t i=0;i<observed*2;++i)if(actual[i])++nonzero;
    if(reference!=frames || observed!=reference || !nonzero || memcmp(expected,actual,observed*4) ||
       BootDisplay_WaitForCdAndRestoreCallbacks() || g_BootDisplayPlaybackActive){
        fprintf(stderr,"Credits PCM mismatch: reference=%zu observed=%zu nonzero=%u compare=%d active=%d\n",
            reference,observed,nonzero,memcmp(expected,actual,observed*4),g_BootDisplayPlaybackActive);return 0;}
    printf("Original credits XA transport: %zu real FMV stereo frames match CUE PCM (%u nonzero samples)\n",observed,nonzero);
    s32 start=g_BootDisplayPlaybackStartPos,end=g_BootDisplayPlaybackEndPos;PeDiscFile file=g_BootDisplayPlaybackFile;
    for(unsigned fault=0;fault<4;++fault){
        u32 sector_size=disc->sector_size;char *buffer=g_BootDisplayPlaybackSectorBuffer;
        if(fault==0)g_CdImage=NULL;
        if(fault==1)disc->sector_size=2048;
        if(fault==2)g_BootDisplayPlaybackSectorBuffer=NULL;
        if(fault==3)g_BootDisplayPlaybackEndPos=start;
        if(BootDisplay_StartPlayback()!=-1 || g_BootDisplayPlaybackActive)return 0;
        g_CdImage=disc;disc->sector_size=sector_size;g_BootDisplayPlaybackSectorBuffer=buffer;g_BootDisplayPlaybackEndPos=end;
    }
    for(unsigned fault=0;fault<2;++fault){
        g_BootDisplayState=g_BootDisplayDone=0;
        g_BootDisplayPlaybackEndPos=start+3;g_BootDisplayPlaybackFile.size=3*2048;
        if(BootDisplay_StartPlayback())return 0;
        u64 bytes=disc->image_size;if(fault)disc->image_size=0;
        Psyz_CdPullSamples(actual,frames);disc->image_size=bytes;
        for(unsigned i=0;i<32;++i)VSync(0);
        if(!g_BootDisplayPlaybackEnded || (fault && (g_BootDisplayState!=-1 || g_BootDisplayDone!=-1)) ||
           (!fault && g_BootDisplayPlaybackCurrentPos!=start+3) || BootDisplay_WaitForCdAndRestoreCallbacks())return 0;
        g_BootDisplayPlaybackFile=file;g_BootDisplayPlaybackEndPos=end;
    }
    if(BootDisplay_SetCdFilter(-1)!=-1 || BootDisplay_SetCdFilter(256)!=-1)return 0;
    for(unsigned images=0;images<2;++images){
        if(CdRom_LoadBootAudio())return 0;
        g_BootDisplayPlaybackFilePath="FMV1/FMV000.STR";
        Boot_BuildRenderFlagTable();g_GameStateFlags=images?0x20000:0;
        credits_vblanks=credits_pcm_frames=credits_pcm_nonzero=0;
        credits_seen_images=credits_pixel_probes=credits_visible_pixels=0;credits_probe_frame=UINT32_MAX;
        VSyncCallbacks(0,credits_drive_audio);
        status=BootDisplay_RunDisplay();VSyncCallbacks(0,NULL);
        printf("Original credits full sprite flow: images=%u status=%d frame=%u timer=%d state=%d done=%d vblanks=%u pcm=%u nonzero=%u seen=%08x probes=%u pixels=%u\n",
            images,status,g_BootDisplayFrameCounter,g_BootDisplayStateTimer,g_BootDisplayState,g_BootDisplayDone,
            credits_vblanks,credits_pcm_frames,credits_pcm_nonzero,credits_seen_images,credits_pixel_probes,credits_visible_pixels);
        if(status || g_BootDisplayDone!=1 || g_BootDisplayPlaybackActive || !g_BootDisplayPlaybackEnded ||
           !credits_pcm_nonzero || g_BootDisplayFrameCounter!=(images?14157:14028) ||
           credits_pixel_probes<10 || credits_visible_pixels<1000 ||
           (images && (g_BootDisplayStateTimer!=30 || g_BootDisplayState!=7 || credits_seen_images!=0x3fffffffu)))return 0;
    }
    g_BootDisplayPlaybackFilePath="missing-credits.xa";g_GameStateFlags=0;
    if(BootDisplay_RunDisplay()!=-2 || g_BootDisplayPlaybackActive)return 0;
    u8 *workspace=g_CdLoadAreaBase;g_CdLoadAreaBase=NULL;
    if(BootDisplay_RunDisplay()!=-1 || g_BootDisplayPlaybackActive)return 0;
    g_CdLoadAreaBase=workspace;
    g_BootDisplayPlaybackFilePath="FMV1/FMV000.STR";g_GameStateFlags=0x20000;
    credits_force_bad_buffer=1;credits_vblanks=0;VSyncCallbacks(0,credits_drive_audio);
    status=BootDisplay_RunDisplay();VSyncCallbacks(0,NULL);credits_force_bad_buffer=0;
    if(status!=-3 || g_BootDisplayPlaybackActive)return 0;
    g_BootDisplayPlaybackFilePath=(char *)authored;
    puts("Original credits playback: four invalid starts, bounded EOF, read failure, cleanup and sprite-mode reentry passed");
    return 1;
}

static int original_credit_sprites(PeDisc *disc)
{
    g_CdImage=disc;glyph_pending=glyph_poll_retry=0;fx_image_io=1;
    if(Asset_OpenPeImage() || CdRom_LoadBootAudio())return 0;
    if(!SDL_Init(SDL_INIT_EVENTS))return 0;ResetGraph(0);Boot_InitMemoryLayout();
    BootDisplay_InitDisplayResources();if(!original_credit_sprite_packets())return 0;
    const u32 sprite_addresses[]={0x80123e08,0x80123f8c,0x8012403c,0x80124598,0x80124704,0x801247f4,0x80124034};
    const u32 image_addresses[]={0x8012489c,0,0x80125094,0x80124c04,0x80125464,0x80124aa4,0x80125444};
    if(memcmp(g_CdLoadAreaBase+0x4e4c,sprite_addresses,sizeof(sprite_addresses)) ||
       memcmp(g_CdLoadAreaBase+0x4e68,image_addresses,sizeof(image_addresses)))return 0;
    BootDisplayCallbacks *callbacks=&g_BootDisplaySpriteCallbacks;
    if(callbacks->init!=BootDisplay_InitDisplayResources || callbacks->step!=BootDisplay_SelectFrameBuffers ||
       callbacks->text!=BootDisplay_StepImageSprite || callbacks->glyph!=BootDisplay_QueueTintedSprite ||
       callbacks->transition!=BootDisplay_QueueFadeSprite || callbacks->present!=BootDisplay_PresentSpriteFrame || callbacks->shutdown!=BootDisplay_DisplayNoOp)return 0;
    callbacks->shutdown();callbacks=&g_BootDisplayImageCallbacks;
    if(callbacks->init!=BootDisplay_InitDisplayBuffers || callbacks->step || callbacks->text!=BootDisplay_StepImageDisplay ||
       callbacks->glyph!=BootDisplay_BrightenGlyph || callbacks->transition!=BootDisplay_BrightenImageRegion ||
       callbacks->present!=BootDisplay_PresentFrame || callbacks->shutdown!=BootDisplay_CallInitSystem)return 0;
    BootDisplay_InitDisplayResources();char *begins[2],*ends[2];RenderNativeTag *tables[2];
    for(unsigned slot=0;slot<2;++slot){g_ActiveDrawSlot=slot;BootDisplay_SelectFrameBuffers();
        begins[slot]=g_BootDisplayPrimitiveCursor;ends[slot]=g_BootDisplayPrimitiveEnd;tables[slot]=g_BootDisplayOrderingTable;
        for(unsigned i=0;i<BOOT_DISPLAY_SPRITE_CAPACITY;++i)BootDisplay_QueueTintedSprite(24,20,'A');
        if(g_BootDisplayState<0 || g_BootDisplayPrimitiveCursor!=ends[slot])return 0;
    }
    if(ends[0]!=begins[1] || ends[1]!=(char *)tables[0] || tables[0]+16!=tables[1])return 0;
    BootDisplay_QueueFadeSprite(128);if(g_BootDisplayState!=-1)return 0;

    static _Alignas(16) u8 table[69632],decoded[0xa800],rgb[0xa000];
    static u16 stage[1024*512],actual[1024*512],expected[1024*512];
    RECT whole={0,0,1024,512};unsigned states=0,draws=0,visible=0;
    MdecDiag_ExpandDecoderTable(table);
    for(unsigned index=0;index<31;++index){
        DecDCTvlcSize2(0);if(DecDCTvlc2(g_BootDisplayCreditImages[index],decoded,table)!=0)return 0;
        DecDCTReset(0);DecDCToutCallback(NULL);DecDCTin(decoded,0);DecDCTout(rgb,0x2800);if(DecDCToutSync(0))return 0;
        BootDisplay_InitDisplayResources();g_BootDisplayBufferIndex=index%2;g_BootDisplayStateTimer=index;
        for(unsigned i=0;i<1024*512;++i)stage[i]=0x1357;LoadImage(&whole,stage);DrawSync(0);
        unsigned steps=0;
        while(g_BootDisplayState!=5 && steps<512){g_ActiveDrawSlot=steps%2;BootDisplay_SelectFrameBuffers();BootDisplay_StepImageSprite();++steps;}
        if(g_BootDisplayState!=5){fprintf(stderr,"Credit sprite image %u stalled state=%d\n",index,g_BootDisplayState);return 0;}
        BootDisplayImageBuffer *screen=&g_BootDisplayImageBuffers[index%2];
        for(unsigned column=0;column<10;++column)for(unsigned y=0;y<128;++y)
            memcpy(&stage[(screen->transferY+y)*1024+screen->transferX+column*16],rgb+column*0x1000+y*32,32);
        StoreImage(&whole,actual);DrawSync(0);if(memcmp(actual,stage,sizeof(stage)))return 0;
        states+=steps;
        for(unsigned position=0;position<4;++position)for(unsigned slot=0;slot<2;++slot){
            LoadImage(&whole,stage);DrawSync(0);g_ActiveDrawSlot=slot;g_BootDisplayBufferIndex=1-index%2;
            g_BootDisplayStateTimer=position;g_BootDisplayState=7;g_BootDisplayTransition=1;
            g_BootDisplayFrameCounter=1;g_BootDisplayImageBrightness=128;
            SetDefDrawEnv(&g_RenderDrawEnvArray[slot],0,slot*224,320,224);g_RenderDrawEnvArray[slot].isbg=1;
            SetDefDispEnv(&g_RenderDispEnvArray[slot],0,(1-slot)*224,320,224);
            BootDisplay_SelectFrameBuffers();BootDisplay_StepImageSprite();BootDisplay_PresentSpriteFrame();DrawSync(0);
            if(g_ActiveDrawSlot!=(s32)(1-slot))return 0;
            StoreImage(&whole,actual);DrawSync(0);
            LoadImage(&whole,stage);DrawSync(0);PutDrawEnv(&g_RenderDrawEnvArray[slot]);
            BootDisplayDrawModePrim mode;BootDisplaySpritePrim sprite;memset(&mode,0,sizeof(mode));memset(&sprite,0,sizeof(sprite));
            BootDisplayImageBuffer *front=&g_BootDisplayImageBuffers[1-index%2];
            SetDrawTPage(&mode,0,0,GetTPage(2,0,front->x,front->y));SetSprt(&sprite);
            sprite.r0=sprite.g0=sprite.b0=128;sprite.w=160;sprite.h=120;
            sprite.x0=position%2?0:160;sprite.y0=position>=2?105:0;
            DrawPrim(&mode);DrawPrim(&sprite);DrawSync(0);StoreImage(&whole,expected);DrawSync(0);
            if(memcmp(actual,expected,sizeof(actual))){fprintf(stderr,"Credit sprite presentation index=%u pos=%u slot=%u differs\n",index,position,slot);return 0;}
            for(unsigned y=0;y<224;++y)for(unsigned x=0;x<320;++x)if(actual[(slot*224+y)*1024+x]&0x7fff)++visible;
            ++draws;
        }
    }
    fx_image_io=0;
    if(!visible || draws!=248)return 0;
    printf("Original credits sprite images: 31 images, %u decoder steps, 310 RGB15 uploads and %u full-VRAM presentations across all positions/both buffers match unmerged direct draw\n",states,draws);return 1;
}

static int original_credit_brightness(void)
{
    static struct {u8 before[64];char pixels[320*224*3];u8 after[64];} screens[2];
    const unsigned seeds[]={0,64,128,240};
    const int edges[][2]={{0,-16},{0,-7},{304,208},{319,100},{-1,100},{0,223},{0,224},{1,0}};
    uint64_t hash=UINT64_C(14695981039346656037);unsigned cases=0;int valid=1;
    for(unsigned slot=0;slot<2;++slot)g_BootDisplayImageBuffers[slot].pixels=screens[slot].pixels;
#define BRIGHT_PUT(data,count) do {const u8 *p_=(const u8 *)(data); \
    for(size_t i_=0;i_<(size_t)(count);++i_)hash=(hash^p_[i_])*UINT64_C(1099511628211);}while(0)
#define BRIGHT_CLEAR(slot,shade) do {unsigned s_=(slot),v_=(shade); \
    memset(&screens[s_],0xa5,sizeof(screens[s_]));u8 *p_=(u8 *)screens[s_].pixels; \
    for(size_t i_=0;i_<sizeof(screens[s_].pixels);i_+=3){p_[i_]=v_;p_[i_+1]=v_/2;p_[i_+2]=(v_+17)&255;}}while(0)
#define BRIGHT_GUARDS() do {for(unsigned i_=0;i_<64;++i_)if(screens[g_BootDisplayBufferIndex].before[i_]!=0xa5 || screens[g_BootDisplayBufferIndex].after[i_]!=0xa5)valid=0;}while(0)
    for(unsigned style=0;style<4;++style){unsigned slot=style%2;g_BootDisplayBufferIndex=slot;g_BootDisplayFontStyle=style;
        for(unsigned character=0;character<256;++character){
            BRIGHT_CLEAR(slot,seeds[style]);
            BootDisplay_BrightenGlyph(100,110,character);
            for(unsigned y=0;y<16;++y)BRIGHT_PUT(screens[slot].pixels+(110+y)*960+300,48);
            BRIGHT_GUARDS();++cases;
        }
    }
    for(unsigned style=0;style<4;++style)for(unsigned edge=0;edge<8;++edge)for(unsigned character=0;character<2;++character){
        unsigned slot=style%2;int x=edges[edge][0],y=edges[edge][1];g_BootDisplayBufferIndex=slot;g_BootDisplayFontStyle=style;
        BRIGHT_CLEAR(slot,seeds[style]);
        BootDisplay_BrightenGlyph(x,y,character?'~':'A');
        int start=(y>0?y:0)*960+x*3;if(start<0)start=0;
        int end=((y+16<224?y+16:224)-1)*960+x*3+48;
        if(end>320*224*3)end=320*224*3;if(end<start)end=start;
        BRIGHT_PUT(screens[slot].pixels+start,end-start);BRIGHT_GUARDS();++cases;
    }
    for(unsigned slot=0;slot<2;++slot)for(unsigned seed=0;seed<4;++seed){
        g_BootDisplayBufferIndex=slot;BRIGHT_CLEAR(slot,seeds[seed]);
        BootDisplay_BrightenImageRegion(0);for(unsigned row=0;row<24;++row)BRIGHT_PUT(screens[slot].pixels+0x19dac+row*960,288);
        BRIGHT_GUARDS();++cases;
    }
#undef BRIGHT_CLEAR
#undef BRIGHT_PUT
    const int outside[][2]={{-1,0},{319,223},{-32768,0},{32767,223}};
    for(unsigned edge=0;edge<4;++edge){g_BootDisplayBufferIndex=edge%2;memset(&screens[edge%2],0xa5,sizeof(screens[edge%2]));
        BootDisplay_BrightenGlyph(outside[edge][0],outside[edge][1],'A');BRIGHT_GUARDS();}
#undef BRIGHT_GUARDS
    if(!valid || cases!=1096 || hash!=UINT64_C(0x62c94d8d86727434)){fprintf(stderr,"Credits brightness: cases=%u hash=%016llx guards=%d\n",cases,(unsigned long long)hash,valid);return 0;}
    puts("Original credits RGB24 brightness: 1096 USA glyph/panel cases, saturation, four styles, both buffers, row clipping and four outer buffer guards pass");return 1;
}

static int original_credit_images(PeDisc *disc)
{
    g_CdImage=disc;glyph_pending=glyph_poll_retry=0;fx_image_io=1;
    if(Asset_OpenPeImage() || CdRom_LoadBootAudio())return 0;
    if(!SDL_Init(SDL_INIT_EVENTS))return 0;ResetGraph(0);Boot_InitMemoryLayout();
    static _Alignas(16) u8 table[69632],decoded[0xa800],rgb[0xf000];
    static u16 actual[1024*512],expected[1024*512],shown[480*224];
    RECT whole={0,0,1024,512};unsigned states=0,uploads=0;
    BootDisplay_InitDisplayBuffers();if(!original_credit_brightness())return 0;
    MdecDiag_ExpandDecoderTable(table);
    for(unsigned index=0;index<31;++index){
        DecDCTvlcSize2(0);
        if(DecDCTvlc2(g_BootDisplayCreditImages[index],decoded,table)!=0)return 0;
        DecDCTReset(0);DecDCToutCallback(NULL);DecDCTin(decoded,1);DecDCTout(rgb,0x3c00);
        if(DecDCToutSync(0))return 0;
        g_ActiveDrawSlot=0;BootDisplay_InitDisplayBuffers();g_BootDisplayBufferIndex=index%2;
        g_BootDisplayStateTimer=index;
        for(unsigned i=0;i<1024*512;++i)expected[i]=0x1357;
        LoadImage(&whole,expected);DrawSync(0);
        unsigned steps=0;
        while(g_BootDisplayState!=5 && steps<512){BootDisplay_StepImageDisplay();++steps;}
        if(g_BootDisplayState!=5){fprintf(stderr,"Credits image %u stalled state=%d after=%u\n",index,g_BootDisplayState,steps);return 0;}
        BootDisplayImageBuffer *screen=&g_BootDisplayImageBuffers[index%2];
        for(unsigned column=0;column<10;++column)for(unsigned y=0;y<120;++y)
            memcpy(&expected[(screen->transferY+52+y)*1024+screen->transferX+120+column*24],rgb+column*0x1800+y*48,48);
        StoreImage(&whole,actual);DrawSync(0);
        if(memcmp(actual,expected,sizeof(actual))){fprintf(stderr,"Credits staged image %u differs from whole-frame MDEC output\n",index);return 0;}
        g_BootDisplayState=6;BootDisplay_PresentFrame();
        if(g_BootDisplayStateTimer!=(s16)(index+1) || g_BootDisplayBufferIndex!=(s32)(1-index%2) ||
           g_BootDisplayState!=(index==30?7:0))return 0;
        g_BootDisplayState=7;BootDisplay_StepImageDisplay();BootDisplay_PresentFrame();
        RECT visible={0,0,480,224};StoreImage(&visible,shown);DrawSync(0);
        for(unsigned y=0;y<224;++y)if(memcmp(shown+y*480,&expected[(screen->transferY+y)*1024+screen->transferX],960)){
            fprintf(stderr,"Credits presented image %u row %u differs\n",index,y);return 0;}
        states+=steps;uploads+=10;
    }
    u8 *workspace=g_CdLoadAreaBase;u16 main_sector=g_MainAssetLbaTbl[0];
    for(unsigned fault=0;fault<3;++fault){
        if(fault==0)g_CdLoadAreaBase=NULL;
        else g_MainAssetLbaTbl[0]=fault==1?0:65535;
        BootDisplay_InitDisplayBuffers();
        if(g_BootDisplayState!=-1 || g_BootDisplayDone!=-1)return 0;
        g_CdLoadAreaBase=workspace;g_MainAssetLbaTbl[0]=main_sector;
    }
    for(unsigned fault=0;fault<7;++fault){
        BootDisplay_InitDisplayBuffers();
        u16 words;memcpy(&words,g_BootDisplayCreditImages[0],2);
        if(fault==0)g_BootDisplayBufferIndex=2;
        if(fault==1)g_BootDisplayStateTimer=31;
        if(fault==2){u16 invalid=65535;memcpy(g_BootDisplayCreditImages[0],&invalid,2);}
        if(fault==3){u16 invalid=0;memcpy(g_BootDisplayCreditImages[0],&invalid,2);}
        if(fault==4)g_BootDisplayState=1;
        if(fault==5)g_BootDisplayImageBuffers[0].pixels=NULL;
        if(fault==6)BootDisplay_SetImageDisplay(2);
        BootDisplay_StepImageDisplay();
        memcpy(g_BootDisplayCreditImages[0],&words,2);
        if(g_BootDisplayState!=-1 || g_BootDisplayDone!=-1)return 0;
    }
    BootDisplay_InitDisplayBuffers();if(g_BootDisplayState || g_BootDisplayDone)return 0;
    fx_image_io=0;
    printf("Original credits images: 31 real images, %u decoder steps, %u RGB24 slice uploads, full staged VRAM and both presented buffers match whole-frame MDEC output; ten invalid states/workspaces fail and recover\n",states,uploads);return 1;
}

static int original_menu_sound_commands(PeDisc *disc)
{
    _Alignas(16) static u8 bank[0x1400];PeDiscFile archive;
    if(!pe_disc_find(disc,"PE.IMG",&archive) ||
       !pe_disc_read(disc,&archive,(u64)g_PeImageSectorTable[7]*2048,bank,sizeof(bank)))return 0;
    u32 directory,packed;memcpy(&directory,bank+4,4);
    if(directory>sizeof(bank)-sizeof(SceneAssetDirectory))return 0;
    memcpy(&packed,bank+directory+0x2c,4);unsigned count=packed>>22,offset=packed&0x3fffff;
    if(offset>sizeof(bank) || count>(sizeof(bank)-offset)/12)return 0;
    void (*play[])(void)={Menu_PlayConfirmSound,Menu_PlayCancelSound,Menu_PlayMoveSound,Menu_PlayErrorSound};
    Pe1GameState saved_state=g_GameState;AkaoStagedCommand saved_command=g_AkaoCmd;
    AkaoQueueEntry saved_queue[16];memcpy(saved_queue,g_AkaoMessageQueue,sizeof(saved_queue));
    int saved_count=g_AkaoMessageQueueCount;AkaoSequenceCounter saved_sequence=g_AkaoCmd24Sequence;
    g_GameState.bank_work_far_end=NULL;g_AkaoMessageQueueCount=0;
    for(unsigned i=0;i<4;++i)play[i]();
    if(g_AkaoMessageQueueCount)return 0;
    g_GameState.bank_work_far_end=bank;
    if(g_AkaoBgmHandle[0]!=bank)return 0;
    for(unsigned i=0;i<4;++i){
        u32 payload=0;
        for(unsigned row=0;row<count;++row){
            u16 key;memcpy(&key,bank+offset+row*12+10,2);
            if(key==0x44c+i){memcpy(&payload,bank+offset+row*12+4,4);payload&=0xffffff;break;}
        }
        if(!payload || payload>sizeof(bank)-4)return 0;
        u32 magic;memcpy(&magic,bank+payload,4);if(magic!=0x4f414b41)return 0;
        play[i]();AkaoQueueEntry *command=&g_AkaoMessageQueue[i];
        if(g_AkaoMessageQueueCount!=(int)i+1 || command->opcode.word!=0x24 ||
           command->arg0.sequence!=bank+payload+4 || command->arg1!=0x100 ||
           command->arg2!=0x80 || command->arg3!=0x7f)return 0;
    }
    g_GameState=saved_state;g_AkaoCmd=saved_command;g_AkaoMessageQueueCount=saved_count;
    g_AkaoCmd24Sequence=saved_sequence;memcpy(g_AkaoMessageQueue,saved_queue,sizeof(saved_queue));
    puts("Original menu audio: canonical loaded-bank alias and all four real USA sound keys queue native AKAO commands; absent bank stays quiet");
    return 1;
}

static int original_akao_resident_tables(PeDisc *disc,PeDiscFile *exe)
{
    const struct {u32 address;size_t size;const void *data;} tables[]={
        {0x8009b85c,96,g_AkaoOpcodeArgLengths},{0x8009b8bc,32,g_AkaoExtOpcodeArgLengths},
        {0x8009b8dc,22,g_AkaoNoteLengthTable},{0x8009b8f4,4,g_AkaoDefaultVoiceProgram},
        {0x8009b8f8,512,g_AkaoPanVolumeTable}};
    u8 bytes[1024];u32 pointers[16];size_t sizes[16];
    for(unsigned i=0;i<5;++i)
        if(!pe_disc_read(disc,exe,tables[i].address-0x80010000u+0x800,bytes,tables[i].size) ||
           memcmp(bytes,tables[i].data,tables[i].size))return 0;
    if(!pe_disc_read(disc,exe,0x8009c080u-0x80010000u+0x800,pointers,sizeof(pointers)))return 0;
    for(unsigned i=0;i<16;++i){
        u32 end=0x8009c080;
        for(unsigned j=0;j<16;++j){
            if(pointers[j]>pointers[i] && pointers[j]<end)end=pointers[j];
            if((pointers[i]==pointers[j])!=(g_AkaoSoundEntryTable[i]==g_AkaoSoundEntryTable[j]))return 0;
        }
        if(pointers[i]<0x8009baf8 || pointers[i]>=end || end-pointers[i]>sizeof(bytes))return 0;
        sizes[i]=end-pointers[i];
        if(!g_AkaoSoundEntryTable[i] || !pe_disc_read(disc,exe,pointers[i]-0x80010000u+0x800,bytes,sizes[i]) ||
           memcmp(bytes,g_AkaoSoundEntryTable[i],sizes[i]))return 0;
    }
    const unsigned periods[]={1,2,255},modes[]={0,1,4};
    int saved_mode=g_AkaoPlaybackMode;uint64_t hash=UINT64_C(14695981039346656037);unsigned frames=0;
    for(unsigned selector=0;selector<16;++selector)for(unsigned period=0;period<3;++period)
    for(unsigned mode=0;mode<3;++mode){
        void *wave=g_AkaoSoundEntryTable[selector];
        AkaoTrack voice={.pitch_lfo_table=wave,.volume_lfo_table=wave,.aux_lfo_table=wave,
            .flags=7,.expression_value=100<<16,.volume_base=100<<8,.pitch_base=0x1000,
            .pan_target=0x7f00,.panpot=0x7f00,.pitch_lfo_phase=1,.volume_lfo_phase=1,.aux_lfo_phase=1,
            .pitch_lfo_duration=periods[period],.volume_lfo_duration=periods[period]+1,
            .aux_lfo_duration=periods[period]+2,.pitch_lfo_depth=1024,.volume_lfo_target=100<<8,
            .aux_lfo_target=64<<8,.update_flags=0x13};
        g_AkaoPlaybackMode=modes[mode];
        for(unsigned tick=0;tick<512;++tick){
            Akao_SetVoiceKeyOn(&voice,0x1000);
#define RESIDENT_AUDIO_PUT(value,count) do {u32 v_=(u32)(value); \
    for(unsigned b_=0;b_<(count);++b_){hash=(hash^(u8)v_)*UINT64_C(1099511628211);v_>>=8;}}while(0)
            RESIDENT_AUDIO_PUT(voice.update_flags,4);
            const u16 values[]={voice.pitch,(u16)voice.volume_left,(u16)voice.volume_right,
                (u16)voice.pitch_lfo_value,(u16)voice.volume_lfo_value,(u16)voice.aux_lfo_value,
                voice.pitch_lfo_phase,voice.volume_lfo_phase,voice.aux_lfo_phase};
            for(unsigned i=0;i<9;++i)RESIDENT_AUDIO_PUT(values[i],2);
            const void *cursor[]={voice.pitch_lfo_table,voice.volume_lfo_table,voice.aux_lfo_table};
            for(unsigned i=0;i<3;++i){
                uintptr_t offset=(uintptr_t)cursor[i]-(uintptr_t)wave;
                if(offset>=sizes[selector])return 0;
                RESIDENT_AUDIO_PUT(offset,4);
            }
#undef RESIDENT_AUDIO_PUT
            ++frames;
        }
    }
    g_AkaoPlaybackMode=saved_mode;
    if(frames!=73728 || hash!=UINT64_C(0xb2a673818a54a74e)){
        fprintf(stderr,"Original resident audio LFO hash %016llx frames %u\n",(unsigned long long)hash,frames);return 0;}
    puts("Original resident audio: all five constant tables and sixteen aliased wave selectors match Disc 1; 73728 full USA MIPS voice/LFO states passed");
    return 1;
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

static int original_script_movie(PeDisc *disc)
{
    _Alignas(16) static u8 work[BOOT_DISPLAY_WORK_SIZE+32],whole_frame[320*240*3],actual[320*240*3];
    Pe1GameState state=g_GameState;PeDisc *image=g_CdImage;u8 *workspace=g_CdLoadAreaBase;
    FieldActor *entity=g_CurrentEntity;ScriptNode *node_before=g_ScriptNodePool;u32 *cursor=g_ScriptCursor;
    CdlATV mix=g_AkaoCdVolume;int old_slot=g_ActiveDrawSlot,background=g_MenuSaveBgFadeState;
    DISPENV displays[2];DRAWENV draws[2];memcpy(displays,g_RenderDispEnvArray,sizeof(displays));memcpy(draws,g_RenderDrawEnvArray,sizeof(draws));
    PeDiscFile exe;u32 pointer=0;int valid=1;
    if(!pe_disc_find(disc,"SLUS_006.62",&exe) ||
       !pe_disc_read(disc,&exe,0x910a0+53*4-0x10000+0x800,&pointer,4) ||
       pointer!=0x80014e30 || g_ScriptQueueOpcodeHandlers[53]!=Script_LoadSceneData)return 0;
    g_CdImage=disc;g_CdLoadAreaBase=work;g_DrawEnabled.value=0;
    int number=0,*args[]={&number},*empty[]={NULL};u32 flags=g_GameState.flags;
    if(Script_LoadSceneData(NULL)!=-1 || Script_LoadSceneData(empty)!=-1)valid=0;
    number=-1;if(Script_LoadSceneData(args)!=-1)valid=0;
    number=47;if(Script_LoadSceneData(args)!=-1)valid=0;number=0;
    g_CdImage=NULL;if(Script_LoadSceneData(args)!=-1)valid=0;g_CdImage=disc;
    g_CdLoadAreaBase=NULL;if(Script_LoadSceneData(args)!=-1)valid=0;
    g_CdLoadAreaBase=work+1;if(Script_LoadSceneData(args)!=-1)valid=0;g_CdLoadAreaBase=work;
    if(g_GameState.flags!=flags || g_DrawEnabled.value)valid=0;
    unsigned rendered=0,rows=0;
    for(unsigned repeat=0;repeat<2;++repeat){
        memset(work,0xa5,sizeof(work));g_GameState.flags=0;g_GameStateFlags=0;
        g_DrawEnabled.value=0;g_ActiveDrawSlot=0;g_MenuSaveBgFadeState=0;
        g_MenuCardVideoDepthFlip=2;g_MenuCardVideoSize[0]=320;g_MenuCardVideoSize[1]=240;
        u32 script[]={53|(1u<<13),0,repeat?0x10000u:0u,32,0};FieldActor actor={0};
        ScriptNode node={.current.script=script,.active=1};g_CurrentEntity=&actor;g_ScriptNodePool=&node;
        glyph_reads=glyph_polls=0;glyph_read_retry=glyph_poll_retry=glyph_pending=1;
        int tick=VSync(-1);Script_RunQueue();
        if(glyph_reads || glyph_polls || VSync(-1)!=tick+1 || g_ScriptNodePool ||
           !(node.flags&0x10) || node.current.script!=script+5 || g_DrawEnabled.bits!=2 ||
           g_MenuCardVideoDepthFlip!=1 || g_MenuCardVideoDecodeMode!=2 || g_MovieIndex ||
           g_SeqElapsed!=1 || (g_GameState.flags&0x8200)!=0x8200 || g_MenuCardVideoDecodeBuffer0!=work)valid=0;
        glyph_read_retry=glyph_poll_retry=glyph_pending=0;
        Pe1SceneAudioState playing=g_GameState.scene_audio;flags=g_GameState.flags;
        if(Script_LoadSceneData(args)!=-2 || g_GameState.flags!=flags || memcmp(&playing,&g_GameState.scene_audio,sizeof(playing)))valid=0;
        unsigned frames=0;tick=VSync(-1);
        for(;;){
            unsigned decoded=g_MenuCardVideoDisplay.decodeIndex,region=g_MenuCardVideoDisplay.region;
            int mode=g_MenuCardVideoDecodeMode;unsigned bpp=mode==3?3:2;
            RECT area={0,region?0:240,mode==3?480:320,240};
            int running=MenuCardFirst_StepVideo();StoreImage(&area,actual);DrawSync(0);
            DecDCToutCallback(NULL);DecDCTin(g_MenuCardVideoDisplay.decode[decoded],mode);
            DecDCTout(whole_frame,320*240*bpp/4);if(DecDCToutSync(0))valid=0;
            DecDCToutCallback(MenuCardFirst_UploadVideoSlice);
            for(unsigned x=0;x<20;++x)for(unsigned y=0;y<15;++y)for(unsigned row=0;row<16;++row){
                if(memcmp(actual+((y*16+row)*320+x*16)*bpp,
                    whole_frame+((x*15+y)*256+row*16)*bpp,16*bpp))valid=0;++rows;}
            if(!frames){
                if(mode!=2 || g_MenuCardVideoDepthFlip!=2)valid=0;
                for(unsigned i=0;i<320*240;++i){u16 pixel;memcpy(&pixel,actual+i*2,2);if(pixel!=0x8000)valid=0;}
            }else if(mode!=3 || g_MenuCardVideoDepthFlip)valid=0;
            if(!g_MovieWide)valid=0;
            ++frames;++rendered;Draw_RenderFrame();
            if(!running || frames>150)break;
        }
        if(frames!=149 || VSync(-1)-tick!=596 || g_SeqElapsed!=150 || g_DrawEnabled.bits!=1 || !g_MenuCardVideoFinished)valid=0;
        DecDCToutCallback(NULL);MenuCardFirst_ResetDisplayState();
        if(g_DrawEnabled.value || g_MenuCardVideoDecodeBuffer0 || g_MenuCardVideoStreamRing)valid=0;
        for(unsigned i=0x55e00;i<sizeof(work);++i)if(work[i]!=0xa5){valid=0;break;}
        /* A failed file lookup closes the player and preserves scene/audio/env.
         * The next successful repeat must still take the same cold path. */
        char *name=g_MenuCardVideoTable[0].name;g_MenuCardVideoTable[0].name="\\ABSENT.STR;1";
        Pe1SceneAudioState audio=g_GameState.scene_audio;flags=g_GameState.flags;CdlATV before_mix=g_AkaoCdVolume;
        DISPENV before_display[2];DRAWENV before_draw[2];
        memcpy(before_display,g_RenderDispEnvArray,sizeof(before_display));memcpy(before_draw,g_RenderDrawEnvArray,sizeof(before_draw));
        if(Script_LoadSceneData(args)!=-3 || g_GameState.flags!=flags ||
           memcmp(&audio,&g_GameState.scene_audio,sizeof(audio)) || memcmp(&before_mix,&g_AkaoCdVolume,sizeof(before_mix)) ||
           memcmp(before_display,g_RenderDispEnvArray,sizeof(before_display)) || memcmp(before_draw,g_RenderDrawEnvArray,sizeof(before_draw)) ||
           g_MenuCardVideoDecodeBuffer0 || g_MenuCardVideoDecodeBuffer1 || g_MenuCardVideoSliceBuffer0 ||
           g_MenuCardVideoSliceBuffer1 || g_MenuCardVideoVlcTable || g_MenuCardVideoStreamRing)valid=0;
        g_MenuCardVideoTable[0].name=name;
    }
    g_GameState=state;g_CdImage=image;g_CdLoadAreaBase=workspace;g_CurrentEntity=entity;g_ScriptNodePool=node_before;g_ScriptCursor=cursor;
    g_AkaoCdVolume=mix;CdMix(&mix);g_ActiveDrawSlot=old_slot;g_MenuSaveBgFadeState=background;
    memcpy(g_RenderDispEnvArray,displays,sizeof(displays));memcpy(g_RenderDrawEnvArray,draws,sizeof(draws));
    if(!valid)fprintf(stderr,"original opcode 53/movie transition failed: frames=%u rows=%u\n",rendered,rows);
    else puts("Original movie opcode 53: real VM dispatch, two cold/recovery plays, RGB15-to-RGB24 first-frame flip, 298 frames/full VRAM, input/busy/missing-file errors and no executable overlay I/O passed");
    return valid;
}

static int original_movie_uploads(PeDisc *disc)
{
    _Alignas(16) static u8 arena[0x55e00], whole_frame[320*240*3], actual[320*240*3];
    PeDisc *old_image=g_CdImage;Pe1SceneAudioState audio=g_GameState.scene_audio;
    CdlATV mix=g_AkaoCdVolume;u32 flags=g_GameStateFlagsWord;
    int old_slot=g_ActiveDrawSlot,background=g_MenuSaveBgFadeState,valid=1;
    for(unsigned first=0;first<2;++first) for(unsigned wide_mode=0;wide_mode<2;++wide_mode) {
    VideoEntry *table=first?g_MenuCardVideoTable:g_MenuCardScreenVideoTable;
    VideoDisplay *display=first?&g_MenuCardVideoDisplay:&g_MenuCardScreenVideoDisplay;
    u8 wide=table[0].wide;
    u8 *buffers[]={arena};unsigned bytes_per_pixel=wide_mode?3:2;
    g_CdImage=disc;g_DrawEnabled.bits=0;g_ActiveDrawSlot=0;
    g_GameStateFlagsWord|=0x200;g_MenuSaveBgFadeState=0;
    table[0].wide=wide_mode;
    if(first)g_MenuCardVideoDepthFlip=0;else g_MenuCardScreenVideoDepthFlip=0;
    if(!(first?MenuCardFirst_InitVideoBuffers(1,buffers):MenuCard_InitVideoBuffers(1,buffers)) ||
       !(first?MenuCardFirst_OpenVideo(0):MenuCard_OpenVideo(0)))return 0;
    unsigned frames=0,bad_rows=0,black=0,sync_errors=0;int tick=VSync(-1);
    for(;;) {
        unsigned decoded=display->decodeIndex;
        unsigned region=display->region;
        RECT area={0,region?0:240,wide_mode?480:320,240};
        int running=first?MenuCardFirst_StepVideo():MenuCard_StepVideo();
        StoreImage(&area,actual);DrawSync(0);
        /* A whole transfer must agree with all twenty original slice callbacks. */
        DecDCToutCallback(NULL);
        DecDCTin(display->decode[decoded],wide_mode?3:2);
        DecDCTout(whole_frame,320*240*bytes_per_pixel/4);
        if(DecDCToutSync(0)){valid=0;++sync_errors;}
        DecDCToutCallback(first?MenuCardFirst_UploadVideoSlice:MenuCard_UploadVideoSlice);
        for(unsigned x=0;x<20;++x)for(unsigned y=0;y<15;++y)for(unsigned row=0;row<16;++row)
            if(memcmp(actual+((y*16+row)*320+x*16)*bytes_per_pixel,
                      whole_frame+((x*15+y)*256+row*16)*bytes_per_pixel,16*bytes_per_pixel)){valid=0;++bad_rows;}
        if(!frames) {
            if(wide_mode){for(unsigned i=0;i<320*240*3;++i)if(actual[i]){valid=0;++black;}}
            else for(unsigned i=0;i<320*240;++i){u16 pixel;memcpy(&pixel,actual+i*2,2);if(pixel!=0x8000){valid=0;++black;}}
        }
        ++frames;
        Draw_RenderFrame();
        if(!running || frames>150)break;
    }
    if((unsigned)(VSync(-1)-tick)!=149*4 || frames!=149 || g_SeqElapsed!=150 || (first?g_MenuCardVideoLastTime:g_MenuCardScreenVideoLastTime)!=150 ||
       (first?g_MenuCardVideoFinished:g_MenuCardScreenVideoFinished)!=1 || g_DrawEnabled.bits!=1)valid=0;
    if(!valid)fprintf(stderr,"movie case first=%u wide=%u frames=%u ticks=%u elapsed=%u last=%d done=%d active=%u rows=%u black=%u sync=%u\n",first,wide_mode,frames,(unsigned)(VSync(-1)-tick),g_SeqElapsed,first?g_MenuCardVideoLastTime:g_MenuCardScreenVideoLastTime,first?g_MenuCardVideoFinished:g_MenuCardScreenVideoFinished,g_DrawEnabled.bits,bad_rows,black,sync_errors);
    DecDCToutCallback(NULL);
    if(first)MenuCardFirst_ResetDisplayState();else MenuCard_ResetDisplayState();
    table[0].wide=wide;
    }
    g_MenuSaveBgFadeState=background;
    g_GameStateFlagsWord=flags;g_GameState.scene_audio=audio;g_ActiveDrawSlot=old_slot;
    g_AkaoCdVolume=mix;CdMix(&mix);g_CdImage=old_image;
    if(!valid)fprintf(stderr,"original movie MDEC/slice/VRAM regression\n");
    else puts("Original movies: both original players in RGB15/RGB24, 149 frames per case through real OpenVideo/StepVideo, 11920 total slice uploads, full VRAM regions and 15fps render path passed headlessly");
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
    static u8 first_bank[4*2048];
    if(!pe_disc_read(disc,&archive,965ull*2048,first_bank,sizeof(first_bank)))return 0;
    for(unsigned i=0;i<47;++i){
        const u8 *row=first_bank+0x1738+i*20;u32 name;s16 speed,end;u16 x,y;
        memcpy(&name,row,4);memcpy(&speed,row+6,2);memcpy(&end,row+8,2);memcpy(&x,row+10,2);memcpy(&y,row+12,2);
        VideoEntry *entry=&g_MenuCardVideoTable[i];
        if(name<0x80120D00u || name-0x80120D00u>=sizeof(first_bank) ||
           !memchr(first_bank+(name-0x80120D00u),0,sizeof(first_bank)-(name-0x80120D00u)))return 0;
        if(strcmp(entry->name,(char *)first_bank+(name-0x80120D00u)) || entry->wide!=row[4] || entry->speed!=speed ||
           entry->end!=end || entry->x!=x || entry->y!=y || memcmp(entry->reserved0e,row+14,6))valid=0;
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
    if(!original_escape_chance(disc,executable) || !original_action_slot() || !original_battle_target_list() || !original_script_sprites() || !original_wireframe_renderer() || !original_wireframe_animation() || !original_textbox_renderer(disc,executable) || !original_scan_overlay(disc,executable) || !original_save_card_images(disc) || !original_hud_packet_initialization() || !original_hud_packet_updates() || !original_spell_effects(disc,executable))valid=0;
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
    const int bounds[]={-32768,-1000,-1,0,1,100,1000,32767};
    uint64_t opcode_hash=UINT64_C(14695981039346656037);
    FieldActor actor={0},*saved_actor=g_CurrentEntity;ScriptNode *saved_node=g_ScriptNodePool;u32 *saved_cursor=g_ScriptCursor;
    g_CurrentEntity=&actor;if(g_ScriptQueueOpcodeHandlers[26]!=Script_Random)valid=0;
    for(unsigned lo=0;lo<8;++lo)for(unsigned hi=0;hi<8;++hi){
        Draw_InitGpuHwRegs();
        for(unsigned draw=0;draw<64;++draw){
            u32 before[33];memcpy(before,g_DrawPrimGeneratorWords,sizeof(before));
            int low=bounds[lo],high=bounds[hi],out=(s32)0x98765432;int *args[]={&out,&low,&high};
            u32 result=Script_Random(args),after[33];memcpy(after,g_DrawPrimGeneratorWords,sizeof(after));
            u32 words[35]={result,(u32)out};memcpy(words+2,after,sizeof(after));
            for(unsigned i=0;i<35;++i)for(unsigned b=0;b<4;++b)
                opcode_hash=(opcode_hash^((words[i]>>(b*8))&255))*UINT64_C(1099511628211);
            memcpy(g_DrawPrimGeneratorWords,before,sizeof(before));
            u32 program[]={26|(3u<<13),0,0x98765432,(u32)low,(u32)high,32,0};
            ScriptNode node={.current.script=program,.active=1,.flags=0x80};g_ScriptNodePool=&node;Script_RunQueue();
            if(result!=1 || low!=bounds[lo] || high!=bounds[hi] || program[2]!=(u32)out ||
               memcmp(after,g_DrawPrimGeneratorWords,sizeof(after)) || node.current.script!=program+7 ||
               node.flags!=0x90 || node.active || g_ScriptNodePool)valid=0;
        }
    }
    g_CurrentEntity=saved_actor;g_ScriptNodePool=saved_node;g_ScriptCursor=saved_cursor;
    if(opcode_hash!=UINT64_C(0xf2f87de64cb28a19)){fprintf(stderr,"Range RNG hash %016llx\n",(unsigned long long)opcode_hash);valid=0;}
    if(valid)puts("Original random opcode 26: 4096 unhooked USA CPU draws, equal/reversed/signed ranges, complete generator state and real dispatcher passed");
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

static int script_movie_error_exit(const char *executable)
{
    const char *args[]={executable,"--script-movie-error",NULL};
    SDL_PropertiesID properties=SDL_CreateProperties();
    SDL_SetPointerProperty(properties,SDL_PROP_PROCESS_CREATE_ARGS_POINTER,(void *)args);
    SDL_SetNumberProperty(properties,SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER,SDL_PROCESS_STDIO_NULL);
    SDL_SetNumberProperty(properties,SDL_PROP_PROCESS_CREATE_STDERR_NUMBER,SDL_PROCESS_STDIO_NULL);
    SDL_Process *process=SDL_CreateProcessWithProperties(properties);SDL_DestroyProperties(properties);
    if(!process){fprintf(stderr,"movie error child failed to start: %s\n",SDL_GetError());return 0;}
    int status=99;int waited=SDL_WaitProcess(process,true,&status);SDL_DestroyProcess(process);
#ifdef _WIN32
    int expected=3;
#else
    int expected=-SIGABRT;
#endif
    if(!waited || status!=expected){fprintf(stderr,"movie opcode failure did not terminate VM: %d\n",status);return 0;}
    puts("Original script error: movie failure terminates dispatcher instead of retrying or continuing the scene");return 1;
}

static int original_native_bootstrap(const char *executable,int field_control)
{
    const char *disc=getenv("PE_TEST_DISC");
    if(!disc || !*disc)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
    if(!SDL_Init(SDL_INIT_EVENTS))return 1;
    char capture[96];snprintf(capture,sizeof(capture),"pe-native-%llx.vram",(unsigned long long)SDL_GetPerformanceCounter());
    unsigned first=field_control>=2?(unsigned)field_control+10:field_control?6:0;
    unsigned end=field_control>=2?first+1:field_control?12:6;
    for(unsigned input=first;input<end;++input){
        const char *limit=input==16?"104000":input==15?"95751":input==14?"82000":input==13?"64554":input==12?"60634":input==11?"58000":input==10?"52000":input==9?"42000":input==8?"35500":input>=6?"33500":input==5?"30000":input==4?"24000":input>=2?"14000":input?"13000":"8000";
        const char *args[640]={executable,"--disc",disc,"--headless","--frame-limit",limit,"--dump-vram",capture};
        const char *events[]={"10000:8","10010:0","11000:0x10","11010:0","11500:0x4000","11510:0"};
        unsigned at=8;
        for(unsigned event=0;event<(input==3?4u:input==2 || input>=4?6u:input?1u:0u);++event){
            unsigned index=input==3 && event>=2?event+2:event;
            args[at++]="--pad-at";args[at++]=events[index];
        }
        char scheduled[320][32];
        if(input==1){
            for(unsigned event=0;event<128;++event){
                snprintf(scheduled[event],sizeof(scheduled[event]),"%u:8",10001+event);
                args[at++]="--pad-at";args[at++]=scheduled[event];
            }
        }
        if(input>=5){
            unsigned event=0;
#define BOOTSTRAP_PRESS(tick,mask) do { \
    snprintf(scheduled[event],sizeof(scheduled[event]),"%u:%u",(unsigned)(tick),(unsigned)(mask)); \
    args[at++]="--pad-at";args[at++]=scheduled[event++]; \
    snprintf(scheduled[event],sizeof(scheduled[event]),"%u:0",(unsigned)(tick)+10); \
    args[at++]="--pad-at";args[at++]=scheduled[event++]; \
}while(0)
            for(unsigned tick=17000;tick<(input==5?28000u:22000u);tick+=600)BOOTSTRAP_PRESS(tick,0x4000);
            if(input>=6){
                BOOTSTRAP_PRESS(23000,8);BOOTSTRAP_PRESS(23600,8);
                for(unsigned tick=24200;tick<32200;tick+=600)BOOTSTRAP_PRESS(tick,0x4000);
                if(input>=7){
                    snprintf(scheduled[event],sizeof(scheduled[event]),"33000:%u",input>=8?16u:32u);
                    args[at++]="--pad-at";args[at++]=scheduled[event++];
                    if(input>=9){
                        for(unsigned tick=35000;tick<=36200;tick+=600){
                            snprintf(scheduled[event],sizeof(scheduled[event]),"%u:%u",tick,16u|0x4000u);
                            args[at++]="--pad-at";args[at++]=scheduled[event++];
                            snprintf(scheduled[event],sizeof(scheduled[event]),"%u:16",tick+10);
                            args[at++]="--pad-at";args[at++]=scheduled[event++];
                        }
                    }
                    snprintf(scheduled[event],sizeof(scheduled[event]),"%u:0",input>=9?39000u:input==8?34700u:33040u);
                    args[at++]="--pad-at";args[at++]=scheduled[event++];
                }
            }
            if(input>=11)for(unsigned tick=52000;tick<57000;tick+=600)BOOTSTRAP_PRESS(tick,0x4000);
            if(input>=12){
                static const unsigned route[][2]={{58000,16},{58200,16},{59769,32},{59869,32},{59994,16}};
                for(unsigned i=0;i<5;++i){
                    snprintf(scheduled[event],sizeof(scheduled[event]),"%u:%u",route[i][0],route[i][1]);
                    args[at++]="--pad-at";args[at++]=scheduled[event++];
                }
            }
            if(input>=13){
                args[at++]="--pad-at";args[at++]="60634:128";
                args[at++]="--pad-at";args[at++]="60734:192";
                for(unsigned tick=61334;tick<(input>=14?81500u:64554u);tick+=510)BOOTSTRAP_PRESS(tick,0x4000);
            }
            if(input>=15){
                static const unsigned route[][2]={
                    {400,16},{600,32},{650,16},{700,48},{600,128},
                    {250,64},{650,48},{550,128},{800,16},{210,32},
                    {650,16},{1800,128},{650,64},{1500,128},{500,0},
                    {10,0x4000},{600,0},{850,16},{780,144},{1001,0}};
                unsigned tick=82000;
                for(unsigned i=0;i<sizeof(route)/sizeof(route[0]);++i){
                    snprintf(scheduled[event],sizeof(scheduled[event]),"%u:%u",tick,route[i][1]);
                    args[at++]="--pad-at";args[at++]=scheduled[event++];
                    tick+=route[i][0];
                }
            }
            if(input==16){
                args[at++]="--pad-at";args[at++]="95751:32";
                args[at++]="--pad-at";args[at++]="96851:0";
                for(unsigned pass=0;pass<2;++pass){
                    for(unsigned tick=pass?100000u:96851u;tick<(pass?102000u:98400u);tick+=128){
                        snprintf(scheduled[event],sizeof(scheduled[event]),"%u:16384",tick);
                        args[at++]="--pad-at";args[at++]=scheduled[event++];
                        snprintf(scheduled[event],sizeof(scheduled[event]),"%u:0",tick+8);
                        args[at++]="--pad-at";args[at++]=scheduled[event++];
                    }
                }
                args[at++]="--pad-at";args[at++]="102000:16";
                args[at++]="--pad-at";args[at++]="103000:0";
            }
#undef BOOTSTRAP_PRESS
        }
        SDL_PropertiesID props=SDL_CreateProperties();
        SDL_SetPointerProperty(props,SDL_PROP_PROCESS_CREATE_ARGS_POINTER,(void *)args);
        SDL_SetNumberProperty(props,SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER,SDL_PROCESS_STDIO_APP);
        SDL_SetNumberProperty(props,SDL_PROP_PROCESS_CREATE_STDERR_NUMBER,SDL_PROCESS_STDIO_NULL);
        SDL_Process *child=SDL_CreateProcessWithProperties(props);SDL_DestroyProperties(props);
        if(!child)return 1;
        int status=-1;size_t bytes=0;char *output=SDL_ReadProcess(child,&bytes,&status);
        SDL_DestroyProcess(child);
        int valid=output && status==0 && strstr(output,"\"event\":\"frame_limit\"") &&
            strstr(output,input==16?"\"vblanks\":104000":input==15?"\"vblanks\":95751":input==14?"\"vblanks\":82000":input==13?"\"vblanks\":64554":input==12?"\"vblanks\":60634":input==11?"\"vblanks\":58000":input==10?"\"vblanks\":52000":input==9?"\"vblanks\":42000":input==8?"\"vblanks\":35500":input>=6?"\"vblanks\":33500":input==5?"\"vblanks\":30000":input==4?"\"vblanks\":24000":input>=2?"\"vblanks\":14000":input?"\"vblanks\":13000":"\"vblanks\":8000");
        if(input==1 && (!output || !strstr(output,"\"pad\":4") || !strstr(output,"\"video_state\":0") ||
           !strstr(output,"\"rgb24\":1")))valid=0;
        if(input==2){
            unsigned frames=0;int movie_frame=0;
            const char *game=output?strstr(output,"\"game_frame\":"):NULL;
            const char *movie=output?strstr(output,"\"movie_frame\":"):NULL;
            if(!output || !strstr(output,"\"scene\":2819109064") || !strstr(output,"\"movie\":2") ||
               !strstr(output,"\"video_state\":2") || !game || !movie ||
               sscanf(game,"\"game_frame\":%u",&frames)!=1 || frames<2 ||
               sscanf(movie,"\"movie_frame\":%d",&movie_frame)!=1 || movie_frame<2)valid=0;
        }
        if(input==3 && (!output || !strstr(output,"\"scene\":2839543880") ||
           !strstr(output,"\"video_state\":0") || !strstr(output,"\"rgb24\":0") ||
           !strstr(output,"\"game_frame\":0")))valid=0;
        if(input==4){
            unsigned frames=0;const char *game=output?strstr(output,"\"game_frame\":"):NULL;
            if(!output || !strstr(output,"\"scene\":2818576456") || !strstr(output,"\"video_state\":0") ||
               !strstr(output,"\"dialogue\":[{\"state\":2,\"page\":9}") ||
               !strstr(output,"\"type\":0,\"sub\":0,\"player\":true") || !game ||
               sscanf(game,"\"game_frame\":%u",&frames)!=1 || frames<4000)valid=0;
        }
        if(input==5 && (!output || !strstr(output,"\"scene\":2818576456") ||
           !strstr(output,"\"video_state\":0") || !strstr(output,"\"flags\":4") ||
           !strstr(output,"\"menu\":{\"mode\":2,\"selection\":17")))valid=0;
        if(input>=6 && input<9){
            int x=0,y=0,z=0,yaw=0;
            const char *player=output?strstr(output,"\"type\":0,\"sub\":0,\"player\":true"):NULL;
            const char *position=player?strstr(player,"\"position\":"):NULL;
            if(!output || !strstr(output,input==8?"\"scene\":2818572744":"\"scene\":2818572616") || !strstr(output,"\"video_state\":0") ||
               !strstr(output,"\"menu\":null") || !position ||
               sscanf(position,"\"position\":[%d,%d,%d],\"yaw\":%d",&x,&y,&z,&yaw)!=4 ||
               x!=(input==8?-17825792:-8323072+(input==7?20*FIELD_WALK_SPEED:0)) ||
               y!=(input==8?-262144:0) || z!=(input==8?178192384:-352387072) ||
               yaw!=(input==8?2048:input==7?3072:2144))valid=0;
        }
        if(input==9 && (!output || !strstr(output,"\"scene\":2818994504") ||
           !strstr(output,"\"movie\":3") || !strstr(output,"\"video_state\":2") ||
           !strstr(output,"\"movie_frame\":257") || !strstr(output,"\"rgb24\":1")))valid=0;
        if(input==10 && (!output || !strstr(output,"\"scene\":2818572872") ||
           !strstr(output,"\"video_state\":0") || !strstr(output,"\"dialogue\":[{\"state\":2,\"page\":33}") ||
           !strstr(output,"\"position\":[65142784,-23855104,457768960],\"yaw\":0")))valid=0;
        if(input==11 && (!output || !strstr(output,"\"scene\":2818572872") ||
           !strstr(output,"\"video_state\":0") || !strstr(output,"\"game_frame\":6211") ||
           !strstr(output,"\"dialogue\":[{\"state\":0,\"page\":35}") ||
           !strstr(output,"\"position\":[-134283264,-21360136,489422848],\"yaw\":0")))valid=0;
        if(input==12 && (!output || !strstr(output,"\"scene\":2818573000") ||
           !strstr(output,"\"video_state\":0") ||
           !strstr(output,"\"position\":[-107559689,-72810496,35984650]")))valid=0;
        if(input==13 && (!output || !strstr(output,"\"scene\":2818573000") ||
           !strstr(output,"\"flags\":2178,") || !strstr(output,"\"move_lock\":4") ||
           !strstr(output,"\"video_state\":0") || !strstr(output,"\"game_frame\":2138") ||
           !strstr(output,"\"dialogue\":[{\"state\":0,\"page\":44}") ||
           !strstr(output,"\"player\":true,\"flags\":0,\"position\":[1441792,-72810496,56885248]")))valid=0;
        if(input==14 && (!output || !strstr(output,"\"scene\":2818573000") ||
           !strstr(output,"\"flags\":0,\"draw_slot\"") || !strstr(output,"\"move_lock\":0") ||
           !strstr(output,"\"video_state\":0") || !strstr(output,"\"game_frame\":2305") ||
           !strstr(output,"\"dialogue\":[{\"state\":0,\"page\":60}") || strstr(output,"\"type\":2,\"sub\":0") ||
           !strstr(output,"\"player\":true,\"flags\":8,\"position\":[22609920,-72810496,41484288],\"yaw\":3625") ||
           !strstr(output,"\"combat\":{\"mode\":12,\"live_enemies\":0,\"player_hp\":14,\"player_max_hp\":45}")))valid=0;
        if(input==15 && (!output || !strstr(output,"\"scene\":2818573512") ||
           !strstr(output,"\"flags\":0,\"draw_slot\"") || !strstr(output,"\"move_lock\":0") ||
           !strstr(output,"\"game_frame\":504") || !strstr(output,"\"video_state\":0") ||
           !strstr(output,"\"dialogue\":[{\"state\":0,\"page\":-1}") ||
           !strstr(output,"\"player\":true,\"flags\":8,\"position\":[61903440,-21495808,-178751558],\"yaw\":3536") ||
           !strstr(output,"\"combat\":{\"mode\":12,\"live_enemies\":0,\"player_hp\":14,\"player_max_hp\":45}")))valid=0;
        if(input==16 && (!output || !strstr(output,"\"scene\":2818576712") ||
           !strstr(output,"\"flags\":0,\"draw_slot\"") || !strstr(output,"\"move_lock\":0") ||
           !strstr(output,"\"game_frame\":770") || !strstr(output,"\"video_state\":0") ||
           !strstr(output,"\"dialogue\":[{\"state\":0,\"page\":-1}") ||
           !strstr(output,"\"player\":true,\"flags\":8,\"position\":[19202048,0,170917888],\"yaw\":0") ||
           !strstr(output,"\"combat\":{\"mode\":12,\"live_enemies\":0,\"player_hp\":14,\"player_max_hp\":45}")))valid=0;
        FILE *file=fopen(capture,"rb");uint64_t hash=UINT64_C(14695981039346656037);
        if(file){
            u8 block[4096];size_t total=0,n;
            while((n=fread(block,1,sizeof(block),file))!=0){total+=n;
                /* Original USA Disc 1, DuckStation software frame 16604,
                 * M010 dialogue 9: skin and cuffs of Aya's date. Allow two
                 * RGB555 levels for the current rasterizer's shading error;
                 * replacing the skin with neutral grey must fail. */
                static const u8 reference_hands[][5]={
                    {72,139,28,28,28},{108,141,30,30,30},
                    {72,141,27,21,17},{74,141,25,20,16},
                    {104,143,28,22,18},{106,143,26,20,17},
                    {108,143,25,19,16},{104,147,25,19,16}};
                if(input==4)for(unsigned p=0;p<sizeof(reference_hands)/sizeof(reference_hands[0]);++p){
                    size_t offset=((size_t)(224+reference_hands[p][1])*1024+reference_hands[p][0])*2;
                    if(offset>=total-n && offset+2<=total){
                        size_t at=offset-(total-n);u16 pixel=block[at]|((u16)block[at+1]<<8);
                        for(unsigned channel=0;channel<3;++channel){
                            int delta=((pixel>>(channel*5))&31)-reference_hands[p][channel+2];
                            if(delta < -2 || delta > 2){valid=0;fprintf(stderr,"M010 PS1 hand colour mismatch at %u,%u channel %u\n",reference_hands[p][0],reference_hands[p][1],channel);}
                        }
                    }
                }
                for(size_t i=0;i<n;++i)hash=(hash^block[i])*UINT64_C(1099511628211);}
            if(total!=1024*512*2 || ferror(file))valid=0;
            fclose(file);remove(capture);
        }else valid=0;
        if(!input && hash!=UINT64_C(0x6c2aef67a1f42efb))valid=0;
        if(input==4 && hash!=UINT64_C(0x42b28cf73effdc8b))valid=0;
        if(input==5 && hash!=UINT64_C(0x1d6ff100102480bb))valid=0;
        if(input==6 && hash!=UINT64_C(0x71756cc6f486b60f))valid=0;
        if(input==7 && hash!=UINT64_C(0x0ba1b9388d40f17f))valid=0;
        if(input==8 && hash!=UINT64_C(0x71d4a7c739498ad8))valid=0;
        if(input==9 && hash!=UINT64_C(0x4f2f3366d72018ad))valid=0;
        if(input==10 && hash!=UINT64_C(0x010725a10ea1b845))valid=0;
        if(input==11 && hash!=UINT64_C(0xd355ef4c28cf53bb))valid=0;
        if(input==12 && hash!=UINT64_C(0x4b5ea0ce332f7bf5))valid=0;
        if(input==13 && hash!=UINT64_C(0xa2276ccc2247ed21))valid=0;
        if(input==14 && hash!=UINT64_C(0xc7ae3598591ad7c3))valid=0;
        if(input==15 && hash!=UINT64_C(0x1a99c887686e1a86))valid=0;
        if(input==16 && hash!=UINT64_C(0x8e856f4fc0b74e6d))valid=0;
        if(!valid){fprintf(stderr,"Native bootstrap failed: input=%u status=%d hash=%016llx output=%s\n",
            input,status,(unsigned long long)hash,output?output:"(none)");SDL_free(output);return 1;}
        printf("Original executable bootstrap: input=%u hash=%016llx, bounded headless main and real CUE/VRAM passed\n",input,(unsigned long long)hash);
        SDL_free(output);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if(argc==2 && !strcmp(argv[1],"--texture-pages")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_scene_texture_pages(&disc);pe_disc_close(&disc);return result?0:156;
    }

    if(argc==3 && !strcmp(argv[1],"--native-stage"))return original_native_bootstrap(argv[2],2);
    if(argc==3 && !strcmp(argv[1],"--native-battle-entry"))return original_native_bootstrap(argv[2],3);
    if(argc==3 && !strcmp(argv[1],"--native-hallway"))return original_native_bootstrap(argv[2],6);
    if(argc==3 && !strcmp(argv[1],"--native-backstage"))return original_native_bootstrap(argv[2],5);
    if(argc==3 && !strcmp(argv[1],"--native-melissa"))return original_native_bootstrap(argv[2],4);
    if(argc==3 && !strcmp(argv[1],"--native-field-control"))return original_native_bootstrap(argv[2],1);
    if(argc==3 && !strcmp(argv[1],"--native-bootstrap"))return original_native_bootstrap(argv[2],0);
    if(argc==2 && !strcmp(argv[1],"--credits-playback")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_credit_playback(&disc);pe_disc_close(&disc);return result?0:155;
    }
    if(argc==2 && !strcmp(argv[1],"--fx-effects")){if(!SDL_Init(SDL_INIT_EVENTS))return 43;ResetGraph(0);return original_fx_shader_packets() && original_fx_blend() && original_fx_uncompressed_order() && original_fx_shader_pixels() && original_fx_shader_guards()?0:147;}
    if(argc==2 && !strcmp(argv[1],"--credits-sprites")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_credit_sprites(&disc);pe_disc_close(&disc);return result?0:154;
    }
    if(argc==2 && !strcmp(argv[1],"--credits-images")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_credit_images(&disc);pe_disc_close(&disc);return result?0:153;
    }
    if(argc==2 && !strcmp(argv[1],"--credits-text")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_credit_text(&disc);pe_disc_close(&disc);return result?0:152;
    }
    if(argc==2 && !strcmp(argv[1],"--fx-step")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_fx_scene_init(&disc) && original_fx_controller();pe_disc_close(&disc);return result?0:151;
    }
    if(argc==2 && !strcmp(argv[1],"--fx-frames")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_fx_scene_init(&disc) && original_fx_frame_loop(&disc);pe_disc_close(&disc);return result?0:150;
    }
    if(argc==2 && !strcmp(argv[1],"--fx-scene")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_fx_scene_init(&disc);pe_disc_close(&disc);return result?0:149;
    }
    if(argc==2 && !strcmp(argv[1],"--fx-motion")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_fx_camera_motion(&disc);pe_disc_close(&disc);return result?0:148;
    }
    if(argc==2 && !strcmp(argv[1],"--fx-models")){
        const char *path=getenv("PE_TEST_DISC");if(!path || !*path)return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
        PeDisc disc={0};if(!pe_disc_open(&disc,path))return 1;
        int result=original_fx_model_render_assets(&disc) && original_fx_poly_render_assets(&disc);pe_disc_close(&disc);return result?0:146;
    }
    if(argc==2 && !strcmp(argv[1],"--fx-display")){if(!SDL_Init(SDL_INIT_EVENTS))return 43;ResetGraph(0);return original_gte_register_transfers() && original_fx_fog() && original_fx_display()?0:145;}
    if(argc==2 && !strcmp(argv[1],"--fx-camera"))return original_fx_node_pool() && original_fx_culling() && original_fx_camera_matrices()?0:143;
    if(argc==2 && !strcmp(argv[1],"--script-movie-error")){
        u32 script[]={53|(1u<<13),0,47,32,0};FieldActor actor={0};
        ScriptNode node={.current.script=script,.active=1};
        g_CurrentEntity=&actor;g_ScriptNodePool=&node;g_GameStateFlags=0;g_CdImage=NULL;
        Script_RunQueue();return 99;
    }
    if(argc==2 && !strcmp(argv[1],"--glint"))return original_positional_audio_projection() && original_glint_lifecycle()?0:141;
    if(!room_callback_context() || !room_actor_view() || !room_owner_state() || !callback_list_storage() || !native_task_cycle() || !canonical_render_buffers() || !native_effect_packet_links() || !native_screen_tint_packets() || !native_effect_model_packets() || !native_oriented_sprite_packets() || !native_ring_packets() || !native_effect_collision_coordinates() || !original_glow_resident_constants() || !native_glow_layer_packets() || !native_object_script_cycle() || !authored_glow_script_cycle() || !authored_m005_effect_cycle() || !authored_orbit_flare_cycle() || !original_actor_render_opcodes() || !original_actor_camera_opcodes() || !original_tracking_angle_opcode() || !original_absolute_movement_opcode() || !original_resident_script_program() || !original_actor_audio_state() || !original_burst_target_selection() || !native_effect_spawn_arguments() || !native_object_slot_bounds() || !process_static_task_programs() || !process_texture_bank_failures() || !native_actor_effect_gates()) return 123;
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
    if(!script_movie_error_exit(argv[0]))return 142;
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
        if (!original_fx_resources(&disc,&exe)) return 144;
        if (!original_icon_packets(&disc,&exe)) return 133;
        if (!original_task_bank_data(&disc) || !original_glint_particles(&disc) || !original_positional_audio_projection() || !original_glint_lifecycle()) return 141;
        if (!original_video_buffers(&disc)) return 135;
        if (!original_video_decoder(&disc)) return 136;
        if (!original_video_stream_errors()) return 137;
        if (!original_movie_uploads(&disc) || !original_script_movie(&disc)) return 138;
        if (!original_akao_resident_tables(&disc,&exe) || !original_image_display_switch(&disc,&exe) || !original_boot_display_data(&disc) || !original_menu_sound_commands(&disc) || !original_enemy_slots(&disc,&exe)) return 140;
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
        MenuWidget_EaseNodePosition(NULL);
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
