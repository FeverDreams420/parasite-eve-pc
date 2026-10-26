#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "pe_requisition.h"
#include "pe_economy.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe1/script_queue.h"
SaveRuntimeState g_SaveRuntimeState;
GameStateBlock g_GameStateBlock;
Pe1GameState g_GameState;
InventoryItemWorkspace g_InventoryItemWorkspace;
u32 g_ScriptVariableBank[1024];
static ItemDataRecord bases[256];
static int loaded=1,economy=1,limit=10,fail_spend,discount;
static unsigned junk=10000,unlocked;
static struct { int id;PeHookFn fn; } hooks[12];
static int nh;
static PeMenuAction buy,club;
static PeMenuPerFrame tick;
static int *selection,*shop_pct,*tool_cap;
static PeMenuSet set_tools;
static PeMenuGet get_tools;
static char message[128];
int pe_game_loaded(void) { return loaded; }
int pe_free_field(void) { return !(g_GameStateFlags & 0x2006u) && !(g_GameStateFlagsWord & 0xB600u); }   /* as host/pe_game.c */
int pe_loot_enabled(void) { return economy; }
int Inv_GetAyaSlotLimit(void) { return limit; }
ItemDataRecord *pe_base_item(unsigned id) { return id>0 && id<256?&bases[id]:NULL; }
uint32_t pe_loot_junk(void) { return junk; }
uint32_t pe_loot_price(uint32_t p) { return discount?p*85/100:p; }
int pe_loot_spend_junk(uint32_t n) { if(fail_spend || junk<n)return 0;junk-=n;return 1; }
void pe_loot_give_junk(uint32_t n) { junk+=n; }
int pe_catalog_armor_unlocked(int e) { return !!(unlocked&(1u<<e)); }
int pe_catalog_unlock_armor(int e) { if(pe_catalog_armor_unlocked(e))return 0;unlocked|=1u<<e;return 1; }
void pe_log(const char *t,const char *f,...) { (void)t;(void)f; }
void pe_toast(const char *s) { snprintf(message,sizeof message,"%s",s); }
int pe_hook_add(int id,PeHookFn fn,void *u,int p,const char *m) {
    (void)u;(void)p;(void)m;hooks[nh].id=id;hooks[nh++].fn=fn;return 0;
}
static void dispatch(int id,void *ctx) { for(int i=0;i<nh;++i)if(hooks[i].id==id)hooks[i].fn(ctx,NULL); }
int pe_menu_register_int(const char *p,const char *l,const char *d,int *v,int a,int b,int s) {
    (void)p;(void)d;(void)a;(void)b;(void)s;
    if(!strcmp(l,"Shop price %"))shop_pct=v;
    if(!strcmp(l,"Tool price cap"))tool_cap=v;
    return 0;
}
int pe_menu_register_live_int(const char *p,const char *l,const char *d,int a,int b,int s,PeMenuGet g,PeMenuSet set) {
    (void)p;(void)l;(void)d;(void)a;(void)b;(void)s;get_tools=g;set_tools=set;return 0;
}
int pe_menu_register_info(const char *p,const char *l,const char *d,PeMenuText f) { (void)p;(void)l;(void)d;(void)f;return 0; }
int pe_menu_register_dynamic_choice(const char *p,const char *l,const char *d,int *v,unsigned max,PeMenuChoiceCount c,PeMenuChoiceName n) {
    (void)p;(void)l;(void)d;assert(max==19 && c()<=max && n(0));selection=v;return 0;
}
int pe_menu_register_action(const char *p,const char *l,const char *d,PeMenuAction a) {
    (void)p;(void)d;if(!strcmp(l,"Buy"))buy=a;else if(!strcmp(l,"Spare Club"))club=a;else assert(0);return 0;
}
int pe_menu_register_tick(PeMenuPerFrame f) { tick=f;return 0; }
static void reset(void) {
    memset(&g_SaveRuntimeState,0,sizeof g_SaveRuntimeState);
    memset(&g_InventoryItemWorkspace,0,sizeof g_InventoryItemWorkspace);
    memset(&g_GameStateBlock,0,sizeof g_GameStateBlock);memset(&g_GameState,0,sizeof g_GameState);
    loaded=economy=1;junk=10000;unlocked=0;fail_spend=discount=0;limit=10;
    *shop_pct=100;*tool_cap=100;
    PeHookInvCommand c={1117,0,0};dispatch(PE_HOOK_INV_COMMAND,&c);
    for(int i=0;i<3;++i)g_KeyItemRecords[i].bonusStats[2]=999;
    g_ScriptVariableBank[80]=0x10000;
}
static int purchase(uint16_t id) { char msg[96];return pe_shop_buy(id,msg,sizeof msg); }
static int row(uint16_t id) { for(int i=0;i<pe_shop_count();++i)if(pe_shop_item(i)==id)return i;return -1; }
int main(void) {
    for(int i=1;i<256;++i) { bases[i].iconId=1;bases[i].kind=10; }
    bases[2].kind=bases[3].kind=16;bases[2].ammo=15;bases[3].ammo=30;
    bases[5].kind=17;bases[5].ammo=1;bases[0x3F].kind=8;
    pe_requisition_register("test");pe_requisition_menu_register("test");reset();
    assert(pe_shop_count()==19 && pe_shop_item(-1)==0 && pe_shop_item(19)==0);
    assert(pe_shop_price(8)==35 && pe_shop_price(9)==80 && pe_shop_price(0x12)==300);
    g_ScriptVariableBank[80]=0x16666;assert(pe_shop_price(0x12)==500);
    assert(purchase(6) && junk==9994 && g_AyaInventoryItems[0]==6);
    for(int i=0;i<10;++i)g_AyaInventoryItems[i]=6;
    unsigned before=junk;assert(!purchase(7) && junk==before);
    assert(purchase(0xF000) && junk==before-125 && pe_shop_count()==18);
    before=junk;assert(!purchase(0xF000) && junk==before);
    reset();g_AyaInventoryItems[0]=0x100;g_InventoryRuntime.equipment[0].iconId=1;
    g_InventoryRuntime.equipment[0].kind=1;g_InventoryRuntime.equipment[0].ammo=1;
    assert(pe_shop_price(2)==5);
    g_KeyItemRecords[0].ammo=990;g_AyaInventoryItems[1]=0x200;
    for(int i=2;i<10;++i)g_AyaInventoryItems[i]=6;
    assert(purchase(2) && g_KeyItemRecords[0].ammo==999 && junk==9995);
    before=junk;assert(!purchase(2) && junk==before);
    g_AyaInventoryItems[1]=6;g_KeyItemRecords[0].ammo=0;
    assert(!purchase(2) && junk==before);g_AyaInventoryItems[9]=0;
    assert(purchase(3) && g_AyaInventoryItems[9]==0x200 && g_KeyItemRecords[0].ammo==30);
    reset();assert(pe_shop_price(0x16)==40);
    for(int i=0;i<8;++i)assert(purchase(0x16));assert(pe_shop_price(0x16)==100);
    discount=1;assert(pe_shop_price(0x16)<=100);set_tools(0);assert(pe_shop_price(0x16)==34);
    g_WayneStorageItems[99]=0x35;assert(pe_shop_count()==18 && row(0x16)==-1 && !purchase(0x16));
    reset();fail_spend=1;assert(!purchase(6) && !g_AyaInventoryItems[0] && junk==10000);
    fail_spend=0;junk=0;assert(!purchase(6));assert(pe_shop_price(2)==0 && purchase(2));
    assert(g_KeyItemRecords[0].ammo==15 && g_AyaInventoryItems[0]==0x200);
    reset();char msg[96];assert(pe_shop_rescue_needed());assert(pe_shop_spare_club(msg,sizeof msg));
    assert(g_AyaInventoryItems[0]==0x100 && g_InventoryRuntime.equipment[0].kind==8);
    assert(!pe_shop_rescue_needed() && !pe_shop_spare_club(msg,sizeof msg));
    reset();g_WayneStorageItems[0]=0x100;g_InventoryRuntime.equipment[0].iconId=1;
    g_InventoryRuntime.equipment[0].kind=6;g_KeyItemRecords[1].ammo=1;
    assert(!pe_shop_rescue_needed() && pe_shop_price(2)==5);
    reset();for(int i=0;i<128;++i)g_InventoryRuntime.equipment[i].iconId=1;
    assert(!pe_shop_spare_club(msg,sizeof msg) && !g_AyaInventoryItems[0]);
    reset();loaded=0;assert(!purchase(6) && !pe_shop_spare_club(msg,sizeof msg));
    loaded=1;economy=0;assert(!purchase(6));economy=1;
    g_GameStateFlags=2;assert(!purchase(6));g_GameStateFlags=4;assert(!purchase(6));
    g_GameStateFlags=0;g_GameStateFlagsWord=0x200;assert(!purchase(6));
    reset();uint8_t card[8192];memset(card,0xCC,sizeof card);set_tools(7);
    dispatch(PE_HOOK_SAVE_WRITE,card);assert(card[0x1E50]=='P' && card[0x1E51]=='E' && card[0x1E52]=='S' && card[0x1E53]=='H');
    assert(card[0x1E60]==7 && card[0x1E64]==0xCC);set_tools(0);
    dispatch(PE_HOOK_SAVE_READ,card);assert(get_tools()==7 && pe_shop_price(0x16)==100);
    card[0x1E60]^=1;dispatch(PE_HOOK_SAVE_READ,card);assert(get_tools()==0);
    set_tools(9);PeHookInvCommand c={1117,0,0};dispatch(PE_HOOK_INV_COMMAND,&c);assert(get_tools()==0);
    reset();*selection=row(6);buy();assert(!g_AyaInventoryItems[0]);buy();assert(g_AyaInventoryItems[0]==6 && junk==9994);
    *selection=row(7);buy();tick();buy();assert(!g_AyaInventoryItems[1]);buy();assert(g_AyaInventoryItems[1]==7);
    *selection=row(8);buy();*selection=row(9);buy();assert(!g_AyaInventoryItems[2]);buy();assert(g_AyaInventoryItems[2]==9);
    *selection=row(6);buy();*shop_pct=200;buy();assert(!g_AyaInventoryItems[3]);buy();assert(g_AyaInventoryItems[3]==6);
    reset();club();assert(!g_AyaInventoryItems[0]);club();assert(g_AyaInventoryItems[0]==0x100);
    puts("Requisition checks passed: prices, ammo/full bags, unlocks, Tool cap/kits, rescue/storage, guards, PESH saves, confirmation.");
}
