/* Claire skin: adds Claire Redfield (Resident Evil 2) to the Wardrobe. The skin is a folder in the
 * game's own player formats, pe_skins/claire/ beside the preferences, built by the player from
 * their own copy of the model (not shipped here); while it is missing the costume stays hidden.
 * Free, so it shows in the Wardrobe straight away. */
#include <stddef.h>

#include "pe_mod_host.h"
#include "pe_services.h"

#define MOD_ID "pe_claire"

static const PeCostume s_claire = {
    "re2/claire", "Claire (RE2)", "Resident Evil", 0, -1, NULL, NULL, "claire"
};

static int claire_init(void) {
    const PeCostumesApi *c = pe_service_get("costumes", 1);
    if (!c) { pe_mod_log(MOD_ID, "no costumes service (pe_collection off)"); return 1; }
    if (c->add(&s_claire) < 0) { pe_mod_log(MOD_ID, "costume not added"); return 1; }
    return 0;
}

static const char *const s_requires[] = {"pe_collection", NULL};
const PeMod pe_mod_pe_claire = {MOD_ID, "Claire skin", "0.1", s_requires, claire_init, NULL};
