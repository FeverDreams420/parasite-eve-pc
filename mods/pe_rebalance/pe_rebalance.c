/* PE Enhancements: rebalance (mod effects, Parasite Energy, weapon power), ported from the
 * psxrecomp build. Rows under F1 > PE Enhancements. */
#include "pe_mod_host.h"

#define MOD_ID "pe_rebalance"

void pe_mod_effects_register(const char *mod_id);
void pe_pe_balance_register(const char *mod_id);
void pe_weapon_power_register(const char *mod_id);
void pe_mod_effects_shutdown(void);

static int rebalance_init(void) {
    pe_weapon_power_register(MOD_ID);
    pe_mod_effects_register(MOD_ID);
    pe_pe_balance_register(MOD_ID);
    return 0;
}

const PeMod pe_mod_pe_rebalance = {MOD_ID, "PE Enhancements: rebalance", "0.1", NULL, rebalance_init, pe_mod_effects_shutdown};
