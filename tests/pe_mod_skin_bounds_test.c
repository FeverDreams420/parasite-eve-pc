/* Bad optional packs must be rejected before their offsets reach the game. */
#include <assert.h>
#include <stdio.h>
#include "../mods/pe_collection/pe_skin_loader.c"
/* The loader's game and host links, unused by these checks. */
Pe1GameState g_GameState;FieldActor *g_PlayerEntity,*g_FieldActorListHead;u16 g_PeImageSectorTable[81];
RenderLightingState g_RenderLightingBlock;
int pe_mod_path(char *o,size_t n,const char *f){(void)o;(void)n;(void)f;return -1;}
int pe_hook_add(int id,PeHookFn fn,void *u,int p,const char *m){(void)id;(void)fn;(void)u;(void)p;(void)m;return 0;}
int pe_menu_register_toggle(const char *s,const char *l,const char *d,int *v,PeMenuPerFrame f){(void)s;(void)l;(void)d;(void)v;(void)f;return 0;}
int pe_save_loaded(void){return 0;}
int pe_game_loaded(void){return 0;}
int pe_in_battle(void){return 0;}
int pe_free_field(void){return 0;}
void pe_log(const char *t,const char *f,...){(void)t;(void)f;}
const PeCostume *pe_wardrobe_worn(void){return NULL;}
int Draw_LoadTimAsset(TimUploadRecord *r,void *b){(void)r;(void)b;return 0;}
int Render_SetupEntityPrims(RenderObjectEntity *o,RenderObjectHeader *m,u8 *p,s16 x,s16 y,s16 u,s16 r,s16 c,s8 **t,int s){(void)o;(void)m;(void)p;(void)x;(void)y;(void)u;(void)r;(void)c;(void)t;(void)s;return 0;}
void Render_DrawWithAnim(RenderObjectEntity *o,void *a,s16 f,union RenderLightingMatrix *l){(void)o;(void)a;(void)f;(void)l;}
void Entity_SetActionMode(BattleEntity *e,int m){(void)e;(void)m;}
int Render_InitRoomPrimState(void *o){(void)o;return 0;}
static void put32(u8 *p,u32 v) { memcpy(p,&v,4); }
int main(void) {
    u8 data[512]={0};PeBlob b={.data=data,.size=sizeof data};
    put32(data+4,8);assert(pe_dir(&b));
    put32(data+4,UINT32_MAX-15u);assert(!pe_dir(&b));
    put32(data+4,sizeof data-sizeof(SceneAssetDirectory)+1u);assert(!pe_dir(&b));
    u8 row[384]={0};row[1]=29;
    PeSkin skin={0};skin.rooms=b;skin.nrooms=1;
    put32(data+8,pe_fingerprint(row));
    put32(data+12,UINT32_MAX-3u);put32(data+16,16);
    assert(pe_room_copy(&skin,row)==NULL);
    put32(data+12,128);put32(data+16,sizeof row);
    memcpy(data+128,row,sizeof row);
    assert(pe_room_copy(&skin,row)==data+128);
    put32(data+16,12);assert(pe_room_copy(&skin,row)==NULL);
    put32(data+16,0);assert(pe_room_copy(&skin,row)==NULL);

    /* A game row with no copy and the wrong bone count gets the skin's idle pose held, with the
     * game row's frames, hit values and starting position; fitting rows stay. */
    static u8 model[1024];RenderObjectHeader header={0};header.part_count=29;
    put32(model+4,16);put32(model+16+0x10,(1u<<22)|128u);put32(model+128+4,256u|(u32)PE_IDLE_MODE<<24);
    u8 *idle=model+256;idle[0]=1;idle[1]=28;idle[2]=1;
    for(int c=0;c<90;++c){idle[12+4*c]=1;if(c<3)idle[12+4*c+2]=(u8)(7+c);else idle[12+4*c+1]=(u8)c;}
    assert(pe_row_length(idle,0x10000u)==372);
    static u8 game[2048];game[0]=1;game[1]=22;game[2]=60;for(int i=4;i<12;++i)game[i]=(u8)(0x10+i);
    u32 at=12;game[at+2]=(u8)(-500&0xff);game[at+3]=(u8)((u16)-500>>8);at+=((60>>1)+1)*4;   /* varying X */
    for(int c=1;c<3+23*3;++c){game[at]=1;at+=4;}
    assert(pe_row_length(game,0x10000u)==at);
    static u8 fits[372];memcpy(fits,idle,372);
    PeSkin *k=&s_skin[0];memset(k,0,sizeof *k);k->ok=1;k->model.data=model;k->model.size=sizeof model;s_skins=1;
    g_GameState.bank_slots[0]=&header;g_GameState.bank_rows[0][33]=game;g_GameState.bank_rows[0][5]=fits;
    pe_guard_rows(k);
    u8 *hold=g_GameState.bank_rows[0][33];
    assert(hold==s_hold[33] && s_replaced[33]==game && g_GameState.bank_rows[0][5]==fits);
    assert(pe_row_length(hold,0x10000u)==372 && hold[1]==28 && hold[2]==60 && !memcmp(hold+3,game+3,9));
    s16 got[90],want[90];assert(pe_first_samples(hold,got,90)==90 && pe_first_samples(idle,want,90)==90);
    assert(got[0]==-500 && got[1]==0 && !memcmp(got+3,want+3,87*sizeof(s16)));
    pe_guard_rows(k);assert(g_GameState.bank_rows[0][33]==hold && s_replaced[33]==game);   /* steady */
    FieldActor player={0};g_PlayerEntity=&player;player.action_data=game;
    pe_running_row(k);assert(player.action_data==hold);   /* the slot's hold, same frames */
    g_GameState.bank_rows[0][33]=NULL;memset(s_replaced,0,sizeof s_replaced);player.action_data=game;
    pe_running_row(k);assert(player.action_data==s_hold[PE_ROWS_PER_BANK]);
    g_GameState.bank_rows[0][33]=hold;s_replaced[33]=game;pe_restore_rows();assert(g_GameState.bank_rows[0][33]==game);
    puts("Skin containers: directory, animation bounds and truncated rows passed; unfit rows held");
    return 0;
}
