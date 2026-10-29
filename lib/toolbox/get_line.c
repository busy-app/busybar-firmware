#include "get_line.h"
#include <stdio.h>

#define ECHO 1

#define CHAR_ETX 3
#define CHAR_EOT 4
#define CHAR_BS  8
#define CHAR_CR  13
#define CHAR_DEL 127

typedef int (*ReadCallback)(void* context);
typedef void (*EchoCallback)(const char* data, size_t data_len, void* context);

static GetLineResult
    get_line_common(size_t max_length, ReadCallback read_cb, EchoCallback echo_cb, void* context) {
    int c = 0;
    size_t count = 0;
    GetLineError error = GetLineErrorNone;
    FuriString* line = furi_string_alloc();
    while(c != CHAR_CR) {
        c = read_cb(context);
        if(c < 0 || c == CHAR_EOT) {
            error = GetLineErrorEOF;
            break;
        } else if(c == CHAR_ETX) {
            error = GetLineErrorInterrupt;
            break;
        } else if(c == CHAR_BS || c == CHAR_DEL) {
            if(count == 0) {
                continue;
            }

            count -= 1;
            if(count < max_length) {
                furi_string_left(line, count);
            }
#if ECHO
            // Backspace, print " ", backspace
            echo_cb("\b \b", 3, context);
#else
            UNUSED(echo);
#endif // ECHO
        } else {
            if(count < max_length) {
                furi_string_push_back(line, c);
            }
#if ECHO
            const char ch = c;
            echo_cb(&ch, 1, context);
#else
            UNUSED(echo);
#endif // ECHO
            count += 1;
        }
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

static int read_stdin(void* context) {
    UNUSED(context);
    int c = getchar();
    if(c == 255) {
        // TODO BUG getchar returns 255 on EOF
        return -1;
    }
    return c;
}

static int read_pipe(void* context) {
    PipeSide* pipe = context;
    char buf;
    if(pipe_receive(pipe, &buf, 1) == 1) {
        if(buf == 255) {
            // TODO BUG pipe_receive receives 255 on EOF
            return -1;
        } else {
            return buf;
        }
    } else {
        return -1;
    }
}

static void echo_stdin(const char* data, size_t data_len, void* context) {
    UNUSED(context);
    for(size_t i = 0; i < data_len; ++i) {
        putchar(data[i]);
    }
    fflush(stdout);
}

static void echo_pipe(const char* data, size_t data_len, void* context) {
    PipeSide* pipe = context;
    pipe_send(pipe, data, data_len);
}

GetLineResult get_line(size_t max_length) {
    return get_line_common(max_length, read_stdin, echo_stdin, NULL);
}

GetLineResult get_line_pipe(PipeSide* pipe, size_t max_length) {
    return get_line_common(max_length, read_pipe, echo_pipe, pipe);
}
