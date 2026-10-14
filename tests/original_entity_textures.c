#include "pe1/render_setup.h"
#include "pe1/entity_frame_update.h"
#include "pe1/gte.h"
#include "pe1/render_lighting.h"
#include <libgpu.h>
#include "pe1/scene_entity_textures.h"
#include "pe1/geom_state.h"
GeomState * volatile g_GeomState;
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

u8 g_SceneAreaType;
s8 g_CurrentStoryDay[1];
u32 g_PeImageBaseLba, g_EntityRenderScratch[8];
u16 g_PeImageSectorTable[64], g_PeImageSectorBoundsNext[64];
static int read_result, poll_result;
static unsigned reads;
static u32 last_lba,last_size;
static void *last_destination;
/* Scripted I/O boundary only: game state transitions and relocation are original. */
int CdRom_ReadSectorsFromLba(u32 lba,void *destination,u32 size) {
    ++reads; last_lba=lba; last_destination=destination; last_size=size;
    return read_result;
}
int CdRom_PollReady(void) { return poll_result; }
int CdRom_ReadAssetSectors(unsigned int k,unsigned int i,int c,void *b,u32 m,int w) { abort(); }
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main(void)
{
    struct Bank {
        SceneAssetBlob header;
        SceneAssetDirectory directory;
        SceneBankAssetRecord rows[2];
        SceneCdStreamRecord streams[3];
        u32 payload[5];
    } bank={0}, scratch={0};
    bank.header.directoryOffset=offsetof(struct Bank,directory);
    scratch.header.directoryOffset=offsetof(struct Bank,directory);
    bank.directory.bankRowEntries=(2u<<22)|offsetof(struct Bank,rows);
    bank.directory.streamEntries=(3u<<22)|offsetof(struct Bank,streams);
    for(unsigned i=0;i<2;++i) bank.rows[i].source.offsetAndId=((i+3)<<24)|(offsetof(struct Bank,payload)+i*4);
    for(unsigned i=0;i<3;++i) bank.streams[i].offset=offsetof(struct Bank,payload)+(i+2)*4;
    bank.streams[0].cdIndex=9;
    g_SceneAreaType=10; g_CurrentStoryDay[0]=2; g_PeImageBaseLba=1000;
    for(unsigned i=0;i<64;++i) { g_PeImageSectorTable[i]=i*10; g_PeImageSectorBoundsNext[i]=i*10+5; }
    g_GameState.requested_entity_bank=10;
    g_GameState.scene_load_scratch=(SceneAssetView *)&scratch;
    g_GameState.entity_texture_blob=(SceneAssetView *)&bank;
    g_GameState.story_day_flags=1;
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==2);
    CHECK(reads==1 && last_lba==1400 && last_size==5 && last_destination==&scratch);
    poll_result=1;
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==2);
    poll_result=-1;
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==1);
    read_result=-1;
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==1);
    read_result=0; poll_result=0;
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==2);
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==5);
    CHECK(last_lba==1240 && last_size==5 && last_destination==&bank);
    CHECK(Scene_LoadEntityTextures()==0 && g_GameState.scene_init_subphase==0);
    CHECK(!(g_GameState.story_day_flags&3) && g_GameState.cd_range_read_mode==9);
    for(unsigned i=0;i<2;++i) CHECK(g_GameState.bank_rows[0][i+3]==&bank.payload[i]);
    for(unsigned i=0;i<3;++i) CHECK(g_GameState.scene_object_tables[i]==&bank.payload[i+2]);
    g_GameState.story_day_flags=2;
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==12);
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==13);
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==13);
    g_GameState.flags=2;
    CHECK(Scene_LoadEntityTextures()==0 && g_GameState.scene_init_subphase==13);
    g_SceneAreaType=9;
    CHECK(Scene_LoadEntityTextures()==0 && g_GameState.scene_init_subphase==13);
    puts("Original entity texture loader: retry/poll transitions and native bank/stream relocation passed");
    return 0;
}
