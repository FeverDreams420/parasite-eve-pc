/*
 * The Bestiary in Aya's own menu (bridge findings pe-bestiary-viewer-plan-2026-10-26.md, phase 3):
 * a "Bestiary" row after the game's rows (host extra rows, pe_main_menu.h), shown once a creature is
 * met, opens a screen built from the game's own widgets:
 *
 *   left   the area's creatures (L1 / R1 change area); unmet ones are "???"
 *   right  a game panel whose inside is the creature's model (image presenter, pe_present.h),
 *          turning slowly with its idle animation; Left / Right turn it by hand
 *   below  what is known, by tier: basics, weaknesses, loot; a locked tier says how to unlock it
 *
 * Unmet creatures (decisions, plan section 9): in an area the player has reached, a black silhouette;
 * in areas not reached, and bosses, nothing (their shapes are reveals). Triangle pauses the
 * animation, Select shows the next form (parts, phases), Circle goes back. Buying, NEW markers and
 * title-bar help come in the next step.
 */
#include <stdio.h>
#include <string.h>

#include "pe_collection.h"
#include "pe_gametext.h"
#include "pe_main_menu.h"
#include "pe_mod_host.h"
#include "pe_model.h"
#include "pe_model_view.h"
#include "pe_present.h"
#include "pe1/draw_state.h"
#include "pe1/menu_draw_list.h"
#include "pe1/menu_sound.h"
#include "pe1/menu_widget.h"
#include "pe1/pad_state.h"

#define PE_MAX            512
#define PE_AREAS          32
#define PE_LIST_ROWS      8
#define PE_INFO_ROWS      4
#define PE_LIST_W         124     /* the creature list; the model's frame takes the rest */
#define PE_GAP            4
#define PE_PICTURE        128     /* largest model picture */
#define PE_SLOT           2       /* presenter slot (the viewer test uses 0 and 1) */
#define PE_BOX_RGB        0x101828u
#define PE_SILHOUETTE_RGB 0x080810u
#define PE_TURN_AUTO      10      /* 4096 a turn */
#define PE_TURN_HAND      96
#define PE_MENU_L1        0x4u
#define PE_MENU_R1        0x8u
#define PE_MENU_LEFT      0x8000u
#define PE_MENU_RIGHT     0x2000u
#define PE_MENU_TRIANGLE  0x10u
#define PE_MENU_SELECT    0x100u

static PeScreen s_screen;
static MenuWidgetNode *s_frame, *s_info;
static u8 s_text[96];

/* areas: the sections creatures register under, in order */
static const char *s_area_name[PE_AREAS];
static int s_areas, s_area, s_member[PE_MAX], s_members;
static int s_cursor[PE_AREAS];                      /* remembered per area */

/* the model on show */
static int s_shown = -1, s_shown_form = -1, s_shown_mode = -1, s_form, s_paused, s_manual;
static int s_yaw, s_tick, s_drawn_tick, s_frame_tick;
static const PeModel *s_model;
static PeViewFit s_fit;
static uint8_t s_rgba[PE_PICTURE * PE_PICTURE * 4];
static int s_box_x, s_box_y, s_box_w, s_box_h;

static const PeBestiaryApi *pe_b(void) { return &g_pe_bestiary_api; }

static int pe_area_of(const PeCreatureInfo *c) {
    int a;
    for (a = 0; a < s_areas; ++a) if (strcmp(s_area_name[a], c->section ? c->section : "") == 0) return a;
    return -1;
}

static void pe_build_areas(void) {
    const PeCreatureInfo *c;
    int h;
    s_areas = 0;
    for (h = 0; h < PE_MAX && (c = pe_b()->info(h)); ++h)
        if (pe_area_of(c) < 0 && s_areas < PE_AREAS) s_area_name[s_areas++] = c->section ? c->section : "";
}

static void pe_build_members(void) {
    const PeCreatureInfo *c;
    int h;
    s_members = 0;
    for (h = 0; h < PE_MAX && (c = pe_b()->info(h)); ++h)
        if (pe_area_of(c) == s_area) s_member[s_members++] = h;
}

static int pe_met(int h) { return (pe_b()->state(h) & PE_BESTIARY_MET) != 0; }

