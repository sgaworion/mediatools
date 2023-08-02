#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
#include <stdbool.h>
#include <string.h>

#include "validation.h"

const AVRational t_1h = { 3600, 1 };
const AVRational t_0h  = { 0, 1 };

static int validate_image_pixel_format(enum AVPixelFormat format)
{
    switch (format) {
    // Still image formats
    case AV_PIX_FMT_GBRAP:
    case AV_PIX_FMT_MONOBLACK:
    case AV_PIX_FMT_MONOWHITE:
    case AV_PIX_FMT_YA16LE:
    case AV_PIX_FMT_YA16BE:
    case AV_PIX_FMT_GRAY8:
    case AV_PIX_FMT_GRAY8A:
    case AV_PIX_FMT_GRAY10LE:
    case AV_PIX_FMT_GRAY10BE:
    case AV_PIX_FMT_GRAY12LE:
    case AV_PIX_FMT_GRAY12BE:
    case AV_PIX_FMT_GRAY16LE:
    case AV_PIX_FMT_GRAY16BE:
    case AV_PIX_FMT_PAL8:
    case AV_PIX_FMT_RGB8:
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_RGB32:
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_RGB48LE:
    case AV_PIX_FMT_RGB48BE:
    case AV_PIX_FMT_RGBA64LE:
    case AV_PIX_FMT_RGBA64BE:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUVJ422P:
    case AV_PIX_FMT_YUVJ444P:
    case AV_PIX_FMT_YUVJ440P:
    case AV_PIX_FMT_YUVA420P:
    case AV_PIX_FMT_YUVA422P:
    case AV_PIX_FMT_YUVA444P:
        return true;

    default:
        return false;
    }
}

static int validate_video_pixel_format(enum AVPixelFormat format)
{
    switch (format) {
    // Video frame formats
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUV420P10LE:
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUV422P10LE:
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUV440P:
    case AV_PIX_FMT_YUV444P10LE:
    case AV_PIX_FMT_YUV420P12LE:
    case AV_PIX_FMT_YUV422P12LE:
    case AV_PIX_FMT_YUV444P12LE:
        return true;

    default:
        return false;
    }
}

static int validate_audio_sample_format(enum AVSampleFormat format)
{
    switch (format) {
    case AV_SAMPLE_FMT_FLT:
    case AV_SAMPLE_FMT_FLTP:
    case AV_SAMPLE_FMT_S16:
    case AV_SAMPLE_FMT_S16P:
    case AV_SAMPLE_FMT_S32:
    case AV_SAMPLE_FMT_S32P:
        return true;

    default:
        return false;
    }
}

