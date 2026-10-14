#include "pe1/gte.h"
#include "pe1/render_object.h"
#include "room_banks.h"
#include "projection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while (0)
int main(void)
{
    const char *path = getenv("PE_TEST_DISC");
    PeDisc disc = {0}; PeRoomCatalog catalog; PeRoom room = {0};
    char error[256]; unsigned models = 0; unsigned long long vertices = 0;
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    CHECK(pe_disc_open(&disc,path)); CHECK(pe_room_catalog_open(&catalog,&disc));
    for (unsigned map=1; map<=PE_USA_ROOM_SLOTS; ++map) {
        PeRoomBanks banks;
        if (!catalog.ranges[map-1].room_sectors) continue;
        CHECK(pe_room_load(&catalog,map,&room));
        CHECK(pe_room_banks(&room,&banks,error,sizeof(error)));
        for (unsigned group=0; group<10; ++group) {
            const PeRoomSlice *slice=&banks.roots[group]; PeModel model;
            RenderObjectEntity object={0}; RenderMatrix matrices[256]={0};
            PeMatrix pose[256]={0}, view;
            PeProjection projection={160u<<16,112u<<16,0,256,0};
            unsigned written;
            if (!slice->present) continue;
            CHECK(pe_model_open(slice->bytes,slice->size,&model,error,sizeof(error)));
            PeProjectedVertex *reference=calloc(model.vertex_count?model.vertex_count:1,sizeof(*reference));
            CHECK(reference);
            pe_matrix_identity(&view);
            for (unsigned i=0; i<model.part_count; ++i) {
                pe_matrix_identity(&pose[i]);
                pose[i].translation[0]=i*20;
                pose[i].translation[2]=1024;
                memcpy(&matrices[i],&pose[i],sizeof(matrices[i]));
            }
            object.header=(RenderObjectHeader *)slice->bytes;
            object.parts=(RenderObjectPart *)(slice->bytes+model.parts);
            object.vertices=(RenderVec3s *)(slice->bytes+model.vertices);
            object.matrices=matrices;
            for (unsigned i=0; i<model.vertex_count+6u; ++i) {
                g_RenderClutLookupTable[i]=0;
                g_RenderVertexWorkWords[i]=0;
            }
            CHECK(pe_model_project(&model,&view,pose,model.part_count,&projection,reference,model.vertex_count,&written));
            gte_SetGeomOffset(160,112); gte_ctc2_26(256); gte_lddqa(0); gte_lddqb(0);
            Render_TransformMorphVertices(&object,(u32 *)&view);
            for (unsigned i=0; i<model.vertex_count; ++i) {
                u32 screen=(u16)reference[i].screen[0]|((u32)(u16)reference[i].screen[1]<<16);
                if (D_800B1644[i]!=screen || (u32)D_800A636C[i]!=reference[i].depth) {
                    fprintf(stderr,"map %u group %u vertex %u differs\n",map,group,i);return 1;
                }
            }
            ++models; vertices+=model.vertex_count; free(reference);
        }
        pe_room_close(&room);
    }
    pe_disc_close(&disc);
    printf("Original model projection corpus: %u models, %llu vertex slots\n",models,vertices);
    CHECK(models>800);
    return 0;
}
