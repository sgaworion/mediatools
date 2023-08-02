#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "thumb.h"
#include "common.h"

int main(int argc, char *argv[]) {
    enum mediatools_result_code resultCode;

    if (argc == 2 && !strcmp(argv[1], "-v")) {
        fprintf(stderr, mediatools_version());
        return 0;
    }

    if (argc != 4) {
        fprintf(stderr, "Usage: <input> <time> <output>\n");
        return 2;
    }

    const char *input = argv[1];
    const char *output = argv[3];
    double time = atof(argv[2]);

    if ((resultCode = mediathumb_generate_thumb(input, time, output)) != SUCCESS) {
        fprintf(stderr, "Error: mediathumb_generate_thumb(): %s\n", mediatools_strerror(resultCode));
        return 1;
    }

    return 0;
}