#ifndef PE1_BATTLE_WORD_RECORDS_H
#define PE1_BATTLE_WORD_RECORDS_H

#include "common.h"

struct Combatant;
struct BattleEntity;

/* One-field record views of single battle words (see room_sound_slot.h).
 * A read through a record is a structure access: GCC reloads it after a
 * store through a structure pointer, where a plain scalar at a constant
 * address is assumed not to alias and is reused. At one word the records
 * stay small data, so -G8 units address them directly instead of keeping
 * the address in a register. The linker map gives each record the address
 * of the word it views. */
typedef struct BattleActorRecord {
    struct Combatant *actor;
} BattleActorRecord;

typedef struct BattleEntityRecord {
    struct BattleEntity *entity;
} BattleEntityRecord;

typedef struct BattleFlagRecord {
    s32 flags;
} BattleFlagRecord;

/* g_BattleResultFlags as bit fields. The low byte is a message id and bits
 * 8-9 keep the info panel up (Battle_Update draws it while they are set,
 * Menu_MainUpdate and Battle_Init clear them). The Scan technique
 * (Battle_ApplySpellEffect case 7) sets message 0x4B and records which of
 * the target's statusFlags2 pairs read 2; the name of each field is the
 * pair's mask. Retail stores these as bit fields: each assignment is one
 * masked read-modify-write of the word. */
typedef union BattleResultFlagsRecord {
    s32 flags;
    struct {
        unsigned int messageId : 8;
        unsigned int infoPanel : 2;
        unsigned int scanPairC0 : 2;
        unsigned int scanPair3 : 1;
        unsigned int scanPairC : 1;
        unsigned int scanPair30 : 1;
        unsigned int scanPairC000 : 1;
        unsigned int scanPair30000 : 1;
        unsigned int scanPair3000 : 1;
        unsigned int scanPairC00 : 1;
        unsigned int reserved19 : 13;
    } bits;
} BattleResultFlagsRecord;

extern BattleActorRecord g_ActiveActorRecord;     /* g_ActiveActor, 0x8009D278 */
extern BattleEntityRecord g_PlayerEntityRecord;   /* g_PlayerEntity, 0x8009D254 */
extern BattleFlagRecord g_GameStateFlagsRecord;   /* g_GameStateFlags, 0x8009D1A0 */
extern BattleResultFlagsRecord g_BattleResultFlagsRecord; /* g_BattleResultFlags, 0x8009D1AC */
extern BattleFlagRecord g_FieldMoveLockRecord;    /* g_FieldMoveLock, 0x8009D2E8 */

#endif
