#include "title.h"
#include "automation.h"
#include "room.h"
#include "pe1/akao.h"
#include "pe1/akao/globals.h"
#include "pe1/akao/note_step.h"
#include "pe1/akao/voice_state.h"
#include "pe1/akao/init_state.h"
#include "pe1/menu_sound.h"
#include <psyz/spu.h>
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
    SDL_AudioStream *audio = NULL;
    PeRoomCatalog catalog;
    PeRoomBlob menu_sounds = {0};
    unsigned char *samples = NULL;
    int result = -1;
    if (!load_images(disc, &assets, images, error, error_size)) goto cleanup;
    if (!pe_room_catalog_open(&catalog, disc) || !pe_sector_bank_load(&catalog, 7, &menu_sounds)) {
        snprintf(error, error_size, "%s", catalog.error);
        goto cleanup;
    }
    PeDiscFile executable;
    unsigned char tables[516];
    if (!pe_disc_find(disc, "SLUS_006.62", &executable) ||
        !pe_disc_read(disc, &executable, 0x9b8f4 - 0x10000 + 0x800, tables, sizeof(tables))) {
        snprintf(error, error_size, "%s", disc->error);
        goto cleanup;
    }
    memcpy(g_AkaoDefaultVoiceProgram, tables, 4);
    for (unsigned i = 0; i < 256; ++i)
        g_AkaoPanVolumeTable[i] = (short)(tables[4 + i * 2] | (unsigned)tables[5 + i * 2] << 8);
    memcpy(g_AkaoOpcodeArgLengths, catalog.akao.lengths, 96);
    memcpy(g_AkaoExtOpcodeArgLengths, catalog.akao.extended_lengths, 32);
    memcpy(g_AkaoNoteLengthTable, catalog.akao.note_lengths, sizeof(catalog.akao.note_lengths));
    for (unsigned i = 0; i < 16; ++i)
        g_AkaoSoundEntryTable[i] = catalog.akao.lfo_words + catalog.akao.lfo_offsets[i];
    size_t sample_size = (size_t)(catalog.sector_bounds[7] - catalog.sector_bounds[6]) * 2048;
    samples = malloc(sample_size);
    if (!samples || !pe_disc_read(disc, &catalog.archive,
        (uint64_t)catalog.sector_bounds[6] * 2048, samples, sample_size)) {
        snprintf(error, error_size, "Cannot load startup sound samples");
        goto cleanup;
    }
    Psyz_SpuInit();
    Psyz_SpuReset(0);
    SpuInit();
    SpuSetTransferMode(0);
    if (Spu_UploadSampleBlock((int *)samples, 1) != 0) {
        snprintf(error, error_size, "Invalid startup sound samples");
        goto cleanup;
    }
    Spu_InitVoiceState();
    Psyz_SpuWrite(0x1aa, Psyz_SpuRead(0x1aa) | 0xc000);
    g_AkaoBgmHandle[0] = menu_sounds.bytes;
    SDL_AudioSpec spec = {SDL_AUDIO_S16, 2, 44100};
    audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!audio || !SDL_ResumeAudioStreamDevice(audio)) goto sdl_error;
    uint64_t audio_tick = 0;
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
    int audio_reported = 1;
    uint64_t leaving_at = 0;
    printf("boot: main menu ready\n");
    fflush(stdout);
    for (;;) {
        if (pe_runtime_should_stop()) {
            snprintf(error, error_size, "Automation: %s", pe_runtime_error());
            goto cleanup;
        }
        if (leaving_at && SDL_GetTicks() >= leaving_at) goto cleanup;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) { result = 0; goto cleanup; }
            if (leaving_at) continue;
            if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat) continue;
            SDL_Keycode key = event.key.key;
            if (key == SDLK_RETURN || key == SDLK_SPACE || key == SDLK_Z) {
                audio_reported = 0;
                Menu_PlayConfirmSound();
                Akao_ProcessMessageQueue();
                if (prompt) prompt = 0;
                else {
                    result = selection == 0 ? 1 : 2;
                    pe_runtime_emit("menu.selected", result);
                    leaving_at = SDL_GetTicks() + 500;
                }
            } else if (key == SDLK_UP || key == SDLK_DOWN || key == SDLK_W || key == SDLK_S) {
                if (!prompt) {
                    selection = 1 - selection;
                    audio_reported = 0;
                    Menu_PlayMoveSound();
                    Akao_ProcessMessageQueue();
                }
            } else if (key == SDLK_ESCAPE) {
                if (prompt) { result = 0; goto cleanup; }
                audio_reported = 0;
                Menu_PlayCancelSound();
                Akao_ProcessMessageQueue();
                prompt = 1;
            }
        }
        /* Run the original 240 Hz timer on the main thread; SDL only consumes
         * PCM, so input and sequence state cannot race an audio callback. */
        uint64_t target_tick = (SDL_GetTicks() - start) * 240 / 1000 + 4;
        while (audio_tick < target_tick) {
            short pcm[368];
            Akao_Tick();
            unsigned frames = (unsigned)(((audio_tick + 1) * 44100 / 240) - (audio_tick * 44100 / 240));
            Psyz_SpuPullSamples(pcm, frames);
            if (!SDL_PutAudioStreamData(audio, pcm, frames * 2 * sizeof(short))) goto sdl_error;
            if (!audio_reported) {
                for (unsigned i = 0; i < frames * 2; ++i) {
                    if (pcm[i]) {
                        audio_reported = 1;
                        pe_runtime_emit("audio.menu.pcm", 1);
                        break;
                    }
                }
            }
            ++audio_tick;
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
    g_AkaoBgmHandle[0] = NULL;
    memset(g_AkaoSoundEntryTable, 0, 16 * sizeof(void *));
    SDL_DestroyAudioStream(audio);
    pe_room_blob_close(&menu_sounds);
    free(samples);
    SDL_DestroyTexture(texture);
    free(assets);
    return result;
}
