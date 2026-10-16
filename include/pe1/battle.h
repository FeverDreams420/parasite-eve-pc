#ifndef PE1_BATTLE_H
#define PE1_BATTLE_H

#include "common.h"
#include "pe1/battle_ailment.h"
#include "pe1/field_actor.h"

/* Scan result word and message overlay fields. */
typedef union BattleResultFlags {
    u32 flags;
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
} BattleResultFlags;

PE1_STATIC_ASSERT(sizeof(BattleResultFlags) == 4, battle_result_flags_size);

typedef struct BattleRewardSlot {
    s16 id;
    s16 extra;
} BattleRewardSlot;

/* Sound effect selected by each battle action. The table at 0x80010760 is
 * copied as a 40-byte block by Battle_AdvancePhase and indexed by action. */
typedef struct BattleActionSoundTable {
    u16 soundId[20];
} BattleActionSoundTable;
PE1_STATIC_ASSERT(sizeof(BattleActionSoundTable) == 0x28,
                  battle_action_sound_table_size);

extern u8 *g_CurItemEffectData;

/* The first word at the shared game-state window is the global mode bitfield.
 * Battle code also views the following 12 bytes as part of one 16-byte record. */
typedef struct BattleGameStateWindow {
    u32 flags;
    u8 unknown04[12];
} BattleGameStateWindow;
PE1_STATIC_ASSERT(sizeof(BattleGameStateWindow) == 16, battle_game_state_window_size);

/* Battle subsystem (ATB combat). Layout reverse-engineered from the battle code
 * (src/main/battle) and validated live on real
 * BIOS (DuckStation GDB) by fighting the first Carnegie Hall encounter and
 * cross-checking enemy stats against the wiki (enemy max HP read as 40).
 *
 * These typedefs are shared runtime views for the native game build.
 * Direct member access is preferred where it preserves code generation; older
 * compiler-sensitive functions derive their accesses from PE1_OFFSETOF so the
 * layout still has one executable source of truth. Fields marked TENTATIVE need
 * more confirmation.
 */

/* ============================================================================
 * TWO-LAYER MODEL (verified)
 *   g_PlayerEntity (0x8009D254)  -> BattleEntity   (field-actor node)
 *   g_FieldActorListHead         -> BattleEntity   (singly-linked, +0x04 = next)
 *   g_ActiveActor  (0x8009D278)  == *g_PlayerEntity == BattleEntity.core
 *                                -> Combatant in the player-side paths below
 *
 *   Proof: Battle_DispatchInit.c:17        g_ActiveActor = *g_PlayerEntity;
 *          battle/turn.c:143 g_ActiveActor = actor->core;
 *          battle/attack.c:496          g_ActiveActor = *(void**)g_PlayerEntity;
 *          Battle_Set/GetContextField use   ctx = *(u8**)g_PlayerEntity, index ctx.
 *
 *   The old prose notes conflated the two: the side byte / screen coords / a
 *   "player max HP" the original note placed on the combatant are really
 *   BattleEntity fields (the note read them off g_PlayerEntity, the ENTITY). On
 *   the Combatant, +0x1C is genuinely max HP and the side byte is entity+0x0D.
 *
 *   Battle_IsActive(): active iff g_BattleActionQueueCount==0
 *     && (g_GameStateFlags[0] & 2)!=0
 *     && !(combatant->stateFlags & 0x10000)        (stateFlags = Combatant+0x4C)
 *
 *   Quick live probe: in battle iff (*(u32*)0x8009D1A0 & 2);
 *     entity    = *(void**)0x8009D254;
 *     combatant = *(void**)0x8009D278;   (== *entity)
 *     curHP = *(u16*)(combatant + 0x0C); ATB = *(s32*)(combatant + 0x34).
 * ==========================================================================*/

/* Packed battle parameters copied from the encounter attribute globals. The
 * bit ranges in parameterWord are proven, but their gameplay names are still
 * uncertain; keep them packed until those meanings are corroborated.
 */
typedef union BattleParameterWord {
    u32 raw;
    struct {
        unsigned int first : 10;
        unsigned int second : 10;
        unsigned int third : 8;
        unsigned int reserved : 4;
    } fields;
} BattleParameterWord;

typedef struct BattleAttributes {
/* 0x00 */ BattleParameterWord parameterWord;
/* 0x04 */ u32 effectFlags;   /* EFFECT_RESIST_* / EFFECT_AUTO_* in battle_ailment.h */
} BattleAttributes;

