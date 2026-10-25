/*
 * The Mod Catalog in Aya's own menu (native rewrite of the psxrecomp build's pe_catalog_menu.c).
 *
 * Main menu: one more row after the game's last (not in battle). PE_HOOK_LIST_ROWS gives the
 * main list (draw == Menu_DrawItemList) the extra row; the game draws a row past its last action
 * with its own spare icon, and PE_HOOK_HELP_TEXT titles it "Mod Catalog"; PE_HOOK_MAIN_MENU_ACTION
 * opens the Catalog when it is confirmed (the game would do nothing).
 *
 * The Catalog is built from the game's own menu widgets, the way its screens are: a panel node
 * with our input handler and a list node with our row drawer (MenuWidget_DrawList), so cursor
 * movement, scrolling and sounds are the game's. Three steps, each its own panel:
 *   Item    your equipped gun / armor
 *   Slot    that item's mod slots
 *   Effect  Remove, the unlocked effects with their price, locked ones dimmed
 * The title bar (PE_HOOK_HELP_TEXT) explains the highlighted row: the effect's full name and
 * price, why it can't go in, or what the last action did. Installing asks for a second press;
 * Circle goes back a step. All rules and prices are the Catalog's (pe_loot_catalog.c).
 */
#include <stdio.h>
#include <string.h>

#include "pe_economy.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_ps1.h"
#include "pe1/draw_state.h"
#include "pe1/menu_draw_list.h"
#include "pe1/menu_sound.h"
#include "pe1/menu_state.h"
#include "pe1/menu_widget.h"
#include "pe_item_table.h"

void Menu_DrawItemList(MenuWidgetNode *node);

#define PE_DESC_PANEL   0x20   /* the options menu's panel and list layout */
#define PE_TITLE_BASE   17     /* a selected_base whose title-bar case always prints */
#define PE_MAX_ROWS     24
#define PE_TEXT_MAX     64

enum { STEP_ITEM, STEP_SLOT, STEP_EFFECT, STEP_COUNT };

static struct {
    MenuWidgetNode *panel[STEP_COUNT], *list[STEP_COUNT];
    MenuWidgetNode *main_list;
    int which;        /* 0 gun, 1 armor */
    int slot;
    int armed;        /* effect row armed by the first press, or -1 */
    char result[PE_TEXT_MAX];
} s_cat = { .armed = -1 };

static u8 s_title[PE_TEXT_MAX + 1];
static u8 s_row_text[PE_TEXT_MAX + 1];

/* ---- game text ---- */
static u8 pe_glyph(char c) {
    if (c >= 'A' && c <= 'Z') return (u8)(0x10 + (c - 'A'));
    if (c >= 'a' && c <= 'z') return (u8)(0x30 + (c - 'a'));
    if (c >= '0' && c <= '9') return (u8)(c - '0');
    switch (c) {
    case '.': return 0x2F; case ',': return 0x4B; case ':': return 0x4A;
    case '!': return 0x2B; case '\'': return 0x2E; case '/': return 0x4C;
    default: return 0x0F;
    }
}

static u8 *pe_game_text(u8 *out, const char *ascii) {
    unsigned n = 0;
    for (; *ascii && n < PE_TEXT_MAX; ++ascii) out[n++] = pe_glyph(*ascii);
    out[n] = 0xFF;
    return out;
}

static const char *pe_item_name(int id) {
    unsigned i;
    for (i = 0; i < PE_ITEM_COUNT; i++)
        if (pe_item_ids[i] == id) return pe_item_names[i];
    return "None";
}

/* Effect names as the Catalog shows them (title case for the menu font). */
static const char *const s_gun_names[21] = {
    "Empty", "Rate of Fire 2", "Rate of Fire 3", "Rate of Fire 5", "Rate of Fire 7", "Rate of Fire 10",
    "Random Fire", "Burst Spray", "Attack All", "Heat Bullets", "Frost Bullets", "Cyanide Bullets",
    "Acid Bullets", "Tranquilizer", "Counterattack", "Quickdraw", "Steal, half attack", "Steal on Attack",
    "Critical Up", "2 Commands", "3 Commands",
};
static const char *const s_armor_names[21] = {
    "Empty", "Resist Poison", "Resist Stiffness", "Resist Darkness", "Less Stolen", "Resist Confusion",
    "Auto Medicine", "Auto Cure", "Bag +1", "Bag +2", "Bag +4", "Max HP Up", "Attack Up", "Slower Active Time",
    "PE Burden Down", "Attack Down", "", "Bargain", "Scavenger", "Lucky Find", "Salvager",
};
static const char *pe_effect_name(int which, int e) {
    if (e < 0 || e > 20) return "?";
    return which ? s_armor_names[e] : s_gun_names[e];
}

