#include <stdio.h>

#include "util.h"
#include "stat.h"
#include "common.h"

int main(int argc, char *argv[]) {
    mediastat_result_t result;
    enum mediatools_result_code resultCode;

    if (argc == 2 && !strcmp(argv[1], "-v")) {
        fprintf(stderr, mediatools_version());
        return 0;
    }

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file path>\n", argv[0]);
        return 2;
    }

    if ((resultCode = mediastat_stat(argv[1], &result)) != SUCCESS) {
        fprintf(stderr, "Error: mediastat_stat(): %s\n", mediatools_strerror(resultCode));
        return 1;
    }

    #ifdef MEDIASTAT_MAGIC
        printf("%ld %lu %d %d %d %d %s\n", result.size, result.frames, result.width, result.height, result.dur_num, result.dur_den, result.mime);
    #else
        printf("%ld %lu %d %d %d %d\n", result.size, result.frames, result.width, result.height, result.dur_num, result.dur_den);
    #endif


    return 0;
}