/* Effect descriptor referenced by an enemy's battle-state record. This is a
 * different pointer from Combatant.action at +0x68.
 */
typedef struct EnemyActionEffect {
/* 0x00 */ u8  state;       /* 1=ready; changed to 3/4 by drop resolution */
/* 0x01 */ u8  effectType;  /* ENEMY_EFFECT_* dispatch id; Battle_ApplyEnemyAttack */
/* 0x02 */ u8  enterMode;
/* 0x03 */ u8  exitMode;
/* 0x04 */ s32 enterStep;
/* 0x08 */ s32 exitStep;
/* 0x0C */ u16 power;
/* 0x0E */ u8  category;    /* selects which packed BattleAttributes value applies */
/* 0x0F */ u8  frame;       /* animation frame that changes state to 3 */
} EnemyActionEffect;

/* Animation-event view of an entity core. The six inline event records
 * overlap fields in EnemyCombatant's battle-state view; keep the views
 * separate until their shared storage/lifetime is established.
 */
/* The selected inline record occupies bits 21..23. The meanings of the
 * separately cleared high bits are not yet established. */
typedef union EntityAnimEventFlags {
    u32 raw;
    struct {
        u32 reservedLow : 21;
        u32 selectedRecord : 3;
        u32 reservedHigh : 6;
        u32 bit30 : 1;
        u32 bit31 : 1;
    } bits;
} EntityAnimEventFlags;

typedef struct EntityAnimEventCore {
/* 0x00 */ EntityAnimEventFlags flags;
/* 0x04 */ u8 rank;
/* 0x05 */ s8 kind;
/* 0x06 */ u8 baseMode;
/* 0x07 */ u8 reserved07;
/* 0x08 */ u8 reserved08[0x10];
/* 0x18 */ EnemyActionEffect *active;
/* 0x1C */ EnemyActionEffect records[6];
/* 0x7C */ u8 parameters[6][4]; /* four per-record bytes written by script setup */
/* 0x94 */ u8 reserved94[0x1E];
/* 0xB2 */ u16 assetId;
} EntityAnimEventCore;

PE1_STATIC_ASSERT(sizeof(EntityAnimEventFlags) == 4, entity_anim_event_flags_size);





PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyActionEffect, enterStep) == 4,
                  enemy_action_effect_enter_step_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyActionEffect, exitStep) == 8,
                  enemy_action_effect_exit_step_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyActionEffect, frame) == 15,
                  enemy_action_effect_frame_offset);



/* Eight-byte descriptor consumed by Battle_DrawStatusPanel. The last two
 * bytes are timer/style values whose gameplay meaning varies by panel slot. */
typedef struct BattleStatusPanel {
    s16 value;
    u16 x;
    u16 y;
    u8 timer;
    u8 style;
} BattleStatusPanel;

PE1_STATIC_ASSERT(sizeof(BattleStatusPanel) == 8, battle_status_panel_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleStatusPanel, x) == 2,
                  battle_status_panel_x_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleStatusPanel, y) == 4,
                  battle_status_panel_y_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleStatusPanel, timer) == 6,
                  battle_status_panel_timer_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleStatusPanel, style) == 7,
                  battle_status_panel_style_offset);

/* Player and enemy contexts share this two-byte prefix. Script context
 * access also reads and writes both bytes as a single signed halfword. */
typedef union CombatantIdentity {
    s16 fieldId04;
    struct {
        u8 rank;
        s8 kind;
    } bytes;
} CombatantIdentity;

PE1_STATIC_ASSERT(sizeof(CombatantIdentity) == 2, combatant_identity_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CombatantIdentity, bytes.kind) == 1,
                  combatant_identity_kind_offset);

/* ----------------------------------------------------------------------------
 * Combatant -- the active/player-side stat record. g_ActiveActor points here
 * in the setup and command paths below. Enemy entity cores share several
 * offsets but diverge at +0x10; use EnemyCombatant for those records.
 * --------------------------------------------------------------------------*/
