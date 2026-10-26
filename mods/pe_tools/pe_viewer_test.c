/*
 * Viewer test: F1 > Debug > Viewer test. A throwaway test for the Bestiary's model viewer
 * (bridge findings pe-bestiary-viewer-look-options-2026-10-26.md, sections 8.5 and 9). Fork only.
 *
 *   - Test card: a 128 x 128 card shown through the image presenter (host/pe_present.c), as a box
 *     or as strips, at a chosen place. A 1-pixel border, colour bars, gradients, a 1-pixel
 *     checkerboard, red corner squares and a see-through hole and corners, so position, scaling,
 *     filtering and transparency show at a glance. Open Aya's menu to see it over the game's own.
 *   - Pink test: fills one 64 x 256 VRAM page with pink (saved first) to see whether anything on
 *     screen uses it; choosing Off puts the page back. A room change or a battle while it is on ends
 *     it without putting it back (the game may have reloaded the page): reload the save then if
 *     anything stays pink. Loading a save puts it back first.
 *   - Dump VRAM: the native VRAM copy to pe_vram_dump.bmp beside the preferences (texture and
 *     palette areas are exact; at internal resolution > 1 the framebuffers there are stale).
 *   - Info rows: the buffer pasted into, video settings, the paste cost.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psyz/video.h>

#include "pe_game.h"
#include "pe_image.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe_present.h"
#include "pe1/display_env.h"
#include "pe1/draw_buffers.h"
#include "pe1/psyq_gpu.h"

#define PE_CARD        128
#define PE_SLOT_TEST   0
#define PE_BOX_RGB     0x182038u    /* a dark viewer-screen blue */
#define PE_PAGE_W      64
#define PE_PAGE_H      256
#define PE_PINK        0x7C1Fu      /* 15-bit r = 31, b = 31 */
#define PE_PAGES       22           /* x 320, 384 .. 960; y 0 and 256 */

static int s_on, s_method, s_x = 176, s_y = 48, s_pink;
static int s_shown_method = -1, s_shown_x = -1, s_shown_y = -1;
static uint8_t s_card_rgba[PE_CARD * PE_CARD * 4];
static int s_card_ready, s_logged;
static const char *const kMethods[] = {"Box", "Strips"};

static char s_pink_names[PE_PAGES + 1][16];
static const char *s_pink_list[PE_PAGES + 1];
static int s_pink_active = -1, s_pink_room = -1;
static uint16_t s_pink_saved[PE_PAGE_W * PE_PAGE_H];
static char s_pink_status[80] = "Off";

/* ---- the test card ---- */
static void pe_card_pixel(int x, int y, uint8_t *p) {
    static const uint8_t bars[8][3] = {{255,255,255},{255,255,0},{0,255,255},{0,255,0},
                                       {255,0,255},{255,0,0},{0,0,255},{0,0,0}};
    const int c = PE_CARD / 2, dx = x - c, dy = y - c, r = 14;
    int cx = x < r ? r : x > PE_CARD - 1 - r ? PE_CARD - 1 - r : x;
    int cy = y < r ? r : y > PE_CARD - 1 - r ? PE_CARD - 1 - r : y;
    p[3] = 255;
    if ((x - cx) * (x - cx) + (y - cy) * (y - cy) > r * r || dx * dx + dy * dy < 22 * 22) {
        p[0] = p[1] = p[2] = p[3] = 0;                                 /* see-through */
        return;
    }
    if (x == 0 || y == 0 || x == PE_CARD - 1 || y == PE_CARD - 1 ||
        (x - cx) * (x - cx) + (y - cy) * (y - cy) > (r - 1) * (r - 1)) {
        p[0] = p[1] = p[2] = 255;                                       /* border */
    } else if ((x < 20 || x >= PE_CARD - 20) && (y < 20 || y >= PE_CARD - 20) &&
               (x >= 14 && x < PE_CARD - 14) && (y >= 14 && y < PE_CARD - 14)) {
        p[0] = 255; p[1] = p[2] = 0;                                    /* corner squares */
    } else if (y < 44) {
        memcpy(p, bars[x * 8 / PE_CARD], 3);                            /* colour bars */
    } else if (y < 88) {
        if (x < c) { p[0] = (uint8_t)(x * 255 / (c - 1)); p[1] = (uint8_t)((y - 44) * 255 / 43); p[2] = 64; }
        else p[0] = p[1] = p[2] = (uint8_t)(((x ^ y) & 1) ? 255 : 0);   /* 1-pixel checkerboard */
    } else {
        p[0] = p[1] = p[2] = (uint8_t)((x / 4) * 255 / 31);             /* 32-step grey ramp */
    }
}

