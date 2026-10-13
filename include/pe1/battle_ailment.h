#ifndef PE1_BATTLE_AILMENT_H
#define PE1_BATTLE_AILMENT_H

/*
 * Combatant.stateFlags (+0x4C) ailments and the Parasite Energy buffs that
 * share that word. Integer macros, not an enum: GCC 2.7 keeps the same
 * code when a literal is replaced by a macro of the same value.
 *
 * The four low pairs use the same protocol. 0 is clear. The afflicted value
 * has one bit set. Both bits set is the one-use immunity left by using the
 * matching cure while healthy; the next matching attack consumes that pair
 * and does not apply the ailment. Battle_ApplyDamage cases 13-16 implement
 * the cure, and Draw_StatusIcons draws one icon per pair.
 *
 * In-game names are PE.IMG string table 2 (Text_LookupTableC), decoded with
 * tools/scripts/pe1_text.py. The cure item id is the Battle_ApplyDamage kind
 * (Inv_SetActiveList mode 0 stores itemId + 3, and the hit path subtracts 3).
 *
 *   item 0x0D Cure-P  "Cures & Prevents Poison"
 *   item 0x0E Cure-D  "Cures & Prevents Darkness"
 *   item 0x0F Cure-C  "Cures & Prevents Confusion"
 *   item 0x10 Cure-M  "Cures & Prevents Stiffness"
 */

#define AYA_POISON_MASK    0x3
#define AYA_POISON         0x1
#define AYA_POISON_IMMUNE  0x3

#define AYA_CONFUSION_MASK    0xC
#define AYA_CONFUSION         0x4
#define AYA_CONFUSION_IMMUNE  0xC

/* While Darkness is afflicted, weapon range is forced to 200. */
#define AYA_DARKNESS_MASK    0x30
#define AYA_DARKNESS         0x10
#define AYA_DARKNESS_IMMUNE  0x30
#define AYA_DARKNESS_RANGE   200

/*
 * Cure-M covers both values, and both draw status-icon code 2. The lock
 * zeroes field motion. The other value cuts field speed to
 * AYA_MOVE_SPEED_STIFF. The PE tech Slow is a different effect: it slows an
 * enemy's AT recharge and does not use this pair.
 */
#define AYA_STIFFNESS_MASK    0xC0
#define AYA_STIFFNESS_SLOW    0x40
#define AYA_STIFFNESS_LOCK    0x80
#define AYA_STIFFNESS_IMMUNE  0xC0

/* Set on D_8009D2E8 while Confusion is afflicted. Reverses facing and motion. */
#define FIELD_MOVE_CONFUSED 0x10

/* Random facing nudge stored in stateFlags bits 17-18 by the confusion attack. */
#define AYA_CONFUSION_ANGLE_MASK 0x60000

#define AYA_MOVE_SPEED_STIFF    0xB4000
#define AYA_MOVE_SPEED_NORMAL   0x168000
#define AYA_MOVE_SPEED_HASTE    0x21C000

/*
 * Buffs set by Parasite Energy. Battle_UseItem looks up item (index + 0xEC)
 * and queues command (index + 0x183). Battle_ApplySpellEffect switches on
 * command - 0x183, so the case number is the tech index. Descriptions are
 * Text_LookupTableC(0xEB + index).
 *
 *   5  Barrier      "Uses PE to Absorb Damage"         sets 0x200
 *   9  Haste        "Speeds Up Movement and AT Recharge" sets 0x100
 *   11 Gene Heal    "Uses PE to Slowly Recover HP"     sets 0x400
 *   12 Preraise     "Revives When HP Reaches 0"        sets 0x800
 *
 * Haste also clears a Stiffness affliction. Barrier makes
 * Entity_ResolveDropTable subtract from PE instead of HP, and it drops when
 * PE is at its floor. Gene Heal adds 1 HP while draining PE. Preraise is
 * consumed on the curHP <= 0 path, in the same branch as item Revive (0x12).
 * Enemy effect type 2 clears all four.
 *
 * 0x1000 is not named here. Enemy effect type 7 sets it, Full Cure / Medic /
 * PE Full Recover clear it, and the damage path scales defense by 3/10 while
 * it or Haste is set. It has its own HUD icon.
 */
