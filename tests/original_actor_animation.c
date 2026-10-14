#include "pe1/entity_animation.h"
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
#include "pe1/geom_state.h"
#include "pe1/field_movement.h"
#include "pe1/pad_state.h"
#include "pe1/scene_flags.h"
#include "pe1/entity_frame_update.h"
#include "disc.h"
#include "pe1/gte.h"
#include "pe1/menu_widget.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <psyz/system.h>
int (*g_ScriptQueueOpcodeHandlers[8192])(int **);
GeomScrollRecord g_GeomScroll;

/* Backing storage for exercising the original boot buffer layout. */
_Alignas(16) u8 g_BootWorkArea[0x20000], D_8010BD00[0x20000];
_Alignas(16) u8 D_80120D08[0x200000], g_SceneLoadScratchArea[0x20000];
static u8 directory[0x1000];
u8 *g_StrFileDirBuffer = directory + 8;
u8 *g_OtBufferTable[2], *g_RenderOtBufferBaseAlt[1];
u8 *g_RenderFrontBufferBase[1], *g_RenderBackBufferBase[1];
u8 *g_RenderAnimBufferBase[1];

static int boot_buffer_layout(void)
{
    Pe1GameState saved = g_GameState;
    Boot_InitGameState();
    if (g_GameState.bank_asset_source != g_BootWorkArea ||
        g_GameState.voice_bank_base != g_BootWorkArea + 0x1800 ||
        g_GameState.voice_bank_base_1400 != g_BootWorkArea + 0x6000 ||
        (void *)g_GameState.entity_texture_blob != g_BootWorkArea + 0x14000 ||
        g_GameState.scene_object_work != D_80120D08 ||
        g_GameState.str_file_directory != directory + 8 ||
        (void *)g_GameState.loaded_scene_assets != directory ||
        g_GameState.scene_load_scratch != g_SceneLoadScratchArea ||
        g_GameState.bank_work_base != g_GameState.voice_bank_base ||
        g_GameState.bank_work_end != g_GameState.voice_bank_base + 0x1400 ||
        g_GameState.bank_work_far_end != g_GameState.voice_bank_base + 0x2800 ||
        !g_GameState.scene_process_slots || !g_GameState.script_sprite_buffer ||
        !g_GameState.unk_170) return 0;
    RenderNativeTag *first = (RenderNativeTag *)g_OtBufferTable[0];
    RenderNativeTag *second = (RenderNativeTag *)g_RenderOtBufferBaseAlt[0];
    if (first + 4096 != second ||
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
u16 g_PeImageSectorTable[81];
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
AkaoVoiceBankEntry g_AkaoVoiceBankEntries[20];
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

int main(int argc, char **argv)
{
    (void)argv;
    if (!bios_random()) return 58;
    if (!glyph_records()) return 55;
    if (!glyph_selection()) return 56;
    if (!boot_buffer_layout()) return 54;
    if (!game_state_reset()) return 53;
    if(!SDL_Init(SDL_INIT_EVENTS))return 43;
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
        MenuWidget_InitPool();
        if (g_MenuWidgetFreeListHead!=g_MenuWidgetNodePool || g_MenuWidgetNodePool[23].next) return 90;
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
        g_PlayerEntity=0;
    }
    puts("Original action/clock/audio trigger and digital/analog player motion passed");
    SDL_Quit();
    return 0;
}