typedef struct Combatant {
/* 0x00 */ u32  coreFlags;     /* bitfield; &0x6000 / &0xC0000 facing, >>13&3 weapon-cat (battle/context.c:302,318,329) */
/* 0x04 */ CombatantIdentity field04;
/* 0x06 */ s16  fieldId06;     /* SetContextField case2 (Battle_SetContextField.c:24) */
/* 0x08 */ s32  exp_or_acc;    /* damage-derived accumulator, clamped to maxAtk@0x28 / 0x10000 (battle/attack.c:537,541) */
/* 0x0C */ u16  curHP;         /* CURRENT HP -- damage & heal land here; clamped to maxHP@0x1C (battle/attack.c:518-530) */
/* 0x0E */ u16  hpMirror;      /* current-HP MIRROR; re-synced from curHP each reset (Battle_ResetEnemyStats.c:42) */
/* 0x10 */ u16  hpAlive;       /* authoritative ">0 == alive" HP (target tests); on player doubles as EXP/level gauge clamped 0x2328 (battle/setup.c:436; battle/encounter.c:373). TENTATIVE width: some reads s32 */
/* 0x12 */ u8   actionMode12;  /* action-mode id table [0x12..0x19], filled per enemy by facing (battle/enemy_actions.c:38-46): 4 idle, 0xD special */
/* 0x13 */ s8   actionMode13;
/* 0x14 */ u8   targetAngleActionModes[3]; /* selected by the relative target angle in Battle_AdvancePhase */
/* 0x17 */ u8   followUpAngleActionModes[3]; /* modes 7/9/11 that end the attack started by targetAngleActionModes */
/* 0x1A */ u8   pad_1A[2];
#if UINTPTR_MAX > UINT32_MAX
    /* The shared FieldActorState view has a host pointer at old offset 0x18. */
    u8 hostActionStatePadding[sizeof(void *) - 4];
#endif
/* 0x1C */ u16  maxHP;         /* MAX HP; curHP clamped to this (battle/attack.c:530; Battle_ResetEnemyStats.c:34) */
/* 0x1E */ s16  stat1E;        /* SetContextField case11 (Battle_SetContextField.c:42) */
/* 0x20 */ s16  stat20;        /* SetContextField case12 (Battle_SetContextField.c:45) */
/* 0x22 */ u16  stat22;        /* base for scaled-offense calc (battle/enemy_actions.c:39) */
/* 0x24 */ u16  scaledOffense; /* = stat22 * tbl[anim] / 10 (battle/enemy_actions.c:39) */
/* 0x26 */ s16  stat26;        /* SetContextField case18 (Battle_SetContextField.c:50) */
/* 0x28 */ s32  maxAtk;        /* cap for exp_or_acc@0x08 (battle/attack.c:533,540; Battle_StartEncounter.c:116) */
/* 0x2C */ s32  atbStep;       /* per-frame increment subtracted from atbRate */
/* 0x30 */ s32  atbRate;       /* divisor/rate used while charging exp_or_acc */
/* 0x34 */ s32  atbGauge;      /* ATB / "AT" gauge; reset 0, latched 0xF0 (Battle_ResetEnemyStats.c:41; Battle_StartEncounter.c:119) */
/* 0x38 */ u16  subActionStep;
/* 0x3A */ u8   subActionCounter;
/* 0x3B */ u8   subActionPeriod;
/* 0x3C */ u16  statusStep3C;  /* decrement used by status timer groups 0x0C..0xC0 */
/* 0x3E */ u16  statusStep3E;  /* decrement used by stateFlags group 0x03 */
/* 0x40 */ u16  statusTimer40;
/* 0x42 */ u16  statusTimer42;
/* 0x44 */ u16  statusTimer44;
/* 0x46 */ u16  statusTimer46;
/* 0x48 */ s8   knockbackFrames;
/* 0x49 */ u8   knockbackDistance;
/* 0x4A */ s16  knockbackAngle;
/* 0x4C */ s32  stateFlags;    /* PRIMARY status/state bitfield. Ailment pairs and buffs: battle_ailment.h. 0x10000="battle ended" (Battle_IsActive), 0x80000 special-move guard, 0x100000 acted-this-frame, 0x200000 action-committed (Battle_ApplyPlayerHit.c:53,63; Battle_UpdatePlayerTurn.c:47) */
/* 0x50 */ s16  panelA_val;    /* floating damage-number panel A: {s16 val; u16 x; u16 y; u8 scale; u8 mode}; drawn by Battle_DrawStatusPanel(0, &panelA) (Battle_PhaseHitReaction.c:191) */
/* 0x52 */ u16  panelA_x;      /* x copied from entity projX@0x210 (Battle_PhaseHitReaction.c:188) */
/* 0x54 */ u16  panelA_y;      /* y from entity projY@0x212 */
/* 0x56 */ u8   panelA_timer;  /* countdown; 0=inactive, 0x1E=just-spawned (Battle_PhaseHitReaction.c:185,192) */
/* 0x57 */ u8   panelA_scale;
/* 0x58 */ s16  panelB_val;    /* floating panel B (Battle_PhaseHitReaction.c:203) */
/* 0x5A */ u16  panelB_x;
/* 0x5C */ u16  panelB_y;
/* 0x5E */ u8   panelB_timer;
/* 0x5F */ s8   panelB_flag;
/* 0x60 */ u16  panelAux_val;
/* 0x62 */ u16  panelAux_x;
/* 0x64 */ u16  panelAux_y;
/* 0x66 */ u8   panelAux_timer; /* set to 0x1E when Battle_SubActionStep emits the descriptor */
/* 0x67 */ u8   panelAux_mode;
/* Scalar event slots; native pointers follow the scalar state. */
              u32 reserved_action_slot, reserved_attributes_slot, reserved_aux_slot;
/* 0x74 */ u8   pad_74[0x14];
/* 0x88 */ s32  field88;       /* GetEnemyCtx case19, clamped >=0 (battle/context.c:309) */
/* 0x8C */ u16  field8C;       /* GetEnemyCtx case20 (battle/context.c:315) */
/* 0x8E */ u8   pad_8E[0x3E];
/* 0xCC */ u32  statusFlags2;  /* secondary status bitfield: 0x1000000 petrify-ish, &0x3C000 / &0xC000 / &0x30000 status, &0x30/0xC attack-element gates (battle/context.c:327; Battle_StartEnemyAttackEffect.c:25-64) */
/* 0xD0 */ s16  panelC_val;    /* enemy floating panel C: Battle_DrawStatusPanel(1, &panelC) (Battle_PhaseHitReaction.c:226) */
/* 0xD2 */ u16  panelC_x;
/* 0xD4 */ u16  panelC_y;
/* 0xD6 */ u8   panelC_timer;
/* 0xD7 */ u8   panelC_mode;
              struct BattleAction *action;
              BattleAttributes *attributes;
              void *field70;
} Combatant;

