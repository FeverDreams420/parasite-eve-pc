/* PE Enhancements: Collection, the Bestiary and the Wardrobe (psxrecomp build's pe_collection.c;
 * bridge findings pe-skins-bestiary-design-2026-10-19.md). F1 > PE Enhancements > Collection.
 * This file: the mod, the profile file and Junk. */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "pe_collection.h"
#include "pe_collection_data.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe_services.h"

#define MOD_ID     "pe_collection"
#define PE_FILE    "pe_collection.dat"
#define PE_MAGIC   0x4C434550u            /* "PECL" */
#define PE_VERSION 1u

_Static_assert(sizeof(PeProfile) == 160, "the recomp's pe_collection.dat layout");

static PeProfile s_prof;
static int s_dirty;
static char s_said[96];

PeProfile *pe_col_profile(void) { return &s_prof; }
void pe_col_changed(void) { s_dirty = 1; }

static uint32_t pe_sum(const PeProfile *p) {
    const uint8_t *b = (const uint8_t *)p;
    uint32_t s = 0x5A17C0DEu;
    size_t i;
    for (i = 0; i < offsetof(PeProfile, checksum); ++i) s = (s << 5) + s + b[i];
    return s;
}

static void pe_defaults(void) {
    memset(&s_prof, 0, sizeof s_prof);
    s_prof.magic = PE_MAGIC;
    s_prof.version = PE_VERSION;
    s_prof.skins = 1u;                    /* Aya */
}

static void pe_load(void) {
    char path[1100];
    PeProfile tmp;
    FILE *f;
    pe_defaults();
    if (pe_mod_path(path, sizeof path, PE_FILE) != 0 || !(f = fopen(path, "rb"))) return;
    if (fread(&tmp, 1, sizeof tmp, f) == sizeof tmp && tmp.magic == PE_MAGIC && tmp.version == PE_VERSION &&
        pe_sum(&tmp) == tmp.checksum)     /* damaged: start over rather than trust it */
        s_prof = tmp;
    fclose(f);
    s_prof.skins |= 1u;
    if (s_prof.worn >= PE_SKINS) s_prof.worn = 0;
}

void pe_col_save_now(void) {
    char path[1100];
    FILE *f;
    s_prof.checksum = pe_sum(&s_prof);
    s_dirty = 0;
    if (pe_mod_path(path, sizeof path, PE_FILE) != 0 || !(f = fopen(path, "wb"))) return;
    fwrite(&s_prof, 1, sizeof s_prof, f);
    fclose(f);
}

static void pe_tick(void) { if (s_dirty) pe_col_save_now(); }

/* ---- Junk ---- */
static const PeJunkApi *pe_junk(void) { return (const PeJunkApi *)pe_service_get("junk", 1); }

int pe_col_junk_ready(char *why, unsigned size) {
    const PeJunkApi *j = pe_junk();
    if (!pe_game_loaded()) { snprintf(why, size, "Load a game first (Junk comes from it)"); return 0; }
    if (!j || !j->enabled()) { snprintf(why, size, "Turn on Junk economy to buy"); return 0; }
    return 1;
}
unsigned pe_col_junk(void) { const PeJunkApi *j = pe_junk(); return j && pe_game_loaded() ? (unsigned)j->junk() : 0u; }
int pe_col_spend(unsigned n) { const PeJunkApi *j = pe_junk(); return j && j->enabled() && j->spend(n); }

void pe_col_say(const char *text) {
    snprintf(s_said, sizeof s_said, "%s", text);
    pe_toast(text);
}
const char *pe_col_last_said(void) { return s_said[0] ? s_said : "-"; }

/* ---- the mod ---- */
static void pe_reset(void) {
    pe_defaults();
    pe_bestiary_reset();
    pe_wardrobe_reset();
    pe_col_save_now();
    pe_col_say("Collection reset");
}

static int collection_init(void) {
    pe_load();
    pe_bestiary_register(MOD_ID);
    pe_wardrobe_register(MOD_ID);
    pe_menu_register_action(PE_PAGE_DEBUG_COLLECTION, "Reset collection", "Forget all Bestiary and Wardrobe unlocks.", pe_reset);
    pe_menu_register_tick(pe_tick);
    return 0;
}

static void collection_shutdown(void) { if (s_dirty) pe_col_save_now(); }

const PeMod pe_mod_pe_collection = {MOD_ID, "PE Enhancements: collection", "0.1", NULL, collection_init, collection_shutdown};
