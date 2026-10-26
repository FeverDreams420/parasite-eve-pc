#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_collection.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe1/entity_allocation.h"
#include "pe1/game_state.h"
FieldActor *g_PlayerEntity;
Pe1GameState g_GameState;
EntityAllocationBlock g_EntityAllocationTable[16];
static PeHookFn frame,render;
static PeMenuAction audit;
static int *enabled;
static char directory[1024];
int pe_game_loaded(void){return g_PlayerEntity!=NULL;}
const PeCostume *pe_wardrobe_worn(void){return NULL;}
void pe_col_say(const char *s){(void)s;}
int pe_mod_path(char *p,size_t n,const char *file){snprintf(p,n,"%s/%s",directory,file);return 0;}
int pe_hook_add(int id,PeHookFn fn,void *u,int p,const char *m){(void)u;(void)p;(void)m;if(id==PE_HOOK_FRAME)frame=fn;if(id==PE_HOOK_RENDER_STATS)render=fn;return 0;}
int pe_menu_register_toggle(const char *p,const char *l,const char *d,int *v,PeMenuPerFrame f){(void)p;(void)l;(void)d;(void)f;enabled=v;return 0;}
int pe_menu_register_action(const char *p,const char *l,const char *d,PeMenuAction f){(void)p;(void)l;(void)d;audit=f;return 0;}
int main(void){
 snprintf(directory,sizeof directory,"%s",getenv("PE_TEST_DIR")?getenv("PE_TEST_DIR"):".");
 char path[1200];pe_mod_path(path,sizeof path,"pe_animation_audit.csv");remove(path);
 pe_animation_audit_register("test");assert(!*enabled && frame && render && audit);
 FieldActor player={0},saved;RenderObjectHeader header={0};RenderObjectPart parts[31]={0};
 unsigned char row[12]={0,28,10};header.part_count=31;parts[1].visible=1;parts[1].vertex_count=3;
 player.render_object.header=&header;player.render_object.parts=parts;player.action_data=row;
 player.allocation_block=1234;g_EntityAllocationTable[0].address=1234;g_EntityAllocationTable[0].blockCount=4;
 g_PlayerEntity=&player;g_GameState.bank_rows[0][4]=row;saved=player;
 frame(NULL,NULL);FILE *f=fopen(path,"r");assert(!f);
 *enabled=1;frame(NULL,NULL);PeHookRenderStats stats={&player.render_object,{12,34,5,6}};render(&stats,NULL);audit();
 assert(!memcmp(&saved,&player,sizeof player));f=fopen(path,"r");assert(f);char text[8192];size_t n=fread(text,1,sizeof text-1,f);text[n]=0;fclose(f);
 assert(strstr(text,"bone_mismatch") && strstr(text,",12,34,5,6") && strstr(text,"loaded_row"));
 *enabled=0;remove(path);puts("Animation audit checks passed: default off, mismatch detection, exact render counts, row audit, actor preserved.");
}
