#ifndef PE1_PM_TYPES_H
#define PE1_PM_TYPES_H

#include "common.h"

/* Both process-manager banks share this prefix. Pm_Exec increments ticks
 * at +4 after calling the command handler; +8 identifies the owning object. */
typedef struct PmSlotHeader {
    u8 state;
    u8 command;
    u8 field02;
    u8 field03;
    u32 ticks;
    void *owner;
} PmSlotHeader;

typedef int (*PmSendCallback)(PmSlotHeader *, int, int, intptr_t, intptr_t, intptr_t);

/* Handler table entries expose both scene-asset and process-manager callbacks. */
typedef struct PmCommand {
    void (*init)(void);
    void (*initialize)(PmSlotHeader *);
    PmSendCallback send;
    int (*start)(PmSlotHeader *);
    int (*execute)(PmSlotHeader *);
    int (*stop)(PmSlotHeader *);
    void (*unload)(void);
} PmCommand;

PE1_STATIC_ASSERT(PE1_OFFSETOF(PmCommand, initialize) == 1 * sizeof(((PmCommand *)0)->init),
                  pm_command_initializer_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmCommand, send) == 2 * sizeof(((PmCommand *)0)->init),
                  pm_command_send_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmCommand, start) == 3 * sizeof(((PmCommand *)0)->init),
                  pm_command_start_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmCommand, execute) == 4 * sizeof(((PmCommand *)0)->init),
                  pm_command_execute_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmCommand, stop) == 5 * sizeof(((PmCommand *)0)->init),
                  pm_command_stop_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmCommand, unload) == 6 * sizeof(((PmCommand *)0)->init),
                  pm_command_unload_offset);
PE1_STATIC_ASSERT(sizeof(PmCommand) == 7 * sizeof(((PmCommand *)0)->init), pm_command_size);

#include "pe1/fieldfx_state.h"
#include "pe1/fieldfx_task.h"

/* Primary slots contain either original object scripts or task owners.
 * Their native layouts can exceed the historical 0xA0C-byte record. */
typedef union PmPrimarySlot {
    struct {
        PmSlotHeader header;
        u8 reserved0C[0xA00];
    };
    FieldFxObject effect;
    FieldFxTaskOwner task;
} PmPrimarySlot;

typedef struct PmSecondarySlot {
    PmSlotHeader header;
    u8 reserved0C[0x100];
} PmSecondarySlot;

/* Scene startup partitions the buffer into eleven slots of each kind. */
typedef struct PmSlotBanks {
    PmPrimarySlot primary[11];
    PmSecondarySlot secondary[11];
} PmSlotBanks;

/* Seven adjacent pointer slots used by PM asset commands at 0x800E10A0. */
typedef struct PmAuxiliaryPointerTable {
    void *entries[7];
} PmAuxiliaryPointerTable;

PE1_STATIC_ASSERT(sizeof(PmSlotHeader) == 8 + sizeof(void *), pm_slot_header_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmSlotHeader, ticks) == 4, pm_slot_ticks_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmSlotHeader, owner) == 8, pm_slot_owner_offset);
PE1_STATIC_ASSERT(sizeof(PmPrimarySlot) >= sizeof(PmSlotHeader) + 0xA00, pm_primary_slot_size);
PE1_STATIC_ASSERT(sizeof(PmPrimarySlot) >= sizeof(FieldFxObject), pm_primary_effect_capacity);
PE1_STATIC_ASSERT(sizeof(PmPrimarySlot) >= sizeof(FieldFxTaskOwner), pm_primary_task_capacity);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmSlotHeader, command) == PE1_OFFSETOF(FieldFxObjectPrefix, asset_type), pm_effect_command_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmSlotHeader, field02) == PE1_OFFSETOF(FieldFxObjectPrefix, owner_kind), pm_effect_kind_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmSlotHeader, field03) == PE1_OFFSETOF(FieldFxObjectPrefix, live_slots), pm_effect_count_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmSlotHeader, owner) == PE1_OFFSETOF(FieldFxObjectPrefix, actor), pm_effect_owner_alias);
PE1_STATIC_ASSERT(sizeof(PmSecondarySlot) == sizeof(PmSlotHeader) + 0x100, pm_secondary_slot_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PmSlotBanks, secondary) == 11 * sizeof(PmPrimarySlot),
                  pm_secondary_bank_offset);
PE1_STATIC_ASSERT(sizeof(PmSlotBanks) == 11 * (sizeof(PmPrimarySlot) + sizeof(PmSecondarySlot)), pm_slot_banks_size);
PE1_STATIC_ASSERT(sizeof(PmAuxiliaryPointerTable) == 7 * sizeof(void *),
                  pm_auxiliary_pointer_table_size);


#endif