/* Enemy entity core. Scalar fields share the field-state storage, but the
 * event records and player pointers have different lifetimes. In particular
 * effect@0x18 selects one of the six records beginning at old offset 0x1C.
 */
typedef struct EnemyCombatant {
/* 0x00 */ u32 coreFlags;
/* 0x04 */ CombatantIdentity field04;
/* 0x06 */ union {
               s16 field06;
               struct { u8 low; u8 entityId; } bytes;
           } lowAndEntityId;
/* 0x08 */ s32 slotBit; /* 1 << allocated enemy slot */
/* 0x0C */ u16 curHP;
/* 0x0E */ u16 hpMirror;
/* 0x10 */ s32 hpAlive;
/* 0x14 */ s32 previousHP; /* previous update snapshot for the floating damage panel */
/* 0x18 */ EnemyActionEffect *effect;
/* Six inline events share scalar enemy-state storage, not the player's
 * native action/attributes pointers. Both views widen only effect@0x18. */
    union {
        EnemyActionEffect records[6];
        struct {
            u8 pad_1C[0x30];
            u32 stateFlags;
            u8 pad_50[0x2C];
        };
    };
/* 0x7C */ s8  contactActionIds[0x0C]; /* four ids per facing group (coreFlags >> 21), -1 ends a group (Entity_CheckActionIdMatch) */
/* 0x88 */ s32 hpAliveCap; /* caps hpAlive when smaller */
/* 0x8C */ u16 defense; /* subtracted from attack power */
/* 0x8E */ u16 chargeStep;
/* 0x90 */ u8  effectChance;
/* 0x91 */ u8  deathAnimFrame;
/* 0x92 */ u8  contactDamage;    /* HP taken from the player on body contact (Menu_MainUpdate) */
/* 0x93 */ u8  contactKnockback; /* knockback distance applied on body contact */
/* 0x94 */ u8  effectLevel;
/* 0x95 */ u8  effectDuration;
/* 0x96 */ s16 periodicDamage;
/* 0x98 */ u16 rewardBase;
/* 0x9A */ u16 rewardFactorA;
/* 0x9C */ u16 rewardFactorB;
/* 0x9E */ u8  rewardSlotId;
/* 0x9F */ u8  stealItemId; /* stealable item; cleared after a successful transfer */
/* 0xA0 */ s16 lootItemId;
/* 0xA2 */ s16 lootItemAux;
/* 0xA4 */ u8  motionPhase;
/* 0xA5 */ u8  motionFrames;
/* 0xA6 */ u16 motionAmplitude;
/* 0xA8 */ s16 motionAngle;
/* 0xAA */ u8  pad_AA[2];
/* 0xAC */ u8  deathAnimPhase;
/* 0xAD */ u8  deathFadeStep;
/* 0xAE */ u8  deathAssetEnabled;
/* 0xAF */ u8  deathPersist;
/* 0xB0 */ u16 attackAssetId;
/* 0xB2 */ u16 eventAssetId;
/* 0xB4 */ u16 deathAssetId;
/* 0xB6 */ u16 contactAssetIds[3]; /* effect asset per facing group (coreFlags >> 21) spawned on contact */
/* 0xBC */ u8  resumePhase;
/* 0xBD */ u8  savedActionMode;
/* 0xBE */ u8  savedInterpolation;
/* 0xBF */ u8  pad_BF;
/* 0xC0 */ s32 savedAnimFrame;
/* 0xC4 */ s32 savedAnimPrev;
/* 0xC8 */ s32 savedAnimStep;
/* 0xCC */ u32 statusFlags2;
/* 0xD0 */ s16 panelC_val;   /* floating hit-reaction panel, drawn in enemy mode */
/* 0xD2 */ u16 panelC_x;
/* 0xD4 */ u16 panelC_y;
/* 0xD6 */ u8  panelC_timer; /* 30 captures projected coordinates; then counts down */
/* 0xD7 */ u8  panelC_mode; /* 0=damage, 1=recovery, 2=alternate damage display */
    /* Same native pointer extension as FieldActorState. These pointers are
     * inactive during the enemy/event-record lifetime. */
    void *action;
    BattleAttributes *attributes;
    void *field70;
} EnemyCombatant;

