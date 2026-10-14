#include "pe1/render_setup.h"
#include "pe1/entity_frame_update.h"
#include "pe1/gte.h"
#include "pe1/render_lighting.h"
#include <libgpu.h>
#include "room_banks.h"
#include "model.h"
#include "script.h"
#include "pe1/reloc_block.h"
#include "vram.h"
#include "pe1/scene_assets.h"
#include "pe1/scene_room.h"
#include "pe1/battle_saved_state.h"
#include "pe1/resident_tables.h"
#include "pe1/inventory_slots.h"
#include "pe1/player_entity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RenderLightColor g_RoomLightTint = {255,255,255};
s32 g_IdentityMatrixSource[8] = {4096,0,4096,0,4096,0,0,0};
s16 g_RenderTexturePageDeltas[4][4];
int g_RenderCounter[1];
RenderVec3s g_RenderNormalTable[240];

#include "pe1/entity_allocation.h"
#include "pe1/render_buffers.h"
#include "pe1/script_state.h"
#include "pe1/scene_archive.h"
uintptr_t g_SceneMapPrimBaseTable[10];
FieldActorCallbacks g_FieldActorTypeCallbacks[256];
u8 g_SceneAreaType;

#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
static u32 room_word(const u8 *bytes)
{
    return (u32)bytes[0] | (u32)bytes[1] << 8 | (u32)bytes[2] << 16 | (u32)bytes[3] << 24;
}

