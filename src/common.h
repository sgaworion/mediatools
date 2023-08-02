#ifndef _COMMON_H_INCLUDED
#define _COMMON_H_INCLUDED

enum mediatools_result_code {
    SUCCESS,
    FILE_READ_ERROR,
    FIND_STREAM_INFO_ERROR,
    FIND_BEST_STREAM_ERROR,
    DECODING_CONTEXT_ERROR,
    DECODE_ERROR,
    IMAGE_WRITE_ERROR,
    FRAME_NOT_FOUND_ERROR,
    FORMAT_VALIDATE_ERROR,
    DURATION_VALIDATE_ERROR,
    #ifdef MEDIASTAT_MAGIC
    MIME_TYPE_ERROR,
    #endif
    _RESULT_CODE_LAST
};

const char *mediatools_strerror(enum mediatools_result_code code);

#endif
