/* Requisition F1 presentation and confirmation. Shop rules and persistence
 * live in pe_requisition.c so a later Wayne menu can share the backend. */
#include <stdio.h>
#include <string.h>
#include "pe_requisition.h"
#include "pe_economy.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"

static int s_selected, s_armed;
static uint16_t s_armed_item;
static uint32_t s_armed_price;
static s16 s_bag[50];
static char s_result[96],s_label[80];
static unsigned s_frames;
static int s_warned;
static void pe_result(const char *text) {
    snprintf(s_result,sizeof s_result,"%s",text);pe_toast(text);
}
static unsigned pe_count(void) { return pe_shop_count(); }
static const char *pe_name(unsigned row) {
    uint16_t id=pe_shop_item(row);
    snprintf(s_label,sizeof s_label,"%s - %u Junk",pe_shop_name(id),(unsigned)pe_shop_price(id));
    return s_label;
}
static void pe_details(char *out,unsigned size) { pe_shop_details(pe_shop_item(s_selected),out,size); }
static void pe_junk(char *out,unsigned size) { snprintf(out,size,"%u",(unsigned)pe_loot_junk()); }
static void pe_status(char *out,unsigned size) {
    const char *why=pe_shop_blocked(pe_shop_item(s_selected));
    snprintf(out,size,"%s",why?why:"Ready to buy");
}
static void pe_last(char *out,unsigned size) {
    snprintf(out,size,"%s",s_result[0]?s_result:"Choose goods, then press Buy twice");
}
static int pe_same_bag(void) { return memcmp(s_bag,g_AyaInventoryItems,sizeof s_bag)==0; }
static void pe_buy(void) {
    uint16_t id=pe_shop_item(s_selected);
    const char *why=pe_shop_blocked(id);
    if(why) { s_armed=0;pe_result(why);return; }
    uint32_t price=pe_shop_price(id);
    if(s_armed!=1 || s_armed_item!=id || s_armed_price!=price || !pe_same_bag()) {
        s_armed=1;s_armed_item=id;s_armed_price=price;
        memcpy(s_bag,g_AyaInventoryItems,sizeof s_bag);
        snprintf(s_result,sizeof s_result,"Press Buy again: %s for %u Junk",pe_shop_name(id),(unsigned)price);
        pe_toast(s_result);return;
    }
    s_armed=0;
    char msg[96];pe_shop_buy(id,msg,sizeof msg);pe_result(msg);
}
static void pe_club(void) {
    if(!pe_game_loaded() || !pe_loot_enabled() ||
        (g_GameStateFlags & 0x2006u) || (g_GameStateFlagsWord & 0xB600u)) {
        s_armed=0;pe_result("Use Requisition in the field with Junk economy on");return;
    }
    if(!pe_shop_rescue_needed()) { s_armed=0;pe_result("Aya can still fight: no spare Club");return; }
    if(s_armed!=2 || !pe_same_bag()) {
        s_armed=2;memcpy(s_bag,g_AyaInventoryItems,sizeof s_bag);
        pe_result("Press Spare Club again to receive a free Club 1");return;
    }
    s_armed=0;
    char msg[96];pe_shop_spare_club(msg,sizeof msg);pe_result(msg);
}
static void pe_tick(void) {
    s_armed=0; /* Leaving F1 always cancels confirmation. */
    if(++s_frames%60 || !pe_game_loaded() || !pe_loot_enabled() ||
        (g_GameStateFlags & 0x2006u) || (g_GameStateFlagsWord & 0xB600u)) return;
    if(!pe_shop_rescue_needed()) { s_warned=0;return; }
    if(!s_warned) {
        s_warned=1;pe_toast("Aya cannot attack: F1 / Economy / Requisition has a spare Club");
    }
}
static int pe_reset(void *ctx,void *user) {
    (void)ctx;(void)user;s_armed=s_warned=0;s_frames=0;s_result[0]=0;return 0;
}
static int pe_command(void *ctx,void *user) {
    if(((PeHookInvCommand *)ctx)->command==1117) pe_reset(ctx,user);return 0;
}
void pe_requisition_menu_register(const char *mod_id) {
    pe_menu_register_info(PE_PAGE_REQUISITION,"Your Junk","The live Junk balance.",pe_junk);
    pe_menu_register_dynamic_choice(PE_PAGE_REQUISITION,"Goods","Tools disappear with a Tool Kit; effects disappear once unlocked.",
        &s_selected,19,pe_count,pe_name);
    pe_menu_register_info(PE_PAGE_REQUISITION,"Details","Price, carried amount and bag space.",pe_details);
    pe_menu_register_info(PE_PAGE_REQUISITION,"Status","Why this purchase is available or blocked.",pe_status);
    pe_menu_register_action(PE_PAGE_REQUISITION,"Buy","Press twice. Rechecks goods, price, balance and bag space.",pe_buy);
    pe_menu_register_action(PE_PAGE_REQUISITION,"Spare Club","Free Club 1 only if no usable weapon exists in bag or storage. Press twice.",pe_club);
    pe_menu_register_info(PE_PAGE_REQUISITION,"Result","The last action or pending confirmation.",pe_last);
    pe_menu_register_tick(pe_tick);
    pe_hook_add(PE_HOOK_SAVE_READ,pe_reset,NULL,PE_PRIORITY_NORMAL,mod_id);
    pe_hook_add(PE_HOOK_SCENE_START,pe_reset,NULL,PE_PRIORITY_NORMAL,mod_id);
    pe_hook_add(PE_HOOK_INV_COMMAND,pe_command,NULL,PE_PRIORITY_NORMAL,mod_id);
}
