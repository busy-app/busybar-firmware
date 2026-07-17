#include "argparse.h"

#include <core/check.h>

#define OPTION_DELIMITER '-'
#define CHAR_OFFSET      (1)

typedef struct {
    char opt;
    const char* optarg;
} ParsedOption;

static const char quote_chars[] = "\"'";
static const char space_chars[] = " \n\r\t";
static const char reserved_chars[] = ":\"'\0";

static bool is_option_argument_required(const char* opts) {
    return (strlen(opts) != 0) && (opts[1] == ':');
}

static size_t parse_option_argument(const char* args, const char** out) {
    size_t consumed_len = 0;

    do {
        const char first_char = *args;
        if(first_char == '\0') {
            break;
        }

        const char* args_cursor = args;
        const char* opening_quote = strchr(quote_chars, first_char);

        if(opening_quote != NULL) {
            args_cursor += CHAR_OFFSET;

            const char* closing_quote = strchr(args_cursor, *opening_quote);
            if(closing_quote == NULL) {
                break;
            }

            consumed_len = closing_quote - args;

        } else {
            consumed_len = strcspn(args_cursor, space_chars);
        }

        *out = args_cursor;

    } while(false);

    return consumed_len;
}

static size_t parse_option(const char* args, const char* opts, ParsedOption* out) {
    size_t consumed_len = 0;

    do {
        if(opts == NULL) {
            break;
        }

        const char opt = *args;
        if(strchr(reserved_chars, opt) != NULL) {
            break;
        }

        const char* matched_opt = strchr(opts, opt);
        if(matched_opt == NULL) {
            break;
        }

        const char* optarg = NULL;
        const char* args_cursor = args + CHAR_OFFSET;

        if(is_option_argument_required(matched_opt)) {
            args_cursor += strspn(args_cursor, space_chars);

            if(*args_cursor == OPTION_DELIMITER) {
                break;
            }

            const size_t optval_len = parse_option_argument(args_cursor, &optarg);
            if(optval_len == 0) {
                break;
            }

            args_cursor += optval_len;
        }

        out->opt = opt;
        out->optarg = optarg;

        consumed_len = args_cursor - args;

    } while(false);

    return consumed_len;
}

static size_t parse_positional_arg(const char* args, ParsedOption* out) {
    const size_t consumed_len = parse_option_argument(args, &out->optarg);

    if(consumed_len > 0) {
        out->opt = 0;
    }

    return consumed_len;
}

bool parse_args(FuriString* args, const char* opts, OptionCallback callback, void* context) {
    furi_check(args);
    furi_check(callback);

    bool success = true;

    const size_t args_len = furi_string_size(args);

    for(size_t i = 0; i < args_len; ++i) {
        if(strchr(space_chars, furi_string_get_char(args, i)) != NULL) {
            continue;
        }

        size_t consumed_len;
        ParsedOption option;

        if(furi_string_get_char(args, i) == OPTION_DELIMITER) {
            i += CHAR_OFFSET;
            consumed_len = parse_option(furi_string_get_cstr(args) + i, opts, &option);
        } else {
            consumed_len = parse_positional_arg(furi_string_get_cstr(args) + i, &option);
        }

        if(consumed_len == 0) {
            success = false;
            break;
        }

        i += consumed_len;

        if(i < args_len) {
            furi_string_set_char(args, i, 0);
        }

        callback(option.opt, option.optarg, context);
    }

    return success;
}
