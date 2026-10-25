#include <string.h>

#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_ps1.h"
#include "pe1/game_timers.h"
#include "pe1/inventory_state.h"
#include "pe1/resident_tables.h"
#include "pe1/save_blob.h"

extern u8 g_MemCardFileBuffer[0x2000];

static const struct { uint32_t base, size; } s_regions[] = {
    { 0x800A8028u, sizeof g_ResidentDataPack.bytes },
    { 0x800C0DE0u, sizeof g_SaveRuntimeState },
    { 0x8009EED0u, 0x2000u },
    { 0x800A77F0u, 0x800u },
    { 0x8009D018u, 4u },
};

static uint8_t *pe_region_base(int i) {
    switch (i) {
    case 0: return g_ResidentDataPack.bytes;
    case 1: return (uint8_t *)&g_SaveRuntimeState;
    case 2: return g_MemCardFileBuffer;
    case 3: return (uint8_t *)g_ScriptVariableBank;
    default: return (uint8_t *)&g_InvBonusSlotCount;
    }
}

void *pe_ps1_ptr(uint32_t addr, uint32_t size) {
    static uint32_t s_logged[16];
    static int s_logged_count;
    int i;
    for (i = 0; i < (int)(sizeof s_regions / sizeof s_regions[0]); ++i)
        if (addr >= s_regions[i].base && addr - s_regions[i].base + size <= s_regions[i].size)
            return pe_region_base(i) + (addr - s_regions[i].base);
    for (i = 0; i < s_logged_count; ++i) if (s_logged[i] == addr) return NULL;
    if (s_logged_count < 16) s_logged[s_logged_count++] = addr;
    pe_mod_log(NULL, "unmapped PS1 address 0x%08X (ported code needs a native variable)", (unsigned)addr);
    return NULL;
}

uint8_t psx_mod_read_byte(uint32_t addr) { uint8_t *p = pe_ps1_ptr(addr, 1); return p ? *p : 0; }
uint16_t psx_mod_read_half(uint32_t addr) { uint16_t v = 0; void *p = pe_ps1_ptr(addr, 2); if (p) memcpy(&v, p, 2); return v; }
uint32_t psx_mod_read_word(uint32_t addr) { uint32_t v = 0; void *p = pe_ps1_ptr(addr, 4); if (p) memcpy(&v, p, 4); return v; }
void psx_mod_write_byte(uint32_t addr, uint8_t v) { uint8_t *p = pe_ps1_ptr(addr, 1); if (p) *p = v; }
void psx_mod_write_half(uint32_t addr, uint16_t v) { void *p = pe_ps1_ptr(addr, 2); if (p) memcpy(p, &v, 2); }
void psx_mod_write_word(uint32_t addr, uint32_t v) { void *p = pe_ps1_ptr(addr, 4); if (p) memcpy(p, &v, 4); }
int psx_mod_game_started(void) { return pe_game_loaded(); }
