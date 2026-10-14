#include "pe1/render_shadow.h"
#include "pe1/render_buffers.h"
#include "pe1/room_floor.h"
#include "pe1/gte.h"
#include "pe1/geom_state.h"
#include "pe1/game_state.h"
GeomState * volatile g_GeomState;
GeomScrollRecord g_GeomScroll;
int Render_ScrollEntries(void);
int Render_StartFadeIn(int frames);
int Render_StepFade(void);
int Render_BeginSceneLoad(void);
int Render_ApplyScreenTint(void);
int Render_ClipPoint(int x,int y,int z);
#include <libgpu.h>
#include <stdio.h>
#include <string.h>
int g_ActiveDrawSlot;
RenderBufferPrefix D_800B0E38;
u32 g_EntityRenderScratch[9];
RoomFloorY g_RoomFloorY[1];
_Static_assert(sizeof(RenderShadowQuad)==sizeof(POLY_FT4),"native shadow packet size");
_Static_assert(offsetof(RenderShadowQuad,xy3)==offsetof(POLY_FT4,x3),"native shadow payload");
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main(void)
{
    struct { GeomState header; GeomEntryView entries[3]; } scrolling={0};
    scrolling.header.entry_offset=(size_t)((u8 *)scrolling.entries-(u8 *)&scrolling);
    scrolling.header.entry_count06=3; g_GeomState=&scrolling.header;
    for(unsigned i=0;i<3;++i) {
        GeomScrollEntry *e=&scrolling.entries[i].scroll;
        e->flags=4; e->modulusX=64; e->modulusY=32;
        e->x=10+i; e->y=20+i; e->speedX=384; e->speedY=-128;
    }
    CHECK(Render_ScrollEntries()==0);
    for(unsigned i=0;i<3;++i) {
        GeomScrollEntry *e=&scrolling.entries[i].scroll;
        CHECK(e->x==11+i && e->y==19+i && e->fractionX.word==128 && e->fractionY.word==128);
    }
    CHECK(Render_ScrollEntries()==0);
    for(unsigned i=0;i<3;++i) {
        GeomScrollEntry *e=&scrolling.entries[i].scroll;
        CHECK(e->x==13+i && e->y==19+i && e->fractionX.word==0 && e->fractionY.word==0);
    }
    g_GeomScroll.state.position.x=50; g_GeomScroll.state.position.originX=10;
    g_GeomScroll.state.position.y=-30; g_GeomScroll.state.position.originY=10;
    for(unsigned i=0;i<3;++i) {
        GeomScrollEntry *e=&scrolling.entries[i].scroll;
        e->flags=8; e->baseX=100+i; e->baseY=200+i;
        e->speedX=128; e->speedY=64;
    }
    g_GeomScroll.state.flags=0x80;
    CHECK(Render_ScrollEntries()==0);
    for(unsigned i=0;i<3;++i) {
        GeomScrollEntry *e=&scrolling.entries[i].scroll;
        CHECK(e->x==120+i && e->y==190+i);
    }
    CHECK(!(g_GeomScroll.state.flags&0x80));
    CHECK(g_GeomScroll.state.position.savedX==50 && (s16)g_GeomScroll.state.position.savedY==-30);
    GeomEntryView saved_entries[3]; memcpy(saved_entries,scrolling.entries,sizeof(saved_entries));
    for(unsigned flag=4;flag<=0x100;flag<<=6) {
        g_GameStateFlags=flag; g_GeomScroll.state.position.x++;
        CHECK(Render_ScrollEntries()==0 && !memcmp(saved_entries,scrolling.entries,sizeof(saved_entries)));
    }
    g_GameStateFlags=0;
    struct { u8 prefix[4],r,g,b,padding[9]; } primitives[3][4];
    for(unsigned i=0;i<3;++i) {
        scrolling.entries[i].render.prim_count=2;
        scrolling.entries[i].render.u30.prim=primitives[i];
    }
    for(unsigned duration=2;duration<=255;++duration) {
        CHECK(Render_StartFadeIn(duration)==0);
        CHECK(g_ScreenTint.fadeFrameCount==duration && g_ScreenTint.fadeFrameCounter==0);
        for(unsigned frame=0;frame<duration;++frame) {
            memset(primitives,0x5a,sizeof(primitives));
            g_ActiveDrawSlot=frame&1;
            CHECK(Render_StepFade()==0);
            for(unsigned entry=0;entry<3;++entry)for(unsigned j=0;j<4;++j) {
                unsigned expected=j/2==(frame&1)?128-frame*128/(duration-1):0x5a;
                CHECK(primitives[entry][j].r==expected && primitives[entry][j].g==expected && primitives[entry][j].b==expected);
            }
        }
        CHECK(!(g_GeomScroll.state.flags&0x400) && (g_GeomScroll.state.flags&0x800));
        CHECK(scrolling.header.field26==0x1ff0);
    }
    g_GeomScroll.state.flags=0; g_GeomScroll.state.position.meshTintLevel=45;
    for(unsigned phase=0;phase<2;++phase) {
        CHECK(Render_BeginSceneLoad()==0);
        for(unsigned slot=0;slot<2;++slot) {
            g_ActiveDrawSlot=slot;
            CHECK(Render_ApplyScreenTint()==0);
            for(unsigned entry=0;entry<3;++entry)for(unsigned j=0;j<2;++j)
                CHECK(primitives[entry][slot*2+j].r==(phase?128:45));
        }
        CHECK(!(g_GeomScroll.state.flags&0x1000));
    }
    GeomState geometry={0}; g_GeomState=&geometry;
    geometry.clip_offset_x=12; geometry.clip_offset_y=-7;
    geometry.clip_min_x=-100; geometry.clip_max_x=100;
    geometry.clip_min_y=-80; geometry.clip_max_y=80; geometry.depth_offset=20;
    for(int x=-150;x<=150;++x)for(int y=-100;y<=100;++y) {
        int expected_x=x+12,expected_y=y-7;
        if(expected_x < -100)expected_x=-100;
        if(expected_x > 100)expected_x=100;
        if(expected_y < -80)expected_y=-80;
        if(expected_y > 80)expected_y=80;
        CHECK(Render_ClipPoint(x,y,30)==0);
        CHECK((s16)geometry.disp_src_x==expected_x && (s16)geometry.disp_src_y==expected_y && geometry.field26==50);
    }
    RenderShadowActor actor={0}; RenderObjectHeader model={0};
    RenderMatrix bone={0}; RenderShadowQuad packets[2]={0};
    RenderNativeTag ordering[4096];
    actor.render_object.header=&model; actor.render_object.matrices=&bone;
    actor.render_object.variant_visible=1; actor.shadow_quads=packets; actor.shadow_intensity=128;
    model.shadow_radius=32;
    bone.rotation[0][0]=bone.rotation[1][1]=bone.rotation[2][2]=4096;
    GteMatrix view={0}; view.m[0][0]=4096; view.m[1][2]=-4096; view.m[2][1]=4096; view.t[2]=1024;
    memcpy(g_EntityRenderScratch,&view,sizeof(view)); g_EntityRenderScratch[8]=256;
    ResetGraph(0); gte_SetGeomOffset(160,112);
    for(unsigned slot=0;slot<2;++slot) {
        ClearOTagR((OT_TYPE *)ordering,4096);
        D_800B0E38.ordering[slot]=ordering; g_ActiveDrawSlot=slot;
        actor.render_object.flags_9C=slot?2:0;
        actor.render_object.fade_red=30; actor.render_object.fade_green=60; actor.render_object.fade_blue=90;
        CHECK(Render_DrawActorShadow(&actor)==0);
        RenderShadowQuad *q=&packets[slot];
        CHECK(q->tag.length==9 && q->code==0x2e);
        CHECK(q->r0==(slot?30:64) && q->g0==q->r0 && q->b0==q->r0);
        CHECK(q->clut==0x7210 && q->tpage==0xcb && q->u3==63 && q->v3==127);
        unsigned linked=0;
        for(unsigned i=0;i<4096;++i) if(ordering[i].address==(uintptr_t)q) ++linked;
        CHECK(linked==1 && q->tag.address!=(uintptr_t)q);
        CHECK(q->xy0!=q->xy1 && q->xy0!=q->xy2);
    }
    RenderShadowQuad saved=packets[1];
    actor.flags=0x400; CHECK(Render_DrawActorShadow(&actor)==0 && !memcmp(&saved,&packets[1],sizeof(saved)));
    actor.flags=0; actor.render_object.variant_visible=0;
    CHECK(Render_DrawActorShadow(&actor)==0 && !memcmp(&saved,&packets[1],sizeof(saved)));
    puts("Original mesh module: 60501 clip points passed");
    puts("Original shadow: native FT4 layout, both slots, projection, shade and ordering links passed");
    return 0;
}