static void pe_card_build(void) {
    int x, y;
    if (s_card_ready) return;
    for (y = 0; y < PE_CARD; ++y)
        for (x = 0; x < PE_CARD; ++x) pe_card_pixel(x, y, s_card_rgba + 4 * (y * PE_CARD + x));
    s_card_ready = 1;
}

static const char *pe_aspect_name(void) {
    return Psyz_VideoGetAspectMode() == PSYZ_ASPECT_SQUARE ? "Square" : "Display";
}

static void pe_log_setup(void) {
    const RECT *a = &g_RenderDrawEnvArray[0].clip, *b = &g_RenderDrawEnvArray[1].clip;
    pe_log("viewer", "draw0=%d,%d %dx%d draw1=%d,%d %dx%d slot=%d res=%ux aa=%d geom=%d filter=%d aspect=%s",
           a->x, a->y, a->w, a->h, b->x, b->y, b->w, b->h, g_ActiveDrawSlot, Psyz_VideoGetInternalResolution(),
           Psyz_VideoGetAntialiasing(), Psyz_VideoGetGeometryCorrection(), Psyz_VideoGetModelFiltering(),
           pe_aspect_name());
}

static void pe_card_tick(void) {
    PeImage card = {PE_CARD, PE_CARD, s_card_rgba};
    if (!s_on) {
        if (s_shown_method >= 0) { pe_present_hide(PE_SLOT_TEST); s_shown_method = -1; }
        return;
    }
    if (s_method == s_shown_method && s_x == s_shown_x && s_y == s_shown_y) return;
    pe_card_build();
    if (pe_present_show(PE_SLOT_TEST, s_x, s_y, &card, s_method ? PE_PRESENT_STRIPS : PE_PRESENT_BOX, PE_BOX_RGB) == 0) {
        s_shown_method = s_method;
        s_shown_x = s_x;
        s_shown_y = s_y;
        pe_log("viewer", "test card %s at %d,%d", kMethods[s_method], s_x, s_y);
        if (!s_logged) { pe_log_setup(); s_logged = 1; }
    }
}

/* ---- pink test ---- */
static RECT pe_page_rect(int n) {             /* n = 1 .. PE_PAGES */
    RECT r;
    r.x = (short)(320 + ((n - 1) % 11) * PE_PAGE_W);
    r.y = (short)(((n - 1) / 11) * PE_PAGE_H);
    r.w = PE_PAGE_W;
    r.h = PE_PAGE_H;
    return r;
}

static void pe_pink_restore(void) {
    RECT r;
    if (s_pink_active <= 0) return;
    r = pe_page_rect(s_pink_active);
    LoadImage(&r, s_pink_saved);
    pe_log("viewer", "pink test: page %s put back", s_pink_names[s_pink_active]);
    s_pink_active = -1;
}

static void pe_pink_tick(void) {
    if (s_pink_active > 0 && (pe_room_number() != s_pink_room || pe_in_battle())) {
        snprintf(s_pink_status, sizeof s_pink_status, "Ended by %s; reload the save if pink stays",
                 pe_in_battle() ? "a battle" : "a room change");
        pe_log("viewer", "pink test: page %s dropped (%s)", s_pink_names[s_pink_active], s_pink_status);
        pe_toast(s_pink_status);
        s_pink_active = -1;
        s_pink = 0;
        return;
    }
    if (s_pink == (s_pink_active < 0 ? 0 : s_pink_active)) return;
    pe_pink_restore();
    if (s_pink > 0) {
        static uint16_t fill[PE_PAGE_W * PE_PAGE_H];
        RECT r = pe_page_rect(s_pink);
        int i;
        StoreImage(&r, s_pink_saved);         /* texture areas: the native copy is exact */
        for (i = 0; i < PE_PAGE_W * PE_PAGE_H; ++i) fill[i] = PE_PINK;
        LoadImage(&r, fill);
        s_pink_active = s_pink;
        s_pink_room = pe_room_number();
        snprintf(s_pink_status, sizeof s_pink_status, "Page %s is pink", s_pink_names[s_pink]);
        pe_log("viewer", "pink test: page %s filled (room %d)", s_pink_names[s_pink], s_pink_room);
    } else {
        snprintf(s_pink_status, sizeof s_pink_status, "Off");
    }
}

static int pe_on_save_read(void *ctx, void *user) {
    (void)ctx;
    (void)user;
    pe_pink_restore();
    s_pink = 0;
    snprintf(s_pink_status, sizeof s_pink_status, "Off");
    return 0;
}

/* ---- VRAM dump ---- */
static void pe_put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

