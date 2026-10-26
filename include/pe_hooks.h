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
    PE_HOOK_TEXTBOXES,   /* NULL: Menu_DrawTextboxEntries entry, before the dialogue boxes draw */
    PE_HOOK_ITEM_ACTIONS, /* PeHookItemActions: an item's action list (Use / Move / Discard ...) just opened */
    PE_HOOK_MAIN_MENU_ICON, /* PeHookMainMenuIcon: a main menu row's icon, before it is drawn */
    PE_HOOK_PLAYER_BANK, /* NULL: Scene_LoadEntityTexture entry, Aya's model / texture load step;
                          * g_SceneAreaType (her outfit bank) may be changed while entity_texture_phase is 0 */
    PE_HOOK_INVENTORY_ROW, /* PeHookInventoryRow: inventory row dimming, presentation only */
    PE_HOOK_POWER_AVAILABLE, /* PeHookPowerAvailable: Menu_CheckItemAffordable, can this PE power be picked now */
    PE_HOOK_PLAYER_ROWS, /* NULL: before Aya's actor takes bank_slots[0] and her rows (g_GameState.bank_rows):
                          * the end of Scene_LoadEntityTexture (every room load: the room's actor
                          * spawn reads them) and Scene_InitEntityPlayer step 0x27 (scene start) */
    PE_HOOK_PLAYER_TEXTURES, /* SceneAssetBlob *: Aya's texture blob just read, before its TIMs go to VRAM;
                              * replaceable (nonzero: the game skips its upload) */
    PE_HOOK_RENDER_STATS, /* PeHookRenderStats: native face submission, diagnostics only */
    PE_HOOK_FRAME_DRAWN, /* NULL: Draw_RenderFrame, the previous frame finished drawing (DrawSync)
                          * and not yet shown; its buffer is g_RenderDrawEnvArray[1 - g_ActiveDrawSlot] */
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

typedef struct PeHookRenderStats {
    struct RenderObjectEntity *object;
    unsigned submitted[4]; /* textured quad/triangle, untextured quad/triangle */
} PeHookRenderStats;

typedef struct PeHookInventoryRow {
    struct MenuWidgetNode *node;
    int slot;
    int *dimmed;
} PeHookInventoryRow;

typedef struct PeHookPowerAvailable {
    int power;    /* Parasite Energy power index (0 Heal 1 .. 19 Liberate) */
    int *enabled; /* the game's answer (PE, HP, ammo); a mod may clear it */
} PeHookPowerAvailable;

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

/* The panel's update and the list's draw and rows may be replaced; g_MenuItemActionStateTable
 * [context] gives the game's action of each row. */
typedef struct PeHookItemActions {
    struct MenuWidgetNode *panel; /* update = Menu_StepEquipScreen */
    struct MenuWidgetNode *list;  /* draw = Menu_DrawContextActionList, 3 rows */
    int context;                  /* g_MenuItemActionContext: 0 consumables, 1 and 2 others */
    int list_id, slot;            /* g_MenuActiveItemList (0 = Aya's bag), g_MenuActiveItemSlot */
} PeHookItemActions;

typedef struct PeHookMainMenuIcon {
    int row;     /* the list row */
    int action;  /* 0-8 the game's rows; 9 = a row past the last one */
    int sprite;  /* action + 0x56; may be changed; -1: the handler drew the icon itself */
} PeHookMainMenuIcon;

#ifdef PE_MODS
int pe_hook_dispatch(int id, void *ctx);
#define PE_HOOK(id, ctx) pe_hook_dispatch((id), (ctx))
#else
#define PE_HOOK(id, ctx) ((void)0, 0)
#endif

#endif
