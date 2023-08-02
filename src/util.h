#ifndef _UTIL_H_DEFINED
#define _UTIL_H_DEFINED

#include <libavformat/avformat.h>
int open_input_correct_demuxer(AVFormatContext **ctx, const char *filename);

#endif
