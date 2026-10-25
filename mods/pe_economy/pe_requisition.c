/* Requisition rules from recomp pe_loot_shop.c. Native inventory access,
 * no UI dependencies beyond registering its existing tuning preferences. */
#include <stdio.h>
#include <string.h>
#include "pe_requisition.h"
#include "pe_economy.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe1/script_queue.h"

static const struct { uint16_t id; const char *name; unsigned price; } s_goods[] = {
    {0x06,"Medicine 1",6}, {0x07,"Medicine 2",12},
    {0x08,"Medicine 3",35}, {0x09,"Medicine 4",80},
    {0x0D,"Cure-P",5}, {0x0E,"Cure-D",5}, {0x0F,"Cure-C",5}, {0x10,"Cure-M",5},
    {0x11,"Full Cure",15}, {0x02,"Bullets x15",5}, {0x03,"Bullets x30",9},
    {0x05,"Rocket",20}, {0x18,"Smoke Grenade",15}, {0x12,"Revive",0}, {0x16,"Tool",0}
};
static const struct { int effect; const char *name; unsigned price; } s_econ[] = {
    {PE_FX_BARGAIN,"Bargain mod",125}, {PE_FX_SCAVENGER,"Scavenger mod",300},
    {PE_FX_LUCKY_FIND,"Lucky Find mod",225}, {PE_FX_SALVAGER,"Salvager mod",75}
};
enum { GOODS = sizeof s_goods / sizeof s_goods[0], ECON = 4, ECON_ID = 0xF000,
       SAVE_AT = 0x1C90 + 0x1C0 };
static struct { uint16_t tools_bought, pad; } s_shop;
_Static_assert(sizeof s_shop == 4, "recomp PESH v1 payload");
static int s_price_pct=100, s_tool_start=40, s_tool_step=10, s_tool_cap=100;
static int s_econ_pct=100, s_revive=300, s_revive_ex=500;

