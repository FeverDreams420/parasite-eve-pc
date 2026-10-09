/* Host smoke test. PsyZ brings up a window and a BIOS font. The decompiled
 * game is not called from here yet. */

#ifdef PLATFORM_IOS
#include <SDL3/SDL_main.h>
#endif

#include <psyz.h>
#include <libetc.h>
#include <libgpu.h>
#include <stdio.h>

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

    (void)argc;
    (void)argv;
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
