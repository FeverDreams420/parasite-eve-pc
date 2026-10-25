#ifndef PE_REQUISITION_H
#define PE_REQUISITION_H
#include <stddef.h>
#include <stdint.h>
/* Stable goods identities survive changes to the visible board. Economy
 * unlocks use 0xF000..0xF003, never real inventory values. */
int pe_shop_count(void);
uint16_t pe_shop_item(int row);
const char *pe_shop_name(uint16_t item);
uint32_t pe_shop_price(uint16_t item);
const char *pe_shop_blocked(uint16_t item);
int pe_shop_buy(uint16_t item, char *message, size_t size);
int pe_shop_rescue_needed(void);
int pe_shop_spare_club(char *message, size_t size);
void pe_shop_details(uint16_t item, char *out, unsigned size);
void pe_requisition_register(const char *mod_id);
void pe_requisition_menu_register(const char *mod_id);
#endif
