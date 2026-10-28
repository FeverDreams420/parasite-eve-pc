#ifndef PE_SERVICES_H
#define PE_SERVICES_H

/* The services mods offer each other (pe_service_publish / pe_service_get), in one neutral
 * place so no mod includes another mod's headers. A service is a versioned table of functions;
 * a mod that doesn't find one carries on without it.
 *
 *   "keys"      v1  pe_qol      the key item pocket
 *
 * Providers (an import pack, a randomizer) register at their init, so they list "pe_collection"
 * in their PeMod requires. Keys are stable names saved in the player's profile ("pe1/rat",
 * "re2/claire"): a key must never change once released; unlocks of packs that aren't loaded stay
 * in the profile untouched.
 */

#include <stddef.h>
#include <stdint.h>

typedef struct PeKeysApi {
    int (*take)(uint16_t item);        /* pocket a key; 1 means handled */
} PeKeysApi;

#endif
