#include <libavformat/avformat.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include "stat.h"
#include "validation.h"
#include "util.h"

static int64_t start_time(AVStream *stream) {
    if (stream->start_time == AV_NOPTS_VALUE) {
        return 0;
    }

    return stream->start_time;
}

#ifdef MEDIASTAT_MAGIC
static bool mediastat_magic(const char *path, mediastat_result_t *result) {
    magic_t magic = magic_open(MAGIC_SYMLINK | MAGIC_MIME_TYPE | MAGIC_ERROR);

    if (magic == NULL) {
        fprintf(stderr, "magic_open(): unknown error\n");
        return false;
    }

    if (magic_load(magic, NULL) != 0) {
        fprintf(stderr, "magic_load(): %s\n", magic_error(magic));
    }

    const char *mime = magic_file(magic, path);

    if (mime == NULL) {
        fprintf(stderr, "magic_file(): %s\n", magic_error(magic));
        magic_close(magic);
        return false;
    }

    strncpy(result->mime, mime, 32);

    magic_close(magic);

    return true;
}
#endif

enum mediatools_result_code mediastat_stat(const char *path, mediastat_result_t *result) {
    struct stat statbuf;
    AVFormatContext *format = NULL;
    AVPacket pkt;

    av_log_set_level(AV_LOG_QUIET);

    if (stat(path, &statbuf) != 0) {
        return FILE_READ_ERROR;
    }

#ifdef MEDIASTAT_MAGIC
    if (!mediastat_magic(path, result)) {
        return MIME_TYPE_ERROR;
    }
#endif

    if (open_input_correct_demuxer(&format, path) != 0) {
    	return FILE_READ_ERROR;
    }

    if (avformat_find_stream_info(format, NULL) < 0) {
        return FILE_READ_ERROR;
    }

    if (!mediatools_validate_video(format)) {
        // Error is printed by validation function
        return -1;
    }

    int vstream_idx = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    if (vstream_idx < 0) {
        return FILE_READ_ERROR;
    }

    uint64_t frames = 0;
    int64_t last_pts = 0;
    int last_stream = 0;

    while (av_read_frame(format, &pkt) >= 0) {
        int64_t new_pts = pkt.pts + pkt.duration;

        if (last_pts < new_pts) {
            last_pts = new_pts;
            last_stream = pkt.stream_index;
        }

        if (pkt.stream_index == vstream_idx)
            ++frames;

        av_packet_unref(&pkt);
    }

    AVStream *stream = format->streams[last_stream];
    AVRational dur   = av_mul_q(av_make_q(last_pts - start_time(stream), 1), stream->time_base);

    if (!mediatools_validate_duration(dur)) {
        return DURATION_VALIDATE_ERROR;
    }

    AVCodecParameters *vpar = format->streams[vstream_idx]->codecpar;

    result->size = statbuf.st_size;
    result->frames = frames;
    result->width = vpar->width;
    result->height = vpar->height;
    result->dur_num = dur.num;
    result->dur_den = dur.den;

    avformat_close_input(&format);

    return SUCCESS;
}