int main(void)
{
    const char *path=getenv("PE_TEST_DISC");
    if(!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
    PeDisc disc={0}; PeRoomCatalog catalog; PeRoom room={0};
    CHECK(pe_disc_open(&disc,path) && pe_room_catalog_open(&catalog,&disc));
    u8 *memory=malloc(16u*ENTITY_ALLOCATION_BLOCK_SIZE); CHECK(memory);
    g_RenderScratchBufferBase[0]=memory;
    struct { SceneAssetBlob header; SceneAssetDirectory directory; SceneBankAssetRecord row; } archive={0};
    archive.header.directoryOffset=sizeof(SceneAssetBlob);
    archive.directory.bankRootEntries=(1u<<22)|(sizeof(SceneAssetBlob)+sizeof(SceneAssetDirectory));
    archive.row.source.bytes.id=1;
    g_LoadedSceneAssetBlock[0]=&archive.header;
    u32 script[]={0}; u8 *scripts[12]={0}; scripts[3]=(u8 *)script;
    g_GameState.bank_reset_944[0]=scripts;
    unsigned spawned=0, relocated=0;
    struct { unsigned slot; u32 address; } handler_variants[512];
    unsigned variant_count = 0, handler_count = 0, pm_count = 0, effect_count = 0;
    unsigned pm_methods = 0, mirror_descriptors = 0;
    unsigned effect_scripts=0, effect_instructions=0;
    unsigned char handler_slots[192] = {0};
    for(unsigned map=1;map<=PE_USA_ROOM_SLOTS;++map) {
        if(!catalog.ranges[map-1].room_sectors) continue;
        CHECK(pe_room_load(&catalog,map,&room));
        const u8 *handler_records;
        unsigned record_count;
        CHECK(pe_room_blob_table(&room.data, 1, 12, &handler_records, &record_count));
        for (unsigned record = 0; record < record_count; ++record) {
            const u8 *entry = handler_records + record * 12;
            unsigned slot = entry[7];
            u32 address = room_word(entry + 8);
            SceneRoomRecord serialized;
            memcpy(&serialized, entry, sizeof(serialized));
            CHECK(serialized.u.handlerAddress == address && serialized.source.bytes.slot == slot);
            /* USA room payload is loaded at 0x8018efe8, including
             * its eight-byte archive prefix. These are serialized descriptor addresses,
             * never callable native pointers. */
            CHECK(slot >= 8 && slot < sizeof(handler_slots));
            CHECK(address >= 0x8018efe8u && !(address & 3));
            size_t offset = address - 0x8018efe8u;
            size_t descriptor_size = slot < 85 ? 7 * 4 : 14 * 4;
            CHECK(offset <= room.data.size && descriptor_size <= room.data.size - offset);
            if (slot < 85) {
                for (unsigned method = 0; method < 7; ++method) {
                    u32 target = room_word(room.data.bytes + offset + method * 4);
                    CHECK(!(target & 3) && target >= 0x8018efe8u);
                    CHECK(target - 0x8018efe8u < room.data.size);
                    ++pm_methods;
                }
            }
            if(slot>=85) {
                u32 script_address=room_word(room.data.bytes+offset+52);
                CHECK(script_address>=0x8018efe8u);
                size_t cursor=script_address-0x8018efe8u;
                int stopped=0;
                for(unsigned instruction=0;instruction<256;++instruction,cursor+=6) {
                    CHECK(cursor<=room.data.size && 6<=room.data.size-cursor);
                    const u8 *code=room.data.bytes+cursor;
                    s16 op=(s16)(code[0]|(u16)code[1]<<8);
                    s16 a=(s16)(code[2]|(u16)code[3]<<8);
                    s16 b=(s16)(code[4]|(u16)code[5]<<8);
                    CHECK(op>=0 && op<=5 && op!=3);
                    if(op==1 || (op==4 && b==1)) CHECK(a>=0 && a<7);
                    if(op==2) CHECK(a>=0 && a<8);
                    ++effect_instructions;
                    if(op==0) { stopped=1; break; }
                }
                CHECK(stopped); ++effect_scripts;
            }
            if (slot == 69) {
                ++mirror_descriptors;
                CHECK(address == 0x8018fb44u);
                CHECK(map == 17 || map == 18 || map == 21 || map == 45 ||
                      map == 102 || map == 151 || map == 319);
                /* Historical M017 symbols: all seven entries, in PM order,
                 * agree with the original mirror_model.c handler table. */
                static const u32 mirror_methods[7] = {
                    0x8018eff4, 0x8018effc, 0x8018f020, 0x8018f0f0,
                    0x8018f1a4, 0x8018f1ac, 0x8018f1bc
                };
                for (unsigned method = 0; method < 7; ++method)
                    CHECK(room_word(room.data.bytes + offset + method * 4) == mirror_methods[method]);
            }
            CHECK((room_word(entry + 4) & 0xffffff) == 8);
            handler_slots[slot] = 1;
            ++handler_count;
            if (slot < 85) ++pm_count; else ++effect_count;
            unsigned variant = 0;
            while (variant < variant_count &&
                   (handler_variants[variant].slot != slot || handler_variants[variant].address != address)) ++variant;
            if (variant == variant_count) {
                CHECK(variant_count < sizeof(handler_variants) / sizeof(handler_variants[0]));
                handler_variants[variant].slot = slot;
                handler_variants[variant_count++].address = address;
            }
        }
        PeRoomBanks banks; char error[256];
        CHECK(pe_room_banks(&room,&banks,error,sizeof(error)));
        PeScriptProgram program;
        CHECK(pe_script_program_open(&room,&program,error,sizeof(error)));
        u8 *original=malloc(room.data.size); CHECK(original);
        memcpy(original,room.data.bytes,room.data.size);
        RelocBlock *block=Script_RelocBlock(room.data.bytes+program.block);
        CHECK(block && block==g_SceneDataTable1 && block->count==program.entries);
        CHECK(block->u0.base==room.data.bytes+program.map_list);
        for(unsigned entry=0;entry<program.entries;++entry) {
            uint32_t offset; CHECK(pe_script_entry(&program,entry,&offset));
            CHECK(block->offsets[entry]==room.data.bytes+offset);
        }
        CHECK(Script_RelocBlock(block)==block);
        CHECK(!memcmp(original,room.data.bytes,room.data.size));
        free(original); ++relocated;
        for(unsigned group=0;group<10;++group) {
            PeRoomSlice *root=&banks.roots[group]; if(!root->present) continue;
            PeModel parsed; CHECK(pe_model_open(root->bytes,root->size,&parsed,error,sizeof(error)));
            u8 *copy=malloc(root->size); CHECK(copy); memcpy(copy,root->bytes,root->size);
            g_SceneMapPrimBaseTable[1]=(uintptr_t)copy;
            for(unsigned full=0;full<2;++full) {
                Entity_ResetAllPools(); Entity_InitFreePool();
                Script_InitNodePool(); Script_InitNodeFreeList();
                u8 scene[]={1,7};
                FieldActor *actor=Scene_LoadMap(scene,NULL,full);
                CHECK(actor && actor==g_FieldActorListHead && actor->prev==NULL && actor->next==NULL);
                CHECK(actor->model_state.model_header==(RenderObjectHeader *)copy);
                CHECK(actor->allocation_block==(uintptr_t)memory && g_EntityFreePoolCount==1);
                CHECK(actor->render_object.primitive_buffer==memory+2*sizeof(RenderShadowQuad));
                CHECK(actor->script_node_lists[2] && actor->script_base==(u8 *)script);
                CHECK(actor->type_id==1 && actor->sub_id==7 && actor->move_speed==4096);
                CHECK(actor->render_object.draw_count==(int)full);
                if(!full) CHECK((actor->flags&0x600000a0)==0x600000a0);
                CHECK((u8 *)(actor->render_object.matrices+parsed.part_count)<=memory+
                      g_EntityAllocationTable[0].blockCount*ENTITY_ALLOCATION_BLOCK_SIZE);
                ++spawned;
            }
            free(copy);
        }
        pe_room_close(&room);
    }
    CHECK(pe_disc_read(&disc,&catalog.archive,(uint64_t)catalog.sector_bounds[2]*2048,
                      &g_ResidentDataPack,sizeof(g_ResidentDataPack)));
    ResetGraph(0);
    RenderNativeTag ordering[4096];
    D_800B0E38.ordering[0]=D_800B0E38.ordering[1]=ordering;
    unsigned players=0;
    for(unsigned bank=18;bank<=20;bank+=2) {
        PeRoomBlob blob={0}; PeRoomBanks banks; char error[256];
        CHECK(pe_sector_bank_load(&catalog,bank,&blob));
        CHECK(pe_entity_banks(&blob,1,&banks,error,sizeof(error)));
        CHECK(banks.roots[0].present && banks.rows[0][0x15].present);
        g_SceneMapPrimBaseTable[0]=(uintptr_t)banks.roots[0].bytes;
        for(unsigned action=0;action<48;++action)
            g_GameState.bank_rows[0][action]=(void *)banks.rows[0][action].bytes;
        scripts[2]=(u8 *)script;
        for(unsigned full=0;full<2;++full) {
            Entity_ResetAllPools(); Entity_InitFreePool();
            Script_InitNodePool(); Script_InitNodeFreeList();
            Battle_SaveAyaState();
            memset(&g_PlayerInventoryState,0,sizeof(g_PlayerInventoryState));
            Inv_BuildItemGridFromCategory();
            Inv_InitNewGameInventory();
            ClearOTagR((OT_TYPE *)ordering,4096);
            u8 scene[]={0,0};
            /* One-player scene descriptor, with a real native relocation view. */
            u32 task_block[8]={20,1,24,0,0,1,0,0};
            RelocBlock *tasks=Script_RelocBlock(task_block);
            CHECK(tasks);
            g_GameState.bank_reset_944[0]=(void *)tasks;
            FieldActor *actor;
            if(full) {
                Script_DrawSyncAndFlush();
                actor=g_FieldActorListHead;
            } else actor=Scene_LoadMap(scene,NULL,full);
            CHECK(actor && actor==g_PlayerEntity && actor==g_FieldActorListHead);
            CHECK((void *)actor->state==&g_AyaBattleState);
            CHECK(g_AyaBattleState.action==&g_SavedBattleStateTail);
            CHECK(actor->action_data==banks.rows[0][0x15].bytes && actor->mode==0x15);
            CHECK(actor->script_base==(u8 *)&task_block[6] && actor->script_node_lists[2]);
            CHECK(actor->move_factor==0x10000 && actor->render_object.matrices);
            CHECK(((Combatant *)((BattleEntity *)actor)->core)->maxHP==g_AyaSaveState.max_hp);
            CHECK(g_AyaBattleState.action->weapon.kind==Item_LookupBaseData(0x43)->kind);
            ++players;
        }
        g_PlayerEntity=NULL;
        pe_room_blob_close(&blob);
    }
    /* Actual New Game map 431 descriptor list and NPC metadata, with the day-one Aya bank. */
    CHECK(pe_room_load(&catalog,431,&room));
    PeRoomBlob aya_bank={0}; PeRoomBanks scene_banks, room_banks;
    PeScriptProgram scene_program; char scene_error[256];
    CHECK(pe_room_banks(&room,&room_banks,scene_error,sizeof(scene_error)));
    memset(&scene_banks,0,sizeof(scene_banks));
    CHECK(pe_sector_bank_load(&catalog,18,&aya_bank));
    CHECK(pe_entity_banks(&aya_bank,1,&scene_banks,scene_error,sizeof(scene_error)));
    for(unsigned group=0;group<10;++group) {
        if(room_banks.roots[group].present)scene_banks.roots[group]=room_banks.roots[group];
        for(unsigned action=0;action<48;++action) {
            if(room_banks.rows[group][action].present)scene_banks.rows[group][action]=room_banks.rows[group][action];
            g_GameState.bank_rows[group][action]=(void *)scene_banks.rows[group][action].bytes;
        }
        g_SceneMapPrimBaseTable[group]=(uintptr_t)scene_banks.roots[group].bytes;
    }
    CHECK(pe_script_program_open(&room,&scene_program,scene_error,sizeof(scene_error)));
    Entity_ResetAllPools(); Entity_InitFreePool();
    Script_InitNodePool(); Script_InitNodeFreeList();
    Battle_SaveAyaState();
    memset(&g_PlayerInventoryState,0,sizeof(g_PlayerInventoryState));
    Inv_BuildItemGridFromCategory(); Inv_InitNewGameInventory();
    g_LoadedSceneAssetBlock[0]=(SceneAssetBlob *)room.data.bytes;
    RelocBlock *scene_tasks=Script_RelocBlock(room.data.bytes+scene_program.block);
    CHECK(scene_tasks); g_GameState.bank_reset_944[0]=(void *)scene_tasks;
    ClearOTagR((OT_TYPE *)ordering,4096);
    Script_DrawSyncAndFlush();
    CHECK(g_EntityFreePoolCount==scene_program.actors);
    FieldActor *walk=g_FieldActorListHead;
    unsigned expected_players=0;
    for(unsigned n=scene_program.actors;n>0;--n) {
        unsigned entry,sub; CHECK(pe_script_actor(&scene_program,n-1,&entry,&sub));
        CHECK(walk && walk->type_id==entry && walk->sub_id==sub);
        if(entry==0) { ++expected_players; CHECK(walk==g_PlayerEntity); }
        CHECK(walk->script_base==scene_tasks->offsets[entry] && walk->script_node_lists[2]);
        if(walk->next)CHECK(walk->next->prev==walk);
        walk=walk->next;
    }
    CHECK(!walk && (g_PlayerEntity!=NULL)==(expected_players!=0));
    printf("Original New Game map 431 spawn list: %u initial descriptors matched (player presence follows data)\n",scene_program.actors);
    pe_room_blob_close(&aya_bank); pe_room_close(&room);
    printf("Original Scene_LoadMap: %u real Aya model spawns with original equipment initialization\n",players);
    unsigned slot_count = 0;
    for (unsigned slot = 0; slot < sizeof(handler_slots); ++slot) slot_count += handler_slots[slot];
    CHECK(handler_count == 479 && pm_count == 298 && effect_count == 181);
    CHECK(slot_count == 98 && variant_count == 212);
    CHECK(pm_methods == 2086 && mirror_descriptors == 7);
    printf("USA room handler data: %u records (%u PM, %u effects), %u slots, %u slot/address variants within room bounds; native dispatch not verified\n",
           handler_count, pm_count, effect_count, slot_count, variant_count);
    free(memory); g_RenderScratchBufferBase[0]=NULL; pe_disc_close(&disc);
    printf("Original script relocation: %u real room blocks, all entries and immutable asset bytes passed\n",relocated);
    CHECK(effect_scripts==181);
    printf("USA effect scripts: %u programs, %u bounded instructions, valid variable/task indices, no opcode 3\n",effect_scripts,effect_instructions);
    printf("Original Scene_LoadMap: %u NPC model spawns, full/non-full native allocations and script nodes\n",spawned);
    return 0;
}