/* An area is reached once a creature in it, or (story areas) in a later story area, is met. */
static int pe_reached(int area) {
    const PeCreatureInfo *c, *first = NULL;
    int h;
    for (h = 0; h < PE_MAX && (c = pe_b()->info(h)); ++h) {
        if (pe_area_of(c) == area && !first) first = c;
        if (pe_met(h) && (pe_area_of(c) == area || (pe_area_of(c) > area && first && !c->ex && !first->ex))) return 1;
    }
    return 0;
}

/* 0 nothing shown, 1 silhouette, 2 the model */
static int pe_view_mode(int h) {
    const PeCreatureInfo *c = pe_b()->info(h);
    if (!c) return 0;
    if (pe_met(h)) return 2;
    return !c->boss && pe_reached(s_area) ? 1 : 0;
}

static int pe_current(void) {
    const int row = s_screen.list ? MenuWidget_GridCellIndex(s_screen.list) : 0;
    return row >= 0 && row < s_members ? s_member[row] : -1;
}

/* ---- the list ---- */
static void pe_row(int row) {
    const int h = row < s_members ? s_member[row] : -1;
    const PeCreatureInfo *c = h >= 0 ? pe_b()->info(h) : NULL;
    if (!c) return;
    if (!pe_met(h)) Draw_SetTextDimmed(1);
    Draw_PrintRawText(pe_text_from_ascii(s_text, sizeof s_text, pe_met(h) ? c->name : "???"));
    Draw_SetTextDimmed(0);
}

/* ---- the model ---- */
static void pe_show_blank(void) {
    memset(s_rgba, 0, sizeof s_rgba);
    pe_present_show(PE_SLOT, s_box_x, s_box_y, &(PeImage){s_box_w, s_box_h, s_rgba}, PE_PRESENT_BOX, PE_BOX_RGB);
}

static void pe_update_model(void) {
    const int h = pe_current(), mode = h >= 0 ? pe_view_mode(h) : 0;
    PeImage img = {s_box_w, s_box_h, s_rgba};
    PeView view = {s_box_w, s_box_h, 0, 0, mode == 1, PE_SILHOUETTE_RGB};
    s_drawn_tick = s_tick;
    if (h != s_shown || s_form != s_shown_form || mode != s_shown_mode) {
        PeModelSource src;
        if (h != s_shown) { s_form = 0; s_manual = 0; s_yaw = 0; }
        s_shown = h;
        s_shown_form = s_form;
        s_shown_mode = mode;
        s_model = NULL;
        if (mode && pe_b()->model(h, s_form, &src) && (s_model = pe_model_load(&src)) && !pe_model_fit(s_model, &s_fit))
            s_model = NULL;
    }
    if (!s_model) { pe_show_blank(); return; }
    view.yaw = s_yaw;
    view.frame = s_frame_tick / 2;                  /* the game animates at 30 frames a second */
    if (!s_manual) s_yaw = (s_yaw + PE_TURN_AUTO) & 4095;
    if (!s_paused) ++s_frame_tick;
    if (pe_model_render(s_model, &s_fit, &view, &img))
        pe_present_show(PE_SLOT, s_box_x, s_box_y, &img, PE_PRESENT_BOX, PE_BOX_RGB);
}

static void pe_draw_list(MenuWidgetNode *node) {
    MenuWidget_DrawList(node, pe_row);
    pe_update_model();                              /* once a menu frame */
}

/* ---- the info panel: what is known, by tier ---- */
static void pe_elements(char *out, size_t size, unsigned bits) {
    static const char *const names[5] = {"heat", "frost", "cyanide", "acid", "tranq"};
    int i, n = 0;
    out[0] = 0;
    for (i = 0; i < 5; ++i)
        if (bits & (1u << i)) n += snprintf(out + n, size - (size_t)n, "%s%s", n ? ", " : "", names[i]);
    if (!n) snprintf(out, size, "none");
}

