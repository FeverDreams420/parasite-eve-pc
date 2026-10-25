#ifndef PE_INVENTORY_SERVICE_H
#define PE_INVENTORY_SERVICE_H
#include <stdint.h>
/* keys v1, published by pe_qol. Optional consumers keep working without it. */
typedef struct PeKeysApi {
    int (*take)(uint16_t item); /* pocket a key; 1 means handled successfully */
} PeKeysApi;
#endif
