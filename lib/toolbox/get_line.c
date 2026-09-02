#include "get_line.h"
#include <stdio.h>

typedef int (*Reader)(void* context);

static GetLineResult get_line_common(size_t max_length, Reader reader, void* context) {
    int c = 0;
    size_t count = 0;
    GetLineError error = GetLineErrorNone;
    FuriString* line = furi_string_alloc();
    while(c != '\n') {
        c = reader(context);
        if(c < 0) {
            error = GetLineErrorEOF;
            break;
        } else if(count < max_length) {
            furi_string_push_back(line, c);
        }
        count += 1;
    }
    if(count > max_length) {
        error = GetLineErrorTooLong;
    }
    if(error != GetLineErrorNone) {
        furi_string_free(line);
        line = NULL;
    }
    return (GetLineResult){
        .error = error,
        .line = line,
    };
}

static int reader_stdin(void* context) {
    UNUSED(context);
    return getchar();
}

static int reader_pipe(void* context) {
    PipeSide* pipe = context;
    char buf;
    if(pipe_receive(pipe, &buf, 1) == 1) {
        return buf;
    } else {
        return -1;
    }
}

GetLineResult get_line(size_t max_length) {
    return get_line_common(max_length, reader_stdin, NULL);
}

GetLineResult get_line_pipe(PipeSide* pipe, size_t max_length) {
    return get_line_common(max_length, reader_pipe, pipe);
}