static int pe_limit(void) {
    int n=Inv_GetAyaSlotLimit(); return n<0?0:n>50?50:n;
}
static int pe_find(int value) {
    for(int i=0;i<pe_limit();++i) if(g_AyaInventoryItems[i]==value) return i;
    return -1;
}
static int pe_kit(void) {
    for(int i=0;i<50;++i) if(g_AyaInventoryItems[i]==0x35 || g_AyaInventoryItems[i]==0x36) return 1;
    for(int i=0;i<100;++i) if(g_WayneStorageItems[i]==0x35 || g_WayneStorageItems[i]==0x36) return 1;
    return 0;
}
static int pe_good(uint16_t id) {
    for(int i=0;i<GOODS;++i) if(s_goods[i].id==id) return i;
    return -1;
}
static int pe_econ(uint16_t id) { return id>=ECON_ID && id<ECON_ID+ECON ? id-ECON_ID : -1; }
static int pe_pool(uint16_t id) {
    ItemDataRecord *base=pe_base_item(id);
    int pool=base?(int)base->kind-16:-1;
    return pool>=0 && pool<3?pool:-1;
}
static int pe_capacity(int pool) {
    ItemDataRecord *p=&g_KeyItemRecords[pool];
    int n=p->baseStats[2]+p->bonusStats[2]; return n<0?0:n>999?999:n;
}
static int pe_weapon(int value) {
    if(value<0x100 || value>=0x180) return 0;
    ItemDataRecord *p=&g_InventoryRuntime.equipment[value-0x100];
    if(!p->iconId) return 0;
    if(p->kind==8) return 1;
    int pool=p->kind>=1 && p->kind<=5?0:p->kind==6?1:p->kind==7?2:-1;
    return pool>=0 && (p->ammo || g_KeyItemRecords[pool].ammo);
}
int pe_shop_rescue_needed(void) {
    if(!pe_game_loaded()) return 0;
    for(int i=0;i<pe_limit();++i) if(pe_weapon(g_AyaInventoryItems[i])) return 0;
    for(int i=0;i<100;++i) if(pe_weapon(g_WayneStorageItems[i])) return 0;
    return 1;
}
int pe_shop_count(void) {
    int n=GOODS-(pe_game_loaded() && pe_kit());
    if(pe_game_loaded()) for(int i=0;i<ECON;++i) n+=!pe_catalog_armor_unlocked(s_econ[i].effect);
    return n;
}
uint16_t pe_shop_item(int row) {
    if(row<0) return 0;
    int goods=GOODS-(pe_game_loaded() && pe_kit());
    if(row<goods) return s_goods[row].id;
    row-=goods;
    if(pe_game_loaded()) for(int i=0;i<ECON;++i)
        if(!pe_catalog_armor_unlocked(s_econ[i].effect) && row--==0) return ECON_ID+i;
    return 0;
}
const char *pe_shop_name(uint16_t id) {
    int e=pe_econ(id), g=pe_good(id);
    return e>=0?s_econ[e].name:g>=0?s_goods[g].name:"None";
}
uint32_t pe_shop_price(uint16_t id) {
    int e=pe_econ(id), g=pe_good(id);
    if(e>=0) return pe_loot_price(s_econ[e].price*(unsigned)s_econ_pct/100u);
    if(g<0) return 0;
    if(id==0x16) {
        uint32_t price=pe_loot_price(s_tool_start+(unsigned)s_tool_step*s_shop.tools_bought);
        return price>(unsigned)s_tool_cap?(unsigned)s_tool_cap:price;
    }
    if(id==0x12) return pe_loot_price(g_ScriptVariableBank[80]>0x10000u?s_revive_ex:s_revive);
    if(id==0x02 && pe_shop_rescue_needed()) return 0;
    uint32_t p=(s_goods[g].price*(unsigned)s_price_pct+50)/100;
    return pe_loot_price(p?p:1);
}
static const char *pe_ready(void) {
    if(!pe_game_loaded()) return "Load a game first";
    if(!pe_loot_enabled()) return "Junk economy is off";
    if((g_GameStateFlags & 0x2006u) || (g_GameStateFlagsWord & 0xB600u))
        return "Use Requisition in the field, outside battle and game menus";
    return NULL;
}
const char *pe_shop_blocked(uint16_t id) {
    const char *why=pe_ready(); if(why) return why;
    int e=pe_econ(id),g=pe_good(id);
    if(e>=0) {
        if(pe_catalog_armor_unlocked(s_econ[e].effect)) return "Already unlocked";
    } else {
        if(g<0) return "No such goods";
        if(id==0x16 && pe_kit()) return "A Tool Kit already covers Tools";
        if(!pe_base_item(id)) return "Item data unavailable";
        int pool=pe_pool(id);
        if(pool>=0) {
            if(g_KeyItemRecords[pool].ammo>=pe_capacity(pool)) return "Ammo is full";
            if(pe_find(0x200+pool)<0 && pe_find(0)<0) return "Bag full";
        } else if(pe_find(0)<0) return "Bag full";
    }
    return pe_loot_junk()<pe_shop_price(id)?"Not enough Junk":NULL;
}
int pe_shop_buy(uint16_t id,char *msg,size_t size) {
    const char *why=pe_shop_blocked(id);
    if(why) { snprintf(msg,size,"%s",why); return 0; }
    uint32_t price=pe_shop_price(id);
    if(price && !pe_loot_spend_junk(price)) { snprintf(msg,size,"Not enough Junk"); return 0; }
    int e=pe_econ(id);
    if(e>=0) {
        if(!pe_catalog_unlock_armor(s_econ[e].effect)) {
            if(price) pe_loot_give_junk(price);
            snprintf(msg,size,"Already unlocked"); return 0;
        }
    } else {
        int pool=pe_pool(id);
        if(pool>=0) {
            if(pe_find(0x200+pool)<0) g_AyaInventoryItems[pe_find(0)]=0x200+pool;
            int rounds=g_KeyItemRecords[pool].ammo+pe_base_item(id)->ammo;
            g_KeyItemRecords[pool].ammo=rounds>pe_capacity(pool)?pe_capacity(pool):rounds;
        } else g_AyaInventoryItems[pe_find(0)]=id;
        if(id==0x16 && s_shop.tools_bought<0xFFFF) ++s_shop.tools_bought;
    }
    pe_log("buy","item=%s price=%u",pe_shop_name(id),(unsigned)price);
    snprintf(msg,size,"Bought %s: %u Junk",pe_shop_name(id),(unsigned)price);
    return 1;
}
int pe_shop_spare_club(char *msg,size_t size) {
    const char *why=pe_ready();
    if(!why && !pe_shop_rescue_needed()) why="Aya can still fight: no spare Club";
    if(why) { snprintf(msg,size,"%s",why);return 0; }
    int slot=pe_find(0),record=-1;
    if(slot<0) why="Bag full";
    for(int i=0;i<128;++i) if(!g_InventoryRuntime.equipment[i].iconId) { record=i;break; }
    ItemDataRecord *base=pe_base_item(0x3F);
    if(!why && (record<0 || !base)) why="No free equipment record";
    if(why) { snprintf(msg,size,"%s",why);return 0; }
    g_InventoryRuntime.equipment[record]=*base;
    g_AyaInventoryItems[slot]=0x100+record;
    snprintf(msg,size,"Spare Club issued"); pe_log("spare_club","item=0x3F");return 1;
}
void pe_shop_details(uint16_t id,char *out,unsigned size) {
    if(!pe_game_loaded()) { snprintf(out,size,"Load a game first");return; }
    int e=pe_econ(id),pool=pe_pool(id),used=0,held=0;
    for(int i=0;i<pe_limit();++i) { used+=g_AyaInventoryItems[i]!=0;held+=g_AyaInventoryItems[i]==id; }
    if(e>=0) snprintf(out,size,"%u Junk: unlocks in Mod Catalog",(unsigned)pe_shop_price(id));
    else if(pool>=0) snprintf(out,size,"%u Junk; ammo %u/%d; bag %d/%d",(unsigned)pe_shop_price(id),
        (unsigned)g_KeyItemRecords[pool].ammo,pe_capacity(pool),used,pe_limit());
    else snprintf(out,size,"%u Junk; carrying %d; bag %d/%d",(unsigned)pe_shop_price(id),held,used,pe_limit());
}

