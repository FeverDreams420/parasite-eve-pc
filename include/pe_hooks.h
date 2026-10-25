#ifndef PE_HOOKS_H
#define PE_HOOKS_H

/* Mod hook points. Each PE_HOOK line in the game hands one moment to the mod
 * host (mods/host), with a small context struct of pointers to the values a
 * mod may read or change. Without PE_MODS the macro is 0 and the game is the
 * unchanged original. A non-zero result means a mod handled the moment and
 * the caller skips its own code; only hooks documented as replaceable check it. */

typedef enum PeHookId {
    PE_HOOK_INIT,        /* PeHookInit: once, before the first scene */
    PE_HOOK_SCENE_START, /* NULL: a field scene finished loading, before its first frame */
    PE_HOOK_FRAME,       /* NULL: every field frame, after pad input is read */
    PE_HOOK_BATTLE_UPDATE, /* NULL: Battle_Update entry, every battle frame */
    PE_HOOK_ENEMY_HIT,   /* BattleEntity *: Battle_StepEnemyMovement entry, before an enemy's damage */
    PE_HOOK_SPELL,       /* PeHookSpell: Battle_ApplySpellEffect entry (battle and menu) */
    PE_HOOK_HELP_TEXT,   /* u8 **: the menu title-bar line, before it is printed; may be replaced */
    PE_HOOK_BATTLE_REWARDS, /* PeHookRewards: Aya_SetTotalExp entry, once per won battle */
    PE_HOOK_INV_COMMAND, /* PeHookInvCommand: Inv_DispatchCommand entry; replaceable */
    PE_HOOK_ITEM_ADD,    /* s32 *: Inv_AddItem entry; change id, or return nonzero for handled/success */
    PE_HOOK_STEAL,       /* NULL: a steal's Inv_AddItem follows (Battle_ResolveHitOnTimer) */
    PE_HOOK_SAVE_WRITE,  /* u8 *: the 8 KB card block, before its CRC (free tail from 0x1C90) */
    PE_HOOK_SAVE_READ,   /* u8 *: the 8 KB card block of a save being loaded */
    PE_HOOK_ITEM_COUNT,  /* int **: Script_CountItemsByValue ({&item, &result}); nonzero uses supplied result */
    PE_HOOK_SLOT_ROW,    /* u8 **: an item row's name (Menu_DrawSlotRow), before it is printed */
    PE_HOOK_TOOL_TRANSFER, /* PeHookToolTransfer: Inv_TransferItemBetweenLists entry (Tools) */
    PE_HOOK_PAD_READ,    /* NULL: Field_HandleStateTransition, before it reads g_PadPackets */
    PE_HOOK_LIST_ROWS,   /* PeHookListRows: Draw_SetPrimCallback entry; the row count may change */
    PE_HOOK_MAIN_MENU_ACTION, /* PeHookMenuAction: Aya's main menu confirm; replaceable */
    PE_HOOK_SCRIPT_RANDOM, /* PeHookScriptRandom: local RNG bounds, before a script roll */
    PE_HOOK_ITEM_USE,    /* int *: Battle_ApplyDamage item id before its effects */
    PE_HOOK_TEXT_LOOKUP, /* PeHookTextLookup: item name/description lookup result */
    PE_HOOK_INVENTORY_SORTED, /* NULL: Aya's native bag sort completed */
    PE_HOOK_INVENTORY_HEADER, /* MenuWidgetNode *: after Aya's Quantity header is drawn */
    PE_HOOK_EQUIP_EFFECT, /* PeHookEquipEffect: effect icon draw; nonzero replaces sprite */
    PE_HOOK_COUNT
} PeHookId;

/* next points just past the current instruction in native loaded bytecode. */
typedef struct PeHookScriptRandom {
    int *start, *end;
    const unsigned int *next;
} PeHookScriptRandom;

typedef struct PeHookTextLookup {
    unsigned table, index; /* table 8 = item names, 12 = item descriptions */
    void **text;
} PeHookTextLookup;

typedef struct PeHookRewards {
    int exp;
    struct BattleRewardSlot *list; /* 10 entries, id 0 ends */
} PeHookRewards;

typedef struct PeHookInvCommand {
    int command, value;
    int result; /* a handler returning nonzero replaces the command with this result */
} PeHookInvCommand;

typedef struct PeHookToolTransfer {
    unsigned selection; /* the donor's saved selection; the target is the other one */
    int modifier;       /* >= 0: that mod moves; < 0: the + bonuses move */
    int flags;          /* bit 0: Super Tool (the donor survives) */
    int cost;
} PeHookToolTransfer;

typedef struct PeHookSpell {
    unsigned spell;           /* Parasite Energy power index */
    struct BattleEntity *target;
} PeHookSpell;

typedef struct PeHookInit {
    int interactive;        /* 0 for headless runs: no window, no menus */
    const char *prefs_file; /* per-user preferences file; mods keep their files beside it */
} PeHookInit;

typedef struct PeHookListRows {
    struct MenuWidgetNode *node;
    int *count;
} PeHookListRows;

typedef struct PeHookEquipEffect {
    int effect; /* lower five bits of the equipment modifier */
    int armor;  /* 0 = weapon, 1 = armor */
} PeHookEquipEffect;

typedef struct PeHookMenuAction {
    int action;                   /* 0-8 the game's rows; 9 = a row past the last one */
    struct MenuWidgetNode *list;  /* the main list node */
} PeHookMenuAction;

#ifdef PE_MODS
int pe_hook_dispatch(int id, void *ctx);
#define PE_HOOK(id, ctx) pe_hook_dispatch((id), (ctx))
#else
#define PE_HOOK(id, ctx) ((void)0, 0)
#endif

#endif
