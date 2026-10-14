#include "pe1/render_setup.h"
#include "pe1/entity_frame_update.h"
#include "pe1/gte.h"
#include "pe1/render_lighting.h"
#include <libgpu.h>
#include "room_banks.h"
#include "model.h"
#include "vram.h"
#include "pe1/scene_assets.h"
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
/* Player equipment is outside this NPC spawn test; fail if called. */
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
    unsigned spawned=0;
    for(unsigned map=1;map<=PE_USA_ROOM_SLOTS;++map) {
        if(!catalog.ranges[map-1].room_sectors) continue;
        CHECK(pe_room_load(&catalog,map,&room));
        PeRoomBanks banks; char error[256];
        CHECK(pe_room_banks(&room,&banks,error,sizeof(error)));
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
    free(memory); g_RenderScratchBufferBase[0]=NULL; pe_disc_close(&disc);
    printf("Original Scene_LoadMap: %u NPC model spawns, full/non-full native allocations and script nodes\n",spawned);
    return 0;
}
