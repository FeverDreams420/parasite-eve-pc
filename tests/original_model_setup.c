#include "pe1/render_setup.h"
#include "pe1/gte.h"
#include <libgpu.h>
#include "room_banks.h"
#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int g_ActiveDrawSlot;
RenderBufferPrefix D_800B0E38;
_Static_assert(sizeof(RenderNativeTag) == sizeof(OT_TYPE), "native ordering tag");
_Static_assert(sizeof(RenderPacket34) == sizeof(POLY_GT4), "native textured quad");
_Static_assert(sizeof(RenderPacket28) == sizeof(POLY_GT3), "native textured triangle");
_Static_assert(sizeof(RenderPacket24) == sizeof(POLY_G4), "native shaded quad");
_Static_assert(sizeof(RenderPacket1C) == sizeof(POLY_G3), "native shaded triangle");
_Static_assert(offsetof(RenderPacket34, sxy0) == offsetof(POLY_GT4, x0), "native payload offset");
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main(void)
{
    const char *path = getenv("PE_TEST_DISC");
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    PeDisc disc = {0}; PeRoomCatalog catalog; PeRoom room = {0};
    unsigned models = 0;
    unsigned long linked = 0;
    char error[256];
    ResetGraph(0);
    CHECK(pe_disc_open(&disc,path)); CHECK(pe_room_catalog_open(&catalog,&disc));
    for (unsigned map = 1; map <= PE_USA_ROOM_SLOTS; ++map) {
        if (!catalog.ranges[map-1].room_sectors) continue;
        PeRoomBanks banks;
        CHECK(pe_room_load(&catalog,map,&room));
        CHECK(pe_room_banks(&room,&banks,error,sizeof(error)));
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
            Render_TransformMorphVertices(&object,(u32 *)&view);
            RenderNativeTag ordering[4096];
            for (unsigned slot=0;slot<2;++slot) {
                g_ActiveDrawSlot = slot;
                ClearOTagR((OT_TYPE *)ordering,4096);
                uintptr_t end_chain = ordering[0].address;
                D_800B0E38.ordering[slot] = ordering;
                Render_DrawTexturedQuads(&object);
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
            }
            ++models; free(work); free(copy);
        }
        pe_room_close(&room);
    }
    pe_disc_close(&disc);
    CHECK(models == 878 && linked != 0);
    printf("Original native ordering chains: %lu visible packets\n",linked);
    printf("Original model setup: %u model packet buffers and matrix tables\n",models);
    return 0;
}
