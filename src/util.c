#include <libavcodec/avcodec.h>
#include <libavutil/version.h>
#include "util.h"

#ifdef MEDIASTAT_MAGIC
    #include <magic.h>
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* I hate this :3 */
static const char *g_mediatools_version =
    "MP4: "
        #ifdef MEDIATOOLS_ALLOW_MP4
            "yes"
        #else
            "no"
        #endif
    ", Magic: "
        #ifdef MEDIASTAT_MAGIC
            "version " STR(MAGIC_VERSION)
        #else
            "no"
        #endif
    ", libav version: "
        LIBAVCODEC_IDENT
    ;

const char *mediatools_version() {
    return g_mediatools_version;
}

static int valid_demuxer(const AVInputFormat *fmt)
{
    // apng:      animated PNG
    // png_pipe:  static PNG
    // image2:    JPEG
    // jpeg_pipe: JPEG
    // gif:       GIF
    // svg_pipe:  SVG (recommended not to use this currently)
    // matroska:  MKV/WebM
    // mp4:       MP4

    return
      fmt == av_find_input_format("apng")      ||
      fmt == av_find_input_format("png_pipe")  ||
      fmt == av_find_input_format("image2")    ||
      fmt == av_find_input_format("jpeg_pipe") ||
      fmt == av_find_input_format("gif")       ||
      fmt == av_find_input_format("svg_pipe")  ||
      fmt == av_find_input_format("matroska")  
      #ifdef MEDIATOOLS_ALLOW_MP4
        || fmt == av_find_input_format("mp4")
      #endif
      ;
}

static const AVInputFormat *image2_demuxer()
{
    return av_find_input_format("image2");
}

int open_input_correct_demuxer(AVFormatContext **ctx, const char *filename)
{
    if (avformat_open_input(ctx, filename, NULL, NULL) < 0) {
        return -1;
    }

    // Should usually happen
    if (valid_demuxer((*ctx)->iformat)) {
        return 0;
    }

    fprintf(stderr, "warning: file has invalid format, falling back to image2\n");

    // Wrong demuxer, force to image2 so we don't error out here
    avformat_close_input(ctx);
    return avformat_open_input(ctx, filename, image2_demuxer(), NULL);
}

