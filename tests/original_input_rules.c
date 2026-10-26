#ifdef PE_HOST_SETTINGS_TEST
extern long SetRCnt(long spec, unsigned short target, long mode);
extern long GetRCnt(long spec);
#undef NDEBUG
#include <SDL3/SDL.h>
#include <assert.h>
#include <stdio.h>

static bool TestPresent(SDL_Renderer *renderer);
static bool TestWait(SDL_Event *event, Sint32 timeout);
static bool TestRename(const char *from, const char *to);
static bool TestMessageBox(SDL_MessageBoxFlags flags, const char *title, const char *message, SDL_Window *window);

#define SDL_RenderPresent TestPresent
#define SDL_WaitEventTimeout TestWait
#define SDL_RenamePath TestRename
#define SDL_ShowSimpleMessageBox TestMessageBox
#define main original_game_main
#include "../src/main/boot/main.c"
#undef main
#undef SDL_RenderPresent
#undef SDL_WaitEventTimeout
#undef SDL_RenamePath
#undef SDL_ShowSimpleMessageBox

static int presents;
static SDL_AtomicInt panel_ready;
static const char *test_directory;

static bool TestMessageBox(SDL_MessageBoxFlags flags, const char *title, const char *message, SDL_Window *window) {
    (void)flags;
    (void)window;
    fprintf(stderr, "host-settings: unexpected message box: %s: %s\n", title, message);
    assert(!"Unexpected message box in automated settings test");
    return false;
}

static bool TestRename(const char *from, const char *to) {
    bool result = SDL_RenamePath(from, to);
    fprintf(stderr, "host-settings: preferences rename=%d error=%s\n", result, result ? "" : SDL_GetError());
    return result;
}

static bool TestWait(SDL_Event *event, Sint32 timeout) {
    static unsigned reported;
    bool result = SDL_WaitEventTimeout(event, timeout);
    if (result && reported++ < 32) {
        fprintf(
            stderr,
            "host-settings: event=%u scancode=%d repeat=%d\n",
            event->type,
            event->type == SDL_EVENT_KEY_DOWN ? event->key.scancode : 0,
            event->type == SDL_EVENT_KEY_DOWN ? event->key.repeat : 0
        );
    }
    return result;
}

static bool TestPresent(SDL_Renderer *renderer) {
    ++presents;
    if (presents == 1) {
        fprintf(stderr, "host-settings: first panel render\n");
    }
    assert(Psyz_AudioIsPaused());
    assert(Psyz_AudioWaitForTimer() == 0);
    if (presents == 1 || presents == 4 || presents == 7) {
        SDL_Surface *surface = SDL_RenderReadPixels(renderer, NULL);
        assert(surface);
        char path[4096];
        snprintf(path, sizeof(path), "%s/settings-%d.bmp", test_directory, presents);
        assert(SDL_SaveBMP(surface, path));
        SDL_DestroySurface(surface);
    }
    bool result = SDL_RenderPresent(renderer);
    if (result) {
        SDL_SetAtomicInt(&panel_ready, 1);
    }
    return result;
}

static void WaitPanel(void) {
    Uint64 deadline = SDL_GetTicks() + 5000;
    while (!SDL_GetAtomicInt(&panel_ready) && SDL_GetTicks() < deadline) {
        SDL_Delay(1);
    }
    assert(SDL_GetAtomicInt(&panel_ready));
    fprintf(stderr, "host-settings: input producer sees ready panel\n");
}

static void Key(SDL_Scancode key) {
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_KEY_DOWN;
    e.key.scancode = key;
    e.key.down = true;
    assert(SDL_PushEvent(&e));
}

static unsigned requested_image;
static unsigned request_count;
static const char *request_path;

static int SupplyImage(unsigned identity) {
    requested_image = identity;
    ++request_count;
    return request_path ? Host_InstallImage(identity, request_path) : 0;
}