static void pe_info_row(int row) {
    const int h = pe_current();
    const PeCreatureInfo *c = h >= 0 ? pe_b()->info(h) : NULL;
    const unsigned state = h >= 0 ? pe_b()->state(h) : 0;
    char line[96], a[48], b[48];
    if (!c) return;
    line[0] = 0;
    if (!(state & PE_BESTIARY_MET)) {
        if (row == 0) snprintf(line, sizeof line, "???");
        if (row == 1) snprintf(line, sizeof line, "%s", c->boss ? "Not met yet" : "Not met yet: defeat one to learn about it");
    } else if (row == 0) {
        snprintf(line, sizeof line, "%s   HP %u   Def %u   Kills %u", c->name, c->hp, c->defense, pe_b()->kills(h));
    } else if (row == 1) {
        if (state & PE_BESTIARY_WEAK) {
            pe_elements(a, sizeof a, c->weak);
            pe_elements(b, sizeof b, c->strong);
            snprintf(line, sizeof line, "Weak: %s   Strong: %s", a, b);
        } else {
            snprintf(line, sizeof line, "Weaknesses: Scan, %s or %u Junk", c->boss ? "defeat" : "5 kills", c->price_weak);
        }
    } else if (row == 2) {
        if (state & PE_BESTIARY_LOOT) snprintf(line, sizeof line, "Drop: %s", c->drop ? c->drop : "-");
        else if (c->boss) snprintf(line, sizeof line, "Loot: %u Junk", c->price_loot);
        else snprintf(line, sizeof line, "Loot: 15 kills or %u Junk", c->price_loot);
    } else if (row == 3 && (state & PE_BESTIARY_LOOT)) {
        snprintf(line, sizeof line, "Steal: %s", c->steal ? c->steal : "-");
    }
    if (!(state & PE_BESTIARY_MET) || row == 0 || (row == 1 && (state & PE_BESTIARY_WEAK)) || (row >= 2 && (state & PE_BESTIARY_LOOT)))
        Draw_SetTextDimmed(0);
    else Draw_SetTextDimmed(1);                     /* locked tiers read dimmer */
    Draw_PrintRawText(pe_text_from_ascii(s_text, sizeof s_text, line));
    Draw_SetTextDimmed(0);
}

static void pe_draw_info(MenuWidgetNode *node) { MenuWidget_DrawList(node, pe_info_row); }

/* ---- input ---- */
static void pe_set_area(int area) {
    if (s_screen.list) s_cursor[s_area] = MenuWidget_GridCellIndex(s_screen.list);
    s_area = (area % s_areas + s_areas) % s_areas;
    pe_build_members();
    pe_screen_rows(&s_screen, s_members ? s_members : 1, s_cursor[s_area]);
}

static void pe_close(void) {
    pe_present_hide(PE_SLOT);
    s_cursor[s_area] = MenuWidget_GridCellIndex(s_screen.list);
    pe_screen_close(&s_screen);
    s_frame = s_info = NULL;
    s_shown = -1;
}

static int pe_input(MenuWidgetNode *panel, unsigned int buttons) {
    const int h = pe_current();
    if (panel != s_screen.panel) return 0;
    if (buttons & PE_MENU_CIRCLE) { pe_close(); Menu_PlayCancelSound(); return 1; }
    if (buttons & (PE_MENU_L1 | PE_MENU_R1)) {
        pe_set_area(s_area + ((buttons & PE_MENU_R1) ? 1 : -1));
        Menu_PlayMoveSound();
        return 1;
    }
    if (buttons & (PE_MENU_LEFT | PE_MENU_RIGHT)) {
        s_manual = 1;
        s_yaw = (s_yaw + ((buttons & PE_MENU_RIGHT) ? PE_TURN_HAND : -PE_TURN_HAND)) & 4095;
        return 1;
    }
    if (buttons & PE_MENU_TRIANGLE) { s_paused = !s_paused; Menu_PlayMoveSound(); return 1; }
    if (buttons & PE_MENU_SELECT) {
        if (h >= 0 && pe_view_mode(h) && pe_b()->model(h, s_form + 1, NULL)) ++s_form;
        else if (s_form) s_form = 0;
        else { Menu_PlayErrorSound(); return 1; }
        Menu_PlayMoveSound();
        return 1;
    }
    if (buttons & ~(0x20000u | PE_MENU_CROSS)) pe_log("bestiary", "screen buttons=0x%X (unhandled)", buttons);
    return 0;
}

