#include "boot.h"
#include "disc.h"
#include "movie.h"
#include "title.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIDEO_QUEUE_SIZE 32
#define AUDIO_RATE 37800
#define AUDIO_CHANNELS 2

typedef struct VideoFrame {
    uint8_t *rgb;
    int width, height, pitch;
    double seconds;
} VideoFrame;

typedef struct BootPlayer {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_AudioStream *audio;
    SDL_Mutex *mutex;
    SDL_Condition *space;
    SDL_Thread *worker;
    SDL_AtomicInt stopped;
    PeDisc *disc;
    PeDiscFile file;
    PeMovieStats stats;
    VideoFrame frames[VIDEO_QUEUE_SIZE];
    unsigned head, count;
    uint64_t start;
    int width, height;
    int quit, skipped, failed, finished, decoded;
    char error[256];
    const char *capture;
    int captured;
} BootPlayer;

static int poll_input(BootPlayer *player) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) player->quit = 1;
        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat &&
            (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_RETURN ||
             event.key.key == SDLK_SPACE)) player->skipped = 1;
    }
    return !player->quit && !player->skipped;
}

/* Decoder callbacks run on the producer thread. Rendering stays on the main
 * thread, and the bounded queue keeps decoding ahead without storing a movie. */
static int movie_video(void *opaque, const uint8_t *rgb, int pitch,
                       int width, int height, double seconds) {
    BootPlayer *player = opaque;
    SDL_LockMutex(player->mutex);
    while (player->count == VIDEO_QUEUE_SIZE && !SDL_GetAtomicInt(&player->stopped))
        SDL_WaitCondition(player->space, player->mutex);
    if (SDL_GetAtomicInt(&player->stopped)) {
        SDL_UnlockMutex(player->mutex);
        return 0;
    }
    VideoFrame *frame = &player->frames[(player->head + player->count) % VIDEO_QUEUE_SIZE];
    size_t size = (size_t)pitch * height;
    frame->rgb = malloc(size);
    if (!frame->rgb) {
        snprintf(player->error, sizeof(player->error), "Cannot allocate movie frame");
        SDL_UnlockMutex(player->mutex);
        return 0;
    }
    memcpy(frame->rgb, rgb, size);
    frame->width = width;
    frame->height = height;
    frame->pitch = pitch;
    frame->seconds = seconds;
    player->count++;
    SDL_UnlockMutex(player->mutex);
    return 1;
}

static int movie_audio(void *opaque, const float *samples, int count,
                       int channels, int rate) {
    BootPlayer *player = opaque;
    if (SDL_GetAtomicInt(&player->stopped)) return 0;
    if (channels != AUDIO_CHANNELS || rate != AUDIO_RATE) {
        snprintf(player->error, sizeof(player->error), "Unexpected intro audio format");
        return 0;
    }
    if (!SDL_PutAudioStreamData(player->audio, samples,
                               count * channels * (int)sizeof(float))) {
        snprintf(player->error, sizeof(player->error), "SDL audio: %s", SDL_GetError());
        return 0;
    }
    return 1;
}

static int decode_worker(void *opaque) {
    BootPlayer *player = opaque;
    char error[256] = {0};
    PeMovieOutput output = {movie_video, movie_audio, player};
    int decoded = pe_movie_decode(player->disc, &player->file, &output,
                                 &player->stats, error, sizeof(error));
    SDL_LockMutex(player->mutex);
    if (!decoded && !player->error[0])
        snprintf(player->error, sizeof(player->error), "%s", error);
    player->decoded = decoded && !player->error[0];
    player->finished = 1;
    SDL_UnlockMutex(player->mutex);
    return 0;
}

static int render_frame(BootPlayer *player, const VideoFrame *frame) {
    if (!player->texture || player->width != frame->width || player->height != frame->height) {
        SDL_DestroyTexture(player->texture);
        player->texture = SDL_CreateTexture(player->renderer, SDL_PIXELFORMAT_RGB24,
                         SDL_TEXTUREACCESS_STREAMING, frame->width, frame->height);
        if (!player->texture) return 0;
        SDL_SetTextureScaleMode(player->texture, SDL_SCALEMODE_LINEAR);
        player->width = frame->width;
        player->height = frame->height;
    }
    if (!SDL_UpdateTexture(player->texture, NULL, frame->rgb, frame->pitch)) return 0;
    int width, height;
    if (!SDL_GetRenderOutputSize(player->renderer, &width, &height)) return 0;
    float scale = (float)width / frame->width;
    if (scale * frame->height > height) scale = (float)height / frame->height;
    SDL_FRect target = {(width - frame->width * scale) / 2,
                        (height - frame->height * scale) / 2,
                        frame->width * scale, frame->height * scale};
    SDL_SetRenderDrawColor(player->renderer, 0, 0, 0, 255);
    if (!SDL_RenderClear(player->renderer) ||
        !SDL_RenderTexture(player->renderer, player->texture, NULL, &target)) return 0;
    /* Optional rendered-frame artifact for visual regression checks. */
    if (player->capture && !player->captured && frame->seconds >= 10.0) {
        SDL_Surface *surface = SDL_RenderReadPixels(player->renderer, NULL);
        int ok = surface && SDL_SaveBMP(surface, player->capture);
        SDL_DestroySurface(surface);
        if (!ok) return 0;
        player->captured = 1;
    }
    return SDL_RenderPresent(player->renderer);
}

