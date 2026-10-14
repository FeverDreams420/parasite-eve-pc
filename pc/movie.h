#ifndef PE_PC_MOVIE_H
#define PE_PC_MOVIE_H

#include "pe1/cdrom.h"

typedef struct PeMovieStats {
    unsigned video_frames;
    uint64_t audio_samples;
    int width;
    int height;
    int sample_rate;
    int channels;
} PeMovieStats;

/* Callbacks receive decoded data valid until their return. Return zero to
 * cancel playback; cancellation is successful but does not imply EOF. */
typedef struct PeMovieOutput {
    int (*video)(void *user, const uint8_t *rgb, int pitch, int width,
                 int height, double seconds);
    int (*audio)(void *user, const float *samples, int count, int channels,
                 int rate);
    void *user;
} PeMovieOutput;

int pe_movie_decode(PeDisc *disc, const PeDiscFile *file,
                    const PeMovieOutput *output, PeMovieStats *stats,
                    char *error, size_t error_size);

#endif
