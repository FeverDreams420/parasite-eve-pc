#ifndef PE1_INVENTORY_RUNTIME_TYPES_H
#define PE1_INVENTORY_RUNTIME_TYPES_H
#include "pe1/inventory.h"

/* Retail adjacency: tracked selections at C0E20, 50 item IDs at C0E48,
 * and 128 equipment records at C0EAC. Reserved bytes are not interpreted. */
typedef struct InventoryRuntime {
    s8 tracked[4];
    u8 reserved04[0x24];
    s16 slots[50];
    ItemDataRecord equipment[128];
    u8 reserved108C[12];
    s16 storage[100];
    s16 specialStorage[82];
} InventoryRuntime;

PE1_STATIC_ASSERT(PE1_OFFSETOF(InventoryRuntime, slots) == 0x28,
                  inventory_runtime_slots_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(InventoryRuntime, equipment) == 0x8C,
                  inventory_runtime_equipment_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(InventoryRuntime, storage) == 0x1098,
                  inventory_runtime_storage_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(InventoryRuntime, specialStorage) == 0x1160,
                  inventory_runtime_special_storage_offset);

#endif