static void DiscImages(void) {
    const char *paths[2] = {getenv("PE_TEST_DISC"), getenv("PE_TEST_DISC2")};
    if (!paths[0] || !paths[1]) {
        assert(!getenv("PE_REQUIRE_GAME_ASSETS"));
        return;
    }

    PeDisc images[2] = {{0}};
    host_images = images;
    assert(pe_disc_open(&images[0], paths[0]));
    g_CdImageVolumes[0] = &images[0];
    g_CdImageVolumes[1] = NULL;
    CdRom_BindImage(&images[0], NULL);
    assert(!Asset_OpenPeImage());

    unsigned identity = 0;
    Pe1GameState before = g_GameState;
    unsigned flags = g_PeImageMountFlags;
    u32 base = g_PeImageBaseLba;
    assert(!Asset_ValidatePeImage(&images[0], &identity) && identity == 1);
    assert(!Host_InstallImage(2, paths[0]));
    assert(!g_CdImageVolumes[1] && !images[1].stream);
    assert(g_CdImage == &images[0] && g_PeImageMountFlags == flags && g_PeImageBaseLba == base);
    assert(!memcmp(&before, &g_GameState, sizeof(before)));

    assert(Host_InstallImage(2, paths[1]));
    assert(g_CdImageVolumes[1] == &images[1] && !strcmp(host_options.discs[1], paths[1]));
    assert(g_CdImage == &images[0] && g_PeImageMountFlags == flags && g_PeImageBaseLba == base);
    assert(!memcmp(&before, &g_GameState, sizeof(before)));

    pe_disc_close(&images[1]);
    g_CdImageVolumes[1] = NULL;
    g_CdImageRequest = SupplyImage;
    request_path = NULL;
    request_count = 0;
    assert(Render_InitDisplayLists(2) == -4 && requested_image == 2 && request_count == 1);
    assert(g_CdImage == &images[0] && images[0].stream);

    request_path = paths[0];
    assert(Render_InitDisplayLists(2) == -4 && !g_CdImageVolumes[1]);
    assert(SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_HEADLESS", "1", true));
    Psyz_VideoSetVsyncMode(PSYZ_VSYNC_LIMITLESS);
    ResetGraph(0);

    request_path = paths[1];
    assert(!Render_InitDisplayLists(2));
    assert(requested_image == 2 && g_CdImage == g_CdImageVolumes[1] && images[0].stream);

    unsigned requests = request_count;
    assert(!Render_InitDisplayLists(1) && request_count == requests);
    CdRom_BindImage(NULL, NULL);
    g_CdImageVolumes[0] = g_CdImageVolumes[1] = NULL;
    pe_disc_close(&images[0]);
    pe_disc_close(&images[1]);

    /* Starting with Disc 2 is valid; the original check requests Disc 1. */
    assert(pe_disc_open(&images[0], paths[1]));
    g_CdImageVolumes[1] = &images[0];
    CdRom_BindImage(&images[0], NULL);
    assert(!Asset_OpenPeImage() && g_PeImageMountFlags == 2);

    request_path = paths[0];
    assert(!Render_InitDisplayLists(1));
    assert(requested_image == 1 && g_CdImage == g_CdImageVolumes[0] && images[0].stream);
    assert(!strcmp(host_options.discs[0], paths[0]) && !strcmp(host_options.discs[1], paths[1]));

    CdRom_BindImage(NULL, NULL);
    g_CdImageVolumes[0] = g_CdImageVolumes[1] = NULL;
    g_CdImageRequest = NULL;
    host_images = NULL;
    pe_disc_close(&images[0]);
    pe_disc_close(&images[1]);
}

static HostOverlayState PromptState(int row) {
    return (HostOverlayState){.open = 1, .row = row, .prompt = 1};
}

static void PromptEvent(HostOverlayState *state, Uint32 type, int code) {
    SDL_Event e;
    SDL_zero(e);
    e.type = type;
    if (type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        e.gbutton.button = (Uint8)code;
    } else {
        e.key.scancode = (SDL_Scancode)code;
        e.key.down = true;
    }
    Host_ProcessSettingsEvent(state, &e, NULL, NULL);
}

static void PromptClick(HostOverlayState *state, float x, float y) {
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    e.button.button = SDL_BUTTON_LEFT;
    e.button.x = x;
    e.button.y = y;
    Host_ProcessSettingsEvent(state, &e, NULL, NULL);
}

/* Skip-movie question: No is preselected, Yes skips, every other way out
 * keeps watching, and the panel paints. */
static void PromptAnswers(void) {
    HostOverlayState state = PromptState(1);
    PromptEvent(&state, SDL_EVENT_KEY_DOWN, SDL_SCANCODE_RETURN);
    assert(!state.open && !state.skip);

    state = PromptState(1);
    PromptEvent(&state, SDL_EVENT_KEY_DOWN, SDL_SCANCODE_UP);
    assert(state.open && state.row == 0 && state.dirty);
    SDL_Event held;
    SDL_zero(held);
    held.type = SDL_EVENT_KEY_DOWN;
    held.key.scancode = SDL_SCANCODE_RETURN;
    held.key.down = true;
    held.key.repeat = true;
    Host_ProcessSettingsEvent(&state, &held, NULL, NULL);
    assert(state.open);
    PromptEvent(&state, SDL_EVENT_KEY_DOWN, SDL_SCANCODE_KP_ENTER);
    assert(!state.open && state.skip);

    state = PromptState(0);
    PromptEvent(&state, SDL_EVENT_KEY_DOWN, SDL_SCANCODE_ESCAPE);
    assert(!state.open && !state.skip);

    state = PromptState(0);
    PromptEvent(&state, SDL_EVENT_GAMEPAD_BUTTON_DOWN, SDL_GAMEPAD_BUTTON_EAST);
    assert(!state.open && !state.skip);

    state = PromptState(1);
    PromptEvent(&state, SDL_EVENT_GAMEPAD_BUTTON_DOWN, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    assert(state.open && state.row == 0);
    PromptEvent(&state, SDL_EVENT_GAMEPAD_BUTTON_DOWN, SDL_GAMEPAD_BUTTON_START);
    assert(!state.open && state.skip);

    state = PromptState(0);
    PromptEvent(&state, SDL_EVENT_GAMEPAD_BUTTON_DOWN, SDL_GAMEPAD_BUTTON_SOUTH);
    assert(!state.open && state.skip);

    /* Settings keys mean nothing here. */
    state = PromptState(1);
    PromptEvent(&state, SDL_EVENT_KEY_DOWN, SDL_SCANCODE_TAB);
    assert(state.open && state.page == 0 && state.row == 1);

    state = PromptState(1);
    SDL_Event quit;
    SDL_zero(quit);
    quit.type = SDL_EVENT_QUIT;
    Host_ProcessSettingsEvent(&state, &quit, NULL, NULL);
    assert(!state.open && state.quit && !state.skip);

    OverlayLayout saved = s_overlayLayout;
    OverlayFitLayout(PROMPT_PANEL_W, OverlayPromptDesignHeight(), 1280, 960, &s_overlayLayout);
    float scale = s_overlayLayout.scale;
    OverlayRect yes = OverlayPromptRowRect(0);
    OverlayRect no = OverlayPromptRowRect(1);
    state = PromptState(1);
    PromptClick(&state, s_overlayLayout.x + (yes.x + 20.0f) * scale,
                s_overlayLayout.y + (yes.y + yes.h * 0.5f) * scale);
    assert(!state.open && state.skip);
    state = PromptState(0);
    PromptClick(&state, s_overlayLayout.x + (no.x + 20.0f) * scale,
                s_overlayLayout.y + (no.y + no.h * 0.5f) * scale);
    assert(!state.open && !state.skip);
    state = PromptState(0);
    PromptClick(&state, 1.0f, 1.0f);
    assert(!state.open && !state.skip);

    OverlaySurface surface = {0};
    state = PromptState(1);
    assert(OverlayFontReady());
    assert(OverlaySurfaceResize(&surface, s_overlayLayout.w, s_overlayLayout.h));
    OverlayPaintPrompt(&surface, &state, &s_overlayLayout);
    unsigned opaque = 0;
    for (int i = 0; i < surface.w * (surface.h - 1); ++i) {
        opaque += surface.pixels[i * 4 + 3] > 200;
    }
    assert(opaque > (unsigned)(surface.w * (surface.h - 1)) / 2);
    OverlaySurfaceRelease(&surface);
    s_overlayLayout = saved;
    fprintf(stderr, "host-settings: skip-movie question passed\n");
}

static int SDLCALL ClosePanel(void *unused) {
    (void)unused;
    WaitPanel();
    Key(SDL_SCANCODE_ESCAPE);
    return 0;
}

static int SDLCALL QuitPanel(void *unused) {
    (void)unused;
    WaitPanel();
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_EVENT_QUIT;
    assert(SDL_PushEvent(&event));
    return 0;
}

static int SDLCALL Drive(void *unused) {
    (void)unused;
    WaitPanel();
    Key(SDL_SCANCODE_RIGHT);  /* graphics scale 1 -> 2 */
    Key(SDL_SCANCODE_TAB);    /* controls */
    Key(SDL_SCANCODE_DOWN);   /* L2 */
    Key(SDL_SCANCODE_RETURN);
    Key(SDL_SCANCODE_F);      /* rebind */
    Key(SDL_SCANCODE_TAB);    /* audio */
    Key(SDL_SCANCODE_LEFT);   /* 100 -> 95 */
    Key(SDL_SCANCODE_ESCAPE);
    return 0;
}

static int SDLCALL DriveFocus(void *window) {
    SDL_Delay(30);
    assert(Psyz_AudioIsPaused());
    unsigned tick = (unsigned)Psyz_VideoVSync(-1);
    long counter = GetRCnt(0xf2000002);
    SDL_Delay(90);
    assert((unsigned)Psyz_VideoVSync(-1) == tick);
    assert(GetRCnt(0xf2000002) == counter);

    SDL_Event gained;
    SDL_zero(gained);
    gained.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
    gained.window.windowID = SDL_GetWindowID(window);
    assert(SDL_PushEvent(&gained));
    return 0;
}

int main(int argc, char **argv) {
#ifdef _WIN32
    /* CI must report assertions instead of waiting for a CRT dialog. */
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    assert(argc == 2);
    test_directory = argv[1];
    assert(SDL_CreateDirectory(test_directory));
    assert(SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy"));
    assert(SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software"));
    assert(SDL_Init(SDL_INIT_VIDEO));
    Host_InitOptions(0);
    Psyz_VideoSetVsyncMode(PSYZ_VSYNC_OFF);
    unsigned frozen = (unsigned)Psyz_VideoVSync(-1);
    Psyz_PauseHostClock_SDL3(true);
    SDL_Delay(80);
    assert((unsigned)Psyz_VideoVSync(-1) == frozen);
    Psyz_PauseHostClock_SDL3(true);
    SDL_Delay(40);
    Psyz_PauseHostClock_SDL3(false);
    assert((unsigned)Psyz_VideoVSync(-1) == frozen);
    Psyz_PauseHostClock_SDL3(false);
    assert((unsigned)Psyz_VideoVSync(-1) - frozen <= 1);
    assert(Psyz_SetKeyboardBinding_SDL3(16, SDL_SCANCODE_A) == -1);
    assert(Psyz_SetKeyboardBinding_SDL3(0, SDL_SCANCODE_ESCAPE) == -1);
    assert(Psyz_SetGamepadBinding_SDL3(16, 0) == -1);
    assert(Psyz_SetGamepadBinding_SDL3(0, -1) == -1);
    assert(Psyz_AudioSetOutputVolume(101) == -1);

    host_options.enabled = 1;
    snprintf(host_options.user_file, sizeof(host_options.user_file), "%s/preferences.toml", test_directory);

    char card_path[4096];
    char expected_card[4096];
    snprintf(expected_card, sizeof(expected_card), "%s/memory-cards/bu00/BASLUS-00662PE01", test_directory);
    assert(Host_CardPath(card_path, "bu00:BASLUS-00662PE01", sizeof(card_path)) == 0);
    assert(!strcmp(card_path, expected_card));

    Psyz_AdjustPathCB(Host_CardPath);
    Psyz_AdjustPath(card_path, "bu00:BASLUS-00662PE01", sizeof(card_path));
    assert(!strcmp(card_path, expected_card));

    FILE *card_file = fopen(card_path, "wb");
    assert(card_file);
    assert(fwrite("private card fixture", 1, 20, card_file) == 20);
    assert(!fclose(card_file));

    card_file = fopen(card_path, "rb");
    assert(card_file);
    char card_bytes[20];
    assert(fread(card_bytes, 1, sizeof(card_bytes), card_file) == sizeof(card_bytes));
    assert(!memcmp(card_bytes, "private card fixture", sizeof(card_bytes)));
    assert(!fclose(card_file));
    assert(!remove(card_path));
    Psyz_AdjustPathCB(NULL);

    assert(Host_CardPath(card_path, "bu10:*", sizeof(card_path)) == 0);
    snprintf(expected_card, sizeof(expected_card), "%s/memory-cards/bu10/", test_directory);
    assert(!strcmp(card_path, expected_card));

    char tiny[2] = {1, 1};
    assert(!Host_CardPath(tiny, "bu00:test", sizeof(tiny)) && !tiny[0]);
    assert(Host_CardPath(card_path, "other:file", sizeof(card_path)) == -1);

    snprintf(host_options.discs[0], sizeof(host_options.discs[0]), "/tmp/zażółć # \"quotes\" \\ disc.cue");
    assert(Host_SaveOptions());
    host_options.discs[0][0] = 0;
    Host_LoadOptions(host_options.user_file);
    assert(!strcmp(host_options.discs[0], "/tmp/zażółć # \"quotes\" \\ disc.cue"));

    char decoded[80];
    assert(Host_String("'C:\\game\\disc.cue' # comment", decoded, sizeof(decoded)));
    assert(!strcmp(decoded, "C:\\game\\disc.cue"));

    char enabled_effect[] = "true", invalid_effect[] = "2";
    assert(Host_ParseOptionValue("graphics", "antialiasing", enabled_effect));
    assert(Host_ParseOptionValue("graphics", "model_filtering", enabled_effect));
    assert(Host_SaveOptions());
    host_options.antialiasing = host_options.model_filtering = 0;
    Host_LoadOptions(host_options.user_file);
    assert(host_options.antialiasing == 1 && host_options.model_filtering == 1);
    assert(!Host_ParseOptionValue("graphics", "antialiasing", invalid_effect));
    assert(!Host_ParseOptionValue("graphics", "model_filtering", invalid_effect));
    assert(host_options.antialiasing == 1 && host_options.model_filtering == 1);
    host_options.antialiasing = host_options.model_filtering = 0;

    char invalid[4096];
    snprintf(invalid, sizeof(invalid), "%s/invalid.toml", test_directory);
    FILE *f = fopen(invalid, "w");
    assert(f);
    fputs(
        "[graphics]\ninternal_resolution = 9\nfullscreen = false\n"
        "[audio]\nvolume = 101\n"
        "[keyboard]\nL2 = 41\n"
        "[discs]\ndisc1 = \"unterminated\n",
        f
    );
    fclose(f);
    Host_LoadOptions(invalid);
    assert(host_options.scale == 1 && host_options.volume == 100);
    assert(!strcmp(host_options.discs[0], "/tmp/zażółć # \"quotes\" \\ disc.cue"));

    assert(SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_AUDIO_CLOCK", "video", true));
    assert(Psyz_AudioInit() == 0 && !Psyz_AudioIsPaused());
    fprintf(stderr, "host-settings: audio initialized; testing focus pause\n");
    assert(SetRCnt(0xf2000002, 65535, 0));

    SDL_Window *focus = SDL_CreateWindow("focus fixture", 320, 240, SDL_WINDOW_HIDDEN);
    assert(focus);
    SDL_Thread *focus_thread = SDL_CreateThread(DriveFocus, "focus pause", focus);
    assert(focus_thread);
    Host_PauseForFocus(focus);
    SDL_WaitThread(focus_thread, NULL);
    fprintf(stderr, "host-settings: active audio focus pause passed\n");
    assert(!Psyz_AudioIsPaused());

    Psyz_AudioPause();
    focus_thread = SDL_CreateThread(DriveFocus, "already paused focus", focus);
    assert(focus_thread);
    Host_PauseForFocus(focus);
    SDL_WaitThread(focus_thread, NULL);
    assert(Psyz_AudioIsPaused());
    Psyz_AudioUnpause();
    SDL_DestroyWindow(focus);
    fprintf(stderr, "host-settings: paused audio focus pause passed; opening settings\n");

    int original_key = host_options.keys[0];
    SDL_SetAtomicInt(&panel_ready, 0);
    SDL_Thread *thread = SDL_CreateThread(Drive, "settings input", NULL);
    assert(thread);

    SDL_Event open;
    SDL_zero(open);
    open.type = SDL_EVENT_KEY_DOWN;
    open.key.scancode = SDL_SCANCODE_ESCAPE;
    open.key.down = true;
    assert(Host_SettingsEvent(&open));
    SDL_WaitThread(thread, NULL);
    fprintf(stderr, "host-settings: settings input completed\n");
    assert(!Psyz_AudioIsPaused());
    assert(host_options.scale == 2 && host_options.volume == 95);
    assert(host_options.keys[0] == SDL_SCANCODE_F && original_key != SDL_SCANCODE_F);
    assert(Psyz_GetKeyboardBinding_SDL3(0) == SDL_SCANCODE_F);
    assert(Psyz_AudioGetOutputVolume() == 95);

    host_options.scale = 1;
    host_options.volume = 100;
    host_options.keys[0] = original_key;
    Host_LoadOptions(host_options.user_file);
    assert(host_options.scale == 2 && host_options.volume == 95 && host_options.keys[0] == SDL_SCANCODE_F);

    SDL_Event ignored;
    SDL_zero(ignored);
    ignored.type = SDL_EVENT_KEY_UP;
    assert(!Host_SettingsEvent(&ignored));
    assert(Psyz_SetOverlayEventFilter_SDL3(Host_SettingsEvent) == NULL);
    assert(Psyz_SetOverlayEventFilter_SDL3(NULL) == Host_SettingsEvent);

    /* Issue #2: All settings tabs share a fixed panel height and top Y anchor. */
    OverlayLayout base_layout = {0};
    OverlayComputeLayout(OVERLAY_PAGE_GRAPHICS, 1280, 720, &base_layout);
    for (int page = 0; page < OVERLAY_PAGE_COUNT; ++page) {
        OverlayLayout page_layout = {0};
        OverlayComputeLayout(page, 1280, 720, &page_layout);
        assert(page_layout.x == base_layout.x && page_layout.y == base_layout.y);
        assert(page_layout.w == base_layout.w && page_layout.h == base_layout.h);
    }

    /* Issue #3: Mouse hover on Controls never auto-scrolls; wheel & keys step by 1. */
    s_overlayLayout = base_layout;
    HostOverlayState scroll_state = {
        .open = 1,
        .page = OVERLAY_PAGE_CONTROLS,
        .row = 0,
        .scroll = 0
    };
    OverlayRect bottom_visible = OverlayRowRect(&scroll_state, OVERLAY_CONTROLS_VISIBLE_BINDINGS);
    SDL_Event hover;
    SDL_zero(hover);
    hover.type = SDL_EVENT_MOUSE_MOTION;
    hover.motion.x = (float)base_layout.x + (bottom_visible.x + 20.0f) * base_layout.scale;
    hover.motion.y = (float)base_layout.y + (bottom_visible.y + 10.0f) * base_layout.scale;
    Host_ProcessSettingsEvent(&scroll_state, &hover, NULL, NULL);
    assert(scroll_state.row == OVERLAY_CONTROLS_VISIBLE_BINDINGS);
    assert(scroll_state.scroll == 0);

    SDL_Event wheel;
    SDL_zero(wheel);
    wheel.type = SDL_EVENT_MOUSE_WHEEL;
    wheel.wheel.y = -1.0f;
    Host_ProcessSettingsEvent(&scroll_state, &wheel, NULL, NULL);
    assert(scroll_state.scroll == 1);

    SDL_Event down_key;
    SDL_zero(down_key);
    down_key.type = SDL_EVENT_KEY_DOWN;
    down_key.key.scancode = SDL_SCANCODE_DOWN;
    down_key.key.down = true;
    Host_ProcessSettingsEvent(&scroll_state, &down_key, NULL, NULL);
    assert(scroll_state.row == OVERLAY_CONTROLS_VISIBLE_BINDINGS + 1);
    assert(scroll_state.scroll == 1);
    Host_ProcessSettingsEvent(&scroll_state, &down_key, NULL, NULL);
    assert(scroll_state.row == OVERLAY_CONTROLS_VISIBLE_BINDINGS + 2);
    assert(scroll_state.scroll == 2);

    /* Multi-selection callback owns paths and rejects a third image. */
    HostDiscDialog dialog = {0};
    const char *files[] = {"second.cue", "first.cue", NULL};
    Host_DiscChosen(&dialog, files, 0);
    assert(SDL_GetAtomicInt(&dialog.done));
    assert(!dialog.failed && !strcmp(dialog.paths[1], "first.cue"));
    SDL_free(dialog.paths[0]);
    SDL_free(dialog.paths[1]);

    memset(&dialog, 0, sizeof(dialog));
    const char *three[] = {"1.cue", "2.cue", "3.cue", NULL};
    Host_DiscChosen(&dialog, three, 0);
    assert(dialog.failed);
    SDL_free(dialog.paths[0]);
    SDL_free(dialog.paths[1]);

    memset(&dialog, 0, sizeof(dialog));
    const char *cancel[] = {NULL};
    Host_DiscChosen(&dialog, cancel, 0);
    assert(!dialog.failed && !dialog.paths[0]);
    Host_DiscChosen(&dialog, NULL, 0);
    assert(dialog.failed);

    Psyz_AudioPause();
    SDL_SetAtomicInt(&panel_ready, 0);
    thread = SDL_CreateThread(ClosePanel, "close paused settings", NULL);
    assert(thread);
    assert(Host_SettingsEvent(&open));
    SDL_WaitThread(thread, NULL);
    assert(Psyz_AudioIsPaused());

    SDL_SetAtomicInt(&panel_ready, 0);
    thread = SDL_CreateThread(QuitPanel, "quit settings", NULL);
    assert(thread);
    Uint64 quit_started = SDL_GetTicksNS();
    assert(Host_SettingsEvent(&open));
    SDL_WaitThread(thread, NULL);
    assert(Psyz_AudioIsPaused());

    SDL_Event returned;
    unsigned quits = 0;
    Uint64 quit_deadline = SDL_GetTicks() + 1000;
    while (!quits && SDL_GetTicks() < quit_deadline) {
        if (SDL_WaitEventTimeout(&returned, 20) && returned.type == SDL_EVENT_QUIT) {
            ++quits;
            assert(returned.quit.timestamp >= quit_started && returned.quit.timestamp <= SDL_GetTicksNS());
        }
    }
    assert(quits == 1 && !SDL_HasEvent(SDL_EVENT_QUIT));
    Psyz_AudioDestroy();
    fprintf(stderr, "host-settings: paused settings completed; checking disc images\n");
    PromptAnswers();
    DiscImages();
    SDL_Quit();
    puts("PASS: both image identities, missing-image checks, additive mounting, TOML roundtrip, invalid values, modal graphics/controls/audio, persistence, ESC consumption, multi-disc callback, skip-movie question");
    return 0;
}

#else

#include <SDL3/SDL.h>
#include <psyz/input.h>
#include <psyz/overlay_sdl3.h>
#include <stdio.h>
#include "pe1/battle.h"
#include "pe1/boot.h"
#include "pe1/menu_widget.h"
#include "pe1/pad_state.h"
#include "pe1/scene_flags.h"

static int menu_cell_indices(void) {
    MenuWidgetNode node = {0};
    node.grid_width = 1;
    node.cell_mask = -1;
    node.cursor_x = -1;
    if (MenuWidget_GetCellIndex(&node) != -1) {
        return 0;
    }
    node.cursor_x = 0;
    node.cursor_y = -1;
    if (MenuWidget_GetCellIndex(&node) != -1) {
        return 0;
    }
    if (MenuWidget_GetCellIndex(NULL) != -1) {
        return 0;
    }

    const unsigned masks[] = {0, 1, 0x80000000u, 0x55555555u, 0xffffffffu};
    for (unsigned mask = 0; mask < sizeof(masks) / sizeof(masks[0]); ++mask) {
        node.cell_mask = (int)masks[mask];
        for (int index = 0; index < 96; ++index) {
            node.cursor_y = index;
            int expected = (masks[mask] & (1u << (index & 31))) ? index : -1;
            if (MenuWidget_GetCellIndex(&node) != expected) {
                return 0;
            }
        }
    }
    puts("Menu cell selection: absent node/cursor and 480 MIPS-masked index/mask cases passed");
    return 1;
}

static unsigned calls;

static void trigger(struct BattleEntity *ctx, int *index) {
    ++calls;
    ctx->motionX += 1;
    *index = 1;
}

static void fallback(struct BattleEntity *ctx, int *index) {
    (void)index;
    ++calls;
    ctx->motionX += 10;
}

int main(void) {
    if (!menu_cell_indices()) {
        return 45;
    }
    if (!SDL_Init(SDL_INIT_EVENTS)) {
        return 10;
    }

    Boot_ResetPadInput();
    Boot_ResetPadInput();

    const SDL_Scancode keys[] = {SDL_SCANCODE_RETURN, SDL_SCANCODE_KP_ENTER};
    for (unsigned i = 0; i < 2; ++i) {
        SDL_Event event;
        SDL_zero(event);
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.scancode = keys[i];
        event.key.down = true;
        if (!SDL_PushEvent(&event) || (g_PadPackets[0].buttons & 8)) {
            return 11;
        }
        if ((g_PadPackets[0].header & 0xf0ff) != 0x4000) {
            return 12;
        }
        event.type = SDL_EVENT_KEY_UP;
        if (!SDL_PushEvent(&event) || !(g_PadPackets[0].buttons & 8)) {
            return 13;
        }
    }

    if (Psyz_SetKeyboardBinding_SDL3(11, SDL_SCANCODE_F)) {
        return 40;
    }
    Boot_ResetPadInput(); /* Reset must not undo an explicit rebind. */
    if (Psyz_GetKeyboardBinding_SDL3(11) != SDL_SCANCODE_F) {
        return 45;
    }

    SDL_Event remapped;
    SDL_zero(remapped);
    remapped.type = SDL_EVENT_KEY_DOWN;
    remapped.key.scancode = SDL_SCANCODE_RETURN;
    remapped.key.down = true;
    SDL_PushEvent(&remapped);
    if (!(g_PadPackets[0].buttons & 8)) {
        return 41;
    }
    remapped.type = SDL_EVENT_KEY_UP;
    SDL_PushEvent(&remapped);

    remapped.type = SDL_EVENT_KEY_DOWN;
    remapped.key.scancode = SDL_SCANCODE_F;
    SDL_PushEvent(&remapped);
    if (g_PadPackets[0].buttons & 8) {
        return 42;
    }
    remapped.type = SDL_EVENT_KEY_UP;
    SDL_PushEvent(&remapped);
    if (Psyz_SetKeyboardBinding_SDL3(11, SDL_SCANCODE_RETURN)) {
        return 43;
    }

    SDL_Event focus;
    SDL_zero(focus);
    focus.type = SDL_EVENT_WINDOW_FOCUS_LOST;
    SDL_PushEvent(&focus);

    SDL_Event key;
    SDL_zero(key);
    key.type = SDL_EVENT_KEY_DOWN;
    key.key.scancode = SDL_SCANCODE_RETURN;
    key.key.down = true;
    SDL_PushEvent(&key);
    if (!(g_PadPackets[0].buttons & 8)) {
        return 15;
    }

    focus.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
    SDL_PushEvent(&focus);
    SDL_PushEvent(&key);
    if (g_PadPackets[0].buttons & 8) {
        return 16;
    }
    key.type = SDL_EVENT_KEY_UP;
    SDL_PushEvent(&key);

    /* Exercise native controller polling without opening a video device. */
    const char *headless = SDL_GetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_HEADLESS");
    char *saved_headless = headless ? SDL_strdup(headless) : NULL;
    SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_HEADLESS", "", true);

    g_BootHostActive = 1;
    Boot_ResetPadInput();
    char controller[PSYZ_PAD_BUF_LEN] = {0};
    controller[1] = 0x41;
    controller[2] = (char)0xff;
    controller[3] = (char)0xbf;
    Psyz_PadsSet(0, controller, sizeof(controller));
    Field_HandleStateTransition();
    if (g_PadPackets[0].buttons & 0x4000) {
        return 44;
    }

    g_BootHostActive = 0;
    Boot_ResetPadInput();
    if (saved_headless) {
        SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_HEADLESS", saved_headless, true);
        SDL_free(saved_headless);
    } else {
        SDL_UnsetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_HEADLESS");
    }

    puts("SDL keyboard watch: Return/keypad Enter reach original START; release, focus and repeated reset passed");

    SceneFlagRecord rules[] = {{1 | 4, 3, 4, trigger}, {0, 0, 0, fallback}};
    SceneFlagRecord *table[] = {rules, 0};
    int mode = 0;
    BattleEntity actor = {0};
#define value actor.motionX
    g_FieldPadBits = 3;
    g_FieldPadPressed = 0;
    Scene_CheckFlagBits(&actor, table, &mode);
    if (value != 10 || calls != 1) {
        return 1;
    }

    mode = 0;
    value = 0;
    calls = 0;
    g_FieldPadPressed = 4;
    Scene_CheckFlagBits(&actor, table, &mode);
    if (value != 1 || calls != 1 || mode != 1) {
        return 2;
    }

    /* Callback mode changes do not redirect the current record walk. */
    SceneFlagRecord continued[] = {{0x20 | 0x10, 0, 0, trigger}, {0x20, 0, 0, trigger}, {0, 0, 0, fallback}};
    table[0] = continued;
    mode = 0;
    value = 0;
    calls = 0;
    Scene_CheckFlagBits(&actor, table, &mode);
    if (value != 2 || calls != 2) {
        return 3;
    }

    rules[0].flags = 2 | 8;
    rules[0].mask_any = 4;
    table[0] = rules;
    mode = 0;
    value = 0;
    calls = 0;
    g_FieldPadBits = 4;
    g_FieldPadReleased = 4;
    Scene_CheckFlagBits(&actor, table, &mode);
    if (value != 1 || calls != 1) {
        return 4;
    }

    puts("Original input rule masks, callback continuation and fallback passed");
    return 0;
}

#endif