/* ---- rows ---- */
static int pe_row_count(int step) {
    if (step == STEP_ITEM) return 2;
    if (step == STEP_SLOT) return pe_cat_slots(s_cat.which) > 0 ? pe_cat_slots(s_cat.which) : 1;
    return 1 + pe_cat_board_count(s_cat.which);                 /* Remove, then the board */
}

static int pe_effect_of_row(int row) { return row <= 0 ? 0 : pe_cat_board_effect(s_cat.which, row - 1); }

static int pe_cursor(int step) { return s_cat.list[step] ? MenuWidget_GridCellIndex(s_cat.list[step]) : 0; }

static void pe_item_row(int index) {
    char text[PE_TEXT_MAX];
    const uint32_t rec = pe_cat_record(index);
    snprintf(text, sizeof text, "%s: %s", index ? "Armor" : "Gun", rec ? pe_item_name(psx_mod_read_byte(rec + 4)) : "None");
    Draw_SetTextDimmed(rec == 0);
    Draw_PrintRawText(pe_game_text(s_row_text, text));
    Draw_SetTextDimmed(0);
}

static void pe_slot_row(int index) {
    char text[PE_TEXT_MAX];
    if (pe_cat_slots(s_cat.which) <= 0) snprintf(text, sizeof text, "No mod slots");
    else snprintf(text, sizeof text, "%d  %s", index + 1, pe_effect_name(s_cat.which, pe_cat_slot_effect(s_cat.which, index)));
    Draw_PrintRawText(pe_game_text(s_row_text, text));
}

static void pe_effect_row(int index) {
    char text[PE_TEXT_MAX];
    const int e = pe_effect_of_row(index);
    int dim = 0;
    if (index == 0) {
        snprintf(text, sizeof text, "Remove");
        dim = pe_cat_slot_effect(s_cat.which, s_cat.slot) == 0;
    } else if (!pe_cat_unlocked(s_cat.which, e)) {
        snprintf(text, sizeof text, "Locked");
        dim = 1;
    } else {
        snprintf(text, sizeof text, "%-18s %u%s", pe_effect_name(s_cat.which, e), (unsigned)pe_cat_price(s_cat.which, e),
                 pe_cat_needs_tool(s_cat.which, e) ? "+Tool" : "");
        dim = pe_cat_install(s_cat.which, s_cat.slot, e, 1) != PE_CAT_OK;
    }
    Draw_SetTextDimmed(dim);
    Draw_PrintRawText(pe_game_text(s_row_text, text));
    Draw_SetTextDimmed(0);
}

static void pe_draw_item_list(MenuWidgetNode *node) { MenuWidget_DrawList(node, pe_item_row); }
static void pe_draw_slot_list(MenuWidgetNode *node) { MenuWidget_DrawList(node, pe_slot_row); }
static void pe_draw_effect_list(MenuWidgetNode *node) { MenuWidget_DrawList(node, pe_effect_row); }

/* ---- title bar ---- */
static const char *pe_install_problem(int code) {
    switch (code) {
    case PE_CAT_NO_ITEM: return "Equip it first";
    case PE_CAT_NO_SLOTS: return "This item has no mod slots";
    case PE_CAT_LOCKED: return "Locked: dismantle gear with this effect";
    case PE_CAT_ALREADY: return "Already in this slot";
    case PE_CAT_BAG: return "Bag mods can't be replaced here";
    case PE_CAT_JUNK: return "Not enough Junk";
    case PE_CAT_TOOL: return "Needs a Tool, or a Tool Kit";
    case PE_CAT_EMPTY: return "This slot is empty";
    case PE_CAT_OFF: return "Junk economy is off";
    default: return "Can't do that now";
    }
}