#define AYA_HASTE      0x100
#define AYA_BARRIER    0x200
#define AYA_GENE_HEAL  0x400
#define AYA_PRERAISE   0x800

/* The same word as bit fields. Retail updates some of these pairs as bit
 * fields: a field store is a read-modify-write of the word, and a field
 * read after it loads the word again. Stiffness values: 1 slow, 2 lock. */
typedef struct CombatantStateBits {
    unsigned int poison : 2;
    unsigned int confusion : 2;
    unsigned int darkness : 2;
    unsigned int stiffness : 2;
    unsigned int haste : 1;
    unsigned int barrier : 1;
    unsigned int geneHeal : 1;
    unsigned int preraise : 1;
    unsigned int reserved : 20;
} CombatantStateBits;
#define AYA_STIFFNESS_SLOW_VALUE 1
#define AYA_STIFFNESS_LOCK_VALUE 2
/* Combatant.stateFlags viewed as CombatantStateBits. */
#define COMBATANT_STATE_BITS(combatant) (*(CombatantStateBits *)&(combatant)->stateFlags)

#define PE_SPELL_COMMAND_BASE 0x183
#define PE_SPELL_HEAL_1       0
#define PE_SPELL_HEAL_2       1
#define PE_SPELL_HEAL_3       2
#define PE_SPELL_DETOX        3
#define PE_SPELL_MEDIC        4
#define PE_SPELL_BARRIER      5
#define PE_SPELL_ENERGY_SHOT  6
#define PE_SPELL_SCAN         7
#define PE_SPELL_SLOW         8
#define PE_SPELL_HASTE        9
#define PE_SPELL_CONFUSE      10
#define PE_SPELL_GENE_HEAL    11
#define PE_SPELL_PRERAISE     12
#define PE_SPELL_FULL_RECOVER 18
#define PE_SPELL_LIBERATE     19

/* Battle_ApplyEnemyAttack dispatch ids (EnemyActionEffect.effectType). */
#define ENEMY_EFFECT_POISON          1
#define ENEMY_EFFECT_CLEAR_BUFFS     2
#define ENEMY_EFFECT_CONFUSION       3
#define ENEMY_EFFECT_DARKNESS        4
#define ENEMY_EFFECT_STIFFNESS_LOCK  5
#define ENEMY_EFFECT_STIFFNESS_SLOW  6
#define ENEMY_EFFECT_HALVE_HP        8
#define ENEMY_EFFECT_REDUCE_HP_TO_1  9
#define ENEMY_EFFECT_STEAL_ITEM      10
#define ENEMY_EFFECT_STEAL_ITEM_ALT  11
#define ENEMY_EFFECT_STEAL_AMMO      12
#define ENEMY_EFFECT_STEAL_AMMO_ALT  13
/* Leaves three quarters of current HP. Types 10/11 and 12/13 share one body. */
#define ENEMY_EFFECT_HP_TO_THREE_QUARTERS 16

/*
 * BattleAttributes.effectFlags. Armor tail bytes 1-7
 * (Inv_BuildArmorList, tailData & 31) set these bits, and the attack
 * path tests the same bits before applying an ailment or a steal.
 * 0x4000 / 0x8000 are the auto-use medicine and auto-use cure mods.
 */
#define EFFECT_RESIST_POISON     0x1
#define EFFECT_RESIST_CONFUSION  0x2
#define EFFECT_RESIST_DARKNESS   0x4
#define EFFECT_RESIST_STIFFNESS  0x8
#define EFFECT_RESIST_STEAL      0x10
#define EFFECT_AUTO_MEDICINE     0x4000
#define EFFECT_AUTO_CURE         0x8000

/* Resist roll: the attack is ignored when rand() % 100 is below this. */
#define AYA_POISON_RESIST_CHANCE         0x46
#define AYA_CONFUSION_RESIST_CHANCE      0x3C
#define AYA_DARKNESS_RESIST_CHANCE       0x3C
#define AYA_STIFFNESS_LOCK_RESIST_CHANCE 0x32
#define AYA_STIFFNESS_SLOW_RESIST_CHANCE 0x46
#define ENEMY_STEAL_RESIST_CHANCE        0x3C

#endif /* PE1_BATTLE_AILMENT_H */
