#include "movie.h"

#include <errno.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <string.h>

/* FFmpeg's STR demuxer consumes complete CD sectors. ISO file lengths count
 * 2048-byte logical blocks, even though XA movie sectors have 2324 data bytes. */
typedef struct MovieInput {
    PeDisc *disc;
    const PeDiscFile *file;
    uint64_t offset;
    uint64_t size;
} MovieInput;

static int read_packet(void *opaque, uint8_t *buffer, int size) {
    MovieInput *input = opaque;
    uint8_t sector[2352];
    if (input->offset >= input->size) return AVERROR_EOF;
    int total = 0;
    while (total < size && input->offset < input->size) {
        uint32_t index = (uint32_t)(input->offset / 2352);
        unsigned start = (unsigned)(input->offset % 2352);
        unsigned count = 2352 - start;
        if (count > (unsigned)(size - total)) count = (unsigned)(size - total);
        if (!pe_disc_read_raw(input->disc, input->file, index, sector))
            return AVERROR(EIO);
        memcpy(buffer + total, sector + start, count);
        input->offset += count;
        total += (int)count;
    }
    return total;
}

static int64_t seek_packet(void *opaque, int64_t offset, int whence) {
    MovieInput *input = opaque;
    if (whence == AVSEEK_SIZE) return (int64_t)input->size;
    whence &= ~AVSEEK_FORCE;
    int64_t origin = whence == SEEK_SET ? 0 : whence == SEEK_CUR ?
                     (int64_t)input->offset : whence == SEEK_END ?
                     (int64_t)input->size : -1;
    if (origin < 0 || offset < -origin || offset > (int64_t)input->size - origin)
        return AVERROR(EINVAL);
    input->offset = (uint64_t)(origin + offset);
    return (int64_t)input->offset;
}

static AVCodecContext *open_decoder(AVStream *stream) {
    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) return NULL;
    AVCodecContext *context = avcodec_alloc_context3(codec);
    if (!context) return NULL;
    if (avcodec_parameters_to_context(context, stream->codecpar) < 0 ||
        avcodec_open2(context, codec, NULL) < 0) avcodec_free_context(&context);
    return context;
}

typedef struct MovieDecode {
    AVCodecContext *video;
    AVCodecContext *audio;
    struct SwsContext *scaler;
    SwrContext *resampler;
    AVFrame *frame;
    uint8_t *rgb;
    float *samples;
    size_t rgb_capacity;
    size_t sample_capacity;
    const PeMovieOutput *output;
    PeMovieStats *stats;
    int stopped;
} MovieDecode;

static int deliver_frames(MovieDecode *decode, AVCodecContext *codec) {
    int result;
    while ((result = avcodec_receive_frame(codec, decode->frame)) >= 0) {
        AVFrame *frame = decode->frame;
        if (codec == decode->video) {
            size_t size = (size_t)frame->width * (size_t)frame->height * 3;
            if (size > decode->rgb_capacity) {
                av_freep(&decode->rgb);
                decode->rgb = av_malloc(size);
                if (!decode->rgb) return AVERROR(ENOMEM);
                decode->rgb_capacity = size;
            }
            enum AVPixelFormat pixel_format = (enum AVPixelFormat)frame->format;
            int full_range = frame->color_range == AVCOL_RANGE_JPEG;
            if (pixel_format == AV_PIX_FMT_YUVJ420P) {
                pixel_format = AV_PIX_FMT_YUV420P;
                full_range = 1;
            }
            decode->scaler = sws_getCachedContext(decode->scaler,
                frame->width, frame->height, pixel_format,
                frame->width, frame->height, AV_PIX_FMT_RGB24, SWS_BILINEAR,
                NULL, NULL, NULL);
            if (!decode->scaler) return AVERROR(ENOMEM);
            const int *coefficients = sws_getCoefficients(SWS_CS_ITU601);
            sws_setColorspaceDetails(decode->scaler, coefficients, full_range,
                                    coefficients, 1, 0, 1 << 16, 1 << 16);
            uint8_t *pixels[] = {decode->rgb, NULL, NULL, NULL};
            int pitch[] = {frame->width * 3, 0, 0, 0};
            if (sws_scale(decode->scaler, (const uint8_t *const *)frame->data,
                          frame->linesize, 0, frame->height, pixels, pitch) < 0)
                return AVERROR_INVALIDDATA;
            decode->stats->width = frame->width;
            decode->stats->height = frame->height;
            double seconds = decode->stats->video_frames / 15.0;
            decode->stats->video_frames++;
            if (decode->output && decode->output->video &&
                !decode->output->video(decode->output->user, decode->rgb,
                    pitch[0], frame->width, frame->height, seconds))
                decode->stopped = 1;
        } else {
            int channels = frame->ch_layout.nb_channels;
            int rate = frame->sample_rate;
            if (!decode->resampler) {
                result = swr_alloc_set_opts2(&decode->resampler, &frame->ch_layout,
                    AV_SAMPLE_FMT_FLT, rate, &frame->ch_layout,
                    (enum AVSampleFormat)frame->format, rate, 0, NULL);
                if (result < 0 || (result = swr_init(decode->resampler)) < 0)
                    return result;
            }
            size_t count = (size_t)frame->nb_samples * (size_t)channels;
            if (count > decode->sample_capacity) {
                av_freep(&decode->samples);
                decode->samples = av_malloc_array(count, sizeof(float));
                if (!decode->samples) return AVERROR(ENOMEM);
                decode->sample_capacity = count;
            }
            uint8_t *samples[] = {(uint8_t *)decode->samples};
            result = swr_convert(decode->resampler, samples, frame->nb_samples,
                                (const uint8_t **)frame->extended_data,
                                frame->nb_samples);
            if (result < 0) return result;
            decode->stats->audio_samples += (unsigned)result;
            decode->stats->sample_rate = rate;
            decode->stats->channels = channels;
            if (decode->output && decode->output->audio &&
                !decode->output->audio(decode->output->user, decode->samples,
                                       result, channels, rate)) decode->stopped = 1;
        }
        av_frame_unref(frame);
        if (decode->stopped) return 0;
    }
    return result == AVERROR(EAGAIN) || result == AVERROR_EOF ? 0 : result;
}

