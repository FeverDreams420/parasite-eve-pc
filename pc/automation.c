#include "automation.h"
#include <SDL3/SDL.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REPLAY_STEPS 128

typedef struct ReplayStep {
    char event[48];
    int64_t value;
    SDL_Keycode key;
    int close;
} ReplayStep;

static struct {
    PeRuntimeObserver observer;
    void *user;
    FILE *trace;
    ReplayStep steps[REPLAY_STEPS];
    unsigned count, cursor;
    uint64_t sequence, start, clock_origin, timeout;
    int headless;
    char error[192];
} runtime;

static int fail(const char *message) {
    if (!runtime.error[0]) snprintf(runtime.error, sizeof(runtime.error), "%s", message);
    return 0;
}

void pe_runtime_set_observer(PeRuntimeObserver observer, void *user) {
    runtime.observer = observer;
    runtime.user = user;
}

void pe_runtime_set_headless(int enabled) { runtime.headless = enabled != 0; }
int pe_runtime_headless(void) { return runtime.headless; }
void pe_runtime_set_replay_timeout(uint64_t milliseconds) { runtime.timeout = milliseconds; }
const char *pe_runtime_error(void) { return runtime.error; }

int pe_runtime_prepare(void) {
    runtime.start = SDL_GetTicks();
    runtime.clock_origin = SDL_GetPerformanceCounter();
    if (!runtime.headless) return 1;
    if (!SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "dummy", SDL_HINT_OVERRIDE) ||
        !SDL_SetHintWithPriority(SDL_HINT_AUDIO_DRIVER, "dummy", SDL_HINT_OVERRIDE) ||
        !SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE))
        return fail("Cannot configure the headless backend");
    return 1;
}

int pe_runtime_open_trace(const char *path) {
    if (runtime.trace) return fail("A trace is already open");
    runtime.trace = fopen(path, "wb");
    return runtime.trace ? 1 : fail("Cannot create runtime trace");
}

static int action(ReplayStep *step, const char *name) {
    static const struct { const char *name; SDL_Keycode key; } keys[] = {
        {"return", SDLK_RETURN}, {"space", SDLK_SPACE}, {"escape", SDLK_ESCAPE},
        {"up", SDLK_UP}, {"down", SDLK_DOWN}, {"z", SDLK_Z}
    };
    if (!strcmp(name, "assert")) return 1;
    if (!strcmp(name, "close")) { step->close = 1; return 1; }
    for (unsigned i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
        if (!strcmp(name, keys[i].name)) { step->key = keys[i].key; return 1; }
    return 0;
}

int pe_runtime_load_replay(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return fail("Cannot open replay");
    runtime.count = runtime.cursor = 0;
    runtime.error[0] = 0;
    char line[256];
    unsigned number = 0;
    while (fgets(line, sizeof(line), file)) {
        ++number;
        char *begin = line;
        while (*begin == ' ' || *begin == '\t') ++begin;
        if (*begin == '#' || *begin == '\n' || *begin == '\r' || !*begin) continue;
        char event[48], value[32], command[24], extra;
        if (runtime.count == REPLAY_STEPS ||
            sscanf(begin, "%47s %31s %23s %c", event, value, command, &extra) != 3) goto malformed;
        char *end;
        errno = 0;
        long long expected = strtoll(value, &end, 10);
        if (errno || *end || !*value) goto malformed;
        ReplayStep step = {0};
        for (const char *p = event; *p; ++p)
            if (!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '.' || *p == '_'))
                goto malformed;
        snprintf(step.event, sizeof(step.event), "%s", event);
        step.value = (int64_t)expected;
        if (!action(&step, command)) goto malformed;
        runtime.steps[runtime.count++] = step;
        continue;
malformed:
        snprintf(runtime.error, sizeof(runtime.error), "Malformed replay at line %u", number);
        fclose(file);
        runtime.count = 0;
        return 0;
    }
    int ok = !ferror(file) && runtime.count;
    fclose(file);
    return ok ? 1 : fail("Replay is empty or unreadable");
}

void pe_runtime_emit(const char *event, int64_t value) {
    ++runtime.sequence;
    uint64_t now = SDL_GetPerformanceCounter();
    if (!runtime.clock_origin) runtime.clock_origin = now;
    uint64_t elapsed_ms = (uint64_t)((now - runtime.clock_origin) * 1000.0 /
                                   SDL_GetPerformanceFrequency());
    if (runtime.trace) {
        if (fprintf(runtime.trace, "{\"seq\":%" PRIu64 ",\"event\":\"%s\",\"value\":%" PRId64
                    ",\"time_ms\":%" PRIu64 "}\n", runtime.sequence, event, value,
                    elapsed_ms) < 0 || fflush(runtime.trace) != 0)
            fail("Cannot write runtime trace");
    }
    if (!runtime.error[0] && runtime.cursor < runtime.count) {
        ReplayStep *step = &runtime.steps[runtime.cursor];
        if (!strcmp(event, step->event)) {
            if (value != step->value) {
                snprintf(runtime.error, sizeof(runtime.error), "Replay step %u: %s expected %" PRId64
                         ", got %" PRId64, runtime.cursor + 1, event, step->value, value);
            } else {
                ++runtime.cursor;
                if (step->key || step->close) {
                    SDL_Event input;
                    SDL_zero(input);
                    input.type = step->close ? SDL_EVENT_QUIT : SDL_EVENT_KEY_DOWN;
                    if (!step->close) input.key.key = step->key;
                    if (!SDL_PushEvent(&input)) fail("Cannot inject replay input");
                }
            }
        }
    }
    if (runtime.observer) runtime.observer(runtime.user, event, value);
}

int pe_runtime_should_stop(void) {
    /* A stalled replay must fail without user intervention. Full intro playback
     * fits the default; an outer test timeout can impose a stricter deadline. */
    if (runtime.count &&
        SDL_GetTicks() - runtime.start > (runtime.timeout ? runtime.timeout : 180000))
        fail(runtime.cursor < runtime.count ? "Replay timeout waiting for the next state" :
                                             "Replay timeout waiting for application shutdown");
    return runtime.error[0] != 0;
}

int pe_runtime_finish(int exit_code) {
    if (runtime.count && runtime.cursor != runtime.count && !runtime.error[0]) {
        snprintf(runtime.error, sizeof(runtime.error), "Replay incomplete at step %u: waiting for %s",
                 runtime.cursor + 1, runtime.steps[runtime.cursor].event);
    }
    if (runtime.trace && fclose(runtime.trace) != 0) fail("Cannot close runtime trace");
    runtime.trace = NULL;
    if (runtime.error[0]) {
        fprintf(stderr, "Automation: %s\n", runtime.error);
        exit_code = 1;
    }
    runtime.count = runtime.cursor = 0;
    runtime.sequence = runtime.clock_origin = runtime.timeout = 0;
    runtime.error[0] = 0;
    runtime.observer = NULL;
    runtime.user = NULL;
    return exit_code;
}
