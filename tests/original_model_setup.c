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
RenderLightColor g_RoomLightTint = {255,255,255};
RenderObjectEntity g_PlayerBattleEntity;
s32 g_IdentityMatrixSource[8] = {4096,0,4096,0,4096,0,0,0};
s16 g_RenderTexturePageDeltas[4][4];
int g_RenderCounter[1];
RenderVec3s g_RenderNormalTable[240];
RenderBufferPrefix D_800B0E38;
_Static_assert(sizeof(RenderNativeTag) == sizeof(OT_TYPE), "native ordering tag");
_Static_assert(sizeof(RenderPacket34) == sizeof(POLY_GT4), "native textured quad");
_Static_assert(sizeof(RenderPacket28) == sizeof(POLY_GT3), "native textured triangle");
_Static_assert(sizeof(RenderPacket24) == sizeof(POLY_G4), "native shaded quad");
_Static_assert(sizeof(RenderPacket1C) == sizeof(POLY_G3), "native shaded triangle");
_Static_assert(offsetof(RenderPacket34, sxy0) == offsetof(POLY_GT4, x0), "native payload offset");
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main(int argc, char **argv)
{
    const char *path = getenv("PE_TEST_DISC");
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    PeDisc disc = {0}; PeRoomCatalog catalog; PeRoom room = {0};
    int room_gpu = argc == 2 && !strcmp(argv[1], "--room-gpu");
    int gpu = room_gpu || (argc == 2 && !strcmp(argv[1], "--gpu"));
    PeVram *expected = room_gpu ? calloc(1,sizeof(PeVram)) : NULL;
    PeVram *actual = room_gpu ? malloc(sizeof(PeVram)) : NULL;
    unsigned uploads = 0, rooms = 0;
    CHECK(!room_gpu || (expected && actual));
    unsigned rendered = 0;
    unsigned long pixels = 0;
    unsigned short image[320*224], previous[320*224];
    RECT viewport = {0,0,320,224};
    unsigned models = 0;
    unsigned long linked = 0;
    char error[256];
    memset(&g_RenderLightingBlock,0xa5,sizeof(g_RenderLightingBlock));
    Boot_SetBackColor((u16 *)&g_RenderLightingState.matrix.m[0][0],40,40,40);
    for(unsigned row=0;row<3;++row)
        for(unsigned col=0;col<3;++col)CHECK(g_RenderLightingState.matrix.m[row][col]==0);
    const unsigned char *untouched=(const unsigned char *)&g_RenderLightingBlock+18;
    for(size_t i=18;i<sizeof(g_RenderLightingBlock);++i)CHECK(untouched[i-18]==0xa5);
    const unsigned colors[3][3]={{255,254,253},{128,127,126},{96,95,94}};
    for(unsigned light=0;light<3;++light)
        Boot_SetLightColor(&g_RenderLightingBlock,light,colors[light][0],colors[light][1],colors[light][2]);
    for(unsigned row=0;row<3;++row)
        for(unsigned light=0;light<3;++light) {
            CHECK(g_RenderLightColorMatrix.matrix.m[row][light]==(int)colors[light][row]);
            CHECK(g_RenderLightingBlock.light_colors[light][row]==colors[light][row]);
            CHECK(g_RenderLightingState.matrix.m[row][light]==0);
        }
    CHECK(&g_RenderLight0DirY==&g_RenderLightingState.matrix.m[0][1]);
    CHECK(&g_RenderLight1DirX==&g_RenderLightingState.matrix.m[1][0]);
    memset(&g_RenderLightingBlock,0,sizeof(g_RenderLightingBlock));
    ResetGraph(0);
    if (gpu) {
        /* No VSync/PutDispEnv: the backend window stays hidden. Synthetic
         * white texture data isolates packet execution from room uploads. */
        unsigned short *vram = malloc(1024*512*sizeof(*vram));
        CHECK(vram);
        for (unsigned i=0;i<1024*512;++i) vram[i] = 0x7fff;
        RECT all = {0,0,1024,512};
        LoadImage(&all,(u_long *)vram); DrawSync(0); free(vram);
        DRAWENV draw;
        SetDefDrawEnv(&draw,0,0,320,224); draw.dtd = 0;
        PutDrawEnv(&draw); DrawSync(0);
    }
    CHECK(pe_disc_open(&disc,path)); CHECK(pe_room_catalog_open(&catalog,&disc));
    PeDiscFile executable;
    CHECK(pe_disc_find(&disc,"SLUS_006.62",&executable));
    CHECK(pe_disc_read(&disc,&executable,0x921d8-0x10000+0x800,
                       g_RenderTexturePageDeltas,sizeof(g_RenderTexturePageDeltas)));
    CHECK(pe_disc_read(&disc,&executable,0x91a58-0x10000+0x800,
                       g_RenderNormalTable,sizeof(g_RenderNormalTable)));
    for (unsigned map = 1; map <= PE_USA_ROOM_SLOTS; ++map) {
        if (!catalog.ranges[map-1].room_sectors) continue;
        PeRoomBanks banks;
        CHECK(pe_room_load(&catalog,map,&room));
        CHECK(pe_room_banks(&room,&banks,error,sizeof(error)));
        if (room_gpu) {
            RECT all = {0,0,1024,512};
            memset(expected,0,sizeof(*expected));
            LoadImage(&all,(u_long *)expected->words); DrawSync(0);
            PeVramStats stats;
            CHECK(pe_room_upload_vram(&room,expected,&stats,error,sizeof(error)));
            const PeRoomBlob *blobs[] = {&room.scratch,&room.textures};
            for (unsigned b=0;b<2;++b) {
                const uint8_t *records; unsigned count;
                CHECK(pe_room_blob_table(blobs[b],10,sizeof(TimUploadRecord),&records,&count));
                for (unsigned i=0;i<count;++i) {
                    TimUploadRecord record;
                    memcpy(&record,records+i*sizeof(record),sizeof(record));
                    Draw_LoadTimAsset(&record,blobs[b]->bytes);
                }
            }
            DrawSync(0);
            StoreImage(&all,(u_long *)actual->words); DrawSync(0);
            CHECK(!memcmp(expected,actual,sizeof(*expected)));
            uploads += stats.uploads; ++rooms;
        }
        for (unsigned group = 0; group < 10; ++group) {
            PeRoomSlice *slice = &banks.roots[group];
            if (!slice->present) continue;
            PeModel model;
            CHECK(pe_model_open(slice->bytes,slice->size,&model,error,sizeof(error)));
            u8 *copy = malloc(slice->size);
            CHECK(copy); memcpy(copy,slice->bytes,slice->size);
            size_t packets = 2 * (model.packets[0]*sizeof(RenderPacket34) +
                model.packets[1]*sizeof(RenderPacket28) + model.packets[2]*sizeof(RenderPacket24) +
                model.packets[3]*sizeof(RenderPacket1C));
            size_t bytes = packets + model.part_count * (sizeof(RenderAnimationLookupEntry) + sizeof(RenderMatrix));
            u8 *work = calloc(1,bytes ? bytes : 1);
            CHECK(work);
            RenderObjectEntity object = {0}; s8 *textures = NULL;
            CHECK(Render_SetupEntityPrims(&object,(void *)copy,work,0,0,0,0,0,&textures,1) == 1);
            CHECK((u8 *)object.parts == copy + model.parts);
            CHECK((u8 *)object.vertices == copy + model.vertices);
            CHECK((u8 *)object.primitive_descriptors == copy + model.descriptors);
            CHECK((u8 *)object.matrix_commands == copy + model.commands);
            CHECK((u8 *)textures == copy + model.textures);
            CHECK((u8 *)object.animation_entries == work + packets);
            const unsigned sizes[] = {sizeof(RenderPacket34),sizeof(RenderPacket28),sizeof(RenderPacket24),sizeof(RenderPacket1C)};
            const unsigned lengths[] = {12,9,8,6};
            const unsigned extra = sizeof(RenderNativeTag)-4;
            const unsigned commands[] = {0x3c,0x34,0x38,0x30};
            const unsigned blend_kinds[] = {0xb,0x10,0x15,0x1a};
            u8 *packet = work;
            for (unsigned kind=0;kind<4;++kind) {
                for (unsigned i=0;i<model.packets[kind];++i) {
                    PeModelPrimitive primitive;
                    CHECK(pe_model_primitive(&model,kind,i,&primitive));
                    CHECK(((RenderNativeTag *)packet)->length == lengths[kind]);
                    CHECK(packet[extra+7] == commands[kind] + (primitive.kind == blend_kinds[kind] ? 2 : 0));
                    CHECK(!memcmp(packet,packet+sizes[kind],sizes[kind]));
                    if (kind < 2) {
                        PeModelTexture texture;
                        CHECK(pe_model_texture(&model,kind,i,&texture));
                        for (unsigned v=0;v<texture.vertices;++v) {
                            CHECK(packet[extra+12+v*12] == texture.uv[v][0]);
                            CHECK(packet[extra+13+v*12] == texture.uv[v][1]);
                        }
                        CHECK((packet[extra+14] | (unsigned)packet[extra+15]<<8) == texture.clut);
                        CHECK((packet[extra+26] | (unsigned)packet[extra+27]<<8) == texture.page);
                    }
                    packet += sizes[kind]*2;
                }
            }
            CHECK((u8 *)(object.matrices + model.part_count) <= work + bytes);
            for (unsigned i=0;i<model.part_count;++i)
                for (unsigned y=0;y<3;++y)
                    for (unsigned x=0;x<3;++x)
                        CHECK(object.matrices[i].rotation[y][x] == (x==y ? 4096 : 0));
            for (unsigned i=0;i<model.part_count;++i) object.matrices[i].translation[2] = 1024;
            GteMatrix view = {0};
            view.m[0][0] = view.m[1][1] = view.m[2][2] = 4096;
            gte_SetGeomOffset(160,112); gte_ctc2_26(256); gte_lddqa(0); gte_lddqb(0);
            RenderNativeTag ordering[4096];
            for (unsigned slot=0;slot<2;++slot) {
                Render_TransformMorphVertices(&object,(u32 *)&view);
                g_ActiveDrawSlot = slot;
                ClearOTagR((OT_TYPE *)ordering,4096);
                uintptr_t end_chain = ordering[0].address;
                D_800B0E38.ordering[slot] = ordering;
                if (gpu) {
                    object.shade = 64;
                    object.lightPositiveY = 24;
                    object.lightNegativeY = 12;
                    /* At this upper limit both height effects must preserve
                     * ordinary drawing; exercise their native scratch/pointers. */
                    object.flags_9C = slot ? 0x19 : 1;
                    object.script_value9a = 32767;
                    for (unsigned v=0;v<model.vertex_count;++v)
                        CHECK(object.vertices[v].pad >= 0 && object.vertices[v].pad < 240);
                    Render_InitRoomPrimState(&object);
                    Render_DrawEntity(&object,&view);
                } else Render_DrawTexturedQuads(&object);
                uintptr_t cursor = (uintptr_t)&ordering[4095];
                unsigned steps = 0;
                while (cursor != end_chain) {
                    CHECK(++steps <= 4096 + model.packets[0] + model.packets[1] + model.packets[2] + model.packets[3]);
                    RenderNativeTag *tag = (void *)cursor;
                    if (cursor >= (uintptr_t)ordering && cursor < (uintptr_t)(ordering+4096)) {
                        CHECK(tag->length == 0);
                    } else {
                        CHECK(cursor >= (uintptr_t)work && cursor < (uintptr_t)(work+packets));
                        CHECK(tag->length == 12 || tag->length == 9 || tag->length == 8 || tag->length == 6);
                        ++linked;
                    }
                    cursor = tag->address;
                }
                if (gpu) {
                    if (room_gpu) {
                        RECT all = {0,0,1024,512};
                        LoadImage(&all,(u_long *)expected->words); DrawSync(0);
                    }
                    ClearImage(&viewport,0,0,0);
                    DrawOTag((OT_TYPE *)&ordering[4095]);
                    DrawSync(0);
                    memset(image,0,sizeof(image));
                    StoreImage(&viewport,(u_long *)image); DrawSync(0);
                    if (!slot) memcpy(previous,image,sizeof(image));
                    else {
                        CHECK(!memcmp(previous,image,sizeof(image)));
                        unsigned lit = 0;
                        for (unsigned i=0;i<320*224;++i) lit += (image[i]&0x7fff) != 0;
                        rendered += lit != 0; pixels += lit;
                    }
                }
            }
            ++models; free(work); free(copy);
        }
        pe_room_close(&room);
    }
    pe_disc_close(&disc);
    if (room_gpu) {
        CHECK(rooms == 414 && uploads == 9017);
        printf("Original texture uploads: %u rooms, %u uploads, complete VRAM matches oracle\n",rooms,uploads);
    }
    free(actual); free(expected);
    CHECK(models == 878 && linked != 0);
    if (gpu) {
        CHECK((room_gpu ? rendered != 0 : rendered == models) && pixels != 0);
        printf("Original GPU packets: %u models with visible pixels, %lu pixels; both slots identical\n",rendered,pixels);
    }
    printf("Original native ordering chains: %lu visible packets\n",linked);
    printf("Original model setup: %u model packet buffers and matrix tables\n",models);
    return 0;
}