int pe_movie_decode(PeDisc *disc, const PeDiscFile *file,
                    const PeMovieOutput *output, PeMovieStats *stats,
                    char *error, size_t error_size) {
    memset(stats, 0, sizeof(*stats));
    if (disc->sector_size != 2352) {
        snprintf(error, error_size, "STR playback requires a raw MODE2/2352 BIN");
        return 0;
    }
    MovieInput input = {disc, file, 0, ((uint64_t)file->size + 2047) / 2048 * 2352};
    MovieDecode decode = {0};
    decode.output = output;
    decode.stats = stats;
    AVFormatContext *format = avformat_alloc_context();
    uint8_t *buffer = av_malloc(2352 * 16);
    AVIOContext *io = NULL;
    AVPacket *packet = av_packet_alloc();
    decode.frame = av_frame_alloc();
    int result = AVERROR(ENOMEM);
    int video_index = -1, audio_index = -1;
    if (!format || !buffer || !packet || !decode.frame) goto cleanup;
    io = avio_alloc_context(buffer, 2352 * 16, 0, &input, read_packet, NULL, seek_packet);
    if (!io) goto cleanup;
    buffer = NULL;
    format->pb = io;
    format->flags |= AVFMT_FLAG_CUSTOM_IO;
    result = avformat_open_input(&format, NULL, av_find_input_format("psxstr"), NULL);
    if (result < 0) goto cleanup;
    result = avformat_find_stream_info(format, NULL);
    if (result < 0) goto cleanup;
    video_index = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    audio_index = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (video_index < 0) { result = video_index; goto cleanup; }
    decode.video = open_decoder(format->streams[video_index]);
    if (!decode.video) { result = AVERROR_DECODER_NOT_FOUND; goto cleanup; }
    if (audio_index >= 0) {
        decode.audio = open_decoder(format->streams[audio_index]);
        if (!decode.audio) { result = AVERROR_DECODER_NOT_FOUND; goto cleanup; }
    }
    while ((result = av_read_frame(format, packet)) >= 0) {
        AVCodecContext *codec = packet->stream_index == video_index ? decode.video :
                               packet->stream_index == audio_index ? decode.audio : NULL;
        if (codec) {
            result = avcodec_send_packet(codec, packet);
            if (result >= 0) result = deliver_frames(&decode, codec);
        }
        av_packet_unref(packet);
        if (result < 0 || decode.stopped) break;
    }
    if (result == AVERROR_EOF) {
        result = avcodec_send_packet(decode.video, NULL);
        if (result >= 0) result = deliver_frames(&decode, decode.video);
        if (result >= 0 && decode.audio) {
            result = avcodec_send_packet(decode.audio, NULL);
            if (result >= 0) result = deliver_frames(&decode, decode.audio);
        }
    }
    if (result >= 0 && !stats->video_frames) result = AVERROR_INVALIDDATA;
cleanup:
    if (result < 0) av_strerror(result, error, error_size);
    else if (error_size) error[0] = 0;
    av_free(buffer);
    av_packet_free(&packet);
    av_frame_free(&decode.frame);
    avcodec_free_context(&decode.video);
    avcodec_free_context(&decode.audio);
    sws_freeContext(decode.scaler);
    swr_free(&decode.resampler);
    av_free(decode.rgb);
    av_free(decode.samples);
    avformat_close_input(&format);
    if (io) { av_freep(&io->buffer); avio_context_free(&io); }
    return result >= 0;
}
