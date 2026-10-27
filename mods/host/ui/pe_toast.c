/* One-line messages over the game for a few seconds (pe_toast). Drawn at the top of the screen
 * without dimming; while the Mods panel is open a toast waits and shows when it closes. */
#include <math.h>
#include <stdio.h>
#include <SDL3/SDL.h>

#include "pe_mod_host.h"
#include "pe_ui.h"

#define PE_TOAST_MS 4000
#define PE_REFERENCE_H 820.0f

static char s_text[160];
static Uint64 s_until;
static int s_dirty, s_shown, s_blocked;
static PeSurface s_surface;

void pe_toast(const char *text) {
    snprintf(s_text, sizeof s_text, "%s", text ? text : "");
    s_until = SDL_GetTicks() + PE_TOAST_MS;
    s_dirty = 1;
    pe_mod_log(NULL, "toast: %s", s_text);
}

/* The panel owns the screen while open; afterwards a running toast is shown again. */
void pe_toast_block(int blocked) {
    s_blocked = blocked;
    if (blocked) s_shown = 0;
    else if (s_until) s_dirty = 1;
}

static int pe_toast_tick(void *ctx, void *user) {
    int w, h, bw, bh;
    float s;
    (void)ctx;
    (void)user;
    if (s_blocked || !s_until) return 0;
    if (SDL_GetTicks() >= s_until) {
        s_until = 0;
        if (s_shown) pe_ui_gpu_hide();
        s_shown = 0;
        return 0;
    }
    if (!s_dirty || !pe_ui_font_ready() || !pe_ui_gpu_window_size(&w, &h)) return 0;
    s = floorf((float)h / PE_REFERENCE_H * 8.0f) / 8.0f;
    if (s < 0.5f) s = 0.5f;
    bw = (int)ceilf(pe_ui_text_width(s_text, 17 * s) + 60 * s);
    bh = (int)ceilf(44 * s);
    if (bw > w) bw = w;
    if (!pe_ui_surface_begin(&s_surface, bw, bh)) return 0;
    pe_ui_fill_round_rect(&s_surface, 0, 0, (float)bw, (float)bh, 10 * s, pe_ui_border);
    pe_ui_fill_round_rect(&s_surface, 1, 1, (float)bw - 2, (float)bh - 2, 9 * s, pe_ui_panel);
    pe_ui_draw_text(&s_surface, 30 * s, 28 * s, 17 * s, s_text, pe_ui_text, PE_ALIGN_LEFT);
    pe_ui_surface_finish(&s_surface);
    s_shown = pe_ui_gpu_show(&s_surface, (w - bw) / 2, (int)((float)h * 0.06f), 0.0f);
    s_dirty = 0;
    return 0;
}

void pe_toast_install(void) {
    pe_hook_add(PE_HOOK_FRAME, pe_toast_tick, NULL, PE_PRIORITY_LATE, "toast");
}
