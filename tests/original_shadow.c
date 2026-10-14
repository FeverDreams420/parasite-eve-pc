#include "pe1/render_shadow.h"
#include "pe1/render_buffers.h"
#include "pe1/room_floor.h"
#include "pe1/gte.h"
#include "pe1/geom_state.h"
GeomState * volatile g_GeomState;
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
