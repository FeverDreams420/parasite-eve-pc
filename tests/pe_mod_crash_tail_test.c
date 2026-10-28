/* Crash report room snapshot, save tail: the mods' save write handlers fill the
 * snapshot block as a real save does, nothing is taken while mod code is
 * running, and later saves still reach the mods. */
#include <stdio.h>
#include <string.h>
#include "pe_hooks.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"

static int writes, inside_mod;
static unsigned char card[0x2000], live[0x2000];

static int on_save_write(void *ctx, void *user) {
    (void)user;
    memcpy((unsigned char *)ctx + 0x1C90, "TEST", 4);
    writes++;
    return 0;
}

static int on_frame(void *ctx, void *user) {
    (void)ctx; (void)user;
    if (inside_mod) inside_mod = pe_mod_snapshot_save_tail(card);
    return 0;
}

static int test_init(void) {
    pe_hook_add(PE_HOOK_SAVE_WRITE, on_save_write, NULL, PE_PRIORITY_NORMAL, "pe_test");
    pe_hook_add(PE_HOOK_FRAME, on_frame, NULL, PE_PRIORITY_NORMAL, "pe_test");
    return 0;
}

static const PeMod test_mod = {"pe_test", "Test", "1", NULL, test_init, NULL};
const PeMod *const g_pe_mods[] = {&test_mod, NULL};

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
void pe_pages_describe(void) {}

static int failures;
#define CHECK(cond) \
    do { \
        if (!(cond)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #cond); failures++; } \
    } while (0)

int main(void) {
    PeHookInit init = {0, NULL};
    CHECK(pe_host_init(&init) == 0);

    /* A normal save still reaches the mods. */
    pe_hook_dispatch(PE_HOOK_SAVE_WRITE, live);
    CHECK(writes == 1 && !memcmp(live + 0x1C90, "TEST", 4));

    /* Asked while mod code is running: nothing is taken. */
    inside_mod = 1;
    pe_hook_dispatch(PE_HOOK_FRAME, NULL);
    CHECK(inside_mod == -1);
    CHECK(writes == 1 && card[0x1C90] == 0);

    /* The snapshot: each handler once, into the snapshot block. */
    memset(live, 0, sizeof(live));
    CHECK(pe_mod_snapshot_save_tail(card) == 0);
    CHECK(writes == 2 && !memcmp(card + 0x1C90, "TEST", 4));

    /* Every room takes one: later saves and snapshots still reach the mods. */
    pe_hook_dispatch(PE_HOOK_SAVE_WRITE, live);
    CHECK(writes == 3 && !memcmp(live + 0x1C90, "TEST", 4));
    memset(card, 0, sizeof(card));
    CHECK(pe_mod_snapshot_save_tail(card) == 0);
    CHECK(writes == 4 && !memcmp(card + 0x1C90, "TEST", 4));

    if (failures) return 1;
    printf("room snapshot save tail: ok\n");
    return 0;
}
