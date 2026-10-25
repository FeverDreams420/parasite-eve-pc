#ifndef PE_COLLECTION_H
#define PE_COLLECTION_H

/* The Collection's private interface, shared by its files:
 *   pe_collection.c  the mod, the profile file (unlocks kept across games), Junk
 *   pe_bestiary.c    creatures met, weaknesses and loot: kills, Scan, purchases; F1 pages
 *   pe_wardrobe.c    Aya's outfits (and character skins once their loader is ported); F1 pages,
 *                    the `wardrobe` service for Wayne's Workshop
 * Everything is cosmetic or information: it never changes a fight. */

#include <stdint.h>

/* Unlocks are permanent across games: pe_collection.dat beside the preferences, not in the memory
 * card save, so a new game, an older save or the EX Game keeps them. Byte-compatible with the
 * psxrecomp build's file. */
enum { C_MET = 1u, C_WEAK = 2u, C_LOOT = 4u };   /* creature bits 0-2; kills in bits 3-7 (max 31) */

typedef struct PeProfile {
    uint32_t magic, version;
    uint64_t skins;                 /* bit i: g_pe_skins[i] owned */
    uint8_t worn, reserved[7];
    uint8_t creature[128];
    uint32_t checksum;
} PeProfile;

PeProfile *pe_col_profile(void);
void pe_col_changed(void);          /* written at the next field frame */
void pe_col_save_now(void);

/* Junk from the running game (the economy's `junk` service). */
int pe_col_junk_ready(char *why, unsigned size);   /* 0 with a reason when Junk can't be spent */
unsigned pe_col_junk(void);
int pe_col_spend(unsigned n);

void pe_col_say(const char *text);  /* a toast, also kept for the pages' Result rows */
const char *pe_col_last_said(void);

void pe_bestiary_register(const char *mod_id);
void pe_wardrobe_register(const char *mod_id);
void pe_bestiary_reset(void);
void pe_wardrobe_reset(void);

#endif
