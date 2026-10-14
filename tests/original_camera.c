#include "pe1/geom_state.h"
#include "pe1/render_camera.h"
#include "pe1/render_lighting.h"
#include "pe1/game_state.h"
#include "pe1/cdrom.h"
#include "pe1/scene_room.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    PeDisc disc={0}; PeDiscFile archive,executable;
    CHECK(pe_disc_open(&disc,path));
    CHECK(pe_disc_find(&disc,"PE.IMG",&archive) && pe_disc_find(&disc,"SLUS_006.62",&executable));
    u8 exe_header[0x800]; u32 base,exe_size;
    CHECK(pe_disc_read(&disc,&executable,0,exe_header,sizeof(exe_header)));
    CHECK(!memcmp(exe_header,"PS-X EXE",8));
    memcpy(&base,exe_header+0x18,4); memcpy(&exe_size,exe_header+0x1c,4);
    SceneRoomSectorRange ranges[SCENE_ROOM_COUNT];
    /* Fixture data from the resident USA table, not a second room loader. */
    CHECK(base<=0x80093378u && exe_size>=sizeof(ranges) &&
        0x80093378u-base<=exe_size-sizeof(ranges) && (u64)exe_size+0x800<=executable.size);
    CHECK(pe_disc_read(&disc,&executable,(u64)0x80093378u-base+0x800,ranges,sizeof(ranges)));
    CdRom_BindImage(&disc,&archive);
    unsigned views=0, frames=0; unsigned long long trace=14695981039346656037ULL;
    for (unsigned map=1;map<=SCENE_ROOM_COUNT;++map) {
        SceneRoomSectorRange *range=&ranges[map-1];
        if (!range->roomSectors) continue;
        u64 sector=(u64)range->start+range->scratchSectors+range->textureSectors;
        size_t room_bytes=(size_t)range->roomSectors*2048;
        CHECK((sector+range->roomSectors)*2048<=archive.size && sector+archive.sector<=UINT32_MAX);
        SceneAssetView *room=malloc(room_bytes); CHECK(room);
        CHECK(CdRom_ReadSectorsFromLba((u32)(sector+archive.sector),room,range->roomSectors)==1);
        CHECK(CdRom_PollReady()==0);
        u32 declared; memcpy(&declared,room,4);
        CHECK(declared>=8 && declared<=room_bytes && room->header.directoryOffset<=declared &&
            sizeof(SceneRoomDirectory)<=declared-room->header.directoryOffset);
        SceneRoomDirectory *directory=SceneAsset_ResolveOffset(room,room->header.directoryOffset);
        u32 table_offset=directory->taskAux&0x3fffff;
        unsigned count=directory->taskAux>>22;
        CHECK(count && table_offset<=declared && count<=(declared-table_offset)/8);
        SceneRoomRecord *record=SceneRoom_FirstRecord(room,directory->taskAux);
        u32 size,offset; memcpy(&size,record,4); offset=record->source.offsetAndSlot;
        size &= 0xffffff; offset &= 0xffffff;
        CHECK(offset<=declared && size<=declared-offset && size>=60);
        const u8 *bytes=SCENE_ROOM_PAYLOAD(room,record);
        u16 view_count; memcpy(&view_count,bytes+10,2);
        void *copy=malloc(size); CHECK(copy); memcpy(copy,bytes,size);
        g_GeomState=copy;
        CHECK(g_GeomState->entry_offset_1C>=60 && (u32)g_GeomState->entry_offset_1C<=size);
        CHECK(view_count && view_count<=(size-g_GeomState->entry_offset_1C)/sizeof(CameraViewport));
        for (unsigned group=0;group<view_count;++group) {
            CameraViewport camera;
            memcpy(&camera,(u8 *)copy+g_GeomState->entry_offset_1C+group*sizeof(camera),sizeof(camera));
            GteMatrix matrix={0};
            memcpy(matrix.m,&camera.prefix.gpu.half02,sizeof(matrix.m));
            memcpy(matrix.t,&camera.prefix.gpu.word14,sizeof(matrix.t));
            s32 distance=camera.prefix.gpu.geom_screen;
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
            CHECK(distance==camera.prefix.gpu.geom_screen);
            CHECK(!memcmp(matrix.m,&camera.prefix.gpu.half02,sizeof(matrix.m)));
            CHECK(!memcmp(matrix.t,&camera.prefix.gpu.word14,sizeof(matrix.t)));
            CHECK(g_GeomScroll.state.position.groupSelect==group);
            g_GeomScroll.state.position.groupSelect=group;
            for (unsigned smooth=0;smooth<2;++smooth) {
                g_GeomScroll.state.flags=0x70;
                g_GeomScroll.state.position.x=camera.minX;
                g_GeomScroll.state.position.y=camera.minY;
                CHECK(Render_SetScrollMode(camera.maxX,camera.maxY,30,smooth?8:0)==0);
                for(unsigned frame=0;frame<30;++frame) {
                    CHECK(Render_StepCameraTransition()==0);
                    const unsigned short samples[]={g_GeomScroll.state.position.x,g_GeomScroll.state.position.y,
                        g_GeomScroll.state.position.screenOffsetX,g_GeomScroll.state.position.screenOffsetY,
                        g_GeomScroll.state.flags,g_GeomScroll.state.position.elapsed};
                    for(unsigned sample=0;sample<6;++sample) for(unsigned byte=0;byte<2;++byte)
                        trace=(trace^((samples[sample]>>(byte*8))&255))*1099511628211ULL;
                    ++frames;
                }
            }
            /* Extreme signed halfwords must clamp to the serialized bounds. */
            CHECK(Render_ClampCameraPosition(0x8000,0x7fff)==0);
            CHECK(g_GeomScroll.state.position.x==camera.minX && g_GeomScroll.state.position.y==camera.maxY);
            CHECK(Render_SaveAndOpenBounds()==0); CHECK(Render_RestoreBounds()==0);
            CHECK(!memcmp(copy,bytes,size));
            g_GeomScroll.state.flags=0xC0;
            const s32 focus[3]={-123*65536, -45*65536, 1024*65536};
            CHECK(Render_SetViewport(focus)==0);
            CHECK(g_GeomScroll.state.position.x>=camera.minX && g_GeomScroll.state.position.x<=camera.maxX);
            CHECK(g_GeomScroll.state.position.y>=camera.minY && g_GeomScroll.state.position.y<=camera.maxY);
            CHECK(Render_UpdateScrollPosition((void *)focus,-1,-1)==0);
            CHECK(g_GeomScroll.state.position.duration==30);
            ++views;
        }
        free(copy);free(room);
    }
    CdRom_BindImage(NULL,NULL);
    pe_disc_close(&disc); CHECK(views && frames);
    /* Regression captured while every frame also matched the former reference
     * implementation. This proves preservation, not PS1 visual fidelity. */
    CHECK(views==509 && frames==30540 && trace==0x4389b6b0c8d9de19ULL);
    printf("Original camera: %u viewports, %u linear/smooth transition frames\n",views,frames);
    return 0;
}