static void pe_title_for(int step, char *out, size_t size) {
    const int row = pe_cursor(step);
    if (s_cat.result[0]) { snprintf(out, size, "%s", s_cat.result); return; }
    if (step == STEP_ITEM) { snprintf(out, size, "Mod Catalog   Junk %u", (unsigned)pe_loot_junk()); return; }
    if (step == STEP_SLOT) { snprintf(out, size, "Pick a slot   Junk %u", (unsigned)pe_loot_junk()); return; }
    if (row == 0) {
        const int code = pe_cat_remove(s_cat.which, s_cat.slot, 1);
        snprintf(out, size, "%s", code == PE_CAT_OK ? "Empty the slot. The effect stays unlocked" : pe_install_problem(code));
        return;
    }
    {
        const int e = pe_effect_of_row(row), code = pe_cat_install(s_cat.which, s_cat.slot, e, 1);
        if (s_cat.armed == row) snprintf(out, size, "Press again to install %s", pe_effect_name(s_cat.which, e));
        else if (code == PE_CAT_OK || code == PE_CAT_JUNK)
            snprintf(out, size, "%s  %u Junk%s  (have %u)", pe_effect_name(s_cat.which, e), (unsigned)pe_cat_price(s_cat.which, e),
                     pe_cat_needs_tool(s_cat.which, e) ? " + Tool" : "", (unsigned)pe_loot_junk());
        else snprintf(out, size, "%s", pe_install_problem(code));
    }
}

static int pe_step_of(const MenuWidgetNode *node) {
    int step;
    for (step = 0; step < STEP_COUNT; ++step) if (node && node == s_cat.list[step]) return step;
    return -1;
}

static int pe_main_row_count(void) {
    int bits = g_MenuItemContextFlag & 0x1EF, n = 0;
    while (bits) { n += bits & 1; bits >>= 1; }
    return n;
}

static int pe_on_help_text(void *ctx, void *user) {
    u8 **text = (u8 **)ctx;
    MenuWidgetNode *current = MenuWidget_GetCurrentNode();
    const int step = pe_step_of(current);
    char title[PE_TEXT_MAX];
    (void)user;
    if (step >= 0) {
        pe_title_for(step, title, sizeof title);
        *text = pe_game_text(s_title, title);
    } else if (current && current->draw == (void (*)())Menu_DrawItemList && !Menu_GetBattleEquipMode() &&
               MenuWidget_GridCellIndex(current) == pe_main_row_count()) {
        *text = pe_game_text(s_title, "Mod Catalog");
    }
    return 0;
}

/* ---- screens ---- */
static int pe_screen_input(MenuWidgetNode *panel, unsigned int buttons);

static void pe_open(int step, MenuWidgetNode *from) {
    static void (*const draws[STEP_COUNT])(MenuWidgetNode *) = { pe_draw_item_list, pe_draw_slot_list, pe_draw_effect_list };
    MenuWidgetNode *panel = MenuWidget_CreateSimpleNode(PE_DESC_PANEL, from, 0, 0);
    MenuWidgetNode *list = MenuWidget_CreateNode(PE_DESC_PANEL, panel, panel);
    panel->update = pe_screen_input;
    list->draw = (void (*)())draws[step];
    list->selected_base = PE_TITLE_BASE;
    panel->x += step * 12;                                    /* each step a little to the right */
    list->x += step * 12;
    s_cat.panel[step] = panel;
    s_cat.list[step] = list;
    s_cat.armed = -1;
    s_cat.result[0] = '\0';
    MenuWidget_SetCurrentNode(list);
    Draw_SetPrimCallback(list, pe_row_count(step));
}

static void pe_close(int step) {
    MenuWidgetNode *back = step > 0 ? s_cat.list[step - 1] : s_cat.main_list;
    MenuWidget_DestroyNode(s_cat.panel[step]);
    s_cat.panel[step] = s_cat.list[step] = NULL;
    s_cat.armed = -1;
    s_cat.result[0] = '\0';
    if (back) MenuWidget_SetCurrentNode(back);
}

