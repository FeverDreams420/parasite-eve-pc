/*
 * Wayne's services from Warner, where he stands guard (found by his retail talk script, not by
 * room). Ported from the psxrecomp build (src/mods/pe_field_wayne.c): Store equipment, Store
 * item and the Workshop (Requisition, Rebuild, Mod Permit Card); not Discard junk -- making guns
 * stays Wayne's.
 *
 * Warner's talk is an ordinary actor script: Cross next to him runs his contact task (script base
 * + 0x154), which locks the field, shows his two lines and unlocks it again. His task is checked
 * word for word against the retail script (s_orig) before anything is done, read-only.
 *
 * Native design (the recomp patched his bytecode to jump into a script of ours; native jumps are
 * offsets from the script's own buffer, so that doesn't carry over):
 *   - while his talk runs, his first line's dialogue box is pointed at our menu text, "Torres sent
 *     supplies." and Store equipment / Store item / Cancel, in Wayne's window style. Wayne's
 *     Workshop (pe_wayne_menu.c) recognises it and adds Mod Permit Card and Workshop above Cancel;
 *   - when the box closes on Store equipment or Store item, the storage screen opens once his
 *     task has unlocked the field, the way his script opcode would (Script_BeginSceneTransition:
 *     Render_BeginSceneLoad, Menu_OpenEquipScreen, menu flags). Talk to him again for more.
 * His second line still shows; nothing in his script or the story changes.
 */
#include <string.h>

#include "pe_economy.h"
#include "pe_game.h"
#include "pe_gametext.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe1/asset_table.h"
#include "pe1/scene_dispatch.h"
#include "pe1/scene_transition.h"
#include "pe1/script_node.h"
#include "pe1/textbox.h"
#include "pe1/textbox_open.h"

void Menu_OpenEquipScreen(int mode);

#define PE_WARNER_MODEL   0x04
#define PE_CONTACT_OFF    0x154   /* his contact task, from the script base */
#define PE_TEXTBOX_SHOWN  2
#define PE_STORE_EQUIPMENT 0
#define PE_STORE_ITEMS    1

/* His contact task, base + 0x154 .. 0x36C, as read 2026-10-14 (retail). */
static const uint32_t s_orig[] = {
    0x01308009,0x00000000,0x00000003,0x00000000,0x00000036,0x00010000,0x01B08009,0x00000000,0x00000007,0x00000001,
    0x00000000,0x00000000,0x00064005,0x00000000,0x00000001,0x000001B2,0x01308009,0x00000000,0x0000000C,0x00000002,
    0x0000004A,0x000000E8,0x00064005,0x00000000,0x00000002,0x000001B2,0x00124024,0x00000000,0x00000000,0x00000001,
    0x00B08009,0x00000000,0x0000000B,0x00000000,0x00000000,0x00000000,0x00064005,0x00000000,0x00000000,0x000001B2,
    0x02308009,0x00000000,0x0000000B,0x00000001,0x00000012,0x00000000,0x00064005,0x00000000,0x00000001,0x000001B2,
    0x00806011,0x00000000,0x00000001,0x00000100,0x00000002,0x00B08009,0x00000000,0x0000000E,0x00000000,0x00000002,
    0x00000000,0x00064005,0x00000000,0x00000000,0x000001B2,0x000000AA,0x00000000,0x0000601C,0x00000000,0x00000000,
    0x00000000,0x000000FE,0x00000079,0x00000000,0x0000202E,0x00000000,0x00000001,0x0000604B,0x00000000,0x00000000,
    0x00000000,0x0000002D,0x0000202E,0x00000000,0x00000000,0x00000078,0x00000000,0x0000200D,0x00000000,0x0000001E,
    0x0000202E,0x00000000,0x00000005,0x0000204E,0x00000000,0x00000000,0x0000202F,0x00000000,0x00000077,0x00002022,
    0x00000000,0x0000001E,0x00002002,0x00000000,0x00000077,0x0000200D,0x00000000,0x0000001F,0x0000202E,0x00000000,
    0x00000003,0x0000204E,0x00000000,0x00000000,0x0000202F,0x00000000,0x00000050,0x00002022,0x00000000,0x0000001F,
    0x00000079,0x00000000,0x0000202E,0x00000000,0x00000000,0x00000078,0x00000000,0x0000601C,0x00000000,0x00000000,
    0x00000000,0x000000FF,0x00000020,0x00000000,
};
#define PE_TASK_BYTES ((unsigned)sizeof s_orig)

static int s_enabled = 1;
static u8 s_text[128];            /* our menu text */
static int s_text_ready;
static BattleEntity *s_warner;
static int s_token = -1;          /* the room we looked in */
static int s_boxes_this_talk;     /* dialogue boxes his current talk has opened */
static u8 *s_last_message;        /* the box message seen last frame */
static TextboxEntry *s_menu_box;  /* the box showing our menu */
static int s_pending = -1;        /* storage screen to open: PE_STORE_*, or -1 */

static int pe_free_field(void) { return !(g_GameStateFlags & 0x2006u) && !(g_GameStateFlagsWord & 0xB600u); }

/* The room number from the scene token's base-32 name ("M374..."). */
static int pe_map_number(void) {
    const unsigned token = (unsigned)g_SceneDispatchToken;
    int n = 0, i;
    for (i = 2; i < 5; ++i) {
        const char c = (char)g_Base32CharTable[(token >> ((5 - i) * 5 + 2)) & 0x1Fu];
        if (c < '0' || c > '9') return -1;
        n = n * 10 + (c - '0');
    }
    return n;
}

