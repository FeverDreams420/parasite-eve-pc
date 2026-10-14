#include "pe1/render_object.h"
#include "room_banks.h"
#include "animation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Anim_BuildRotationMatrices(RenderObjectEntity *, RenderAnimationDataHeader *, int);
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while (0)
int main(void)
{
    const char *path = getenv("PE_TEST_DISC");
    PeDisc disc = {0};
    PeRoomCatalog catalog;
    PeRoom room = {0};
    unsigned streams = 0;
    unsigned long long frames = 0, bones = 0;
    char error[256];
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    CHECK(pe_disc_open(&disc, path));
    CHECK(pe_room_catalog_open(&catalog, &disc));
    for (unsigned map = 0; map < PE_USA_ROOM_SLOTS; ++map) {
        PeRoomBanks banks;
        if (!catalog.ranges[map].room_sectors) continue;
        CHECK(pe_room_load(&catalog,map+1,&room));
        CHECK(pe_room_banks(&room,&banks,error,sizeof(error)));
        for (unsigned group = 0; group < 10; ++group)
        for (unsigned slot = 0; slot < 48; ++slot) {
            const PeRoomSlice *slice = &banks.rows[group][slot];
            PeAnimation animation;
            if (!slice->present) continue;
            CHECK(pe_animation_open(slice->bytes,slice->size,&animation,error,sizeof(error)));
            ++streams;
            for (unsigned sample=0; sample<(animation.frames?animation.frames:1); ++sample) {
                RenderObjectEntity object = {0};
                RenderMatrix original[PE_ANIMATION_BONES] = {0};
                PeMatrix reference[PE_ANIMATION_BONES] = {0};
                PeAnimationFrame decoded;
                object.matrices = original;
                CHECK(pe_animation_frame(&animation,sample,&decoded));
                CHECK(pe_animation_matrices(&decoded,NULL,reference,PE_ANIMATION_BONES));
                Anim_BuildRotationMatrices(&object,(RenderAnimationDataHeader *)slice->bytes,(int)sample);
                for (unsigned bone=0; bone<animation.bones; ++bone) {
                    if (memcmp(original[bone].rotation,reference[bone].rotation,18)) {
                        fprintf(stderr,"map %u group %u slot %u frame %u bone %u\n",map,group,slot,sample,bone);
                        return 1;
                    }
                }
                CHECK(!memcmp(original[0].translation,reference[0].translation,12));
                ++frames; bones+=animation.bones;
            }
        }
        pe_room_close(&room);
    }
    pe_disc_close(&disc);
    printf("Original animation corpus: %u streams, %llu frames, %llu bone matrices\n",streams,frames,bones);
    CHECK(streams>9000 && frames>300000);
    return 0;
}
