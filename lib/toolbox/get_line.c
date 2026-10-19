#include "get_line.h"
#include <stdio.h>

#define ECHO 1

#define CHAR_ETX 3
#define CHAR_EOT 4
#define CHAR_CR  13
typedef int (*Reader)(void* context);

static GetLineResult get_line_common(size_t max_length, Reader reader, void* context) {
    int c = 0;
    size_t count = 0;
    GetLineError error = GetLineErrorNone;
    FuriString* line = furi_string_alloc();
    while(c != CHAR_CR) {
        c = reader(context);
        if(c < 0 || c == CHAR_EOT) {
            error = GetLineErrorEOF;
            break;
        } else if(c == CHAR_ETX) {
            error = GetLineErrorInterrupt;
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
    } else {
        furi_string_trim(line, "\r\n");
    }
    return (GetLineResult){
        .error = error,
        .line = line,
    };
}

static int reader_stdin(void* context) {
    UNUSED(context);
    int c = getchar();
    if(c == 255) {
        // TODO BUG getchar returns 255 on EOF
        return -1;
    }
#if ECHO
    putchar(c);
    fflush(stdout);
#endif
    return c;
}

static int reader_pipe(void* context) {
    PipeSide* pipe = context;
    char buf;
    if(pipe_receive(pipe, &buf, 1) == 1) {
        if(buf == 255) {
            // TODO BUG pipe_receive receives 255 on EOF
            return -1;
        } else {
#if ECHO
            pipe_send(pipe, &buf, 1);
#endif
            return buf;
        }
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