/* Is `e` Warner, with his retail contact task? */
static int pe_is_warner(BattleEntity *e) {
    return e->modelIndex == PE_WARNER_MODEL && e->scriptBase &&
           e->scriptCursor1A0 == (u32 *)(e->scriptBase + PE_CONTACT_OFF) &&
           memcmp(e->scriptBase + PE_CONTACT_OFF, s_orig, PE_TASK_BYTES) == 0;
}

static BattleEntity *pe_find_warner(void) {
    BattleEntity *e = (BattleEntity *)g_FieldActorListHead;
    int guard;
    for (guard = 0; e && guard < 40; ++guard, e = e->next)
        if (pe_is_warner(e)) return e;
    return NULL;
}

/* Is his contact task running now (a script node inside it)? */
static int pe_talk_running(void) {
    const u8 *from = s_warner->scriptBase + PE_CONTACT_OFF, *to = from + PE_TASK_BYTES;
    int list, guard;
    for (list = 0; list < 3; ++list) {
        const ScriptNode *n = s_warner->scriptNodeLists[list];
        for (guard = 0; n && guard < 16; ++guard, n = n->next) {
            const u8 *at = (const u8 *)n->current.script;
            if (at >= from && at < to) return 1;
        }
    }
    return 0;
}

static void pe_write_text(void) {
    PeText t;
    if (s_text_ready) return;
    pe_t_init(&t);
    pe_t_str(&t, "Torres sent supplies.");
    pe_t_byte(&t, G_NEWLINE);
    pe_t_choice(&t, 3);
    pe_t_option(&t, 1); pe_t_str(&t, "Store equipment");
    pe_t_option(&t, 0); pe_t_str(&t, "Store item");
    pe_t_option(&t, 0); pe_t_str(&t, "Cancel");
    pe_t_byte(&t, G_END);
    memcpy(s_text, t.b, t.n < sizeof s_text ? t.n : sizeof s_text);
    s_text_ready = 1;
}

/* For pe_wayne_menu.c: a box showing this is Wayne's menu (Warner's version). */
const u8 *pe_field_wayne_text(void) { return s_text_ready ? s_text : NULL; }

static void pe_forget(void) {
    s_warner = NULL;
    s_boxes_this_talk = 0;
    s_last_message = NULL;
    s_menu_box = NULL;
}

/* During his talk: his first dialogue box shows our menu instead of his line. */
static void pe_talk_tick(void) {
    int i;
    for (i = 0; i < 4; ++i) {
        TextboxEntry *e = &g_TextboxEntries[i];
        if (e->state == 0 || !e->message) continue;
        if (e == s_menu_box) {                          /* ours: closing on a storage choice? */
            if (e->state != PE_TEXTBOX_SHOWN) continue;
            return;
        }
        if (e->message == s_last_message) return;
        s_last_message = e->message;
        if (++s_boxes_this_talk == 1) {
            pe_write_text();
            e->message = s_text;
            e->style = 1;                               /* a window at the box's own spot, like Wayne's */
            e->background = 0;
            s_menu_box = e;
            pe_log("field_wayne", "menu shown");
        }
        return;
    }
}

/* Before the dialogue boxes draw (ahead of Wayne's Workshop): Warner, his talk, his box. */
static int pe_on_textboxes(void *ctx, void *user) {
    int token;
    (void)ctx;
    (void)user;
    if (!pe_game_loaded()) return 0;
    token = g_SceneDispatchToken;
    if (token != s_token) { s_token = token; pe_forget(); s_pending = -1; }

    /* Our menu box closed: the dialogue box left the chosen row in g_MenuEquipSlotIndex. */
    if (s_menu_box && s_menu_box->state != PE_TEXTBOX_SHOWN) {
        if (g_MenuEquipSlotIndex == PE_STORE_EQUIPMENT || g_MenuEquipSlotIndex == PE_STORE_ITEMS)
            s_pending = g_MenuEquipSlotIndex;
        s_menu_box = NULL;
    }
    if (!s_enabled || !pe_loot_enabled()) { pe_forget(); return 0; }
    if (s_warner && !pe_is_warner(s_warner)) pe_forget();   /* a room reload brings a fresh actor */
    if (!s_warner) {
        /* Found by his retail talk script (checked word for word), in whatever room he stands. */
        if (!(s_warner = pe_find_warner())) return 0;
        pe_mod_log("pe_economy", "Warner found in room %d: storage and Workshop offered", pe_map_number());
    }
    if (pe_talk_running()) {
        pe_talk_tick();
    } else {
        s_boxes_this_talk = 0;                          /* between talks */
        s_last_message = NULL;
    }
    return 0;
}

/* Every frame: once his task has unlocked the field, open the storage screen chosen. */
static void pe_field_wayne_tick(void) {
    if (!pe_game_loaded() || !s_warner || pe_talk_running()) return;
    if (s_pending >= 0 && pe_free_field()) {
        const int mode = s_pending;
        s_pending = -1;
        Render_BeginSceneLoad();
        Menu_OpenEquipScreen(mode);
        g_GameState.flags |= 0x9000;
        g_GameStateFlags |= 4;
        pe_log("field_wayne", "storage mode=%d", mode);
    }
}

void pe_field_wayne_register(const char *mod_id) {
    pe_menu_register_toggle(PE_PAGE_ECONOMY, "Warner helps out", "Warner offers storage and the Workshop wherever he stands guard.",
                            &s_enabled, pe_field_wayne_tick);
    pe_hook_add(PE_HOOK_TEXTBOXES, pe_on_textboxes, NULL, PE_PRIORITY_EARLY, mod_id);  /* before pe_wayne_menu */
}
