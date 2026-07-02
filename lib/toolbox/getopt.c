#include "getopt.h"

#include <core/check.h>

#define CHAR_OFFSET (1)

typedef struct {
    char opt;
    const char* optval;
} ParsedOption;

static const char getopt_space_chars[] = " \n\r\t";
static const char getopt_reserved_chars[] = ":\"'\0";
static const char getopt_quote_chars[] = "\"'";

static bool is_optval_required(const char* opts) {
    return (strlen(opts) != 0) && (opts[1] == ':');
}

static size_t parse_optval(const char* args, const char** out) {
    size_t consumed_len = 0;

    const size_t start_offset = strspn(args, getopt_space_chars);
    const char* start = args + start_offset;

    const char* opening_quote = strchr(getopt_quote_chars, *start);

    if(opening_quote != NULL) {
        start += CHAR_OFFSET;

        const char* closing_quote = strchr(start, *opening_quote);
        if(closing_quote != NULL) {
            consumed_len = closing_quote - args;
        }

    } else {
        consumed_len = strcspn(start, getopt_space_chars) + start_offset;
    }

    *out = start;

    return consumed_len;
}

static size_t parse_opt(const char* args, const char* opts, ParsedOption* out) {
    size_t consumed_len = 0;

    do {
        const char opt = *args;
        if(strchr(getopt_reserved_chars, opt) != NULL) {
            break;
        }

        if(strlen(opts) == 0) {
            break;
        }

        const char* matched_opt = strchr(opts, opt);
        if(matched_opt == NULL) {
            break;
        }

        consumed_len += CHAR_OFFSET;

        const char* optval = NULL;

        if(is_optval_required(matched_opt)) {
            const size_t optval_len = parse_optval(args + consumed_len, &optval);

            if(optval_len == 0) {
                consumed_len = 0;
                break;
            }

            consumed_len += optval_len;
        }

        out->opt = opt;
        out->optval = optval;

    } while(false);

    return consumed_len;
}

static size_t parse_posarg(const char* args, ParsedOption* out) {
    const size_t consumed_len = parse_optval(args, &out->optval);

    if(consumed_len > 0) {
        out->opt = 0;
    }

    return consumed_len;
}

bool getopts(FuriString* args, const char* opts, OptionCallback callback, void* context) {
    furi_check(args);
    furi_check(opts);
    furi_check(callback);

    bool success = true;

    const size_t args_len = furi_string_size(args);

    for(size_t i = 0; i < args_len; ++i) {
        if(strchr(getopt_space_chars, furi_string_get_char(args, i)) != NULL) {
            continue;
        }

        ParsedOption opt;

        size_t consumed_len;

        if(furi_string_get_char(args, i) == '-') {
            i += CHAR_OFFSET;
            consumed_len = parse_opt(furi_string_get_cstr(args) + i, opts, &opt);
        } else {
            consumed_len = parse_posarg(furi_string_get_cstr(args) + i, &opt);
        }

        if(consumed_len == 0) {
            success = false;
            break;
        }

        i += consumed_len;
        furi_string_set_char(args, i, 0);

        callback(opt.opt, opt.optval, context);
    }

    return success;
}
