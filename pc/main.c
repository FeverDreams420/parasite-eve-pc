/* Native boot frontend and optional independent PsyZ window smoke test. */

#ifdef PLATFORM_IOS
#include <SDL3/SDL_main.h>
#endif

#include <psyz.h>
#include <libetc.h>
#include <libgpu.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "disc.h"
#include "movie.h"
#include "boot.h"
#include "automation.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
#define FRAME_LIMIT 8

typedef struct Db {
    DRAWENV draw;
    DISPENV disp;
    OT_TYPE ot[1];
} Db;

static Db db[2];
static Db *cdb;

static void init_graphics(void) {
    SetDefDrawEnv(&db[0].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&db[1].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&db[1].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetVideoMode(MODE_NTSC);
    ResetGraph(0);
    PutDrawEnv(&db[0].draw);
    PutDispEnv(&db[0].disp);
    ClearOTag(db[0].ot, 1);
    ClearOTag(db[1].ot, 1);
    SetDispMask(1);
    cdb = &db[0];
}

static void display(void) {
    cdb = (cdb == &db[0]) ? &db[1] : &db[0];
    FntFlush(-1);
    ClearOTag(cdb->ot, 1);
    DrawSync(0);
    VSync(0);
    ClearImage(&cdb->draw.clip, 16, 16, 24);
    DrawOTag(&cdb->ot[0]);
}

int main(int argc, char **argv) {
    int frame = 0;
    for (int i = 1; i < argc;) {
        int consumed = 0;
        if (!strcmp(argv[i], "--headless")) {
            pe_runtime_set_headless(1);
            consumed = 1;
        } else if (!strcmp(argv[i], "--replay-timeout-ms")) {
            if (i + 1 == argc) { fprintf(stderr, "Missing replay timeout\n"); return 1; }
            char *end;
            errno = 0;
            unsigned long long timeout = strtoull(argv[i + 1], &end, 10);
            if (errno || *end || argv[i + 1][0] < '0' || argv[i + 1][0] > '9' ||
                !timeout || timeout > 86400000) {
                fprintf(stderr, "Invalid replay timeout\n"); return pe_runtime_finish(1);
            }
            pe_runtime_set_replay_timeout(timeout);
            consumed = 2;
        } else if (!strcmp(argv[i], "--trace") || !strcmp(argv[i], "--replay")) {
            if (i + 1 == argc) { fprintf(stderr, "Missing automation path\n"); return 1; }
            int ok = !strcmp(argv[i], "--trace") ? pe_runtime_open_trace(argv[i + 1]) :
                                                 pe_runtime_load_replay(argv[i + 1]);
            if (!ok) { fprintf(stderr, "%s\n", pe_runtime_error()); return pe_runtime_finish(1); }
            consumed = 2;
        }
        if (consumed) {
            for (int j = i; j + consumed <= argc; ++j) argv[j] = argv[j + consumed];
            argc -= consumed;
        } else ++i;
    }

    if (argc == 1) {
        const char *disc_path = getenv("PE_DISC_IMAGE");
        if (disc_path && *disc_path) return pe_runtime_finish(pe_boot_game(disc_path, 0));
        fprintf(stderr, "Usage: %s --disc <Disc 1.bin>\n"
                        "Or set PE_DISC_IMAGE to your MODE2/2352 BIN.\n", argv[0]);
        return 1;
    }
    if (argc == 3 && strcmp(argv[1], "--disc") == 0)
        return pe_runtime_finish(pe_boot_game(argv[2], 0));
    if (argc == 3 && strcmp(argv[1], "--menu") == 0)
        return pe_runtime_finish(pe_boot_game(argv[2], 1));
    if (argc == 3 && strcmp(argv[1], "--play-intro") == 0)
        return pe_runtime_finish(pe_boot_intro(argv[2]));
    if (argc == 3 && strcmp(argv[1], "--decode-intro") == 0) {
        PeDisc disc;
        PeDiscFile movie;
        PeMovieStats stats;
        char error[256];
        if (!pe_disc_open(&disc, argv[2])) {
            fprintf(stderr, "%s\n", disc.error);
            return 1;
        }
        int ok = pe_disc_find(&disc, "FMV1/FMV001.STR", &movie);
        if (ok) ok = pe_movie_decode(&disc, &movie, NULL, &stats, error, sizeof(error));
        else snprintf(error, sizeof(error), "%s", disc.error);
        pe_disc_close(&disc);
        if (!ok) { fprintf(stderr, "%s\n", error); return 1; }
        printf("intro: %u frames, %dx%d, %llu audio samples, %d Hz, %d channels\n",
               stats.video_frames, stats.width, stats.height,
               (unsigned long long)stats.audio_samples, stats.sample_rate, stats.channels);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "--inspect-disc") == 0) {
        PeDisc disc;
        if (!pe_disc_open(&disc, argv[2])) {
            fprintf(stderr, "%s\n", disc.error);
            return 1;
        }
        const char *paths[] = {"SYSTEM.CNF", "PE.IMG", "FMV1/FMV000.STR", "FMV1/FMV001.STR"};
        for (unsigned i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) {
            PeDiscFile file;
            if (!pe_disc_find(&disc, paths[i], &file)) {
                fprintf(stderr, "%s\n", disc.error);
                pe_disc_close(&disc);
                return 1;
            }
            printf("%s: sector %u, %u bytes\n", paths[i], file.sector, file.size);
        }
        pe_disc_close(&disc);
        return 0;
    }
    if (argc != 2 || strcmp(argv[1], "--smoke-test") != 0) {
        fprintf(stderr, "Unknown arguments. Use --disc <Disc 1.bin> or --smoke-test.\n");
        return 1;
    }
    if (pe_runtime_headless()) {
        fprintf(stderr, "The PsyZ GPU smoke test requires a graphics backend; use --headless with --disc or --menu.\n");
        return pe_runtime_finish(1);
    }
    init_graphics();
    FntLoad(960, 256);
    SetDumpFnt(FntOpen(8, 16, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512));
    printf("parasite-eve-pc: psyz window up\n");
    while (frame < FRAME_LIMIT && !Psyz_QuitRequested()) {
        FntPrint("Parasite Eve PC\n");
        FntPrint("frame %d\n", frame);
        display();
        frame++;
    }
    printf("parasite-eve-pc: exited after %d frames\n", frame);
    return frame == 0;
}
