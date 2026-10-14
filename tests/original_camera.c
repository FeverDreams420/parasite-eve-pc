#include "pe1/geom_state.h"
#include "pe1/render_camera.h"
#include "pe1/render_lighting.h"
#include "pe1/game_state.h"
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
GeomState *volatile g_GeomState;
RenderLightColor g_RoomLightTint;
#define CHECK(x) do { if (!(x)) {fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;} } while(0)
int Render_SetEntryLimit(int index,int frame);
int Render_SetFlagSlot(int index,int enabled);
s16 Render_GetEntryField6(int index);
int Render_FillEntrySlotValues(int index,u8 duration);
int Render_SetEntrySlotValue(int index,int slot,u8 duration);
int Render_SetEntryField8(int index,unsigned value);
int Menu_SetSlotEntry(int index,unsigned limit,unsigned slot);
static int animation_controls(void) {
    struct { GeomState header; GeomCtrlEntry controls[2]; GeomEntryView entries[4]; GeomAnimationSlot slots[2][2]; } data={0};
    g_GeomState=&data.header;
    data.header.ctrl_offset=(u8 *)data.controls-(u8 *)&data;
    data.header.entry_offset=(u8 *)data.entries-(u8 *)&data;
    data.header.entry_count=2; data.header.entry_count06=4;
    for(unsigned i=0;i<2;++i) {
        data.controls[i].slot_offset=(u8 *)data.slots[i]-(u8 *)&data.controls[i];
        data.controls[i].head.packed=2<<8;
        data.controls[i].position.parts.groupFraction=i;
        data.slots[i][0]=(GeomAnimationSlot){2*i,1};
        data.slots[i][1]=(GeomAnimationSlot){2*i+1,1};
    }
    CHECK(Render_ResetAllEntries()==0);
    for(unsigned i=0;i<2;++i) {
        CHECK(data.controls[i].step==256 && data.controls[i].head.b.flags==1);
        CHECK(data.entries[2*i].render.head.b.flags==2);
    }
    g_GeomScroll.state.position.groupSelect=0; g_GameStateFlags=0; g_GameState.flags=0;
    CHECK(Render_SetFlagSlot(0,1)==0 && Render_SetFlagSlot(1,1)==0);
    CHECK(Render_StepGroupAnimations()==0);
    CHECK(data.controls[0].position.parts.frame==1 && data.controls[1].position.parts.frame==0);
    CHECK(Render_StepGroupAnimations()==0);
    CHECK(!(data.entries[0].render.head.b.flags&2) && (data.entries[1].render.head.b.flags&2));
    CHECK(data.controls[0].position.parts.frame==0 && !(data.controls[0].head.b.flags&4));
    CHECK(Render_FillEntrySlotValues(1,3)==0);
    CHECK(data.slots[1][0].duration==3 && data.slots[1][1].duration==3);
    CHECK(Render_SetEntrySlotValue(1,1,5)==0 && data.slots[1][1].duration==5);
    CHECK(Render_SetEntryField8(1,0xffff0000u)==0 && data.controls[1].step==-256);
    CHECK(Render_SetEntryLimit(1,-1)==0 && Render_GetEntryField6(1)==-1);
    CHECK(Render_SetEntryLimit(1,0)==0);
    CHECK(Menu_SetSlotEntry(1,0,1)==0 && data.controls[1].step==256 && data.slots[1][1].duration==-1);
    g_GeomScroll.state.position.groupSelect=1;
    for(unsigned i=0;i<4;++i)CHECK(Render_StepGroupAnimations()==0);
    CHECK(data.slots[1][1].duration==0 && !(data.controls[1].head.b.flags&4));
    CHECK(Render_SetFlagSlot(1,1)==0);
    g_GameStateFlags=4; GeomCtrlEntry saved=data.controls[1];
    CHECK(Render_StepGroupAnimations()==0 && !memcmp(&saved,&data.controls[1],sizeof(saved)));
    g_GameStateFlags=0; g_GeomState=NULL;
    puts("Original geometry animation: native multi-entry reset, group selection, timing, stop marker and pause passed");
    return 0;
}
int main(void) {
    CHECK(animation_controls()==0);
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
            CHECK(Render_LoadGeomState(group)==0);
            CHECK(distance==camera.distance);
            CHECK(!memcmp(matrix.m,camera.view.rotation,sizeof(matrix.m)));
            CHECK(!memcmp(matrix.t,camera.view.translation,sizeof(matrix.t)));
            CHECK(g_GeomScroll.state.position.groupSelect==group);
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