int mediatools_validate_video(AVFormatContext *format)
{
    uint64_t num_vstreams = 0;
    uint64_t num_astreams = 0;
    int64_t vstream_idx = -1;
    int64_t astream_idx = -1;

    for (size_t i = 0; i < format->nb_streams; ++i) {
        AVCodecParameters *codecpar = format->streams[i]->codecpar;

        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            num_vstreams++;
            vstream_idx = i;
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            num_astreams++;
            astream_idx = i;
        } else if (codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            // Allow subtitles
        } else {
            fprintf(stderr, "Unknown codec type %s\n", av_get_media_type_string(codecpar->codec_type));
            return false;
        }
    }

    if (num_vstreams != 1) {
        fprintf(stderr, "Found %lu video streams (must be 1)\n", num_vstreams);
        return false;
    }

    if (num_astreams > 1) {
        fprintf(stderr, "Found %lu audio streams (must be 0 or 1)\n", num_astreams);
        return false;
    }

    const AVInputFormat *iformat = format->iformat;
    const AVCodecParameters *vpar = format->streams[vstream_idx]->codecpar;
    const AVCodecParameters *apar = NULL;

    if (astream_idx != -1) {
        apar = format->streams[astream_idx]->codecpar;
    }

    #ifdef MEDIATOOLS_ALLOW_MP4

    if (strstr(iformat->name, "mp4")) {
        switch (vpar->codec_id) {
            default:
                fprintf(stderr, "Bad format for MP4 container (must be H.264 or H.265)\n");
                return false;
            case AV_CODEC_ID_H264:
            case AV_CODEC_ID_H265:
                ;
        }

    }
    else /* This becomes part of the following if statement */

    #endif

    if (strstr(iformat->name, "matroska")) {
        switch (vpar->codec_id) {
        default:
            fprintf(stderr, "Bad video codec for WebM container (must be VP8 or VP9)\n");
            return false;
        case AV_CODEC_ID_VP8:
        case AV_CODEC_ID_VP9:
            ;
        }

        if (!validate_video_pixel_format(vpar->format)) {
            fprintf(stderr, "Found unsupported pixel format %s\n", av_get_pix_fmt_name(vpar->format));
            return false;
        }

        if (apar) {
            switch (apar->codec_id) {
            default:
                fprintf(stderr, "Bad audio codec for WebM container (must be Opus or Vorbis)\n");
                return false;
            case AV_CODEC_ID_VORBIS:
            case AV_CODEC_ID_OPUS:
                ;
            }

            if (!validate_audio_sample_format(apar->format)) {
                fprintf(stderr, "Found unsupported audio sample format %s\n", av_get_sample_fmt_name(apar->format));
                return false;
            }
        }
    } else if (strcmp(iformat->name, "gif") == 0) {
        switch (vpar->codec_id) {
        default:
            fprintf(stderr, "Bad video codec for GIF container (must be GIF)\n");
            return false;
        case AV_CODEC_ID_GIF:
            ;
        }

        if (!validate_image_pixel_format(vpar->format)) {
            fprintf(stderr, "Found unsupported pixel format %s\n", av_get_pix_fmt_name(vpar->format));
            return false;
        }
    } else if (strcmp(iformat->name, "image2") == 0 || strcmp(iformat->name, "jpeg_pipe") == 0) {
        switch (vpar->codec_id) {
        default:
            fprintf(stderr, "Bad video codec for JPEG container (must be JPEG)\n");
            return false;
        case AV_CODEC_ID_MJPEG:
            ;
        }

        if (!validate_image_pixel_format(vpar->format)) {
            fprintf(stderr, "Found unsupported pixel format %s\n", av_get_pix_fmt_name(vpar->format));
            return false;
        }
    } else if (strcmp(iformat->name, "png_pipe") == 0 || strcmp(iformat->name, "apng") == 0) {
        switch (vpar->codec_id) {
        default:
            fprintf(stderr, "Bad video codec for PNG container (must be PNG)\n");
            return false;
        case AV_CODEC_ID_PNG:
        case AV_CODEC_ID_APNG:
            ;
        }

        if (!validate_image_pixel_format(vpar->format)) {
            fprintf(stderr, "Found unsupported pixel format %s\n", av_get_pix_fmt_name(vpar->format));
            return false;
        }
    } else if (strcmp(iformat->name, "svg_pipe") == 0) {
        switch (vpar->codec_id) {
        default:
            fprintf(stderr, "Bad video codec for SVG container (must be SVG)\n");
            return false;
        case AV_CODEC_ID_SVG:
            ;
        }
    } else {
        fprintf(stderr, "Unknown input format\n");
        return false;
    }

    if (vpar->width < 1 || vpar->width > 32767) {
        fprintf(stderr, "Invalid width %d\n", vpar->width);
        return false;
    }

    if (vpar->height < 1 || vpar->height > 32767) {
        fprintf(stderr, "Invalid height %d\n", vpar->height);
        return false;
    }

    return true;
}

int mediatools_validate_duration(AVRational dur)
{
    if (av_cmp_q(dur, t_0h) < 0 || av_cmp_q(dur, t_1h) > 0) {
        fprintf(stderr, "Invalid duration (must be 0..1 hour)\n");
        return false;
    }

    return true;
}
