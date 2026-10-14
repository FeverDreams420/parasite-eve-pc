#include "pe1/cdrom.h"
#include "boot.h"
#include "automation.h"
#include "movie.h"
#include "title.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)

static void key(SDL_Keycode code) {
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = code;
    CHECK(SDL_PushEvent(&event));
}

typedef struct InputCheck { unsigned prompts, options, selections, frames; } InputCheck;

static void state_input(void *opaque, const char *event, int64_t value) {
    InputCheck *check = opaque;
    if (!strcmp(event, "intro.frame")) {
        CHECK(value == 0);
        ++check->frames;
        key(SDLK_SPACE);
    } else if (!strcmp(event, "menu.prompt")) {
        ++check->prompts;
        key(SDLK_RETURN);
    } else if (!strcmp(event, "menu.options")) {
        ++check->options;
        CHECK(value == 0 || value == 1);
        key(value == 0 ? SDLK_DOWN : SDLK_RETURN);
    } else if (!strcmp(event, "menu.selected")) {
        CHECK(value == 2);
        ++check->selections;
    }
}

int main(void) {
    const char *path = getenv("PE_TEST_DISC");
    if (!path || !*path) {
        puts("Set PE_TEST_DISC to a USA Disc 1 MODE2/2352 BIN for the asset integration test.");
        return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    }
    PeDisc disc;
    PeDiscFile intro;
    PeMovieStats stats;
    char error[256];
    CHECK(pe_disc_open(&disc, path));
    CHECK(pe_disc_find(&disc, "FMV1/FMV001.STR", &intro));
    CHECK(pe_movie_decode(&disc, &intro, NULL, &stats, error, sizeof(error)));
    CHECK(stats.width == 320 && stats.height == 240);
    CHECK(stats.video_frames > 2000 && stats.video_frames < 2200);
    CHECK(stats.sample_rate == 37800 && stats.channels == 2);
    double video_duration = stats.video_frames / 15.0;
    double audio_duration = stats.audio_samples / (double)stats.sample_rate;
    CHECK(fabs(video_duration - audio_duration) < 0.1);
    printf("intro verified: %.3fs video, %.3fs stereo audio\n", video_duration, audio_duration);
    pe_runtime_set_headless(1);
    CHECK(pe_runtime_prepare());
    CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));
    SDL_Window *window;
    SDL_Renderer *renderer;
    CHECK(SDL_CreateWindowAndRenderer("Boot integration test", 960, 720,
                                      SDL_WINDOW_HIDDEN, &window, &renderer));
    InputCheck check = {0};
    pe_runtime_set_observer(state_input, &check);
    CHECK(pe_title_menu(&disc, renderer, error, sizeof(error)) == 2);
    CHECK(check.prompts == 1 && check.options == 2 && check.selections == 1);
    CHECK(pe_runtime_finish(0) == 0);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    pe_disc_close(&disc);
    puts("menu verified: rendered Start/options states, Down, confirm Tutorial");
    memset(&check, 0, sizeof(check));
    pe_runtime_set_observer(state_input, &check);
    CHECK(pe_boot_game(path, 0) == 0);
    CHECK(check.frames == 1 && check.prompts == 1 && check.options == 2 && check.selections == 1);
    CHECK(pe_runtime_finish(0) == 0);
    puts("boot verified: headless, state-driven input, intro cancellation and clean shutdown");
    return 0;
}
