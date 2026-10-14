#include "title.h"
#include "automation.h"
#include <stdlib.h>
#include <string.h>

/* USA title asset record: the initial STR directory/image bank read by
 * CdRom_LoadInitialImage. These are offsets in PE.IMG, never host addresses. */
#define TITLE_ASSET_SECTOR 978u
#define TITLE_ASSET_SECTORS 133u
#define TITLE_BUNDLE_OFFSET 0x4264u

typedef struct TitleImage {
    const uint8_t *pixels;
    unsigned width, height;
} TitleImage;

static uint32_t read32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int load_images(PeDisc *disc, uint8_t **data, TitleImage *images,
                       char *error, size_t error_size) {
    PeDiscFile archive;
    const size_t size = TITLE_ASSET_SECTORS * 2048u;
    if (!pe_disc_find(disc, "PE.IMG", &archive)) goto disc_error;
    *data = malloc(size);
    if (!*data) { snprintf(error, error_size, "Cannot allocate title assets"); return 0; }
    if (!pe_disc_read(disc, &archive, TITLE_ASSET_SECTOR * 2048u, *data, size))
        goto disc_error;
    const uint8_t *bundle = *data + TITLE_BUNDLE_OFFSET;
    if (read32(bundle) != 9) goto format_error;
    for (unsigned i = 0; i < 8; ++i) {
        uint64_t offset = TITLE_BUNDLE_OFFSET + (uint64_t)read32(bundle + 4 + i * 4);
        if (offset > size || size - offset < 20) goto format_error;
        const uint8_t *tim = *data + offset;
        if (read32(tim) != 16 || read32(tim + 4) != 3) goto format_error;
        unsigned words = (unsigned)tim[16] | (unsigned)tim[17] << 8;
        unsigned height = (unsigned)tim[18] | (unsigned)tim[19] << 8;
        if (!words || !height || words * 2 % 3) goto format_error;
        unsigned width = words * 2 / 3;
        uint64_t bytes = (uint64_t)width * height * 3;
        if (bytes > size - offset - 20 || read32(tim + 8) != bytes + 12)
            goto format_error;
        images[i] = (TitleImage){tim + 20, width, height};
    }
    if (images[0].width != 320 || images[0].height != 204) goto format_error;
    return 1;
disc_error:
    snprintf(error, error_size, "%s", disc->error);
    return 0;
format_error:
    snprintf(error, error_size, "Unsupported title image bank; expected the USA Disc 1 assets");
    return 0;
}

/* Match MenuCard_BlendImage's maximum-channel composition, including the
 * green highlight, without pointer casts or packed 32-bit loads. */
static void blend(uint8_t *canvas, const TitleImage *image, int x, int y,
                   unsigned intensity) {
    for (unsigned row = 0; row < image->height; ++row) {
        for (unsigned column = 0; column < image->width; ++column) {
            int target_x = x + (int)column;
            int target_y = y + (int)row;
            if (target_x < 0 || target_x >= 320 || target_y < 0 || target_y >= 240)
                continue;
            const uint8_t *source = image->pixels + (row * image->width + column) * 3;
            uint8_t *target = canvas + (target_y * 320 + target_x) * 3;
            for (unsigned channel = 0; channel < 3; ++channel) {
                unsigned value = source[channel] * intensity / 256;
                if (target[channel] < value) target[channel] = (uint8_t)value;
            }
        }
    }
}

