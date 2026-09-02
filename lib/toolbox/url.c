#include "url.h"

#include <core/check.h>
#include <core/string.h>

struct Url {
    FuriString* source;
    StringSlice slices[UrlPartMax];
};

typedef struct {
    UrlPart part;
    uint8_t walk_back;
    const char* delim;
} UrlParseStep;

static const UrlParseStep url_parse_steps[] = {
    {
        .part = UrlPartProtocol,
        .delim = "//",
    },
    {
        .part = UrlPartHostname,
        .delim = ":",
    },
    {
        .part = UrlPartPort,
        .delim = "/",
    },
    {
        .part = UrlPartPathname,
        .walk_back = 1,
        .delim = "?",
    },
    {
        .part = UrlPartSearch,
        .walk_back = 1,
        .delim = "",
    },
};

Url* url_alloc(void) {
    Url* instance = malloc(sizeof(Url));
    instance->source = furi_string_alloc();
    return instance;
}

void url_free(Url* instance) {
    furi_check(instance);

    furi_string_free(instance->source);
    free(instance);
}

static void url_reset(Url* instance) {
    for(uint32_t i = 0; i < COUNT_OF(instance->slices); ++i) {
        instance->slices[i] = (const StringSlice){
            .first_char = "",
            .length = 0,
        };
    }
}

bool url_parse(Url* instance, const char* source_str) {
    furi_check(instance);
    furi_check(source_str);

    bool success = false;
    url_reset(instance);

    FuriString* source = instance->source;
    furi_string_set(source, source_str);

    const size_t source_len = furi_string_size(source);

    size_t offset = 0;

    for(uint32_t i = 0; i < COUNT_OF(url_parse_steps); ++i) {
        const UrlParseStep* step = &url_parse_steps[i];

        const char* delim = step->delim;
        const size_t delim_len = strlen(delim);

        const size_t part_idx = delim_len ? furi_string_search(source, delim, offset) : source_len;
        if(part_idx == FURI_STRING_FAILURE) {
            break;
        }

        const uint8_t walk_back = step->walk_back;
        if(offset < walk_back) {
            break;
        }

        StringSlice* part_slice = &instance->slices[step->part];
        part_slice->first_char = furi_string_get_cstr(source) + offset - walk_back;
        part_slice->length = part_idx + walk_back - offset;

        offset = part_idx + strlen(step->delim);
    }

    if(offset == source_len) {
        success = true;
    }

    return success;
}

void url_get_part(const Url* instance, UrlPart part, StringSlice* out) {
    furi_check(instance);
    furi_check(part < UrlPartMax);
    furi_check(out);

    *out = instance->slices[part];
}