static void pe_effect_confirm(int row) {
    char name[40];
    int code;
    if (row == 0) {
        code = pe_cat_remove(s_cat.which, s_cat.slot, 0);
        if (code != PE_CAT_OK) { Menu_PlayErrorSound(); return; }
        snprintf(s_cat.result, sizeof s_cat.result, "Removed. It stays unlocked");
        Menu_PlayConfirmSound();
        return;
    }
    {
        const int e = pe_effect_of_row(row);
        code = pe_cat_install(s_cat.which, s_cat.slot, e, 1);
        if (code != PE_CAT_OK) { s_cat.armed = -1; Menu_PlayErrorSound(); return; }
        if (s_cat.armed != row) { s_cat.armed = row; Menu_PlayConfirmSound(); return; }
        snprintf(name, sizeof name, "%s", pe_effect_name(s_cat.which, e));
        code = pe_cat_install(s_cat.which, s_cat.slot, e, 0);
        s_cat.armed = -1;
        if (code != PE_CAT_OK) { Menu_PlayErrorSound(); return; }
        snprintf(s_cat.result, sizeof s_cat.result, "Installed %s", name);
        pe_log("catalog_menu", "install which=%d slot=%d effect=%d", s_cat.which, s_cat.slot, e);
        Menu_PlayConfirmSound();
    }
}

static int pe_screen_input(MenuWidgetNode *panel, unsigned int buttons) {
    int step;
    for (step = 0; step < STEP_COUNT && panel != s_cat.panel[step]; ++step) {}
    if (step == STEP_COUNT) return 0;
    if (buttons & 0x40) {                                   /* Circle: back a step */
        pe_close(step);
        Menu_PlayCancelSound();
        return 1;
    }
    if (buttons & 0x10000) {                                /* Cross */
        const int row = pe_cursor(step);
        s_cat.result[0] = '\0';
        if (step == STEP_ITEM) {
            if (!pe_cat_record(row)) { Menu_PlayErrorSound(); return 1; }
            s_cat.which = row;
            pe_open(STEP_SLOT, s_cat.list[STEP_ITEM]);
            Menu_PlayConfirmSound();
        } else if (step == STEP_SLOT) {
            if (pe_cat_slots(s_cat.which) <= 0) { Menu_PlayErrorSound(); return 1; }
            s_cat.slot = row;
            pe_open(STEP_EFFECT, s_cat.list[STEP_SLOT]);
            Menu_PlayConfirmSound();
        } else {
            pe_effect_confirm(row);
        }
        return 1;
    }
    if (step == STEP_EFFECT && s_cat.armed >= 0 && pe_cursor(step) != s_cat.armed) s_cat.armed = -1;
    return 0;
}

/* ---- main menu row ---- */
static int pe_on_list_rows(void *ctx, void *user) {
    const PeHookListRows *r = (const PeHookListRows *)ctx;
    (void)user;
    if (r->node && r->node->draw == (void (*)())Menu_DrawItemList && !Menu_GetBattleEquipMode() &&
        pe_loot_enabled() && pe_game_loaded())
        ++*r->count;
    return 0;
}

static int pe_on_main_action(void *ctx, void *user) {
    const PeHookMenuAction *a = (const PeHookMenuAction *)ctx;
    (void)user;
    if (a->action != 9 || Menu_GetBattleEquipMode() || !pe_loot_enabled()) return 0;
    memset(s_cat.panel, 0, sizeof s_cat.panel);
    memset(s_cat.list, 0, sizeof s_cat.list);
    s_cat.main_list = a->list;
    pe_open(STEP_ITEM, a->list);
    return 1;                                              /* handled: the game plays the confirm sound */
}

void pe_catalog_menu_register(const char *mod_id) {
    pe_hook_add(PE_HOOK_LIST_ROWS, pe_on_list_rows, NULL, PE_PRIORITY_NORMAL, mod_id);
    pe_hook_add(PE_HOOK_MAIN_MENU_ACTION, pe_on_main_action, NULL, PE_PRIORITY_NORMAL, mod_id);
    pe_hook_add(PE_HOOK_HELP_TEXT, pe_on_help_text, NULL, PE_PRIORITY_LATE, mod_id);
}
