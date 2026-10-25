/* Inventory menu presentation only. Sorting policy lives in pe_bag_sort.c. */
#include "pe_inventory.h"
#include "pe_mod_host.h"
#include "pe1/field_pad.h"
#include "pe1/menu_equipment.h"
#include "pe1/menu_inventory.h"
#include "pe1/menu_state.h"
#include "pe1/menu_widget.h"

static int s_square, s_hidden, s_hidden_mask;

void pe_inventory_menu_shutdown(void) {
    /* A script may have replaced the menu mask since we hid the row. */
    if (s_hidden && g_MenuItemContextFlag == s_hidden_mask)
        g_MenuItemContextFlag |= 0x40;
    s_hidden = s_square = 0;
}
static int pe_menu_restore(void *ctx, void *user) {
    (void)ctx; (void)user;
    pe_inventory_menu_shutdown();
    return 0;
}
static int pe_menu_command(void *ctx, void *user) {
    (void)user;
    if (((PeHookInvCommand *)ctx)->command == 1117) pe_inventory_menu_shutdown();
    return 0;
}
static int pe_menu_rows(void *ctx, void *user) {
    PeHookListRows *c = ctx;
    (void)user;
    if (c->node && c->node->draw == Menu_DrawItemList && pe_game_loaded() && !pe_in_battle() &&
        (g_MenuItemContextFlag & 0x40)) {
        g_MenuItemContextFlag &= ~0x40;
        s_hidden_mask = g_MenuItemContextFlag;
        s_hidden = 1;
        if (*c->count > 0) --*c->count;
    }
    return 0;
}
static int pe_menu_frame(void *ctx, void *user) {
    (void)ctx; (void)user;
    int square = (g_FieldPadBits & 0x80000000u) != 0;
    int pressed = square && !s_square;
    s_square = square;
    if (!pe_game_loaded() || !(g_GameStateFlags & 4u)) {
        /* Preserve the held-button edge across menu closing. */
        pe_inventory_menu_shutdown(); s_square = square;
        return 0;
    }
    if (pe_in_battle()) return 0;
    /* Saving restores the original mask. Rebuild through the normal row hook
     * afterwards, so Catalog and other row extensions retain their positions. */
    if (g_MenuItemContextFlag & 0x40) {
        for (MenuWidgetNode *n = g_MenuWidgetActiveListHead; n; n = n->next) {
            if (n->draw != Menu_DrawItemList) continue;
            unsigned mask = g_MenuItemContextFlag & 0x1EF;
            int count = 0;
            for (; mask; mask >>= 1) count += mask & 1;
            Draw_SetPrimCallback(n, count);
            break;
        }
    }
    if (!pressed) return 0;
    MenuWidgetNode *list = MenuWidget_GetCurrentNode();
    if (!list || list->draw != Menu_DrawSelectableEquipSlotList) return 0;
    MenuWidgetNode *panel = list->parent;
    if (!panel || panel->mode != 1 || panel->selected_base != 1 ||
        panel->update != Menu_InventoryInputHandler) return 0;
    MenuWidgetNode *main_list = panel->parent;
    if (!main_list || main_list->draw != Menu_DrawItemList || g_MenuActionSubmenuOpen ||
        g_InvSelectedItemIndex >= 0 || list->appearance.cursorTargetX >= 0 ||
        g_MenuEquipMode || Menu_GetBattleEquipMode()) return 0;

    /* Native destruction returns focus to main_list. The stock Sort view
     * returns to Use Item after sorting and to the main menu on cancel. */
    MenuWidget_DestroyNode(panel);
    MenuWidget_NavScrollTo(0x1B);
    MenuWidget_SetCurrentNode(main_list);
    Menu_CreateInvSwapView(main_list, 0);
    return 0;
}
void pe_inventory_menu_register(const char *mod_id) {
    pe_hook_add(PE_HOOK_LIST_ROWS, pe_menu_rows, NULL, PE_PRIORITY_EARLY, mod_id);
    pe_hook_add(PE_HOOK_FRAME, pe_menu_frame, NULL, PE_PRIORITY_NORMAL, mod_id);
    pe_hook_add(PE_HOOK_SAVE_WRITE, pe_menu_restore, NULL, PE_PRIORITY_EARLY, mod_id);
    pe_hook_add(PE_HOOK_SAVE_READ, pe_menu_restore, NULL, PE_PRIORITY_EARLY, mod_id);
    pe_hook_add(PE_HOOK_SCENE_START, pe_menu_restore, NULL, PE_PRIORITY_EARLY, mod_id);
    pe_hook_add(PE_HOOK_INV_COMMAND, pe_menu_command, NULL, PE_PRIORITY_EARLY, mod_id);
}