/* ---- the title bar: the area and how much of it is met ---- */
static int pe_on_help_text(void *ctx, void *user) {
    u8 **text = (u8 **)ctx;
    char line[96];
    int i, met = 0;
    (void)user;
    if (!s_screen.list || MenuWidget_GetCurrentNode() != s_screen.list) return 0;
    for (i = 0; i < s_members; ++i) met += pe_met(s_member[i]);
    snprintf(line, sizeof line, "%s   %d / %d", s_areas ? s_area_name[s_area] : "", met, s_members);
    *text = pe_text_from_ascii(s_text, sizeof s_text, line);
    return 0;
}

/* The screen's model stays only while the screen draws (a battle or a closed menu stops it). */
static int pe_on_frame(void *ctx, void *user) {
    (void)ctx;
    (void)user;
    ++s_tick;
    if (s_shown >= 0 && s_tick - s_drawn_tick > 2) { pe_present_hide(PE_SLOT); s_shown = -1; }
    return 0;
}

static void pe_open(MenuWidgetNode *main_list) {
    const MenuWidgetSimpleDescriptor *d = MenuWidget_LookupSimpleDescriptor(1);   /* the game's screen area */
    const int x = d && d->x > 0 ? d->x : 28, y = d && d->y > 0 ? d->y : 24, width = d && d->width > 0 ? d->width : 252;
    int ix, iy, iw, ih;
    pe_build_areas();
    if (!s_areas) return;
    if (s_area >= s_areas) s_area = 0;
    pe_screen_open(&s_screen, main_list, x, y, PE_LIST_W, PE_LIST_ROWS, pe_draw_list, pe_input);
    s_frame = pe_screen_panel(&s_screen, x + PE_LIST_W + PE_GAP, y, width - PE_LIST_W - PE_GAP, PE_LIST_ROWS, NULL);
    pe_screen_inside(s_screen.list, &ix, &iy, &iw, &ih);
    s_info = pe_screen_panel(&s_screen, x, iy + ih + 3 * PE_GAP, width, PE_INFO_ROWS, pe_draw_info);
    pe_screen_inside(s_frame, &s_box_x, &s_box_y, &s_box_w, &s_box_h);
    if (s_box_w > PE_PICTURE) s_box_x += (s_box_w - PE_PICTURE) / 2, s_box_w = PE_PICTURE;
    if (s_box_h > PE_PICTURE) s_box_y += (s_box_h - PE_PICTURE) / 2, s_box_h = PE_PICTURE;
    pe_log("bestiary", "screen: list %d,%d %dx%d  model box %d,%d %dx%d", ix, iy, iw, ih, s_box_x, s_box_y, s_box_w, s_box_h);
    s_shown = -1;
    pe_build_members();
    pe_screen_rows(&s_screen, s_members ? s_members : 1, s_cursor[s_area]);
}

/* Shown once a creature is met. */
static int pe_row_shown(void) {
    int h;
    for (h = 0; h < PE_MAX && pe_b()->info(h); ++h) if (pe_met(h)) return 1;
    return 0;
}

/* The row's icon: the menu's own P.E. icon twice, the second a few pixels up and right. */
static void pe_icon(PeHookMainMenuIcon *icon) {
    const int pe = PE_MAIN_MENU_SPRITE + 1;
    Draw_OffsetCursor(3, -2);
    Draw_AllocSprite(pe);
    Draw_OffsetCursor(-3, 2);
    icon->sprite = pe;
}

static const PeMainMenuRow s_row = { "Bestiary", 0, pe_row_shown, pe_open, pe_icon };

void pe_bestiary_menu_register(const char *mod_id) {
    pe_main_menu_add(&s_row, 25);                   /* between Key Items and the Wardrobe */
    pe_hook_add(PE_HOOK_HELP_TEXT, pe_on_help_text, NULL, PE_PRIORITY_LATE, mod_id);
    pe_hook_add(PE_HOOK_FRAME, pe_on_frame, NULL, PE_PRIORITY_NORMAL, mod_id);
}