static int play_intro(BootPlayer *player) {
    player->worker = SDL_CreateThread(decode_worker, "intro-decode", player);
    if (!player->worker) return 0;
    /* Prebuffer half a second before starting the common playback clock. */
    for (;;) {
        SDL_LockMutex(player->mutex);
        int ready = player->count >= 8 || player->finished;
        SDL_UnlockMutex(player->mutex);
        if (ready) break;
        if (!poll_input(player)) return 1;
        SDL_Delay(1);
    }
    player->start = SDL_GetTicksNS();
    if (!SDL_ResumeAudioStreamDevice(player->audio)) return 0;
    for (;;) {
        if (!poll_input(player)) return 1;
        SDL_LockMutex(player->mutex);
        int finished = player->finished;
        int have_frame = player->count != 0;
        VideoFrame frame = {0};
        if (have_frame) {
            frame = player->frames[player->head];
            player->frames[player->head].rgb = NULL;
            player->head = (player->head + 1) % VIDEO_QUEUE_SIZE;
            player->count--;
            SDL_SignalCondition(player->space);
        }
        SDL_UnlockMutex(player->mutex);
        if (!have_frame) {
            if (finished) break;
            SDL_Delay(1);
            continue;
        }
        uint64_t due = player->start + (uint64_t)(frame.seconds * 1000000000.0);
        while (SDL_GetTicksNS() < due && poll_input(player)) SDL_Delay(1);
        int ok = player->quit || player->skipped || render_frame(player, &frame);
        free(frame.rgb);
        if (!ok) return 0;
    }
    SDL_FlushAudioStream(player->audio);
    while (SDL_GetAudioStreamQueued(player->audio) > 0 && poll_input(player))
        SDL_Delay(1);
    /* A device may have consumed the stream queue into its output buffer.
     * Keep it alive until the final samples have actually had time to play. */
    SDL_AudioSpec device_spec;
    int device_frames = 0;
    double duration = player->stats.audio_samples / (double)AUDIO_RATE;
    double video_duration = player->stats.video_frames / 15.0;
    if (duration < video_duration) duration = video_duration;
    if (SDL_GetAudioDeviceFormat(SDL_GetAudioStreamDevice(player->audio),
                                 &device_spec, &device_frames))
        duration += device_frames / (double)device_spec.freq;
    uint64_t end = player->start + (uint64_t)(duration * 1000000000.0);
    while (SDL_GetTicksNS() < end && poll_input(player)) SDL_Delay(1);
    return 1;
}

static int run_boot(const char *disc_path, int menu, int skip_intro) {
    PeDisc disc;
    BootPlayer player = {0};
    int result = 1;
    if (!pe_disc_open(&disc, disc_path)) {
        fprintf(stderr, "%s\n", disc.error);
        return 1;
    }
    player.disc = &disc;
    player.capture = getenv("PE_INTRO_CAPTURE");
    if (!pe_disc_find(&disc, "FMV1/FMV001.STR", &player.file)) {
        fprintf(stderr, "%s\n", disc.error);
        goto close_disc;
    }
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        fprintf(stderr, "SDL: %s\n", SDL_GetError());
        goto close_disc;
    }
    if (!SDL_CreateWindowAndRenderer("Parasite Eve PC", 960, 720,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
        &player.window, &player.renderer)) goto close_player;
    SDL_AudioSpec audio_spec = {SDL_AUDIO_F32, AUDIO_CHANNELS, AUDIO_RATE};
    player.audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                             &audio_spec, NULL, NULL);
    player.mutex = SDL_CreateMutex();
    player.space = SDL_CreateCondition();
    if (!player.audio || !player.mutex || !player.space) goto close_player;
    if (!skip_intro && !play_intro(&player)) {
        fprintf(stderr, "SDL playback: %s\n", SDL_GetError());
        goto close_player;
    }
    result = 0;
close_player:
    SDL_SetAtomicInt(&player.stopped, 1);
    if (player.worker) {
        SDL_LockMutex(player.mutex);
        SDL_SignalCondition(player.space);
        SDL_UnlockMutex(player.mutex);
        SDL_WaitThread(player.worker, NULL);
        if (!player.decoded) {
            fprintf(stderr, "Intro: %s\n", player.error);
            result = 1;
        } else {
            printf("intro: %s after %u frames; %llu decoded audio samples\n",
                   player.quit ? "window closed" : player.skipped ? "skipped" : "finished",
                   player.stats.video_frames, (unsigned long long)player.stats.audio_samples);
        }
    }
    if (menu && result == 0 && !player.quit) {
        /* Join the decoder before the archive reader reuses the same FILE. */
        SDL_DestroyAudioStream(player.audio);
        player.audio = NULL;
        char menu_error[256] = {0};
        int action = pe_title_menu(&disc, player.renderer, menu_error, sizeof(menu_error));
        if (action < 0) {
            fprintf(stderr, "%s\n", menu_error);
            result = 1;
        } else if (action > 0) {
            printf("menu: selected %s; gameplay integration pending\n",
                   action == 1 ? "New Game" : "Tutorial");
        }
    }
    for (unsigned i = 0; i < VIDEO_QUEUE_SIZE; ++i) free(player.frames[i].rgb);
    SDL_DestroyCondition(player.space);
    SDL_DestroyMutex(player.mutex);
    SDL_DestroyAudioStream(player.audio);
    SDL_DestroyTexture(player.texture);
    SDL_DestroyRenderer(player.renderer);
    SDL_DestroyWindow(player.window);
    SDL_Quit();
close_disc:
    pe_disc_close(&disc);
    return result;
}

int pe_boot_intro(const char *disc_path) {
    return run_boot(disc_path, 0, 0);
}

int pe_boot_game(const char *disc_path, int skip_intro) {
    return run_boot(disc_path, 1, skip_intro);
}
