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
#include "animation.h"
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
#ifndef PE_TEST_REAL_DISC
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
#else
#include "host_disc.h"
int CdRom_ReadAssetSectors(unsigned int k,unsigned int i,int c,void *b,u32 m,int w) { abort(); }
#endif
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
#ifndef PE_TEST_REAL_DISC
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
#else
int main(void)
{
    const char *path=getenv("PE_TEST_DISC");
    if(!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
    PeDisc disc={0}; PeRoomCatalog catalog;
    CHECK(pe_disc_open(&disc,path) && pe_room_catalog_open(&catalog,&disc));
    pe_host_disc_bind(&disc,&catalog.archive);
    g_PeImageBaseLba=catalog.archive.sector;
    for(unsigned i=0;i<64;++i) {
        g_PeImageSectorTable[i]=catalog.sector_bounds[i];
        g_PeImageSectorBoundsNext[i]=catalog.sector_bounds[i+1];
    }
    PeRoomBlob resident={0};
    CHECK(pe_sector_bank_load(&catalog,12,&resident));
    RenderObjectHeader *scene_model=Asset_FindTable08ByU32Key(resident.bytes,(int)0xC4B5BA04u);
    CHECK(scene_model && (u8 *)scene_model>=resident.bytes && (u8 *)scene_model<resident.bytes+resident.size);
    PeModel parsed_model; char model_error[256];
    CHECK(pe_model_open((u8 *)scene_model,resident.size-((u8 *)scene_model-resident.bytes),&parsed_model,model_error,sizeof(model_error)));
    CHECK(parsed_model.part_count>=2);
    size_t model_work_bytes=2*(parsed_model.packets[0]*sizeof(RenderPacket34)+parsed_model.packets[1]*sizeof(RenderPacket28)+
        parsed_model.packets[2]*sizeof(RenderPacket24)+parsed_model.packets[3]*sizeof(RenderPacket1C))+
        parsed_model.part_count*(sizeof(RenderAnimationLookupEntry)+sizeof(RenderMatrix));
    u8 *model_work=calloc(1,model_work_bytes); CHECK(model_work);
    FieldActor player={0};
    unsigned animated_frames=0;
    unsigned loaded=0, uploads=0;
    PeVram *expected=calloc(1,sizeof(*expected)), *actual=malloc(sizeof(*actual));
    CHECK(expected && actual);
    ResetGraph(0);
    RECT all={0,0,1024,512};
    for(unsigned area=10;area<=12;area+=2) for(unsigned day=1;day<=7;++day) {
        unsigned index=(area-10)/2*8+day+0x16;
        PeRoomBlob reference={0};
        CHECK(pe_sector_bank_load(&catalog,index,&reference));
        size_t read_bytes=(size_t)(catalog.sector_bounds[index+1]-catalog.sector_bounds[index])*2048u;
        CHECK(read_bytes>=reference.size);
        u8 *buffer=calloc(1,read_bytes); CHECK(buffer);
        g_SceneAreaType=area; g_CurrentStoryDay[0]=day;
        g_GameState.requested_entity_bank=area;
        g_GameState.entity_texture_blob=(SceneAssetView *)buffer;
        g_GameState.story_day_flags=1; g_GameState.scene_init_subphase=0;
        PeRoomBlob tim_reference={0}; u8 *scratch_buffer=NULL;
        memset(expected,0,sizeof(*expected));
        LoadImage(&all,(u_long *)expected->words); DrawSync(0);
        if(day>=2 && day<=6) {
            unsigned tim_index=day+0x26;
            CHECK(pe_sector_bank_load(&catalog,tim_index,&tim_reference));
            size_t scratch_bytes=(size_t)(catalog.sector_bounds[tim_index+1]-catalog.sector_bounds[tim_index])*2048u;
            CHECK(scratch_bytes>=tim_reference.size);
            scratch_buffer=malloc(scratch_bytes); CHECK(scratch_buffer);
            g_GameState.scene_load_scratch=(SceneAssetView *)scratch_buffer;
            PeVramStats stats; char error[256];
            CHECK(pe_blob_upload_vram(&tim_reference,expected,&stats,error,sizeof(error)));
            uploads+=stats.uploads;
            CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==2);
            CHECK(!memcmp(scratch_buffer,tim_reference.bytes,tim_reference.size));
        }
        CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==5);
        CHECK((g_GameState.flags&0x1004000u)==0x1004000u);
        CHECK(!memcmp(buffer,reference.bytes,reference.size));
        CHECK(Scene_LoadEntityTextures()==0 && g_GameState.scene_init_subphase==0);
        CHECK(!(g_GameState.flags&0x1004000u));
        const u8 *records; unsigned count;
        CHECK(pe_room_blob_table(&reference,4,sizeof(SceneBankAssetRecord),&records,&count));
        for(unsigned i=0;i<count;++i) {
            SceneBankAssetRecord row; memcpy(&row,records+i*sizeof(row),sizeof(row));
            CHECK(g_GameState.bank_rows[0][row.source.bytes.id]==buffer+(row.source.offsetAndId&0xffffff));
        }
        DrawSync(0); StoreImage(&all,(u_long *)actual->words); DrawSync(0);
        CHECK(!memcmp(expected,actual,sizeof(*expected)));
        PeRoomBlob player_bank={0}; PeRoomBanks player_banks={0};
        CHECK(pe_sector_bank_load(&catalog,area==10?18:20,&player_bank));
        CHECK(pe_entity_banks(&player_bank,1,&player_banks,model_error,sizeof(model_error)));
        CHECK(pe_entity_banks(&reference,0,&player_banks,model_error,sizeof(model_error)));
        PeModel player_model;
        CHECK(pe_model_open(player_banks.roots[0].bytes,player_banks.roots[0].size,&player_model,model_error,sizeof(model_error)));
        CHECK(player_model.part_count>18);
        size_t player_work_bytes=2*(player_model.packets[0]*sizeof(RenderPacket34)+player_model.packets[1]*sizeof(RenderPacket28)+
            player_model.packets[2]*sizeof(RenderPacket24)+player_model.packets[3]*sizeof(RenderPacket1C))+
            player_model.part_count*(sizeof(RenderAnimationLookupEntry)+sizeof(RenderMatrix));
        u8 *player_work=calloc(1,player_work_bytes); CHECK(player_work);
        s8 *texture_data=NULL;
        memset(&player,0,sizeof(player));
        CHECK(Render_SetupEntityPrims(&player.render_object,(RenderObjectHeader *)player_banks.roots[0].bytes,
            player_work,0,0,0,0,0,&texture_data,1)==1);
        RenderMatrix *pose=player.render_object.matrices;
        /* Rebuild with the real player model, then follow every bank action. */
        g_PlayerEntity=&player;
        g_GameState.scene_object_model=scene_model;
        g_GameState.scene_object_work=model_work;
        g_GameState.scene_init_subphase=11;
        CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==12);
        CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==13);
        CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==0);
        CHECK(g_GameState.scene_object.animation_source==&player.render_object);
        CHECK(g_GameState.scene_object.animation_state==3 && g_GameState.scene_object.animation_id==18);
        CHECK(!memcmp(&g_GameState.scene_object.matrices[0],&pose[18],sizeof(RenderMatrix)));
        CHECK(!memcmp(&g_GameState.scene_object.matrices[1],&pose[18],sizeof(RenderMatrix)));
        CHECK(!(g_GameState.story_day_flags&3));
        player.render_object.variant_visible=0;
        for(unsigned action=0;action<48;++action) {
            PeRoomSlice *slice=&player_banks.rows[0][action];
            if(!slice->present) continue;
            PeAnimation animation;
            CHECK(pe_animation_open(slice->bytes,slice->size,&animation,model_error,sizeof(model_error)));
            CHECK(animation.bones<=player_model.part_count);
            for(unsigned frame=0;frame<(animation.frames?animation.frames:1);++frame) {
                Anim_BuildRotationMatrices(&player.render_object,(RenderAnimationDataHeader *)slice->bytes,frame);
                Render_TransformVertices(&player.render_object);
                CHECK(Scene_UpdateBgDraw()==0);
                CHECK(!memcmp(&g_GameState.scene_object.matrices[0],&pose[18],sizeof(RenderMatrix)));
                CHECK(!memcmp(&g_GameState.scene_object.matrices[1],&pose[18],sizeof(RenderMatrix)));
                CHECK(Psyz_GteCtrlRead(24)==(160u<<16) && Psyz_GteCtrlRead(25)==(112u<<16));
                ++animated_frames;
            }
        }
        free(player_work); pe_room_blob_close(&player_bank);
        g_PlayerEntity=NULL;
        free(scratch_buffer); pe_room_blob_close(&tim_reference);
        g_GameState.scene_load_scratch=NULL;
        free(buffer); pe_room_blob_close(&reference); ++loaded;
    }
    free(model_work); pe_room_blob_close(&resident);
    free(expected); free(actual);
    unsigned char guard=0xa5;
    CHECK(CdRom_ReadSectorsFromLba(catalog.archive.sector-1,&guard,1)==-1 && guard==0xa5);
    CHECK(CdRom_ReadSectorsFromLba(catalog.archive.sector,&guard,UINT32_MAX)==-1 && guard==0xa5);
    pe_host_disc_bind(NULL,NULL);
    CHECK(CdRom_ReadSectorsFromLba(catalog.archive.sector,&guard,1)==-1 && guard==0xa5);
    pe_disc_close(&disc);
    printf("Original entity loader: %u real area/day banks, %u TIM uploads with full VRAM comparison, 14 model rebuilds, %u real animation attachment frames\n",loaded,uploads,animated_frames);
    return 0;
}
#endif