/* Bit layout of EnemyCombatant.coreFlags as hit resolution writes it. */
typedef struct EnemyCoreFlagBits {
    u32 low : 13;
    u32 hitState : 2;  /* 0x6000 */
    u32 reaction : 3;  /* 0x38000: evasion reaction of the last hit */
    u32 category : 2;  /* 0xC0000: weapon category of the last hit */
    u32 high : 12;     /* >> 21: contact facing group */
} EnemyCoreFlagBits;

/* ----------------------------------------------------------------------------
 * BattleAction -- the per-actor action/command descriptor (Combatant.action,
 * i.e. *(void**)(combatant+0x68)). Holds the queued action id and timing.
 * --------------------------------------------------------------------------*/
typedef struct BattleAction {
/* 0x00 */ u16  powerBonus; /* added to stat1E/5 to form attack power */
/* 0x02 */ s16  range;         /* compared vs target distance for cycling (Battle_CycleTarget.c:93) */
/* 0x04 */ u16  field04;
/* 0x06 */ union {
               s16 actionId;   /* 5=, 6=fire-weapon, 8=item/special */
               struct { u8 actionIdByte; u8 field07; } bytes;
           } actionCode;
/* 0x08 */ s32  hitTrigger;    /* anim-row byte matched against the hit timer */
/* 0x0C */ u32  attackWord;    /* &0x3FF = shot/hit countdown (consumed per hit), >>0x14&3 = weapon category (Battle_DrawATBGauge.c:55; battle/attack.c:51) */
/* 0x10 */ u32  turnWord;      /* &0xF = remaining actions, >>4&3 = active turn slot, &0xC0 = action mode, 0x100..0x1000 = attack element (battle/setup.c:195; battle/turn.c:537; Battle_StartEnemyAttackEffect.c:26) */
/* 0x14 */ u8   animMode[4];   /* per-facing anim/effect ids [0x14..0x17], filled from a 5-byte table (battle/setup.c:197; Battle_SetupEntityTarget.c:55) */
                               /* ... continues; rest unmapped */
} BattleAction;

/* Bit layouts of the packed BattleAction and BattleAttributes words. Retail
 * copies the menu's action record into the actor field by field, so the
 * copies read and write these words through bitfield views. */
typedef struct BattleAttackBits {
    u32 count : 10;    /* shots/hits left, consumed per hit */
    u32 power : 10;
    u32 category : 2;  /* weapon category */
    u32 reserved : 10;
} BattleAttackBits;

