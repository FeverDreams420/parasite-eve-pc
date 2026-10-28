#ifndef PE_PAGES_H
#define PE_PAGES_H

/* Where the PE Enhancements rows live in the Mods panel (F1). Every feature registers under one
 * of these, so the panel reads the same however the mods are combined:
 *
 *   PE Enhancements
 *     Movement            always run, speeds                        (pe_qol)
 *     Inventory           key item pocket, bag sorting              (pe_qol)
 *
 * The sub-page strings are passed as `sub` to pe_menu_register_* while PE_MENU_PAGE is the
 * current page; pe_pages_describe() gives each page its description and order. */

/* The PE Enhancements suite's version, one for all of its mods: they are released together. */
#define PE_ENHANCEMENTS_VERSION "0.9.0"

#define PE_PAGE_MOVEMENT       "Movement"
#define PE_PAGE_COMBAT         "Combat"
#define PE_PAGE_INVENTORY       "Inventory"
#define PE_PAGE_KEYS            "Inventory/Key items"

/* Descriptions and order of the pages above (host/pe_pages.c; called by the host at start). */
void pe_pages_describe(void);

#endif
