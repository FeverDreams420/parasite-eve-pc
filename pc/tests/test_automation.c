#include "automation.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%d: %s\n", __LINE__, #expr); exit(1); } } while (0)

static void fixture(const char *text) {
    FILE *file = fopen("test-automation.replay", "wb");
    CHECK(file);
    CHECK(fputs(text, file) != EOF);
    CHECK(fclose(file) == 0);
}

int main(void) {
    pe_runtime_set_headless(1);
    CHECK(pe_runtime_prepare());
    CHECK(SDL_Init(SDL_INIT_EVENTS));
    fixture("menu.prompt 0 return\nmenu.options 0 down\nmenu.selected 2 assert\n");
    CHECK(pe_runtime_load_replay("test-automation.replay"));
    CHECK(pe_runtime_open_trace("test-automation.jsonl"));
    pe_runtime_emit("backend.ready", 1);
    pe_runtime_emit("menu.prompt", 0);
    SDL_Event event;
    int found = 0;
    while (SDL_PollEvent(&event))
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_RETURN) found = 1;
    CHECK(found);
    pe_runtime_emit("menu.options", 0);
    found = 0;
    while (SDL_PollEvent(&event))
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_DOWN) found = 1;
    CHECK(found);
    pe_runtime_emit("menu.selected", 2);
    CHECK(pe_runtime_finish(0) == 0);
    FILE *trace = fopen("test-automation.jsonl", "rb");
    CHECK(trace);
    char line[256];
    unsigned lines = 0;
    while (fgets(line, sizeof(line), trace)) {
        CHECK(strstr(line, "\"seq\":") && strstr(line, "\"event\":") && strstr(line, "\"value\":"));
        ++lines;
    }
    CHECK(lines == 4);
    fclose(trace);
    fixture("menu.options 0 assert\n");
    CHECK(pe_runtime_load_replay("test-automation.replay"));
    pe_runtime_emit("menu.options", 1);
    CHECK(pe_runtime_should_stop());
    CHECK(pe_runtime_finish(0) == 1);
    fixture("menu.selected 2 assert\n");
    CHECK(pe_runtime_load_replay("test-automation.replay"));
    CHECK(pe_runtime_finish(0) == 1);
    fixture("menu.options 9999999999999999999999999 assert\n");
    CHECK(!pe_runtime_load_replay("test-automation.replay"));
    CHECK(pe_runtime_finish(0) == 1);
    fixture("menu.options 0 unknown\n");
    CHECK(!pe_runtime_load_replay("test-automation.replay"));
    CHECK(pe_runtime_finish(0) == 1);
    CHECK(remove("test-automation.replay") == 0);
    CHECK(remove("test-automation.jsonl") == 0);
    SDL_Quit();
    puts("automation: inputs, trace, mismatch, incomplete replay and invalid commands passed");
    return 0;
}