typedef struct BattleTurnBits {
    u32 actions : 4;   /* remaining actions */
    u32 slot : 2;      /* active turn slot */
    u32 mode : 2;      /* target mode */
    u32 element8 : 1;  /* 0x100..0x1000: attack elements */
    u32 element9 : 1;
    u32 element10 : 1;
    u32 element11 : 1;
    u32 element12 : 1;
    u32 hitState : 2;
    u32 flag15 : 1;
    u32 flag16 : 1;
    u32 flag17 : 1;
    u32 reserved : 14;
} BattleTurnBits;

typedef struct BattleEffectBits {
    u32 resistPoison : 1;
    u32 resistConfusion : 1;
    u32 resistDarkness : 1;
    u32 resistStiffness : 1;
    u32 resistSteal : 1;
    u32 armorMode : 4; /* one bit per armor mod 8..10 */
    u32 mod14 : 1;     /* bits 9..13 and 16..17: armor mod flags */
    u32 mod16 : 1;
    u32 mod11 : 1;
    u32 mod17 : 1;
    u32 mod12 : 1;
    u32 autoMedicine : 1;
    u32 autoCure : 1;
    u32 mod13 : 1;
    u32 mod15 : 1;
    u32 reserved : 14;
} BattleEffectBits;

PE1_STATIC_ASSERT(sizeof(BattleAttackBits) == 4, battle_attack_bits_size);
PE1_STATIC_ASSERT(sizeof(BattleTurnBits) == 4, battle_turn_bits_size);
PE1_STATIC_ASSERT(sizeof(BattleEffectBits) == 4, battle_effect_bits_size);

/* ----------------------------------------------------------------------------
 * BattleEntity -- the field-actor / linked-list node. g_PlayerEntity and every
 * g_FieldActorListHead node point here. This is very likely the same field-
 * actor struct used outside battle (see field_actor.h / actor_model.h: the
 * render object lives at +0x1B4). Only battle-relevant offsets are listed; the
 * sparse high offsets (>=0x18C) are proven by address, not adjacency (TENTATIVE
 * ordering).
 * --------------------------------------------------------------------------*/
typedef struct BattleEntity {
/* 0x000 */ void *core; /* Combatant for player paths, EnemyCombatant for enemy entities */
/* 0x004 */ struct BattleEntity *next; /* next in actor list (battle/setup.c:63) */
/* 0x008 */ struct BattleEntity *prev;
/* 0x00C */ u8   modelIndex;     /* selects a 0xC0-byte action-pointer table (Entity_SetActionMode) */
/* 0x00D */ u8   teamId;        /* group/team id; target must differ/match (Battle_SetupEntityTarget.c:66) */
/* 0x00E */ u8   actionMode;     /* current action/animation mode; <4 and ==0xC are battle-state tests */
/* 0x00F */ u8   animLastFrame;  /* last valid frame index, loaded from action data byte 2 minus one */
/* 0x010 */ u16  stepHeight;    /* largest flat-floor height change taken without blocking (Entity_UpdateAndRender) */
/* 0x012 */ u16  animStopFrame;  /* optional stop/clamp frame when entityFlags bit 0x200 is set */
/* 0x014 */ s32  animFrame;      /* current animation frame, 16.16 fixed point */
/* 0x018 */ union {
               s32 fixed; /* previous animation frame, 16.16 fixed point */
               struct { u16 fraction; u16 integer; } parts;
           } animPrev;
/* 0x01C */ s32  animStep;       /* signed per-update 16.16 animation increment */
/* 0x020 */ s32  moveFactor;
/* 0x024 */ u16  fieldSfxId;
/* 0x026 */ u16  moveSpeed;
/* 0x028 */ Pe1Fixed16_16 posX;
/* 0x02C */ Pe1Fixed16_16 posY;
/* 0x030 */ Pe1Fixed16_16 posZ;
/* 0x034 */ u8   pad_034[4];
/* 0x038 */ s16  rotationX;
/* 0x03A */ s16  facingAngle;   /* facing/heading, set from target (battle/enemy_movement.c:275) */
/* 0x03C */ s16  rotationZ;
/* 0x03E */ u8   pad_03E[2];
/* 0x040 */ s32  baseX;
/* 0x044 */ s32  baseY;
/* 0x048 */ s32  baseZ;
/* 0x04C */ u8   pad_04C[0x1C];
/* 0x068 */ s32  motionX;
/* 0x06C */ s32  motionY;
/* 0x070 */ s32  motionZ;
/* 0x074 */ u8   pad_074[0x24];
/* 0x098 */ u32  entityFlags;   /* &0x40 visible, &0x4000 skip-as-target, &0x40000000 area-target, &0x6000 hit-state (battle/setup.c:435; Battle_SetupEntityTarget.c:60) */
/* 0x09C */ u8  *scriptBase;
/* 0x0A0 */ ScriptNode *scriptNodeLists[3];
              u32 scriptLocals[0x38];
/* 0x18C */ struct BattleEntity *parent; /* parent/owner; linked animations recurse through this entity */
/* 0x190 */ void (*frameCallback)(FieldActor *);
/* 0x194 */ void (*actionCheckFn)(FieldActor *, int, FieldActor *, int);
/* 0x198 */ u8   pad_198[4];
/* 0x19C */ u32 *scriptCursor19C;
/* 0x1A0 */ u32 *scriptCursor1A0;
/* 0x1A4 */ void *collisionFace;
/* 0x1A8 */ void *collisionFaceMirror;
/* 0x1AC */ uintptr_t allocationActive; /* nonzero when allocationBlock must be released */
/* 0x1B0 */ void *actionData;    /* current animation/action record */
/* 0x1B4 */ RenderObjectEntity renderObject;
/* 0x270 */ u8   pad_270[0x08];
/* 0x278 */ uintptr_t allocationBlock;
    u8 shadowMatrixIndex;
    u8 shadowIntensity;
} BattleEntity;

