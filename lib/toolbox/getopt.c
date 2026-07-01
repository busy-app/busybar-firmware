#include "getopt.h"

#include <core/check.h>

typedef struct {
    char opt;
    const char* optval;
} ParsedOption;

static size_t parse_optval(const char* args, size_t args_len, const char** out) {
    size_t len = 0;

    for(; len < args_len; ++len) {
        if(!isspace((int)args[len])) {
            break;
        }
    }

    *out = &args[len];

    for(; len < args_len; ++len) {
        if(isspace((int)args[len])) {
            break;
        }
    }

    return len;
}

static size_t parse_opt(const char* args, size_t args_len, const char* opts, ParsedOption* out) {
    size_t len = 0;

    do {
        if(args_len == 0) {
            break;
        }

        const size_t opts_len = strlen(opts);
        if(opts_len == 0) {
            break;
        }

        const char* opt_ptr = strchr(opts, args[0]);
        if(opt_ptr == NULL) {
            // Option not in list
            break;
        }

        const char opt = *opt_ptr;
        if(opt == ':') {
            // Malformed option: -: is not allowed
            break;
        }

        const char* optval = NULL;

        opt_ptr++;
        len++;

        const size_t l = opt_ptr - opts;

        if((l < opts_len) && (*opt_ptr == ':')) {
            const size_t optval_len = parse_optval(args + len, args_len - len, &optval);

            if(optval_len == 0) {
                // Required value is missing
                len = 0;
                break;
            }

            len += optval_len;
        }

        out->opt = opt;
        out->optval = optval;

    } while(false);

    return len;
}

static size_t parse_posarg(const char* args, size_t args_len, ParsedOption* out) {
    const size_t len = parse_optval(args, args_len, &out->optval);

    if(len > 0) {
        out->opt = 0;
    }

    return len;
}

bool getopts(FuriString* args, const char* opts, OptionCallback callback, void* context) {
    furi_check(args);
    furi_check(opts);
    furi_check(callback);

    bool success = true;

    const size_t args_len = furi_string_size(args);

    for(size_t i = 0; i < args_len; ++i) {
        if(isspace((int)furi_string_get_char(args, i))) {
            continue;
        }

        ParsedOption opt;

        size_t consumed_len;

        if(furi_string_get_char(args, i) == '-') {
            ++i;
            consumed_len = parse_opt(furi_string_get_cstr(args) + i, args_len - i, opts, &opt);
        } else {
            consumed_len = parse_posarg(furi_string_get_cstr(args) + i, args_len - i, &opt);
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
