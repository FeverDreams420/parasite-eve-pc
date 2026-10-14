#include "pe1/render_setup.h"
#include "room_banks.h"
#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int g_ActiveDrawSlot;
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main(void)
{
    const char *path = getenv("PE_TEST_DISC");
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    PeDisc disc = {0}; PeRoomCatalog catalog; PeRoom room = {0};
    unsigned models = 0;
    char error[256];
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
            const unsigned sizes[] = {52,40,36,28};
            const unsigned commands[] = {0x3c,0x34,0x38,0x30};
            const unsigned blend_kinds[] = {0xb,0x10,0x15,0x1a};
            u8 *packet = work;
            for (unsigned kind=0;kind<4;++kind) {
                for (unsigned i=0;i<model.packets[kind];++i) {
                    PeModelPrimitive primitive;
                    CHECK(pe_model_primitive(&model,kind,i,&primitive));
                    CHECK(packet[3] == sizes[kind]/4-1);
                    CHECK(packet[7] == commands[kind] + (primitive.kind == blend_kinds[kind] ? 2 : 0));
                    CHECK(!memcmp(packet,packet+sizes[kind],sizes[kind]));
                    if (kind < 2) {
                        PeModelTexture texture;
                        CHECK(pe_model_texture(&model,kind,i,&texture));
                        for (unsigned v=0;v<texture.vertices;++v) {
                            CHECK(packet[12+v*12] == texture.uv[v][0]);
                            CHECK(packet[13+v*12] == texture.uv[v][1]);
                        }
                        CHECK((packet[14] | (unsigned)packet[15]<<8) == texture.clut);
                        CHECK((packet[26] | (unsigned)packet[27]<<8) == texture.page);
                    }
                    packet += sizes[kind]*2;
                }
            }
            CHECK((u8 *)(object.matrices + model.part_count) <= work + bytes);
            for (unsigned i=0;i<model.part_count;++i)
                for (unsigned y=0;y<3;++y)
                    for (unsigned x=0;x<3;++x)
                        CHECK(object.matrices[i].rotation[y][x] == (x==y ? 4096 : 0));
            ++models; free(work); free(copy);
        }
        pe_room_close(&room);
    }
    pe_disc_close(&disc);
    CHECK(models == 878);
    printf("Original model setup: %u model packet buffers and matrix tables\n",models);
    return 0;
}
