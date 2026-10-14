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
#include "pe1/battle_saved_state.h"
#include "pe1/resident_tables.h"
#include "pe1/inventory_slots.h"
#include "pe1/player_entity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int g_ActiveDrawSlot;
RenderLightingMatrix g_RenderLightingState, g_RenderLightColorMatrix;
RenderLightColor g_RoomLightTint = {255,255,255};
RenderObjectEntity g_PlayerBattleEntity;
s32 g_IdentityMatrixSource[8] = {4096,0,4096,0,4096,0,0,0};
s16 g_RenderTexturePageDeltas[4][4];
int g_RenderCounter[1];
RenderVec3s g_RenderNormalTable[240];
RenderBufferPrefix D_800B0E38;

#include "pe1/entity_allocation.h"
#include "pe1/render_buffers.h"
#include "pe1/script_state.h"
#include "pe1/scene_archive.h"
uintptr_t g_SceneMapPrimBaseTable[10];
FieldActorCallbacks g_FieldActorTypeCallbacks[256];
SceneAssetBlob *g_LoadedSceneAssetBlock[1];
u8 g_SceneAreaType;
u32 g_EntityRenderScratch[9];

#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
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
    for(unsigned map=1;map<=PE_USA_ROOM_SLOTS;++map) {
        if(!catalog.ranges[map-1].room_sectors) continue;
        CHECK(pe_room_load(&catalog,map,&room));
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
    printf("Original Scene_LoadMap: %u real Aya model spawns with original equipment initialization\n",players);
    free(memory); g_RenderScratchBufferBase[0]=NULL; pe_disc_close(&disc);
    printf("Original script relocation: %u real room blocks, all entries and immutable asset bytes passed\n",relocated);
    printf("Original Scene_LoadMap: %u NPC model spawns, full/non-full native allocations and script nodes\n",spawned);
    return 0;
}