static void pe_dump_vram(void) {
    enum { W = 1024, H = 512 };
    char path[1200];
    RECT all = {0, 0, W, H};
    uint16_t *vram = malloc((size_t)W * H * 2);
    uint8_t *row = malloc((size_t)W * 3), header[54] = {'B', 'M'};
    FILE *f;
    int x, y;
    if (!vram || !row || pe_mod_path(path, sizeof path, "pe_vram_dump.bmp") != 0 || !(f = fopen(path, "wb"))) {
        free(vram); free(row); pe_toast("VRAM dump failed"); return;
    }
    StoreImage(&all, vram);
    pe_put32(header + 2, 54 + W * H * 3); pe_put32(header + 10, 54); pe_put32(header + 14, 40);
    pe_put32(header + 18, W); pe_put32(header + 22, H); header[26] = 1; header[28] = 24;
    pe_put32(header + 34, W * H * 3);
    fwrite(header, 1, sizeof header, f);
    for (y = H - 1; y >= 0; --y) {                         /* BMP rows bottom up, BGR */
        for (x = 0; x < W; ++x) {
            const uint16_t c = vram[y * W + x];
            row[3 * x + 0] = (uint8_t)(((c >> 10) & 31) << 3);
            row[3 * x + 1] = (uint8_t)(((c >> 5) & 31) << 3);
            row[3 * x + 2] = (uint8_t)((c & 31) << 3);
        }
        fwrite(row, 1, (size_t)W * 3, f);
    }
    fclose(f);
    free(vram);
    free(row);
    pe_log("viewer", "VRAM dumped to pe_vram_dump.bmp (room %d)", pe_room_number());
    pe_toast("VRAM dumped to pe_vram_dump.bmp");
}

/* ---- info rows ---- */
static void pe_frame_info(char *out, unsigned size) {
    PePresentStats st;
    pe_present_stats(&st);
    if (!st.frame_w) { snprintf(out, size, "-"); return; }
    snprintf(out, size, "slot %d: %dx%d at %d,%d", st.draw_slot, st.frame_w, st.frame_h, st.frame_x, st.frame_y);
}

static void pe_video_info(char *out, unsigned size) {
    snprintf(out, size, "%ux, AA %s, geometry %s, filter %s, %s", Psyz_VideoGetInternalResolution(),
             Psyz_VideoGetAntialiasing() ? "on" : "off", Psyz_VideoGetGeometryCorrection() ? "on" : "off",
             Psyz_VideoGetModelFiltering() ? "on" : "off", pe_aspect_name());
}

static void pe_cost_info(char *out, unsigned size) {
    PePresentStats st;
    pe_present_stats(&st);
    snprintf(out, size, "%.1f us a frame, %u pastes", st.avg_us, st.pastes);
}

static void pe_pink_info(char *out, unsigned size) { snprintf(out, size, "%s", s_pink_status); }

static void pe_tick(void) {
    static int started;
    if (!started) { started = 1; s_on = 0; s_pink = 0; }   /* saved rows: the test always starts off */
    pe_card_tick();
    pe_pink_tick();
}

void pe_viewer_test_register(const char *mod_id) {
    const char *page = PE_PAGE_DEBUG_VIEWER;
    int n;
    snprintf(s_pink_names[0], sizeof s_pink_names[0], "Off");
    for (n = 1; n <= PE_PAGES; ++n) {
        RECT r = pe_page_rect(n);
        snprintf(s_pink_names[n], sizeof s_pink_names[n], "x%d y%d", r.x, r.y);
    }
    for (n = 0; n <= PE_PAGES; ++n) s_pink_list[n] = s_pink_names[n];
    pe_menu_register_toggle(page, "Test card", "Show a 128 x 128 test card through the image presenter. Off by default.", &s_on, NULL);
    pe_menu_register_choice(page, "Method", "Box: one opaque paste. Strips: only the card's pixels, row by row.", &s_method, kMethods, 2);
    pe_menu_register_int(page, "X", "Card's left edge on the 320 x 224 game screen.", &s_x, 0, 320 - PE_CARD, 4);
    pe_menu_register_int(page, "Y", "Card's top edge on the 320 x 224 game screen.", &s_y, 0, 224 - PE_CARD, 4);
    pe_menu_register_info(page, "Frame", "The buffer the card was pasted into last.", pe_frame_info);
    pe_menu_register_info(page, "Video", "Internal resolution and filters (change them in Escape > Graphics).", pe_video_info);
    pe_menu_register_info(page, "Paste cost", "Average time of a frame's pastes, and how many.", pe_cost_info);
    pe_menu_register_choice(page, "Pink test page", "Fill a VRAM page with pink to see what uses it; Off puts it back.",
                            &s_pink, s_pink_list, PE_PAGES + 1);
    pe_menu_register_info(page, "Pink test", "What the pink test is doing.", pe_pink_info);
    pe_menu_register_action(page, "Dump VRAM", "Writes pe_vram_dump.bmp beside the preferences.", pe_dump_vram);
    pe_menu_register_tick(pe_tick);
    pe_hook_add(PE_HOOK_SAVE_READ, pe_on_save_read, NULL, PE_PRIORITY_NORMAL, mod_id);
}
