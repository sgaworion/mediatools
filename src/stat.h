#ifndef _STAT_H_INCLUDED
#define _STAT_H_INCLUDED

#include <stdint.h>

#ifdef MEDIASTAT_MAGIC
    #include <magic.h>
    #include <stdbool.h>
#endif

#include "common.h"

typedef struct mediastat_result {
    int64_t size;     /* Size of file in bytes */
    uint64_t frames;  /* Number of frames */
    uint32_t width;   /* Width of media in pixels */
    uint32_t height;  /* Height of media in pixels */
    uint32_t dur_num; /* Duration numerator */
    uint32_t dur_den; /* Duration denominator */
} mediastat_result_t;

enum mediatools_result_code mediastat_stat(const char *path, mediastat_result_t *result);

#endif