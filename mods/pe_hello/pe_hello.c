/* Example mod: proves the host, hooks, services, events and Mods panel work.
 * Logs scene starts and a frame heartbeat, offers a "hello" service and
 * event, and shows a few saved and live rows under F1 > Hello. */
#include <stdio.h>

#include "common.h"
#include "pe1/game_state.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"

#define MOD_ID "pe_hello"

typedef struct PeHelloApi {
    int (*scene_count)(void);
} PeHelloApi;

static int s_log_scenes = 1, s_heartbeat = 0, s_speed = 1, s_scene_count, s_event_scene;
static unsigned s_frames;
static const char *const kSpeedNames[] = {"Slow", "Normal", "Fast"};

static int hello_scene_count(void) { return s_scene_count; }
static const PeHelloApi s_api = {hello_scene_count};

static int on_scene_start(void *ctx, void *user) {
    (void)ctx;
    (void)user;
    s_scene_count++;
    if (s_log_scenes)
        pe_mod_log(MOD_ID, "scene %d started, game flags %08X", s_scene_count,
                   (unsigned)g_GameState.flags);
    pe_event_emit(s_event_scene, &s_scene_count);
    return 0;
}

static int on_frame(void *ctx, void *user) {
    (void)ctx;
    (void)user;
    if (++s_frames % 600 == 0 && s_heartbeat) pe_mod_log(MOD_ID, "frame %u", s_frames);
    return 0;
}

static int get_scenes(void) { return s_scene_count; }
static int set_scenes(int v) { s_scene_count = v; return 1; }
static void say_hello(void) { pe_mod_log(MOD_ID, "hello from the Mods panel (%u frames)", s_frames); }

static int hello_init(void) {
    s_event_scene = pe_event_id("hello.scene");
    pe_service_publish("hello", 1, &s_api, MOD_ID);
    pe_hook_add(PE_HOOK_SCENE_START, on_scene_start, NULL, PE_PRIORITY_NORMAL, MOD_ID);
    pe_hook_add(PE_HOOK_FRAME, on_frame, NULL, PE_PRIORITY_NORMAL, MOD_ID);

    pe_menu_begin("Hello");
    pe_menu_register_toggle(0, "Log scene starts", "Write a line to pe_mods.log per scene.",
                            &s_log_scenes, NULL);
    pe_menu_register_toggle(0, "Heartbeat", "Log every 600 frames.", &s_heartbeat, NULL);
    pe_menu_register_choice(0, "Speed (does nothing)", "A saved choice row.", &s_speed,
                            kSpeedNames, 3);
    pe_menu_register_action(0, "Say hello", "Log a line.", say_hello);
    pe_menu_register_live_int("Details", "Scenes this run", "A live row: edits the counter.",
                              0, 9999, 1, get_scenes, set_scenes);
    pe_menu_begin(0);
    return 0;
}

const PeMod pe_mod_pe_hello = {MOD_ID, "Hello (example)", "0.1", NULL, hello_init, NULL};