/* ----------------------------------------------------------------------------
 * BattleTarget -- native target records; original USA entries had a 0xC stride.
 * Built by Battle_BuildTargetList, sorted by Battle_SortTargets, cycled by
 * Battle_CycleTarget. g_BattleTargetCount entries; g_BattleTargetIndex selects.
 * --------------------------------------------------------------------------*/
typedef struct BattleTarget {
    struct BattleEntity *actor;
    s32 dist;
    s16 angle;
    u8 pad_0A[2];
} BattleTarget;

enum { BATTLE_TARGET_CAPACITY = 7 };

typedef struct BattleInitSlot {
    BattleEntity *actor;
    s16 command; /* turn command or spell id; 1/2 are actor action kinds */
    s16 actionGeneration; /* signed byte generation shared by entries of one queued action */
} BattleInitSlot;
extern BattleInitSlot g_BattleActionQueue[45];





extern BattleTarget g_BattleTargetList[BATTLE_TARGET_CAPACITY + 1];
extern int g_BattleAilmentPmSlot; /* last scene asset/effect result selected by battle */

void Battle_SwapRecords(BattleTarget *records, int from, int to);
void Battle_SortTargets(BattleTarget *records, s8 first, s8 last);

/* These partial records are deliberately padded through their last verified
 * field. Keep the offsets executable: prose comments alone do not protect the
 * shared ABI from an accidental member-width or alignment change. */














PE1_STATIC_ASSERT(sizeof(BattleAction) == 0x18, battle_action_partial_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleAction, turnWord) == 0x10,
                  battle_action_turn_word_offset);
PE1_STATIC_ASSERT(sizeof(BattleParameterWord) == 4, battle_parameter_word_size);
PE1_STATIC_ASSERT(sizeof(BattleAttributes) == 0x08, battle_attributes_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleAttributes, effectFlags) == 0x04,
                  battle_attributes_effect_flags_offset);
PE1_STATIC_ASSERT(sizeof(EnemyActionEffect) == 0x10, enemy_action_effect_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyActionEffect, power) == 0x0C,
                  enemy_action_effect_power_offset);























PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyCombatant, curHP) ==
                  PE1_OFFSETOF(Combatant, curHP), combatant_views_hp_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyCombatant, effect) ==
                  PE1_OFFSETOF(EntityAnimEventCore, active), enemy_event_active_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyCombatant, records) ==
                  PE1_OFFSETOF(EntityAnimEventCore, records), enemy_event_records_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyCombatant, contactActionIds) ==
                  PE1_OFFSETOF(EntityAnimEventCore, parameters), enemy_event_parameters_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyCombatant, eventAssetId) ==
                  PE1_OFFSETOF(EntityAnimEventCore, assetId), enemy_event_asset_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyCombatant, stateFlags) ==
                  PE1_OFFSETOF(FieldActorState, flags), enemy_state_flags_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyCombatant, statusFlags2) ==
                  PE1_OFFSETOF(FieldActorState, status_flags2), enemy_state_status_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(EnemyCombatant, action) ==
                  PE1_OFFSETOF(FieldActorState, action), enemy_state_action_match);