int pe_title_menu(PeDisc *disc, SDL_Renderer *renderer, char *error, size_t error_size) {
    uint8_t *assets = NULL;
    TitleImage images[8];
    uint8_t pixels[320 * 240 * 3];
    SDL_Texture *texture = NULL;
    int result = -1;
    if (!load_images(disc, &assets, images, error, error_size)) goto cleanup;
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                               SDL_TEXTUREACCESS_STREAMING, 320, 240);
    if (!texture) goto sdl_error;
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    const char *capture = getenv("PE_MENU_CAPTURE");
    const char *timeout = getenv("PE_MENU_TEST_TIMEOUT_MS");
    const char *capture_state = getenv("PE_MENU_CAPTURE_STATE");
    uint64_t start = SDL_GetTicks();
    unsigned timeout_ms = timeout ? (unsigned)strtoul(timeout, NULL, 10) : 0;
    int prompt = 1, selection = 0, captured = 0;
    int reported_prompt = -1, reported_selection = -1;
    printf("boot: main menu ready\n");
    fflush(stdout);
    for (;;) {
        if (pe_runtime_should_stop()) {
            snprintf(error, error_size, "Automation: %s", pe_runtime_error());
            goto cleanup;
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) { result = 0; goto cleanup; }
            if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat) continue;
            SDL_Keycode key = event.key.key;
            if (key == SDLK_RETURN || key == SDLK_SPACE || key == SDLK_Z) {
                if (prompt) prompt = 0;
                else {
                    result = selection == 0 ? 1 : 2;
                    pe_runtime_emit("menu.selected", result);
                    goto cleanup;
                }
            } else if (key == SDLK_UP || key == SDLK_DOWN || key == SDLK_W || key == SDLK_S) {
                if (!prompt) selection = 1 - selection;
            } else if (key == SDLK_ESCAPE) {
                if (prompt) { result = 0; goto cleanup; }
                prompt = 1;
            }
        }
        memset(pixels, 0, sizeof(pixels));
        blend(pixels, &images[0], 0, 20, 256);
        blend(pixels, &images[2], (320 - (int)images[2].width) / 2, 216, 256);
        if (prompt) {
            unsigned pulse = 160 + (unsigned)((SDL_GetTicks() - start) / 16 % 96);
            blend(pixels, &images[1], (320 - (int)images[1].width) / 2, 176, pulse);
        } else {
            blend(pixels, &images[7], 118, selection == 0 ? 154 : 194, 256);
            blend(pixels, &images[3], 118, 154, 256);
            /* Continue is unavailable until native save support is integrated. */
            blend(pixels, &images[4], 118, 174, 64);
            blend(pixels, &images[5], 118, 194, 256);
        }
        if (!SDL_UpdateTexture(texture, NULL, pixels, 320 * 3)) goto sdl_error;
        int width, height;
        if (!SDL_GetRenderOutputSize(renderer, &width, &height)) goto sdl_error;
        float scale = (float)width / 320;
        if (scale * 240 > height) scale = (float)height / 240;
        SDL_FRect target = {(width - 320 * scale) / 2, (height - 240 * scale) / 2,
                            320 * scale, 240 * scale};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        if (!SDL_RenderClear(renderer) || !SDL_RenderTexture(renderer, texture, NULL, &target))
            goto sdl_error;
        if (capture && !captured &&
            (!capture_state || strcmp(capture_state, "options") != 0 || !prompt)) {
            SDL_Surface *surface = SDL_RenderReadPixels(renderer, NULL);
            int ok = surface && SDL_SaveBMP(surface, capture);
            SDL_DestroySurface(surface);
            if (!ok) goto sdl_error;
            captured = 1;
        }
        if (!SDL_RenderPresent(renderer)) goto sdl_error;
        if (prompt != reported_prompt || (!prompt && selection != reported_selection)) {
            pe_runtime_emit(prompt ? "menu.prompt" : "menu.options", prompt ? 0 : selection);
            reported_prompt = prompt;
            reported_selection = selection;
        }
        if (timeout_ms && SDL_GetTicks() - start >= timeout_ms) { result = 0; break; }
        SDL_Delay(16);
    }
    goto cleanup;
sdl_error:
    snprintf(error, error_size, "SDL title: %s", SDL_GetError());
cleanup:
    SDL_DestroyTexture(texture);
    free(assets);
    return result;
}
