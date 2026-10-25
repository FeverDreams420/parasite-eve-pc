#ifndef PE_COLLECTION_H
#define PE_COLLECTION_H

/* The Collection's private interface, shared by its files:
 *   pe_collection.c   the mod, Junk, messages
 *   pe_profile.c      the profile: unlocks by stable key, kept across games
 *   pe_bestiary.c     the creature registry (`bestiary` service), kills, Scan; F1 pages
 *   pe_wardrobe.c     the costume registry (`costumes` and `wardrobe` services); F1 pages
 *   pe_pack_pe1.c     Parasite Eve's own creatures and costumes, registered like any other pack
 * Everything is cosmetic or information: it never changes a fight. Other mods add creatures and
 * costumes through the services (host/pe_services.h), never through these files. */

#include <stdint.h>

#include "pe_services.h"

/* ---- profile (pe_profile.c) ----
 * pe_collection.ini beside the preferences, a line per record:
 *   creature <key> <bits> <kills>
 *   costume <key>            (owned)
 *   worn <key>
 * Records of packs that aren't loaded are kept as read. The psxrecomp build's pe_collection.dat
 * (creatures and skins by index) is imported once through the PE pack's keys. */
void pe_profile_load(void);
void pe_profile_save(void);               /* now */
void pe_profile_changed(void);            /* at the next field frame */
void pe_profile_tick(void);
void pe_profile_clear(void);
unsigned pe_profile_creature(const char *key, unsigned *kills);   /* bits */
void pe_profile_set_creature(const char *key, unsigned bits, unsigned kills);
int pe_profile_owned(const char *key);
void pe_profile_own(const char *key);
const char *pe_profile_worn(void);        /* "" = Aya's own */
void pe_profile_set_worn(const char *key);

/* The v1 import needs the PE pack's keys by the recomp's indexes. */
const char *pe_pack_pe1_creature_key(int index);
const char *pe_pack_pe1_costume_key(int index);
void pe_pack_pe1_register(void);

/* ---- Junk and messages (pe_collection.c) ---- */
int pe_col_junk_ready(char *why, unsigned size);   /* 0 with a reason when Junk can't be spent */
unsigned pe_col_junk(void);
int pe_col_spend(unsigned n);
void pe_col_say(const char *text);        /* a toast, also kept for the pages' Result rows */
const char *pe_col_last_said(void);

/* ---- registries ---- */
extern const PeBestiaryApi g_pe_bestiary_api;
extern const PeCostumesApi g_pe_costumes_api;
void pe_bestiary_register(const char *mod_id);
void pe_wardrobe_register(const char *mod_id);
void pe_bestiary_reset(void);
void pe_wardrobe_reset(void);

#endif