PE1_STATIC_ASSERT(sizeof(EnemyCombatant) == sizeof(FieldActorState), enemy_state_size_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Combatant, curHP) ==
                  PE1_OFFSETOF(FieldActorState, amount), state_views_hp_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Combatant, atbStep) ==
                  PE1_OFFSETOF(FieldActorState, progress_step), state_views_step_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Combatant, stateFlags) ==
                  PE1_OFFSETOF(FieldActorState, flags), state_views_flags_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Combatant, attributes) ==
                  PE1_OFFSETOF(FieldActorState, substate), state_views_substate_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Combatant, action) ==
                  PE1_OFFSETOF(FieldActorState, action), state_views_action_match);
PE1_STATIC_ASSERT(sizeof(Combatant) == sizeof(FieldActorState),
                  state_views_size_match);




















/* BattleEntity and FieldActor are two naming views of the same runtime object.
 * Keep their independently useful vocabularies, but make layout drift a build
 * error until the remaining call sites can be migrated to one typedef. */
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, next) ==
                  PE1_OFFSETOF(FieldActor, next), entity_views_next_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, animFrame) ==
                  PE1_OFFSETOF(FieldActor, anim),
                  entity_views_anim_frame_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, posX) ==
                  PE1_OFFSETOF(FieldActor, pos_x), entity_views_pos_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, moveFactor) ==
                  PE1_OFFSETOF(FieldActor, move_factor), entity_views_move_factor_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, facingAngle) ==
                  PE1_OFFSETOF(FieldActor, rot_y), entity_views_rotation_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, entityFlags) ==
                  PE1_OFFSETOF(FieldActor, flags), entity_views_flags_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, scriptBase) ==
                  PE1_OFFSETOF(FieldActor, script_base), entity_views_script_base_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, parent) ==
                  PE1_OFFSETOF(FieldActor, parent), entity_views_parent_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, scriptCursor19C) ==
                  PE1_OFFSETOF(FieldActor, script_cursor_19c), entity_views_script_cursor_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, renderObject) ==
                  PE1_OFFSETOF(FieldActor, render_object), entity_views_render_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, renderObject.table_index) ==
                  PE1_OFFSETOF(FieldActor, render_object.table_index), entity_views_render_field_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleEntity, allocationBlock) ==
                  PE1_OFFSETOF(FieldActor, allocation_block), entity_views_tail_match);
PE1_STATIC_ASSERT(sizeof(BattleEntity) == sizeof(FieldActor),
                  entity_views_size_match);

int Battle_RollEscapeChance(void);

/* Restoring integer square root (battle/commands.c). */
int Math_IntSqrt(int value);

/* Gauge colours. The pulsing battle colours are the vertex colours of the
 * POLY_G4 gauge quads in battle_status.h, one set per draw buffer and written
 * as g_X[g_ActiveDrawSlot].rN/gN/bN: g_BattlePeGauges[slot][1] is seeded by
 * Battle_InitEntityColors with alternating {0xFF,0x3D,0x81} and
 * {0x83,0x13,0x01} and recoloured from g_FrameRngCounter & 3 in
 * Battle_PhaseInitEnemyTurn, Battle_PhaseEndTurn and Menu_MainUpdate;
 * g_BattleAtbGauges[slot] takes {0x46,0x82,0x9F}/{0xFF,0xF9,0x00}/
 * {0xA3,0xBE,0x50} in Battle_Update and Battle_PhaseInitEnemyTurn. */

/* ============================================================================
 * PERSISTENT STATS (out of battle) -- see aya.h for the full AyaSaveState.
 *   Aya's saved stats are the g_AyaStat* block at 0x800C0E00 (max HP +0x06,
 *   current HP +0x08, level +0x0A, bonus points +0x10, offense +0x2A, ...).
 *   In-battle damage does NOT touch this block; the combatant HP is synced
 *   back to it at battle end, so writing HP here is the clean persistent way.
 *
 * STATIC ENEMY DEFINITION (not yet located): the runtime Combatant does NOT
 *   hold Level / EXP / BP / status resistances -- those come from a per-enemy
 *   DEFINITION record streamed from PE.IMG into a separate table (TODO: locate;
 *   likely reached via BattleEntity.defPtr@0x238).
 * ==========================================================================*/

void Battle_ApplyDamage(int action);
void Battle_ApplyEnemyAttack(EnemyCombatant *enemy);
void Battle_ApplySpellEffect(u32 spell, BattleEntity *target);

#endif /* PE1_BATTLE_H */
