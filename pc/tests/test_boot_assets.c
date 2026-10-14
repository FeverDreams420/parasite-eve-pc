#include "disc.h"
#include "boot.h"
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

/* Replay real frontend events, so accepting Start, moving the selection and
 * confirming an option are exercised rather than bypassed by a state setter. */
static int menu_input(void *opaque) {
    (void)opaque;
    SDL_Delay(100);
    key(SDLK_RETURN);
    SDL_Delay(100);
    key(SDLK_DOWN);
    SDL_Delay(100);
    key(SDLK_RETURN);
    return 0;
}

static int skip_intro_input(void *opaque) {
    (void)opaque;
    for (unsigned i = 0; i < 500 && !SDL_WasInit(SDL_INIT_VIDEO); ++i) SDL_Delay(10);
    SDL_Delay(150);
    key(SDLK_SPACE);
    SDL_Delay(300);
    key(SDLK_RETURN);
    SDL_Delay(150);
    key(SDLK_DOWN);
    SDL_Delay(150);
    key(SDLK_RETURN);
    return 0;
}

int main(void) {
    const char *path = getenv("PE_TEST_DISC");
    if (!path || !*path) {
        puts("Set PE_TEST_DISC to a USA Disc 1 MODE2/2352 BIN for the asset integration test.");
        return 77;
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
    CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));
    SDL_Window *window;
    SDL_Renderer *renderer;
    CHECK(SDL_CreateWindowAndRenderer("Boot integration test", 960, 720,
                                      SDL_WINDOW_HIDDEN, &window, &renderer));
    SDL_Thread *input = SDL_CreateThread(menu_input, "menu-test-input", NULL);
    CHECK(input);
    CHECK(pe_title_menu(&disc, renderer, error, sizeof(error)) == 2);
    SDL_WaitThread(input, NULL);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    pe_disc_close(&disc);
    puts("menu verified: Start, Down, confirm Tutorial");
    input = SDL_CreateThread(skip_intro_input, "skip-intro-test-input", NULL);
    CHECK(input);
    CHECK(pe_boot_game(path, 0) == 0);
    SDL_WaitThread(input, NULL);
    puts("boot verified: skip intro, open options, select Tutorial, clean shutdown");
    return 0;
}
