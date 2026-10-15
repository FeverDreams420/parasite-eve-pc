#ifndef PE1_BATTLE_CMD_H
#define PE1_BATTLE_CMD_H

#include <stddef.h>

struct ItemDataRecord;

typedef struct SquareMessageEntry {
    /* 0x00 */ unsigned char opcode;
    /* 0x01 */ unsigned char status;
    /* 0x02 */ short slot_idx;
    /* 0x04 */ int arg_04;
    /* 0x08 */ int arg_08;
    /* 0x0C */ union {
        struct {
            unsigned short arg_0C;
            unsigned short arg_0E;
        } halves;
        int word;
    } u;
    /* 0x10 */ int unk_10;
    /* 0x14 */ int arg_14;
    /* 0x18 */ int unk_18;
    /* 0x1C */ int arg_1C;
    /* 0x20 */ int arg_20;
} SquareMessageEntry;

typedef union BattleCmdHeader {
    /* Command code is written and compared as a full word by the battle queue. */
    unsigned int word;
    struct {
        unsigned char opcode;
        unsigned char status;
        short slot_index;
    } bytes;
} BattleCmdHeader;

typedef union BattleCmdPayload {
    struct {
        int arg_04;
        int arg_08;
        int arg_0C;
        int arg_10;
        int arg_14;
        int arg_18;
        int arg_1C;
        int arg_20;
    } generic;
    /* opcode 0: restore an inventory slot removed by a pending command. */
    struct {
        int item_id;
        int slot_index;
        unsigned char reserved0C[0x18];
    } inventory_restore;
    /* opcode 1: pending ammunition consumption for an inventory item. */
    struct {
        int item_index;
        int amount;
        unsigned char reserved0C[0x18];
    } ammo_spend;
    /* opcodes 2 and 3: restore the previous equipped item. */
    struct {
        struct ItemDataRecord *item_data;
        unsigned char reserved08[0x1C];
    } equip_restore;
    /* opcode 4: restore ammo values across three item-data records. */
    struct {
        struct ItemDataRecord *item_data0;
        struct ItemDataRecord *item_data1;
        struct ItemDataRecord *item_data2;
        int reserved10;
        int ammo0;
        int reserved18;
        int ammo1;
        int ammo2;
    } ammo_restore;
} BattleCmdPayload;

typedef struct BattleCmdEntry {
    /* 0x00 */ BattleCmdHeader header;
    /* 0x04 */ BattleCmdPayload payload;
} BattleCmdEntry;

/* Runtime records contain native pointers; they are not serialized data. */
_Static_assert(sizeof(BattleCmdHeader) == 4, "command header");
_Static_assert(offsetof(BattleCmdEntry, payload.ammo_spend.amount) ==
               offsetof(BattleCmdEntry, payload) + sizeof(int), "ammo amount");
_Static_assert(offsetof(BattleCmdPayload, ammo_restore.ammo2) ==
               3 * sizeof(void *) + 4 * sizeof(int), "restore tail");

/* Four pending commands, independent of the host's global-variable layout. */
extern BattleCmdEntry g_BattleCmdStack[4];
extern BattleCmdEntry *g_BattleCmdStackTop;

void BattleCmd_InitTableCursor(void);
void BattleCmd_ResetTableCursor(void);
BattleCmdEntry *BattleCmd_AllocSlot(void);
void BattleCmd_UndoPending(void);
void BattleCmd_UndoAllPending(void);
int BattleCmd_GetRemainingAmmo(int *out);
void BattleCmd_CommitAndSyncAmmo(int item);
/* Synchronizes ammo, then rebuilds the supplied native action record. */
int BattleCmd_CommitAmmoAndUpdate(void *out);
void BattleCmd_ChangeWeaponAndSync(int item);


void BattleCmd_SetCurrentHP(int value);
void BattleCmd_SetCurrentMP(int value);

int Battle_GetEnemyEscapeFlag(void);

void BattleCmd_SyncActiveAmmo(void);

#endif
