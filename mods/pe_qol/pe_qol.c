/* PE Enhancements: quality of life (movement; more as they are ported), from the psxrecomp
 * build. Rows at the top of F1 > PE Enhancements. */
#include "pe_mod_host.h"

#define MOD_ID "pe_qol"

void pe_move_register(const char *mod_id);

static int qol_init(void) {
    pe_move_register(MOD_ID);
    return 0;
}

const PeMod pe_mod_pe_qol = {MOD_ID, "PE Enhancements: quality of life", "0.1", NULL, qol_init, NULL};
