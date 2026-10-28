/* A mod that asks to run alone (pe_mod_run_alone, in its init): when it runs, no other mod does, so
 * none of their hooks, services or ticks reach the game; a later request, or one outside an init,
 * is refused. Every toggle in Manage mods is on here. */
#include <stdio.h>
#include <string.h>
#include "pe_hooks.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"

static int frames_a, frames_b, frames_alone, frames_late, late_request = 1;
static const int api = 1;

static int on_frame_a(void *ctx, void *user) { (void)ctx; (void)user; frames_a++; return 0; }
static int on_frame_b(void *ctx, void *user) { (void)ctx; (void)user; frames_b++; return 0; }
static int on_frame_alone(void *ctx, void *user) { (void)ctx; (void)user; frames_alone++; return 0; }
static int on_frame_late(void *ctx, void *user) { (void)ctx; (void)user; frames_late++; return 0; }

static int init_a(void) {
    pe_service_publish("pe_test_a", 1, &api, "pe_a");
    return pe_hook_add(PE_HOOK_FRAME, on_frame_a, NULL, PE_PRIORITY_NORMAL, "pe_a") < 0;
}
static int init_b(void) { return pe_hook_add(PE_HOOK_FRAME, on_frame_b, NULL, PE_PRIORITY_NORMAL, "pe_b") < 0; }
static int init_alone(void) {
    if (pe_mod_run_alone() != 0 || pe_mod_run_alone() != 0) return 1;   /* asking twice is fine */
    return pe_hook_add(PE_HOOK_FRAME, on_frame_alone, NULL, PE_PRIORITY_NORMAL, "pe_alone") < 0;
}
static int init_late(void) {
    late_request = pe_mod_run_alone();
    return pe_hook_add(PE_HOOK_FRAME, on_frame_late, NULL, PE_PRIORITY_NORMAL, "pe_late") < 0;
}

static const char *const needs_a[] = {"pe_a", NULL};
static const PeMod mod_a = {"pe_a", "A", "1", NULL, init_a, NULL};
static const PeMod mod_b = {"pe_b", "B", "1", needs_a, init_b, NULL};
static const PeMod mod_alone = {"pe_alone", "Alone", "1", NULL, init_alone, NULL};
static const PeMod mod_late = {"pe_late", "Late", "1", NULL, init_late, NULL};
const PeMod *const g_pe_mods[] = {&mod_a, &mod_b, &mod_alone, &mod_late, NULL};

/* The host's menu and panel, not under test: every toggle on. */
void pe_main_menu_init(void) {}
void pe_equip_sync_init(void) {}
void pe_present_install(void) {}
void pe_menu_begin(const char *page) { (void)page; }
void pe_menu_load_settings(void) {}
int pe_menu_register_toggle(const char *sub, const char *label, const char *description, int *flag,
                            PeMenuPerFrame per_frame) {
    (void)sub; (void)label; (void)description; (void)per_frame;
    *flag = 1;
    return 0;
}
PeMenuRow *pe_menu_row(int index) { (void)index; return NULL; }
int pe_menu_row_count(void) { return 0; }
int pe_menu_top_page(const char *page, const char *description, int order) {
    (void)page; (void)description; (void)order;
    return 0;
}
void pe_panel_install(void) {}
void pe_toast(const char *text) { (void)text; }
int pe_free_field(void) { return 1; }
void pe_pages_describe(void) {}

static int failures;
#define CHECK(cond) \
    do { \
        if (!(cond)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #cond); failures++; } \
    } while (0)

int main(void) {
    PeHookInit init = {0, NULL};
    CHECK(pe_mod_run_alone() == -1);                 /* not in an init */
    CHECK(pe_host_init(&init) == 0);

    CHECK(late_request == -1);                       /* the first request won */
    CHECK(pe_mod_is_running("pe_alone"));
    CHECK(!pe_mod_is_running("pe_a") && !pe_mod_is_running("pe_b") && !pe_mod_is_running("pe_late"));
    CHECK(pe_mod_active("pe_alone") && !pe_mod_active("pe_a") && !pe_mod_active("pe_late"));
    CHECK(pe_mod_active(NULL));                      /* the host's own stay active */
    CHECK(pe_service_get("pe_test_a", 1) == NULL);   /* an inactive mod offers nothing */

    pe_hook_dispatch(PE_HOOK_FRAME, NULL);
    CHECK(frames_alone == 1);
    CHECK(frames_a == 0 && frames_b == 0 && frames_late == 0);
    CHECK(pe_mod_run_alone() == -1);                 /* after start-up */

    if (failures) return 1;
    puts("pe_mod_run_alone: ok");
    return 0;
}
