/* Infinite ammo from the recomp's pe_cheats.c. Native records only:
 * spare pools, equipped weapon and Aya's current battle action. Off by
 * default; Debug settings use the host's normal ini persistence. */
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"

enum { PE_AMMO_OFF, PE_AMMO_SPARES, PE_AMMO_NO_RELOAD };
static int s_mode;
static const char *const s_names[] = { "Off", "Refill spares", "Never reload" };

static unsigned pe_capacity(const ItemDataRecord *record) {
    int cap = record->baseStats[2] + record->bonusStats[2];
    return cap < 0 ? 0 : cap > 999 ? 999 : (unsigned)cap;
}

static ItemDataRecord *pe_pool(const ItemDataRecord *gun) {
    unsigned kind = gun->kind;
    if (kind >= 1 && kind < 8) return &g_KeyItemRecords[kind > 4 ? kind - 5 : 0];
    if (kind >= 19 && kind <= 21) return &g_KeyItemRecords[kind - 19];
    if (kind == 8) return &g_InvFallbackAmmoRecord;
    return NULL;
}

static void pe_fill(ItemDataRecord *record) {
    unsigned cap = pe_capacity(record);
    if (record->ammo < cap) record->ammo = (u16)cap;
}

static void pe_ammo_tick(void) {
    ItemDataRecord *gun, *pool;
    Combatant *core;
    BattleAction *action;
    unsigned cap;
    if (s_mode == PE_AMMO_OFF || !pe_game_loaded()) return;
    gun = pe_equipped(0);
    if (!gun || !(pool = pe_pool(gun))) return;
    pe_fill(pool);
    if (s_mode != PE_AMMO_NO_RELOAD) return;
    pe_fill(gun);
    if (!pe_in_battle() || !(core = pe_aya_core()) || !(action = core->action)) return;
    cap = pe_capacity(gun);
    if ((action->attackWord & 0x3FFu) < cap)
        action->attackWord = (action->attackWord & ~0x3FFu) | cap;
}

void pe_ammo_register(const char *mod_id) {
    (void)mod_id;
    pe_menu_register_choice(PE_PAGE_DEBUG, "Infinite ammo",
        "Refill spare rounds, or also keep the gun loaded.", &s_mode, s_names, 3);
    pe_menu_register_tick(pe_ammo_tick);
}
