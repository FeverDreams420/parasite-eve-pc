#include "pe1/geom_state.h"
#include "pe1/render_camera.h"
#include "pe1/render_lighting.h"
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
GeomState *volatile g_GeomState;
RenderLightColor g_RoomLightTint;
#define CHECK(x) do { if (!(x)) {fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;} } while(0)
int main(void) {
    const char *path=getenv("PE_TEST_DISC");
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
    PeDisc disc={0}; PeRoomCatalog catalog; PeRoom room={0};
    CHECK(pe_disc_open(&disc,path)); CHECK(pe_room_catalog_open(&catalog,&disc));
    unsigned views=0, frames=0;
    for (unsigned map=1;map<=PE_USA_ROOM_SLOTS;++map) {
        if (!catalog.ranges[map-1].room_sectors) continue;
        CHECK(pe_room_load(&catalog,map,&room));
        PeCameraBank bank; CHECK(pe_room_cameras(&room,&bank));
        void *copy=malloc(bank.size); CHECK(copy); memcpy(copy,bank.bytes,bank.size);
        g_GeomState=copy;
        for (unsigned group=0;group<bank.count;++group) {
            PeCamera camera; CHECK(pe_camera_group(&bank,group,&camera));
            GteMatrix matrix={0};
            memcpy(matrix.m,camera.view.rotation,sizeof(matrix.m));
            memcpy(matrix.t,camera.view.translation,sizeof(matrix.t));
            s32 distance=camera.distance;
            memset(&g_GeomScroll,0xA5,sizeof(g_GeomScroll));
            CHECK(Render_InitViewState(&matrix,&distance)==0);
            CHECK(g_GeomScroll.state.position.matrixWords==(u32 *)&matrix);
            CHECK(g_GeomScroll.state.position.projectionDistance==&distance);
            CHECK(g_GeomScroll.state.position.x==0 && g_GeomScroll.state.position.originX==0);
            CHECK(g_GeomScroll.state.position.focusHeight==0x180);
            CHECK(g_GeomScroll.state.position.meshTintLevel==0x60);
            CHECK(g_GeomScroll.state.position.groupSelect==0);
            CHECK(g_GeomScroll.state.position.baseAngleDigital==0 &&
                  g_GeomScroll.state.position.baseAngleAnalog==0);
            CHECK(g_GeomScroll.state.position.otZ==0);
            g_GeomScroll.state.position.groupSelect=group;
            for (unsigned smooth=0;smooth<2;++smooth) {
                PeCameraScroll oracle={0}; uint32_t flags=0x70;
                oracle.position[0]=camera.min_x;oracle.position[1]=camera.min_y;
                g_GeomScroll.state.flags=flags;
                g_GeomScroll.state.position.x=camera.min_x;
                g_GeomScroll.state.position.y=camera.min_y;
                CHECK(Render_SetScrollMode(camera.max_x,camera.max_y,30,smooth?8:0)==0);
                CHECK(pe_camera_transition_start(&oracle,&flags,camera.max_x,camera.max_y,30,smooth)==0);
                for(unsigned frame=0;frame<30;++frame) {
                    CHECK(Render_StepCameraTransition()==0);
                    CHECK(pe_camera_transition_step(&camera,&oracle,&flags)==0);
                    CHECK(g_GeomScroll.state.flags==flags);
                    CHECK(g_GeomScroll.state.position.x==oracle.position[0]);
                    CHECK(g_GeomScroll.state.position.y==oracle.position[1]);
                    CHECK(g_GeomScroll.state.position.screenOffsetX==oracle.screen_offset[0]);
                    CHECK(g_GeomScroll.state.position.screenOffsetY==oracle.screen_offset[1]);
                    ++frames;
                }
            }
            s16 expected[2];
            pe_camera_clamp(&camera,0x40,0x8000,0x7fff,expected);
            CHECK(Render_ClampCameraPosition(0x8000,0x7fff)==0);
            CHECK(g_GeomScroll.state.position.x==expected[0] && g_GeomScroll.state.position.y==expected[1]);
            CHECK(Render_SaveAndOpenBounds()==0); CHECK(Render_RestoreBounds()==0);
            CHECK(!memcmp(copy,bank.bytes,bank.size));
            g_GeomScroll.state.flags=0xC0;
            const s32 focus[3]={-123*65536, -45*65536, 1024*65536};
            CHECK(Render_SetViewport(focus)==0);
            CHECK(g_GeomScroll.state.position.x>=camera.min_x && g_GeomScroll.state.position.x<=camera.max_x);
            CHECK(g_GeomScroll.state.position.y>=camera.min_y && g_GeomScroll.state.position.y<=camera.max_y);
            CHECK(Render_UpdateScrollPosition((void *)focus,-1,-1)==0);
            CHECK(g_GeomScroll.state.position.duration==30);
            ++views;
        }
        free(copy);pe_room_close(&room);
    }
    pe_disc_close(&disc); CHECK(views && frames);
    printf("Original camera: %u viewports, %u linear/smooth transition frames\n",views,frames);
    return 0;
}
