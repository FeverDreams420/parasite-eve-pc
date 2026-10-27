/*
 * Movement: Always run, run / walk / combat speed, battle-start speed. Ported from the psxrecomp
 * build (src/mods/pe_mod_menu.c, "movement"). F1 > PE Enhancements.
 *
 * SPEED: while Aya moves in the field, her moveFactor (BattleEntity.moveFactor, 1.0 = 0x10000;
 * walking moves 5.0 x it a logic frame, running 22.5 x it) becomes the walk or run factor, and
 * goes back to 1.0 when she stops -- only from one of our own values, so a value a script set is
 * left alone. In battle, while she can move (battle phase 0), the combat factor applies.
 *
 * ALWAYS RUN: the run bit of the field input is input bit 0 of the button mapping table
 * (g_FieldRenderFlagTable[0], Circle by default). In free field movement with Always run on it
 * maps to the d-pad, so moving means running, and to nothing while the run button is held, so
 * holding it walks. Menus and battle get the original mapping back.
 *
 * ANALOG: in analog mode the game sets the run bit only when one stick axis is past full tilt,
 * so a round stick gate dropped Aya to a walk on diagonals. Before the game reads the pad
 * (PE_HOOK_PAD_READ) the stick is reshaped, both axes scaled together so the steering angle is
 * unchanged: to run, out until the larger axis reaches its edge; to walk, in until neither axis
 * is at full tilt. Cross always walks (examining has to stop her).
 *
 * BATTLE START: as a fight opens (phase 7) Aya walks to her spot with a fixed velocity that
 * ignores moveFactor; while her heading holds steady, (factor - 1) x that velocity is added to
 * her position each frame and her animation step matches. Off by default.
 */
#include <stdint.h>

#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe1/boot.h"
#include "pe1/pad_state.h"

#define PE_RUN_MAP_DPAD     0x00F0u   /* up/right/down/left, internal pad bits */
#define PE_PAD_CROSS        0x2000u
#define PE_STICK_CENTER     0x80
#define PE_STICK_FULL_R2    (100 * 100)
#define PE_MOVE_FACTOR_STOCK 0x10000
#define PE_ANIM_STEP_STOCK  0x10000
#define PE_ENTRY_ACTION_MODE 5u

static const char *const s_speed_names[] = { "Normal", "1.25x", "1.5x", "1.75x", "2x", "2.25x", "2.5x", "3x" };
static const int32_t s_speed_factor[] = { 0x10000, 0x14000, 0x18000, 0x1C000, 0x20000, 0x24000, 0x28000, 0x30000 };
#define PE_SPEED_CHOICES 8
static const char *const s_entry_names[] = { "Normal", "1.5x", "2x", "3x", "4x" };
static const int32_t s_entry_factor[] = { 0x10000, 0x18000, 0x20000, 0x30000, 0x40000 };
#define PE_ENTRY_CHOICES 5

static int s_always_run = 1;
static int s_run_choice = 3, s_walk_choice = 3;   /* 1.75x field movement by default */
static int s_combat_choice = 0, s_entry_choice = 0;
static int32_t s_run_mask = 0x4000;               /* the run bit's own buttons (Circle) */

static int pe_is_boost_value(int32_t mf) {
    int i;
    for (i = 1; i < PE_SPEED_CHOICES; ++i) if (mf == s_speed_factor[i]) return 1;
    return 0;
}
static int32_t pe_speed(int choice) { return s_speed_factor[choice >= 0 && choice < PE_SPEED_CHOICES ? choice : 0]; }

/* The raw pad as the game's internal button bits (Circle/Cross swapped) and the left stick. */
typedef struct { uint32_t bits; int analog, x, y, stick; } PePad;

static PePad pe_read_pad(void) {
    PePad p = { 0, 0, PE_STICK_CENTER, PE_STICK_CENTER, 0 };
    const unsigned header = g_PadPackets[0].header, id = (header >> 8) & 0xFFu;
    uint32_t inv;
    if ((header & 0xFFu) != 0 || id == 0 || id == 0xFFu) return p;
    inv = ~(uint32_t)g_PadPackets[0].buttons & 0xFFFFu;
    p.bits = inv & 0x9FFFu;
    if (inv & 0x2000u) p.bits |= 0x4000u;
    if (inv & 0x4000u) p.bits |= 0x2000u;
    if ((header & 0xF000u) == 0x7000u) {
        p.analog = 1;
        p.x = g_PadPackets[0].leftX;
        p.y = g_PadPackets[0].leftY;
        p.stick = p.x < 0x5A || p.x >= 0xA1 || p.y < 0x5A || p.y >= 0xA1;
    }
    return p;
}

/* Should a stick push run? (Always run: unless the run button is held; never while Cross is.) */
static int pe_stick_runs(const PePad *p) {
    int dx, dy;
    if (p->bits & PE_PAD_CROSS) return 0;
    if (s_always_run) return !(p->bits & (uint32_t)s_run_mask);
    dx = p->x - PE_STICK_CENTER;
    dy = p->y - PE_STICK_CENTER;
    return dx * dx + dy * dy >= PE_STICK_FULL_R2;
}

