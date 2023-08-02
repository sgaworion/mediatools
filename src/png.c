#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include "png.h"

enum AVPixelFormat pix_fmt(AVFrame *in_frame)
{
    // [swscaler @ 0x219f6c0]
    // deprecated pixel format used, make sure you did set range correctly
    switch (in_frame->format) {
    case AV_PIX_FMT_YUVJ420P:
        return AV_PIX_FMT_YUV420P;
    case AV_PIX_FMT_YUVJ422P:
        return AV_PIX_FMT_YUV422P;
    case AV_PIX_FMT_YUVJ444P:
        return AV_PIX_FMT_YUV444P;
    case AV_PIX_FMT_YUVJ440P:
        return AV_PIX_FMT_YUV440P;
    default:
        return in_frame->format;
    }
}

int mediatools_write_frame_to_png(AVFrame *in_frame, const char *path)
{
    struct SwsContext *sws_ctx = NULL;
    AVFormatContext *format = NULL;
    const AVOutputFormat *png = NULL;
    AVCodecContext *vctx = NULL;
    AVStream *vstream = NULL;
    AVFrame *out_frame = NULL;
    const AVCodec *vcodec = NULL;
    AVPacket *pkt = NULL;

    int ret = -1;

    png = av_guess_format(NULL, NULL, "image/png");
    vcodec = avcodec_find_encoder(AV_CODEC_ID_APNG);
    vctx = avcodec_alloc_context3(vcodec);
    format = avformat_alloc_context();
    out_frame = av_frame_alloc();

    if (!format || !vctx || !vcodec || !png || !out_frame)
        goto error;

    format->oformat = png;
    format->url = av_strdup(path);

    vstream = avformat_new_stream(format, NULL);
    if (!vstream)
        goto error;

    if (png->flags & AVFMT_GLOBALHEADER)
        vctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    vctx->time_base = (AVRational) { 1, 1 };
    vctx->width = in_frame->width;
    vctx->height = in_frame->height;
    vctx->pix_fmt = AV_PIX_FMT_RGBA;

    if (avcodec_parameters_from_context(vstream->codecpar, vctx) < 0)
        goto error;

    if (avcodec_open2(vctx, vcodec, NULL) < 0)
        goto error;

    avio_open(&format->pb, path, AVIO_FLAG_WRITE);

    if (avformat_write_header(format, NULL) < 0)
        goto error;

    pkt = av_packet_alloc();
    if (!pkt)
        goto error;

    out_frame->format = AV_PIX_FMT_RGBA;
    out_frame->width = in_frame->width;
    out_frame->height = in_frame->height;

    if (av_frame_get_buffer(out_frame, 0) < 0)
        goto error;

    if (av_frame_make_writable(out_frame) < 0)
        goto error;

    // Rewrite frame into correct color format

    sws_ctx = sws_getContext(in_frame->width, in_frame->height, pix_fmt(in_frame), out_frame->width, out_frame->height, AV_PIX_FMT_RGBA, SWS_LANCZOS, NULL, NULL, NULL);
    if (!sws_ctx)
        goto error;

    sws_scale(sws_ctx, (const uint8_t **)in_frame->data, in_frame->linesize, 0, in_frame->height, out_frame->data, out_frame->linesize);
    if (avcodec_send_frame(vctx, out_frame) < 0)
        goto error;
    if (avcodec_send_frame(vctx, NULL) < 0)
        goto error;
    if (avcodec_receive_packet(vctx, pkt) < 0)
        goto error;

    pkt->stream_index = vstream->index;
    pkt->pts = 1;
    pkt->dts = 1;

    if (av_write_frame(format, pkt) < 0)
        goto error;

    av_packet_unref(pkt);

    if (av_write_trailer(format) < 0)
        goto error;

    ret = 0;

error:
    if (sws_ctx)
        sws_freeContext(sws_ctx);
    if (pkt)
        av_packet_free(&pkt);
    if (format->pb)
        avio_close(format->pb);
    if (vctx)
        avcodec_free_context(&vctx);
    if (out_frame)
        av_frame_free(&out_frame);
    if (format)
        avformat_free_context(format);

    return ret;
}
