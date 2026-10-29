#include "get_line.h"

#include <stdio.h>

#include <cli/cli_ansi.h>

static int read_stdin(void) {
    const int c = getchar();
    if(c == 255) {
        // TODO BUG getchar returns 255 on EOF
        return -1;
    }
    return c;
}

GetLineResult get_line(size_t max_length) {
    size_t count = 0;

    GetLineError error = GetLineErrorNone;
    FuriString* line = furi_string_alloc();

    for(int c = 0; c != CliKeyCR;) {
        c = read_stdin();
        if(c < 0 || c == CliKeyEOT) {
            error = GetLineErrorEOF;
            break;
        } else if(c == CliKeyETX) {
            error = GetLineErrorInterrupt;
            break;
        } else if(c == CliKeyBackspace || c == CliKeyDEL) {
            if(count == 0) {
                continue;
            }

            count -= 1;
            if(count < max_length) {
                furi_string_left(line, count);
            }
            // Backspace, " ", backspace
            printf("\b \b");

        } else {
            if(count < max_length) {
                furi_string_push_back(line, c);
            }
            count += 1;

            putchar(c);
        }

        fflush(stdout);
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