static int pe_on_pad_read(void *ctx, void *user) {
    PePad p;
    int dx, dy, ax, ay, big, run, full, reach, x, y;
    (void)ctx;
    (void)user;
    if (!pe_game_loaded() || !pe_free_field()) return 0;
    p = pe_read_pad();
    if (!p.analog || !p.stick) return 0;
    dx = p.x - PE_STICK_CENTER;
    dy = p.y - PE_STICK_CENTER;
    ax = dx < 0 ? -dx : dx;
    ay = dy < 0 ? -dy : dy;
    big = ax > ay ? ax : ay;
    run = pe_stick_runs(&p);
    full = p.x < 0x14 || p.x >= 0xE7 || p.y < 0x14 || p.y >= 0xE7;
    if (big <= 0 || run == full) return 0;            /* already what we want */
    reach = run ? 127 : 100;
    x = PE_STICK_CENTER + (dx * reach + (dx < 0 ? -big / 2 : big / 2)) / big;
    y = PE_STICK_CENTER + (dy * reach + (dy < 0 ? -big / 2 : big / 2)) / big;
    g_PadPackets[0].leftX = (u8)(x < 0 ? 0 : x > 0xFF ? 0xFF : x);
    g_PadPackets[0].leftY = (u8)(y < 0 ? 0 : y > 0xFF ? 0xFF : y);
    return 0;
}

static void pe_entry_walk_tick(BattleEntity *player) {
    static int s_walking, s_turning = 1;
    static int32_t s_prev_mx, s_prev_mz, s_our_step;
    const int32_t factor = s_entry_factor[s_entry_choice >= 0 && s_entry_choice < PE_ENTRY_CHOICES ? s_entry_choice : 0];
    const int walk = pe_in_battle() && g_BattleModeState == 7 && player->actionMode == PE_ENTRY_ACTION_MODE;
    const int32_t mx = walk ? player->motionX : 0, mz = walk ? player->motionZ : 0;
    int push;
    if (!walk || factor == 0x10000) {
        s_walking = 0;
    } else if (!s_walking) {
        s_walking = 1;
        s_turning = 1;                                   /* wait for a steady heading */
        s_prev_mx = mx;
        s_prev_mz = mz;
    } else if (mx != s_prev_mx || mz != s_prev_mz) {     /* steady: within ~4 degrees, not reversed */
        const int64_t ax = mx >> 8, az = mz >> 8, bx = s_prev_mx >> 8, bz = s_prev_mz >> 8;
        const int64_t dot = ax * bx + az * bz;
        s_turning = dot <= 0 || dot * dot * 1000 < (ax * ax + az * az) * (bx * bx + bz * bz) * 995;
        s_prev_mx = mx;
        s_prev_mz = mz;
    }
    push = s_walking && !s_turning && (mx || mz);
    if (!push) {
        if (s_our_step && player->animStep == s_our_step) player->animStep = PE_ANIM_STEP_STOCK;
        s_our_step = 0;
        return;
    }
    {
        const int64_t extra = (int64_t)factor - 0x10000;  /* per logic frame */
        player->posX.fixed += (int32_t)(((int64_t)mx * extra) >> 16);
        player->posZ.fixed += (int32_t)(((int64_t)mz * extra) >> 16);
    }
    if (player->animStep == PE_ANIM_STEP_STOCK || (s_our_step && player->animStep == s_our_step)) {
        s_our_step = factor;
        player->animStep = s_our_step;
    }
}

static void pe_move_tick(void) {
    const PePad p = pe_read_pad();
    const int dpad = (p.bits & PE_RUN_MAP_DPAD) != 0, in_field = pe_free_field();
    BattleEntity *player = pe_player();
    int32_t map, want_map, mf, want;
    int run_held, moving, running, combat;
    if (!pe_game_loaded() || !player) return;

    /* Always run: retarget the run entry of the mapping table (any value not ours is the game's). */
    map = g_FieldRenderFlagTable[0];
    if (map != (int32_t)PE_RUN_MAP_DPAD && map != 0) s_run_mask = map;
    run_held = (p.bits & (uint32_t)s_run_mask) != 0;
    want_map = s_run_mask;
    if (s_always_run && in_field) want_map = run_held ? 0 : (int32_t)PE_RUN_MAP_DPAD;
    if (want_map != map) g_FieldRenderFlagTable[0] = want_map;

    pe_entry_walk_tick(player);
    mf = player->moveFactor;
    moving = in_field && (dpad || p.stick);
    running = dpad ? (s_always_run ? !run_held : run_held) : pe_stick_runs(&p);
    combat = pe_in_battle() && g_BattleModeState == 0;
    want = mf;
    if (combat && (mf == PE_MOVE_FACTOR_STOCK || pe_is_boost_value(mf))) want = pe_speed(s_combat_choice);
    else if (moving && (mf == PE_MOVE_FACTOR_STOCK || pe_is_boost_value(mf)))
        want = pe_speed(running ? s_run_choice : s_walk_choice);
    else if (!moving && pe_is_boost_value(mf)) want = PE_MOVE_FACTOR_STOCK;
    if (want != mf) player->moveFactor = want;
}

void pe_move_register(const char *mod_id) {
    pe_menu_register_toggle(PE_PAGE_MOVEMENT, "Always run", "Hold the run button to walk (d-pad and stick).", &s_always_run, NULL);
    pe_menu_register_choice(PE_PAGE_MOVEMENT, "Run speed", "How fast Aya runs in the field.", &s_run_choice, s_speed_names, PE_SPEED_CHOICES);
    pe_menu_register_choice(PE_PAGE_MOVEMENT, "Walk speed", "How fast Aya walks in the field.", &s_walk_choice, s_speed_names, PE_SPEED_CHOICES);
    pe_menu_register_choice(PE_PAGE_MOVEMENT, "Combat speed", "How fast Aya moves during battle.", &s_combat_choice, s_speed_names, PE_SPEED_CHOICES);
    pe_menu_register_choice(PE_PAGE_MOVEMENT, "Battle start speed", "Aya's walk to her spot as a fight opens.", &s_entry_choice,
                            s_entry_names, PE_ENTRY_CHOICES);
    pe_menu_register_tick(pe_move_tick);
    pe_hook_add(PE_HOOK_PAD_READ, pe_on_pad_read, NULL, PE_PRIORITY_NORMAL, mod_id);
}
