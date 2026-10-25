#include "pe_game.h"

BattleEntity *pe_player(void) { return D_8009D254; }

Combatant *pe_aya_core(void) {
    BattleEntity *player = D_8009D254;
    return player ? (Combatant *)player->core : NULL;
}

int pe_game_loaded(void) { return g_AyaSaveState.max_hp != 0 && D_8009D254 != NULL; }

int pe_in_battle(void) { return (g_GameStateFlags & 2) != 0; }

ItemDataRecord *pe_equipped(int armor) {
    int slot = armor ? g_AyaSaveState.equipped_armor_slot : g_AyaSaveState.equipped_weapon_slot;
    int value;
    if (slot < 0 || slot >= 50) return NULL;
    value = g_AyaSaveState.inventory_items[slot];
    if (value < 0x100 || value >= 0x180) return NULL;
    return &g_InventoryRuntime.equipment[value - 0x100];
}

ItemDataRecord *pe_base_item(unsigned id) {
    return id >= 1 && id <= 0xFF ? Item_LookupBaseData(id - 1) : NULL;
}

int pe_rec_has_effect(const ItemDataRecord *rec, int effect) {
    int n, m;
    if (!rec) return 0;
    n = rec->tailCount > 10 ? 10 : rec->tailCount;
    for (m = 0; m < n; ++m)
        if ((rec->tailData[m] & 0x1F) == effect) return 1;
    return 0;
}

unsigned pe_isqrt(unsigned v) {
    unsigned r = 0, b = 1u << 30;
    while (b > v) b >>= 2;
    while (b) {
        if (v >= r + b) { v -= r + b; r = (r >> 1) + b; }
        else r >>= 1;
        b >>= 2;
    }
    return r;
}
