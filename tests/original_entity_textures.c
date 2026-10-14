#include "pe1/render_setup.h"
#include "pe1/entity_frame_update.h"
#include "pe1/gte.h"
#include "pe1/render_lighting.h"
#include <libgpu.h>
#include "pe1/scene_room_load.h"
#include "pe1/geom_state.h"
#include "room_banks.h"
#include "model.h"
#include "animation.h"
#include "vram.h"
#include "pe1/scene_assets.h"
#include "pe1/pe_image.h"
#include "pe1/scene_room.h"
#include "pe1/room_fx_model.h"
#include "pe1/render_matrix_slot.h"
#include "pe1/render_camera.h"
#include "pe1/render_shadow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RenderLightColor g_RoomLightTint = {255,255,255};
RenderObjectEntity g_PlayerBattleEntity;
s32 g_IdentityMatrixSource[8] = {4096,0,4096,0,4096,0,0,0};
s16 g_RenderTexturePageDeltas[4][4];
int g_RenderCounter[1];
RenderVec3s g_RenderNormalTable[240];

u8 g_SceneAreaType;
/* Controlled battle-step input for the original loading gate. */
u8 g_AyaActionStep;
u16 g_PeImageSectorTable[64], g_PeImageSectorBoundsNext[64];
#ifndef PE_TEST_REAL_DISC
static int read_result, poll_result;
static unsigned reads, polls;
static u32 last_lba,last_size;
static void *last_destination;
/* Scripted I/O boundary only: game state transitions and relocation are original. */
int CdRom_ReadSectorsFromLba(u32 lba,void *destination,u32 size) {
    ++reads; last_lba=lba; last_destination=destination; last_size=size;
    return read_result;
}
int CdRom_PollReady(void) { ++polls; return poll_result; }
int CdRom_ReadAssetSectors(unsigned int k,unsigned int i,int c,void *b,u32 m,int w) { abort(); }
#else
#include "pe1/cdrom.h"
#include "pe1/akao.h"
#include "pe1/akao/stream.h"
#include "pe1/akao/globals.h"
#include <psyz/spu.h>
#include "pe1/psyq_spu_api.h"
#endif
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
#ifdef PE_TEST_REAL_DISC
SceneRoomSectorRange g_RoomSectorTable[PE_USA_ROOM_SLOTS];
AkaoVoiceBankEntry g_AkaoVoiceBankEntries[20];
static u32 image_word(const u8 *p)
{
    return (u32)p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24;
}
static int room_key_images(const PeRoomBlob *room, PeVram *expected, unsigned *uploads)
{
    const u8 *records; unsigned count;
    CHECK(pe_room_blob_table(room, 2, 12, &records, &count));
    for (u32 key = 0x73decd80; ; key += 4) {
        unsigned i = 0;
        while (i < count && image_word(records + i * 12 + 8) != key) ++i;
        if (i == count) break;
        size_t image = image_word(records + i * 12 + 4) & 0xffffff;
        CHECK(image <= room->size && room->size - image >= 12);
        u32 source = image_word(room->bytes + image + 4);
        u32 geometry = image_word(room->bytes + image + 8);
        size_t pixels = image + (source & 0xffffff);
        unsigned width = geometry & 1023, height = source >> 24;
        if (!height) height = 256;
        unsigned x = (geometry >> 10) & 2047, y = geometry >> 21;
        CHECK(x <= 1024 && y <= 512 && width <= 1024 - x && height <= 512 - y);
        CHECK(pixels <= room->size && (size_t)width * height * 2 <= room->size - pixels);
        for (unsigned row = 0; row < height; ++row)
            for (unsigned col = 0; col < width; ++col) {
                const u8 *p = room->bytes + pixels + 2 * ((size_t)row * width + col);
                expected->words[(y + row) * 1024 + x + col] = p[0] | (u16)p[1] << 8;
            }
        ++*uploads;
    }
    return 0;
}
#endif
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
    g_GameState.scene_load_scratch=(u8 *)&scratch;
    g_GameState.entity_texture_blob=(SceneAssetView *)&bank;
    g_GameState.story_day_flags=1;
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==2);
    CHECK(reads==1 && last_lba==1400 && last_size==5 && last_destination==&scratch);
    poll_result=1;
    CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==2);
    poll_result=-1;
    CHECK(Scene_LoadEntityTextures()==-1 && g_GameState.scene_init_subphase==1);
    read_result=-1;
    CHECK(Scene_LoadEntityTextures()==-1 && g_GameState.scene_init_subphase==1);
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
    /* The original row byte indexes the flattened native table, including
     * the next group and the highest representable id. */
    const unsigned row_ids[2][2]={{47,48},{0,255}};
    bank.directory.bankRootEntries=(1u<<22)|offsetof(struct Bank,rows);
    g_GameState.voice_bank_base_1400=(u8 *)&bank;
    for(unsigned pair=0;pair<2;++pair) {
        memset(g_GameState.bank_rows,0,sizeof(g_GameState.bank_rows));
        for(unsigned row=0;row<2;++row)
            bank.rows[row].source.offsetAndId=(row_ids[pair][row]<<24)|(offsetof(struct Bank,payload)+row*4);
        g_GameState.flags=0x200000;
        g_GameState.entity_texture_phase=6;
        CHECK(Scene_LoadEntityTexture()==0 && g_GameState.entity_texture_phase==0);
        CHECK(g_GameState.bank_slots[0]==&bank.payload[0]);
        for(unsigned row=0;row<2;++row)
            CHECK(g_GameState.bank_rows[row_ids[pair][row]/48][row_ids[pair][row]%48]==&bank.payload[row]);
        CHECK(!(g_GameState.flags&0x200000));
    }
    g_GameState.flags=0x220000;
    g_GameState.entity_texture_phase=2;
    g_GameState.scene_load_scratch=(u8 *)&scratch;
    poll_result=0; read_result=0; g_AyaActionStep=0;
    CHECK(Scene_LoadEntityTexture()==1 && g_GameState.entity_texture_phase==2);
    CHECK(!(g_GameState.flags&0x40000));
    g_AyaActionStep=2;
    CHECK(Scene_LoadEntityTexture()==1 && g_GameState.entity_texture_phase==5);
    CHECK(g_GameState.flags&0x40000);
    CHECK(Scene_LoadEntityTexture()==0 && g_GameState.entity_texture_phase==0);
    /* Native image errors remain visible through both entity loaders and
     * their original player-initialization caller. Pending polls still wait. */
    g_SceneAreaType=10;
    const unsigned base_read_phases[]={1,4};
    const unsigned base_poll_phases[]={2,5};
    for(unsigned test=0;test<2;++test) {
        g_GameState.entity_texture_phase=base_read_phases[test];
        read_result=-1;
        CHECK(Scene_LoadEntityTexture()==-1 && g_GameState.entity_texture_phase==base_read_phases[test]);
        read_result=0;
        CHECK(Scene_LoadEntityTexture()==1 && g_GameState.entity_texture_phase==base_poll_phases[test]);
        poll_result=1;
        CHECK(Scene_LoadEntityTexture()==1 && g_GameState.entity_texture_phase==base_poll_phases[test]);
        poll_result=-1;
        CHECK(Scene_LoadEntityTexture()==-1 && g_GameState.entity_texture_phase==base_read_phases[test]);
    }
    const unsigned day_read_phases[]={1,4};
    const unsigned day_poll_phases[]={2,5};
    for(unsigned test=0;test<2;++test) {
        g_GameState.scene_init_subphase=day_read_phases[test];
        read_result=-1;
        CHECK(Scene_LoadEntityTextures()==-1 && g_GameState.scene_init_subphase==day_read_phases[test]);
        read_result=0;
        CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==day_poll_phases[test]);
        poll_result=1;
        CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==day_poll_phases[test]);
        poll_result=-1;
        CHECK(Scene_LoadEntityTextures()==-1 && g_GameState.scene_init_subphase==day_read_phases[test]);
    }
    read_result=-1; poll_result=0;
    g_GameState.scene_init_phase=36; g_GameState.entity_texture_phase=1;
    CHECK(Scene_InitEntityPlayer(0)==-1 && g_GameState.scene_init_phase==36);
    g_GameState.scene_init_phase=38; g_GameState.scene_init_subphase=1;
    CHECK(Scene_InitEntityPlayer(0)==-1 && g_GameState.scene_init_phase==38);
    read_result=0;
    puts("Original entity initialization: read/poll errors propagated, retry phases retained, pending polls preserved");
    /* Only I/O outcomes are scripted; run the original TIM state machine. */
    g_GameState.loaded_scene_assets = (SceneAssetView *)&bank;
    g_GameState.scene_load_scratch = (u8 *)&scratch;
    g_GameState.pe_image_base_lba = 2000;
    for (unsigned force = 0; force < 2; ++force)
        for (unsigned requested = 0; requested < 2; ++requested)
            for (unsigned pending = 0; pending < 2; ++pending) {
                g_GameStateFlags = 0x80 | (requested ? 2 : 0);
                g_GameState.flags = 0x400 | (pending ? 8 : 0);
                g_GameState.tim_load_state = 0;
                reads = polls = 0; read_result = -1; poll_result = 1;
                int needs_read = pending && (force || requested);
                CHECK(Asset_LoadTimTextures(force) == (needs_read ? -1 : 0));
                CHECK(reads == (unsigned)needs_read && polls == 0);
                if (!needs_read) {
                    CHECK(g_GameState.tim_load_state == 0);
                    CHECK(g_GameState.flags == (0x400u | (pending ? 8u : 0u)));
                    continue;
                }
                CHECK(g_GameState.tim_load_state == 0x34);
                CHECK(last_lba == 2050 && last_size == 10 && last_destination == &scratch);
                CHECK(Asset_LoadTimTextures(force) == -1 && reads == 2 && polls == 0);
                read_result = 0;
                CHECK(Asset_LoadTimTextures(force) == 1 && g_GameState.tim_load_state == 0x35);
                CHECK(Asset_LoadTimTextures(force) == 1 && reads == 3 && polls == 1);
                CHECK(g_GameState.flags == 0x408);
                poll_result = -1;
                CHECK(Asset_LoadTimTextures(force) == -1 && g_GameState.tim_load_state == 0x34);
                CHECK(reads == 3 && polls == 2 && g_GameState.flags == 0x408);
                CHECK(Asset_LoadTimTextures(force) == 1 && reads == 4);
                poll_result = 0;
                CHECK(Asset_LoadTimTextures(force) == 0 && polls == 3);
                CHECK(g_GameState.tim_load_state == 0 && g_GameState.flags == 0x400);
                CHECK(Asset_LoadTimTextures(force) == 0 && reads == 4 && polls == 3);
            }
    puts("Original common TIM loader: request gates, read failure, pending poll, retry and completion flags passed");
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
    CdRom_BindImage(NULL,NULL);
    CHECK(Asset_OpenPeImage() == -1 && g_PeImageMountFlags == 0);
    PeDisc closed={0};
    CdRom_BindImage(&closed,NULL);
    CHECK(Asset_OpenPeImage() == -1 && g_PeImageMountFlags == 0);
    PeDisc invalid=disc;
    invalid.root.size=0;
    CdRom_BindImage(&invalid,NULL);
    CHECK(Asset_OpenPeImage() == -2 && g_PeImageMountFlags == 0);
    CdRom_BindImage(&disc,NULL);
    CHECK(Asset_OpenPeImage() == 0);
    CHECK(g_PeImageMountFlags == 1 && g_PeImageBaseLba == catalog.archive.sector);
    CHECK(&g_PeImageBaseLba == &g_GameState.pe_image_base_lba);
    CHECK(&g_PeImageMountFlags == &g_GameState.display_list_modes);
    memcpy(g_SceneSectorDirectory,&catalog.audio_base,4);
    memcpy(g_SceneSectorDirectory+4,catalog.audio_bounds,sizeof(catalog.audio_bounds));
    Psyz_SpuInit();
    SpuInit();
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
    unsigned native_rooms=0, native_geometry_entries=0;
    int new_game_room_loaded=0;
    size_t native_packet_bytes=0;
    for (unsigned map=1;map<=PE_USA_ROOM_SLOTS;++map) {
        if(!catalog.ranges[map-1].room_sectors) continue;
        PeRoom room = {0};
        CHECK(pe_room_load(&catalog,map,&room));
        unsigned descriptor_count=room.data.directory[1]>>22;
        int mirror=map==17 || map==18 || map==21 || map==45 || map==102 || map==151 || map==319;
        if(descriptor_count && !mirror && map!=5) { pe_room_close(&room); continue; }
        PeRoomRange *range = &catalog.ranges[map - 1];
        g_RoomSectorTable[map - 1].start = range->start;
        g_RoomSectorTable[map - 1].scratchSectors = range->scratch_sectors;
        g_RoomSectorTable[map - 1].textureSectors = range->texture_sectors;
        g_RoomSectorTable[map - 1].roomSectors = range->room_sectors;
        u8 *scratch = calloc(range->scratch_sectors, 2048);
        u8 *textures = calloc(range->texture_sectors, 2048);
        u8 *data = calloc(range->room_sectors, 2048);
        CHECK(scratch && textures && data);
        u8 *spu_expected=malloc(0x80000); CHECK(spu_expected);
        memset(spu_expected,0xa5,0x80000);
        memset(Psyz_SpuGetRam(),0xa5,0x80000);
        Pe1GameState saved = g_GameState;
        PmTables tables = g_PmTables;
        PmCommand *handlers[85] = {0};
        g_PmTables.handlers = handlers;
        g_GameState.scene_load_scratch = scratch;
        g_GameState.texture_load_scratch = (SceneAssetView *)textures;
        g_GameState.loaded_scene_assets = (SceneAssetView *)data;
        g_GameState.pe_image_base_lba = catalog.archive.sector;
        /* First load starts with empty audio caches and uses original uploads. */
        g_GameState.pending_stream_banks[0] = g_GameState.pending_stream_banks[1] = 0;
        g_GameState.pending_sample_bank = -1;
        const u8 *streams; unsigned count;
        CHECK(pe_room_blob_table(&room.data, 9, 8, &streams, &count));
        for (unsigned i = 0; i < count; ++i) {
            const u8 *entry = streams + i * 8;
            if (entry[3] & 0x10) continue;
            u16 bank = entry[4] | (u16)entry[5] << 8;
            if (entry[3] & 0x20) {
                u16 channel = entry[6] | (u16)entry[7] << 8;
                CHECK(channel < 2 && bank <= 255);
                /* The loader fills the stream cache from the disc image. */
            }
        }
        char name[8], encoded[8];
        snprintf(name, sizeof(name), "m0%03u0", map);
        unsigned room_id = Text_ParseBase32Id((signed char *)name);
        Text_EncodeBase32(encoded, room_id);
        CHECK(encoded[0] == 'M' && Text_ParseMapNumber((signed char *)encoded) == (int)map);
        g_GameState.bank_slots[0] = scratch;
        if (map == 5) {
            CHECK(Scene_LoadRoom(room_id) == -2);
            CHECK(!handlers[69] && g_GameState.bank_slots[0] == scratch);
        } else {
            CHECK(Scene_LoadRoom(room_id) == 0);
            CHECK(handlers[69] == (mirror ? &g_RoomFxMirrorHandlers : NULL));
            CHECK(g_GameState.cd_read_phase==0 && g_CdReadRemainingSectors==0);
            for(unsigned i=0;i<count;++i) {
                const u8 *entry=streams+i*8;
                if(entry[3]&0x10)continue;
                unsigned bank=entry[4]|(unsigned)entry[5]<<8;
                unsigned channel=entry[6]|(unsigned)entry[7]<<8;
                u8 *bytes=NULL;size_t size=0;
                CHECK(pe_audio_bank_read(&catalog,bank,&bytes,&size));
                CHECK(size>=0x40 && size<=(size_t)range->scratch_sectors*2048);
                unsigned first=image_word(bytes+0x18),end=image_word(bytes+0x1c);
                if(!end)end=256;
                CHECK(first<end && end<=256);
                unsigned instruments=end-first,payload=image_word(bytes+0x14);
                size_t offset=0x40+(size_t)instruments*0x40;
                CHECK(offset<=size && payload<=size-offset);
                unsigned address=image_word(bytes+0x10);
                if(entry[3]&0x20) {
                    address=0x68000+(channel<<13);
                    CHECK(channel<2 && instruments<=16);
                    CHECK(g_GameState.pending_stream_banks[channel]==bank);
                    unsigned delta=address-image_word(bytes+0x40);
                    for(unsigned instrument=0;instrument<instruments;++instrument) {
                        AkaoInstrument expected_instrument;
                        memcpy(&expected_instrument,bytes+0x40+instrument*0x40,0x40);
                        expected_instrument.start_address+=delta;
                        expected_instrument.loop_address+=delta;
                        CHECK(!memcmp(&g_AkaoStreamInstrumentCacheB[channel*16+instrument],
                                      &expected_instrument,sizeof(expected_instrument)));
                    }
                } else {
                    CHECK(g_GameState.pending_sample_bank==(int)bank);
                    CHECK(!memcmp(&g_AkaoInstrumentTable[first],bytes+0x40,instruments*0x40));
                }
                CHECK(address<=0x80000 && payload<=0x80000-address);
                memcpy(spu_expected+address,bytes+offset,payload);
                free(bytes);
            }
            CHECK(!memcmp(Psyz_SpuGetRam(),spu_expected,0x80000));
            /* Same bank IDs must take the original cache-hit path. */
            memset(spu_expected,0x6d,0x80000);
            memset(Psyz_SpuGetRam(),0x6d,0x80000);
            CHECK(Scene_LoadRoom(room_id)==0);
            CHECK(!memcmp(Psyz_SpuGetRam(),spu_expected,0x80000));
            CHECK(g_GameState.bank_reset_944[0] != NULL);
            if(map==17) {
                /* Restrict the borrowed archive extent at each read boundary.
                 * A file error must return, not spin waiting for a drive. */
                for(unsigned stage=0;stage<3;++stage) {
                    PeDiscFile limited=catalog.archive;
                    size_t limit=range->start;
                    if(stage>0) limit+=range->scratch_sectors;
                    if(stage>1) limit+=range->texture_sectors;
                    limited.size=(u32)(limit*2048);
                    Pe1GameState previous=g_GameState;
                    CdRom_BindImage(&disc,&limited);
                    CHECK(Scene_LoadRoom(room_id)==-1);
                    CHECK(g_GameState.geometry_state==previous.geometry_state);
                    CHECK(!memcmp(g_GameState.bank_slots,previous.bank_slots,sizeof(previous.bank_slots)));
                    CdRom_BindImage(&disc,&catalog.archive);
                    CHECK(Scene_LoadRoom(room_id)==0);
                }
                Pe1GameState before_invalid=g_GameState;
                const char *invalid_rooms[]={"m00000","m04510","m04500"};
                for(unsigned invalid=0;invalid<3;++invalid)
                    CHECK(Scene_LoadRoom(Text_ParseBase32Id((signed char *)invalid_rooms[invalid]))==-1);
                CHECK(g_GameState.geometry_state==before_invalid.geometry_state);
                g_GameState.pe_image_base_lba=UINT32_MAX;
                CHECK(Scene_LoadRoom(room_id)==-1);
                g_GameState.pe_image_base_lba=before_invalid.pe_image_base_lba;
                CdRom_BindImage(NULL,NULL);
                CHECK(Scene_LoadRoom(room_id)==-1);
                CdRom_BindImage(&disc,&catalog.archive);
                CHECK(Scene_LoadRoom(room_id)==0);
                puts("Original Scene_LoadRoom: three extent failures, invalid/empty room IDs, LBA overflow, unmount and reload recovery passed");
                /* Corrupt a temporary copy, never the user's disc image. */
                size_t image_bytes=(size_t)(range->scratch_sectors+range->texture_sectors+range->room_sectors)*2048;
                u8 *bad=malloc(image_bytes);CHECK(bad);
                CHECK(pe_disc_read(&disc,&catalog.archive,(uint64_t)range->start*2048,bad,image_bytes));
                const u8 *aux;unsigned aux_count;
                CHECK(pe_room_blob_table(&room.data,7,8,&aux,&aux_count) && aux_count);
                size_t geometry_offset=image_word(aux+4)&0xffffff;
                u32 invalid_entries=0x7fffffff;
                memcpy(bad+(size_t)(range->scratch_sectors+range->texture_sectors)*2048+geometry_offset+20,
                       &invalid_entries,sizeof(invalid_entries));
                FILE *temporary=tmpfile();CHECK(temporary);
                CHECK(fwrite(bad,1,image_bytes,temporary)==image_bytes && fflush(temporary)==0);
                PeDisc bad_disc={.stream=temporary,.image_size=image_bytes,.sector_size=2048};
                PeDiscFile bad_archive={.size=(u32)image_bytes};
                SceneRoomSectorRange original_range=g_RoomSectorTable[map-1];
                Pe1GameState previous=g_GameState;
                g_RoomSectorTable[map-1].start=0;
                g_GameState.pe_image_base_lba=0;
                CdRom_BindImage(&bad_disc,&bad_archive);
                CHECK(Scene_LoadRoom(room_id)==-3);
                CHECK(g_GameState.geometry_state==previous.geometry_state);
                CHECK(!memcmp(g_GameState.bank_slots,previous.bank_slots,sizeof(previous.bank_slots)));
                CdRom_BindImage(&disc,&catalog.archive);
                g_RoomSectorTable[map-1]=original_range;
                g_GameState.pe_image_base_lba=previous.pe_image_base_lba;
                fclose(temporary);free(bad);
                CHECK(Scene_LoadRoom(room_id)==0);
            }
            const u8 *geometry_records;unsigned geometry_count;
            CHECK(pe_room_blob_table(&room.data,7,8,&geometry_records,&geometry_count));
            CHECK(geometry_count>0);
            unsigned geometry_offset=image_word(geometry_records+4)&0xffffff;
            unsigned geometry_size=image_word(geometry_records)&0xffffff;
            CHECK(geometry_offset<=room.data.size && geometry_size<=room.data.size-geometry_offset);
            CHECK(g_GeomState && g_GeomState!=(GeomState *)(data+geometry_offset));
            CHECK((void *)g_GeomState==g_GameState.bank_reset_94c[0]);
            const u8 *geometry=room.data.bytes+geometry_offset;
            CHECK(geometry_size>=60);
            unsigned declared=image_word(geometry),view_offset=image_word(geometry+28);
            unsigned views=geometry[10]|(unsigned)geometry[11]<<8;
            CHECK(declared<=geometry_size && view_offset>=60 && view_offset<=declared);
            CHECK(views>0 && views<=(declared-view_offset)/52);
            GeomScrollRecord saved_scroll=g_GeomScroll;
            unsigned wire_entries=image_word(geometry+20);
            size_t packet_bytes=0;
            GeomEntry *native_entries=(GeomEntry *)((u8 *)g_GeomState+g_GeomState->entry_offset);
            for(unsigned entry=0;entry<g_GeomState->entry_count06;++entry) {
                const u8 *wire=geometry+wire_entries+entry*0x38;
                GeomEntry *native=&native_entries[entry];
                CHECK(!memcmp(native,wire,0x28));
                if(native->prim_count) {
                    s32 old_pos=(s32)image_word(wire+0x28),old_tex=(s32)image_word(wire+0x2c);
                    CHECK(!memcmp((u8 *)native+native->u28.pos_ptr,wire+old_pos,native->prim_count*4));
                    CHECK(!memcmp((u8 *)native+native->u2C.uv_ptr,wire+old_tex,native->prim_count*8));
                }
                packet_bytes+=(size_t)native->prim_count*2*(sizeof(SPRT_16)+sizeof(DR_TPAGE));
            }
            u8 *packet_arena=malloc(packet_bytes+16);CHECK(packet_arena);
            memset(packet_arena,0xa5,packet_bytes+16);
            SceneAssetView *saved_texture=g_GameState.texture_load_scratch;
            g_GameState.texture_load_scratch=(SceneAssetView *)packet_arena;
            CHECK(Render_PrepareFrame()==0);
            for(unsigned guard=0;guard<16;++guard)CHECK(packet_arena[packet_bytes+guard]==0xa5);
            for(unsigned entry=0;entry<g_GeomState->entry_count06;++entry) {
                GeomEntry *native=&native_entries[entry];
                CHECK((u8 *)native->u30.prim>=packet_arena && (u8 *)native->u30.prim<=packet_arena+packet_bytes);
                CHECK((u8 *)native->u34.pagePackets>=packet_arena && (u8 *)native->u34.pagePackets<=packet_arena+packet_bytes);
                const u8 *wire=geometry+wire_entries+entry*0x38;
                const u8 *positions=native->prim_count?wire+(s32)image_word(wire+0x28):wire;
                const u8 *textures=native->prim_count?wire+(s32)image_word(wire+0x2c):wire;
                SPRT_16 *sprites=native->u30.prim;
                DR_TPAGE *pages=(DR_TPAGE *)native->u34.pagePackets;
                for(unsigned pass=0;pass<2;++pass)for(unsigned primitive=0;primitive<native->prim_count;++primitive) {
                    unsigned index=pass*native->prim_count+primitive;
                    const u8 *texture=textures+primitive*8;
                    u32 position=image_word(positions+primitive*4),word=image_word(texture);
                    CHECK(getlen(&sprites[index])==3 && getlen(&pages[index])==1);
                    CHECK(sprites[index].r0==128 && sprites[index].g0==128 && sprites[index].b0==128);
                    CHECK(sprites[index].code==(texture[7]&0x10?0x7e:0x7c));
                    CHECK((u16)sprites[index].x0==(u16)(native->disp_x+(position>>22)));
                    CHECK((u16)sprites[index].y0==(u16)(native->disp_y+((position>>12)&1023)));
                    CHECK(sprites[index].u0==texture[4] && sprites[index].v0==texture[3]);
                    CHECK(sprites[index].clut==(((word&511)<<6)|((word>>9)&63)));
                    u32 page=0xe1000600|0x80|((word>>22)&3)<<5|((word>>16)&15)|((word>>15)&1)<<4;
                    CHECK((u32)pages[index].code[0]==page);
                }
            }
            printf("Original room geometry M%03u: %u native entries, %zu packet bytes, full preparation passed\n",
                   map,g_GeomState->entry_count06,packet_bytes);
            g_GameState.texture_load_scratch=saved_texture;
            for(unsigned group=0;group<views;++group) {
                const u8 *view=geometry+view_offset+group*52;
                memset(g_EntityRenderScratch,0,sizeof(g_EntityRenderScratch));
                CHECK(Render_InitViewState(g_EntityRenderScratch,g_EntityRenderScratch+8)==0);
                CHECK(Render_LoadGeomState(group)==0);
                CHECK(g_EntityRenderScratch[8]==(view[0]|(unsigned)view[1]<<8));
                CHECK(!memcmp(g_EntityRenderScratch,view+2,18));
                for(unsigned component=0;component<3;++component)
                    CHECK(g_EntityRenderScratch[5+component]==image_word(view+20+component*4));
            }
            ++native_rooms;
            if(map==431) new_game_room_loaded=1;
            native_geometry_entries+=g_GeomState->entry_count06;
            native_packet_bytes+=packet_bytes;
            g_GeomScroll=saved_scroll;
            free(packet_arena);
            const u8 *entries; unsigned entries_count;
            CHECK(pe_room_blob_table(&room.data, 3, 12, &entries, &entries_count));
            for (unsigned i = 0; i < entries_count; ++i) {
                const u8 *entry = entries + i * 12;
                CHECK(entry[7] < 10);
                CHECK(g_GameState.bank_slots[entry[7]] == data + (image_word(entry + 4) & 0xffffff));
            }
            CHECK(pe_room_blob_table(&room.data, 4, 12, &entries, &entries_count));
            for (unsigned i = 0; i < entries_count; ++i) {
                const u8 *entry = entries + i * 12;
                CHECK(entry[11] < 10 && entry[7] < 48);
                CHECK(g_GameState.bank_rows[entry[11]][entry[7]] == data + (image_word(entry + 4) & 0xffffff));
            }
        }
        CHECK(!memcmp(data, room.data.bytes, room.data.size));
        g_GameState = saved;
        g_PmTables = tables;
        free(scratch); free(textures); free(data); free(spu_expected);
        pe_room_close(&room);
        printf("Original Scene_LoadRoom M%03u: %s (cold + cached audio)\n", map,
               map == 5 ? "unsupported descriptor rejected" : "native room and script/bank relocation passed");
    }

    CHECK(new_game_room_loaded && native_rooms==236 && native_geometry_entries==2763);
    CHECK(native_packet_bytes==23350656);
    printf("Original full room loading corpus: %u rooms, %u native geometry entries, %zu packet bytes\n",native_rooms,native_geometry_entries,native_packet_bytes);

    {
        PeRoomBlob common = {0};
        CHECK(pe_sector_bank_load(&catalog, 5, &common));
        size_t bytes = (size_t)(catalog.sector_bounds[6] - catalog.sector_bounds[5]) * 2048;
        u8 *scratch = malloc(bytes); CHECK(scratch && bytes >= common.size);
        struct { SceneAssetBlob header; SceneAssetDirectory directory; } empty_scene = {0};
        empty_scene.header.directoryOffset = sizeof(SceneAssetBlob);
        g_GameState.loaded_scene_assets = (SceneAssetView *)&empty_scene;
        g_GameState.scene_load_scratch = scratch;
        g_GameState.pe_image_base_lba = catalog.archive.sector;
        /* PM initialization has its own original actor-lifecycle fixture. */
        g_GameStateFlags = 0x80;
        g_GameState.flags = 8;
        g_GameState.tim_load_state = 0;
        memset(expected, 0, sizeof(*expected));
        LoadImage(&all, (u_long *)expected->words); DrawSync(0);
        PeVramStats stats; char error[256];
        CHECK(pe_blob_upload_vram(&common, expected, &stats, error, sizeof(error)));
        CHECK(stats.uploads > 0);
        memset(scratch,0xa5,bytes);
        PeDiscFile limited=catalog.archive;
        limited.size=(u32)((size_t)catalog.sector_bounds[5]*2048);
        CdRom_BindImage(&disc,&limited);
        CHECK(Asset_LoadTimTextures(1)==-1 && g_GameState.tim_load_state==0x34 && (g_GameState.flags&8));
        for(size_t i=0;i<bytes;++i) CHECK(scratch[i]==0xa5);
        DrawSync(0); StoreImage(&all,(u_long *)actual->words); DrawSync(0);
        for(size_t i=0;i<sizeof(actual->words)/sizeof(actual->words[0]);++i) CHECK(actual->words[i]==0);
        CdRom_BindImage(&disc,&catalog.archive);
        CHECK(Asset_LoadTimTextures(1) == 1 && g_GameState.tim_load_state == 0x35);
        CHECK(!memcmp(scratch, common.bytes, common.size));
        /* Force an actual backend error between read and loader completion.
         * Valid scratch bytes must not be uploaded after a failed poll. */
        CHECK(CdRom_PollReady()==0);
        u8 read_guard=0xa5;
        CHECK(CdRom_ReadSectorsFromLba(catalog.archive.sector,&read_guard,UINT32_MAX)==-1 && read_guard==0xa5);
        CHECK(Asset_LoadTimTextures(1)==-1 && g_GameState.tim_load_state==0x34 && (g_GameState.flags&8));
        DrawSync(0); StoreImage(&all,(u_long *)actual->words); DrawSync(0);
        for(size_t i=0;i<sizeof(actual->words)/sizeof(actual->words[0]);++i) CHECK(actual->words[i]==0);
        CdRom_BindImage(&disc,&catalog.archive);
        memset(scratch,0xa5,bytes);
        CHECK(Asset_LoadTimTextures(1)==1 && g_GameState.tim_load_state==0x35);
        CHECK(!memcmp(scratch,common.bytes,common.size));
        CHECK(Asset_LoadTimTextures(1) == 0 && g_GameState.tim_load_state == 0);
        CHECK(!(g_GameState.flags & 8));
        DrawSync(0); StoreImage(&all, (u_long *)actual->words); DrawSync(0);
        CHECK(!memcmp(expected->words, actual->words, sizeof(expected->words)));
        CHECK(Asset_LoadTimTextures(0) == 0 && g_GameState.tim_load_state == 0);
        printf("Original common TIM loader: %u real uploads, archive/poll failures, fresh-read recovery and full VRAM comparison passed\n", stats.uploads);
        unsigned rooms = 0, keyed_uploads = 0, textured_rooms = 0;
        PeRoom room = {0};
        for (unsigned map = 1; map <= PE_USA_ROOM_SLOTS; ++map) {
            if (!catalog.ranges[map - 1].room_sectors) continue;
            CHECK(pe_room_load(&catalog, map, &room));
            memset(expected, 0, sizeof(*expected));
            LoadImage(&all, (u_long *)expected->words); DrawSync(0);
            CHECK(pe_blob_upload_vram(&common, expected, &stats, error, sizeof(error)));
            unsigned before = keyed_uploads;
            CHECK(room_key_images(&room.data, expected, &keyed_uploads) == 0);
            textured_rooms += keyed_uploads != before;
            g_GameState.loaded_scene_assets = (SceneAssetView *)room.data.bytes;
            g_GameState.flags = 8;
            g_GameState.tim_load_state = 0;
            CHECK(Asset_LoadTimTextures(1) == 1);
            CHECK(Asset_LoadTimTextures(1) == 0 && !(g_GameState.flags & 8));
            DrawSync(0); StoreImage(&all, (u_long *)actual->words); DrawSync(0);
            CHECK(!memcmp(expected->words, actual->words, sizeof(expected->words)));
            ++rooms;
        }
        CHECK(rooms == 414 && textured_rooms == 123 && keyed_uploads == 296);
        printf("Original room TIM loader: %u rooms, %u keyed images in %u rooms, full VRAM comparison passed\n",
               rooms, keyed_uploads, textured_rooms);
        pe_room_close(&room);
        g_GameStateFlags = 0;
        g_GameState.loaded_scene_assets = NULL;
        g_GameState.scene_load_scratch = NULL;
        free(scratch); pe_room_blob_close(&common);
    }
    unsigned base_banks=0, base_uploads=0;
    for(unsigned area=10;area<=14;++area) {
        PeRoomBlob tim_bank={0}, model_bank={0};
        CHECK(pe_sector_bank_load(&catalog,area+3,&tim_bank));
        CHECK(pe_sector_bank_load(&catalog,area+8,&model_bank));
        size_t tim_bytes=(size_t)(catalog.sector_bounds[area+4]-catalog.sector_bounds[area+3])*2048;
        size_t model_bytes=(size_t)(catalog.sector_bounds[area+9]-catalog.sector_bounds[area+8])*2048;
        u8 *tim_buffer=malloc(tim_bytes), *model_buffer=malloc(model_bytes);
        CHECK(tim_buffer && model_buffer);
        memset(expected,0,sizeof(*expected));
        LoadImage(&all,(u_long *)expected->words); DrawSync(0);
        PeVramStats stats; char error[256];
        CHECK(pe_blob_upload_vram(&tim_bank,expected,&stats,error,sizeof(error)));
        base_uploads+=stats.uploads;
        g_SceneAreaType=area;
        g_GameState.requested_entity_bank=area;
        g_GameState.flags=0x200000;
        g_GameState.entity_texture_phase=0;
        g_GameState.scene_load_scratch=tim_buffer;
        g_GameState.voice_bank_base_1400=model_buffer;
        PeDiscFile limited=catalog.archive;
        limited.size=(u32)((size_t)catalog.sector_bounds[area+3]*2048);
        CdRom_BindImage(&disc,&limited);
        CHECK(Scene_LoadEntityTexture()==-1 && g_GameState.entity_texture_phase==1);
        CdRom_BindImage(&disc,&catalog.archive);
        CHECK(Scene_LoadEntityTexture()==1 && g_GameState.entity_texture_phase==2);
        CHECK(!memcmp(tim_buffer,tim_bank.bytes,tim_bank.size));
        g_GameState.entity_texture_phase=4;
        limited.size=(u32)((size_t)catalog.sector_bounds[area+8]*2048);
        CdRom_BindImage(&disc,&limited);
        CHECK(Scene_LoadEntityTexture()==-1 && g_GameState.entity_texture_phase==4);
        CdRom_BindImage(&disc,&catalog.archive);
        g_GameState.entity_texture_phase=2;
        CHECK(Scene_LoadEntityTexture()==1 && g_GameState.entity_texture_phase==5);
        CHECK(!memcmp(model_buffer,model_bank.bytes,model_bank.size));
        CHECK(Scene_LoadEntityTexture()==0 && g_GameState.entity_texture_phase==0);
        CHECK(g_GameState.loaded_entity_bank==area && !(g_GameState.flags&0x1204000));
        const u8 *roots,*rows; unsigned root_count,row_count;
        CHECK(pe_room_blob_table(&model_bank,3,sizeof(SceneBankAssetRecord),&roots,&root_count));
        CHECK(root_count>0);
        CHECK(g_GameState.bank_slots[0]==model_buffer+(image_word(roots+4)&0xffffff));
        CHECK(pe_room_blob_table(&model_bank,4,sizeof(SceneBankAssetRecord),&rows,&row_count));
        for(unsigned row=0;row<row_count;++row) {
            const u8 *record=rows+row*sizeof(SceneBankAssetRecord);
            unsigned id=record[7];
            CHECK(g_GameState.bank_rows[id/48][id%48]==model_buffer+(image_word(record+4)&0xffffff));
        }
        DrawSync(0); StoreImage(&all,(u_long *)actual->words); DrawSync(0);
        CHECK(!memcmp(expected,actual,sizeof(*expected)));
        printf("Original base entity loader area %u: %u TIM uploads, %u model/action rows, native state phases passed\n",area,stats.uploads,row_count);
        memset(g_GameState.bank_rows,0,sizeof(g_GameState.bank_rows));
        g_GameState.bank_slots[0]=NULL;
        g_GameState.scene_load_scratch=NULL;
        g_GameState.voice_bank_base_1400=NULL;
        free(tim_buffer); free(model_buffer);
        pe_room_blob_close(&tim_bank); pe_room_blob_close(&model_bank);
        ++base_banks;
    }
    CHECK(base_banks==5);
    printf("Original base entity loader: %u banks, %u TIM uploads with full VRAM comparison\n",base_banks,base_uploads);
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
            g_GameState.scene_load_scratch=scratch_buffer;
            PeVramStats stats; char error[256];
            CHECK(pe_blob_upload_vram(&tim_reference,expected,&stats,error,sizeof(error)));
            uploads+=stats.uploads;
            PeDiscFile limited=catalog.archive;
            limited.size=(u32)((size_t)catalog.sector_bounds[tim_index]*2048);
            CdRom_BindImage(&disc,&limited);
            CHECK(Scene_LoadEntityTextures()==-1 && g_GameState.scene_init_subphase==1);
            CdRom_BindImage(&disc,&catalog.archive);
            CHECK(Scene_LoadEntityTextures()==1 && g_GameState.scene_init_subphase==2);
            CHECK(!memcmp(scratch_buffer,tim_reference.bytes,tim_reference.size));
        }
        unsigned previous_phase=g_GameState.scene_init_subphase;
        g_GameState.scene_init_subphase=4;
        PeDiscFile limited=catalog.archive;
        limited.size=(u32)((size_t)catalog.sector_bounds[index]*2048);
        CdRom_BindImage(&disc,&limited);
        CHECK(Scene_LoadEntityTextures()==-1 && g_GameState.scene_init_subphase==4);
        CdRom_BindImage(&disc,&catalog.archive);
        g_GameState.scene_init_subphase=previous_phase;
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
        size_t shadow_bytes=2*sizeof(RenderShadowQuad);
        u8 *player_allocation=calloc(1,shadow_bytes+player_work_bytes+16); CHECK(player_allocation);
        u8 *player_work=player_allocation+shadow_bytes;
        memset(player_allocation,0xa5,shadow_bytes);
        memset(player_work+player_work_bytes,0xa5,16);
        s8 *texture_data=NULL;
        memset(&player,0,sizeof(player));
        CHECK(Render_SetupEntityPrims(&player.render_object,(RenderObjectHeader *)player_banks.roots[0].bytes,
            player_work,0,0,0,0,0,&texture_data,1)==1);
        /* Execute the full original scene player initialization, including
         * real base/day reads and both-buffer model preparation. */
        size_t base_model_bytes=(size_t)(catalog.sector_bounds[area+9]-catalog.sector_bounds[area+8])*2048;
        size_t base_tim_bytes=(size_t)(catalog.sector_bounds[area+4]-catalog.sector_bounds[area+3])*2048;
        size_t startup_scratch_bytes=base_tim_bytes;
        if(day>=2 && day<=6) {
            size_t daily_bytes=(size_t)(catalog.sector_bounds[day+0x27]-catalog.sector_bounds[day+0x26])*2048;
            if(daily_bytes>startup_scratch_bytes) startup_scratch_bytes=daily_bytes;
        }
        u8 *base_model_buffer=malloc(base_model_bytes), *startup_scratch=malloc(startup_scratch_bytes);
        CHECK(base_model_buffer && startup_scratch);
        g_GameState.voice_bank_base_1400=base_model_buffer;
        g_GameState.scene_load_scratch=startup_scratch;
        g_GameState.scene_area_type_backup=area;
        g_GameState.loaded_entity_bank=0;
        g_GameState.requested_entity_bank=area;
        g_GameState.scene_init_phase=32;
        g_GameState.scene_init_subphase=0;
        g_GameState.entity_texture_phase=0;
        g_GameState.flags=0;
        g_GameState.story_day_flags=0;
        g_GameState.current_story_day=day;
        player.allocation_block=(uintptr_t)player_allocation;
        g_PlayerEntity=&player;
        g_AyaActionStep=2;
        unsigned init_ticks=0;
        while(Scene_InitEntityPlayer(0)) CHECK(++init_ticks<64);
        CHECK(g_GameState.scene_init_phase==32 && !(g_GameState.flags&0xe0000));
        CHECK(g_GameState.loaded_entity_bank==area && g_GameState.current_story_day==day);
        CHECK(player.model_state.model_header==g_GameState.bank_slots[0]);
        CHECK(player.render_object.header==player.model_state.model_header);
        CHECK(player.mode==0x15 && player.action_data==g_GameState.bank_rows[0][0x15]);
        CHECK(player.render_object.animation_data==player.action_data);
        CHECK(player.render_object.primitive_buffer==player_work);
        for(size_t byte=0;byte<shadow_bytes;++byte) CHECK(player_allocation[byte]==0xa5);
        for(unsigned byte=0;byte<16;++byte) CHECK(player_work[player_work_bytes+byte]==0xa5);
        printf("Original scene player init: area %u, day %u, %u ticks, both packet buffers within native allocation\n",area,day,init_ticks+1);
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
        free(player_allocation); pe_room_blob_close(&player_bank);
        g_GameState.voice_bank_base_1400=NULL;
        free(base_model_buffer); free(startup_scratch);
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
    CdRom_BindImage(NULL,NULL);
    CHECK(CdRom_ReadSectorsFromLba(catalog.archive.sector,&guard,1)==-1 && guard==0xa5);
    pe_disc_close(&disc);
    printf("Original entity loader: %u real area/day banks, %u TIM uploads with full VRAM comparison, 14 model rebuilds, %u real animation attachment frames\n",loaded,uploads,animated_frames);
    return 0;
}
#endif