/* PESH v1 retains the recomp's exact 4-byte payload and checksum. */
static uint32_t pe_checksum(const void *data) {
    const uint8_t *p=data;uint32_t sum=0x5A17C0DEu;
    for(unsigned i=0;i<sizeof s_shop;++i) sum=(sum<<5)+sum+p[i];return sum;
}
static int pe_save(void *ctx,void *user) {
    (void)user; uint8_t *p=(uint8_t *)ctx+SAVE_AT;
    uint32_t header[]={0x48534550u,1,sizeof s_shop,pe_checksum(&s_shop)};
    memcpy(p,header,16);memcpy(p+16,&s_shop,sizeof s_shop);return 0;
}
static int pe_load(void *ctx,void *user) {
    (void)user;const uint8_t *p=(const uint8_t *)ctx+SAVE_AT;uint32_t h[4];memcpy(h,p,16);
    memset(&s_shop,0,sizeof s_shop);
    if(h[0]==0x48534550u && h[1]==1 && h[2]==sizeof s_shop && h[3]==pe_checksum(p+16))
        memcpy(&s_shop,p+16,sizeof s_shop);
    return 0;
}
static int pe_command(void *ctx,void *user) {
    (void)user;if(((PeHookInvCommand *)ctx)->command==1117) memset(&s_shop,0,sizeof s_shop);return 0;
}
static int pe_tools(void) { return s_shop.tools_bought; }
static int pe_set_tools(int n) { if(n<0 || n>65535) return 0;s_shop.tools_bought=n;return 1; }
void pe_requisition_register(const char *mod_id) {
    pe_hook_add(PE_HOOK_SAVE_WRITE,pe_save,NULL,PE_PRIORITY_NORMAL,mod_id);
    pe_hook_add(PE_HOOK_SAVE_READ,pe_load,NULL,PE_PRIORITY_NORMAL,mod_id);
    pe_hook_add(PE_HOOK_INV_COMMAND,pe_command,NULL,PE_PRIORITY_NORMAL,mod_id);
    pe_menu_register_int(PE_PAGE_DEBUG_LOOT,"Shop price %","Goods prices, excluding Tools, Revive and economy mods.",&s_price_pct,10,300,5);
    pe_menu_register_int(PE_PAGE_DEBUG_LOOT,"Tool price start","First Tool's price.",&s_tool_start,0,100,5);
    pe_menu_register_int(PE_PAGE_DEBUG_LOOT,"Tool price step","Added per Tool bought.",&s_tool_step,0,50,1);
    pe_menu_register_int(PE_PAGE_DEBUG_LOOT,"Tool price cap","Most a Tool costs, after discounts.",&s_tool_cap,10,100,5);
    pe_menu_register_int(PE_PAGE_DEBUG_LOOT,"Revive price","Revive on a first playthrough.",&s_revive,0,999,25);
    pe_menu_register_int(PE_PAGE_DEBUG_LOOT,"Revive price (EX Game)","Revive on later playthroughs.",&s_revive_ex,0,999,25);
    pe_menu_register_int(PE_PAGE_DEBUG_LOOT,"Economy mod price %","Of each effect's individual price.",&s_econ_pct,0,300,10);
    pe_menu_register_live_int(PE_PAGE_DEBUG_LOOT,"Tools bought","Sets the next Tool's price.",0,65535,1,pe_tools,pe_set_tools);
}
