/* Passive costume diagnostics. Never changes the actor, pose, camera, or profile.
 * The headless tools/animation_sweep.c performs exhaustive native decoding. */
#include <stdio.h>
#include <stdint.h>
#include "pe_collection.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe1/entity_allocation.h"
#include "pe1/game_state.h"
#include "pe1/player_entity.h"
#include "pe1/render_shadow.h"
static int s_logging;
static unsigned s_sample;
static size_t pe_required(const RenderObjectHeader *h) {
    return h ? 2*(h->packet34_count*sizeof(RenderPacket34)+h->packet28_count*sizeof(RenderPacket28)+
        h->packet24_count*sizeof(RenderPacket24)+h->packet1c_count*sizeof(RenderPacket1C))+
        h->animation_entry_count*sizeof(RenderAnimationLookupEntry)+h->part_count*sizeof(RenderMatrix)+
        2*sizeof(RenderShadowQuad) : 0;
}
static size_t pe_capacity(const FieldActor *p) {
    for(int i=0;i<16;++i)if(g_EntityAllocationTable[i].address==p->allocation_block)
        return (size_t)g_EntityAllocationTable[i].blockCount*ENTITY_ALLOCATION_BLOCK_SIZE;
    return 0;
}
static FILE *pe_open(void) {
    char path[1200];
    if(pe_mod_path(path,sizeof path,"pe_animation_audit.csv"))return NULL;
    FILE *f=fopen(path,"a+");if(!f)return NULL;
    fseek(f,0,SEEK_END);
    if(ftell(f)==0)fprintf(f,"sample,kind,costume,bank,row,frame,parts,bones,required,capacity,visible_parts,draw_count,flags,variant,model,animation,status,faces0,faces1,faces2,faces3\n");
    return f;
}
static void pe_write(FILE *f,const char *kind,int row,void *animation,const unsigned *faces) {
    FieldActor *p=g_PlayerEntity;
    if(!p || !p->render_object.header)return;
    RenderObjectEntity *o=&p->render_object;
    const PeCostume *c=pe_wardrobe_worn();
    unsigned parts=o->header->part_count,bones=animation?((u8 *)animation)[1]+1u:0,visible=0;
    for(unsigned i=0;o->parts && i<parts;++i)visible+=o->parts[i].visible==1 && o->parts[i].vertex_count>0;
    size_t required=pe_required(o->header),capacity=pe_capacity(p);
    const char *status=animation && bones!=parts?"bone_mismatch":capacity && required>capacity?"allocation_too_small":!visible?"no_visible_parts":"ok";
    fprintf(f,"%u,%s,%s,%u,%d,%u,%u,%u,%zu,%zu,%u,%d,%u,%u,%p,%p,%s,%d,%d,%d,%d\n",
        ++s_sample,kind,c?c->key:"story",(unsigned)g_SceneAreaType,row,(unsigned)p->anim.parts.integer,
        parts,bones,required,capacity,visible,o->draw_count,o->flags_9C,o->variant_visible,
        (void *)o->header,animation,status,
        faces?(int)faces[0]:-1,faces?(int)faces[1]:-1,faces?(int)faces[2]:-1,faces?(int)faces[3]:-1);
}
static void pe_audit(void) {
    if(!pe_game_loaded() || !g_PlayerEntity){pe_col_say("Load a game before auditing costumes");return;}
    FILE *f=pe_open();if(!f){pe_col_say("Cannot open pe_animation_audit.csv");return;}
    pe_write(f,"current",-1,g_PlayerEntity->action_data,NULL);
    for(int row=0;row<48;++row)if(g_GameState.bank_rows[0][row])
        pe_write(f,"loaded_row",row,g_GameState.bank_rows[0][row],NULL);
    fclose(f);pe_col_say("Costume audit written to pe_animation_audit.csv");
}
static int pe_frame(void *ctx,void *user) {
    (void)ctx;(void)user;
    if(!s_logging || !pe_game_loaded() || !g_PlayerEntity)return 0;
    FILE *f=pe_open();if(f){pe_write(f,"frame",-1,g_PlayerEntity->action_data,NULL);fclose(f);}return 0;
}
static int pe_render(void *ctx,void *user) {
    const PeHookRenderStats *stats=ctx;(void)user;
    if(!s_logging || !g_PlayerEntity || stats->object!=&g_PlayerEntity->render_object)return 0;
    FILE *f=pe_open();if(f){pe_write(f,"render",-1,g_PlayerEntity->action_data,stats->submitted);fclose(f);}return 0;
}
void pe_animation_audit_register(const char *mod_id) {
    pe_menu_register_toggle(PE_PAGE_DEBUG_COLLECTION,"Log costume frames",
        "Record live pose, visibility and render allocation to pe_animation_audit.csv. Off by default.",&s_logging,NULL);
    pe_menu_register_action(PE_PAGE_DEBUG_COLLECTION,"Audit current costume",
        "Log current costume and every loaded animation row without changing the pose.",pe_audit);
    pe_hook_add(PE_HOOK_RENDER_STATS,pe_render,NULL,PE_PRIORITY_NORMAL,mod_id);
    pe_hook_add(PE_HOOK_FRAME,pe_frame,NULL,PE_PRIORITY_LATE+1,mod_id);
    pe_hook_add(PE_HOOK_BATTLE_UPDATE,pe_frame,NULL,PE_PRIORITY_LATE+1,mod_id);
}
