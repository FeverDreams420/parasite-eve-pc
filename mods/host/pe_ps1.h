#ifndef PE_PS1_H
#define PE_PS1_H

/* PS1-address access for code ported from the psxrecomp build, limited to the native objects
 * whose layout IS the PS1 one, so offsets within them stay valid:
 *
 *   0x800A8028 +0x8004  g_ResidentDataPack   (directory, text, item base data, level tables)
 *   0x800C0DE0 +0x12E4  g_SaveRuntimeState   (Aya, bag, equipment records, Wayne's storage;
 *                                             copied byte for byte into the save file)
 *   0x8009EED0 +0x2000  g_MemCardFileBuffer  (the 8 KB card block)
 *   0x800A77F0 +0x0800  g_ScriptVariableBank
 *   0x8009D018          g_InvBonusSlotCount
 *
 * Anything else reads 0 and ignores writes, and is logged once: native pointers and other
 * globals must use the game's own variables instead. */

#include <stdint.h>

uint8_t psx_mod_read_byte(uint32_t addr);
uint16_t psx_mod_read_half(uint32_t addr);
uint32_t psx_mod_read_word(uint32_t addr);
void psx_mod_write_byte(uint32_t addr, uint8_t v);
void psx_mod_write_half(uint32_t addr, uint16_t v);
void psx_mod_write_word(uint32_t addr, uint32_t v);
int psx_mod_game_started(void);
/* Native pointer for a PS1 address in one of the regions above, or NULL. */
void *pe_ps1_ptr(uint32_t addr, uint32_t size);

#endif
