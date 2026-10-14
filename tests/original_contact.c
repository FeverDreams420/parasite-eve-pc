#include "pe1/entity_allocation.h"
#include "pe1/entity_frame_update.h"
#include "pe1/script_state.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
extern FieldActor *g_FieldActorListHead;
static unsigned callbacks;
static void contact(FieldActor *self,int part,FieldActor *other,int other_part)
{
    (void)self;(void)other;
    if ((part==7 && other_part==9) || (part==9 && other_part==7)) ++callbacks;
}
static int wide_contacts(void)
{
    const int positions[]={-32768,-20000,-1,0,1,20000,32767};
    for(unsigned a=0;a<7;++a) for(unsigned b=0;b<7;++b) {
        FieldActor actors[2]={0};
        u32 script[]={0};
        Script_InitNodePool(); Script_InitNodeFreeList();
        actors[0].next=&actors[1];
        for(unsigned i=0;i<2;++i) {
            actors[i].move_speed=4096;
            actors[i].script_cursor_1a0=script;
            actors[i].render_object.hit_body.radius=20000;
            actors[i].field_sfx_id=i+1;
        }
        actors[0].render_object.hit_body.value0=positions[a];
        actors[1].render_object.hit_body.value0=positions[b];
        actors[0].render_object.hit_body.value1=positions[b];
        actors[1].render_object.hit_body.value1=positions[a];
        actors[0].render_object.hit_body.value2=positions[a];
        actors[1].render_object.hit_body.value2=positions[b];
        g_FieldActorListHead=actors;
        Scene_UpdateEntityPositions();
        /* MIPS multiplies/adds modulo 2^32, then compares signed words. */
        int64_t delta=(int64_t)positions[a]-positions[b];
        uint32_t bits=(uint32_t)(3*delta*delta);
        int64_t distance=bits<=INT32_MAX?bits:(int64_t)bits-4294967296LL;
        int expected=distance<=1600000000;
        if (!!actors[0].script_node_lists[1]!=expected ||
            !!actors[1].script_node_lists[1]!=expected) return 1;
    }
    g_FieldActorListHead=NULL;
    return 0;
}
int main(void)
{
    FieldActor actors[2]={0};
    u32 script[]={0};
    ScriptNode *node;
    Script_InitNodePool(); Script_InitNodeFreeList();
    actors[0].next=&actors[1];
    for (unsigned i=0;i<2;++i) {
        actors[i].type_id=i;actors[i].sub_id=10+i;actors[i].field_sfx_id=20+i;
        actors[i].move_speed=4096;
        actors[i].render_object.hit_body.radius=20;
        actors[i].script_cursor_1a0=script;
    }
    actors[1].render_object.hit_body.value0=10;
    g_FieldActorListHead=actors;
    g_GameStateFlags=0;
    if (g_GameStateBlock.flags) return 1;
    Scene_UpdateEntityPositions();
    node=actors[0].script_node_lists[1];
    if (!node || node->trigger_value!=21 || node->target18.coordinate!=1 ||
        node->target1c!=11 || !(node->flags&1)) return 2;
    if (!actors[1].script_node_lists[1]) return 3;
    Scene_UpdateEntityPositions();
    if (actors[0].script_node_lists[1]!=node || node->next) return 4;
    actors[0].flags=0x20;actors[1].render_object.hit_body.value0=100;
    Scene_UpdateEntityPositions();
    if (actors[0].script_node_lists[1]!=node) return 5;
    actors[0].flags=actors[1].flags=0;
    actors[0].model_state.allocation_active=actors[1].model_state.allocation_active=1;
    for (unsigned i=0;i<2;++i) actors[i].render_object.hit_cylinder.radius=10;
    actors[1].render_object.hit_cylinder.value0=10;
    actors[0].pos_x=10<<16;actors[0].base_x=0;
    actors[1].pos_x=actors[1].base_x=10<<16;
    actors[0].rot_y=99;actors[0].saved_rot_y=5;
    actors[1].rot_y=66;actors[1].saved_rot_y=6;
    actors[1].render_object.hit_body.value0=10;
    g_PlayerEntity=&actors[0];
    Scene_UpdateEntityPositions();
    if (actors[0].pos_x || !(actors[0].flags&0x40000) || actors[0].rot_y!=99 ||
        actors[1].rot_y!=6 || !(actors[1].flags&0x02000000)) return 7;
    {
        RenderObjectHeader headers[2]={{0}};
        RenderAnimationLookupEntry parts[2]={{0}};
        for (unsigned i=0;i<2;++i) {
            headers[i].animation_entry_count=1;
            actors[i].render_object.header=&headers[i];
            actors[i].render_object.animation_entries=&parts[i];
            actors[i].contact_callback=contact;
            parts[i].radius=10;parts[i].value0=i*10;parts[i].animation_id=i?9:7;
        }
        g_GameStateFlags=2;
        Scene_UpdateEntityPositions();
        if (callbacks!=2) return 8;
        actors[1].parent=&actors[0];callbacks=0;
        Scene_UpdateEntityPositions();
        if (callbacks) return 9;
    }
    g_PlayerEntity=0;
    if(wide_contacts()) return 10;
    /* Native scalar and record aliases must observe identical flags. */
    g_GameStateBlock.flags=2;
    if (g_GameStateFlags!=2) return 6;
    g_GameStateFlags=0;g_FieldActorListHead=0;
    puts("Original actor body contacts, event tasks, duplicate suppression and flags alias passed");
    return 0;
}
