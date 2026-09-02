#include "url.h"

#include <core/check.h>
#include <core/string.h>

struct Url {
    FuriString* source;
    StringSlice slices[UrlPartMax];
};

typedef enum {
    UrlParseStepIdxProtocol,
    UrlParseStepIdxHostname,
    UrlParseStepIdxPort,
    UrlParseStepIdxPathname,
    UrlParseStepIdxSearch,
    UrlParseStepIdxMax,
} UrlParseStepIdx;

typedef struct {
    UrlPart part;
    bool is_required;
    uint8_t walk_back;
    const char* delim;
} UrlParseStep;

typedef struct {
    UrlPart part;
    uint8_t subparts_count;
    const UrlParseStepIdx* subparts;
} UrlCompoundPart;

/* clang-format off */

static const UrlParseStep url_parse_steps[] = {
    [UrlParseStepIdxProtocol] = {
        .part = UrlPartProtocol,
        .is_required = true,
        .delim = "//",
    },
    [UrlParseStepIdxHostname] = {
        .part = UrlPartHostname,
        .is_required = true,
        .delim = ":",
    },
    [UrlParseStepIdxPort] = {
        .part = UrlPartPort,
        .delim = "/",
    },
    [UrlParseStepIdxPathname] = {
        .part = UrlPartPathname,
        .walk_back = 1,
        .delim = "?",
    },
    [UrlParseStepIdxSearch] = {
        .part = UrlPartSearch,
        .walk_back = 1,
        .delim = "",
    },
};

static const UrlCompoundPart url_compound_parts[] = {
    {
        .part = UrlPartHref,
        .subparts_count = 5,
        .subparts = (const UrlParseStepIdx[5]) {
            UrlParseStepIdxProtocol,
            UrlParseStepIdxHostname,
            UrlParseStepIdxPort,
            UrlParseStepIdxPathname,
            UrlParseStepIdxSearch,
        },
    },
    {
        .part = UrlPartOrigin,
        .subparts_count = 3,
        .subparts = (const UrlParseStepIdx[3]) {
            UrlParseStepIdxProtocol,
            UrlParseStepIdxHostname,
            UrlParseStepIdxPort,
        },
    },
    {
        .part = UrlPartHost,
        .subparts_count = 2,
        .subparts = (const UrlParseStepIdx[2]) {
            UrlParseStepIdxHostname,
            UrlParseStepIdxPort,
        },
    },
};

/* clang-format on */

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

static void url_calc_compound_parts(Url* instance) {
    for(uint32_t i = 0; i < COUNT_OF(url_compound_parts); ++i) {
        const UrlCompoundPart* cp = &url_compound_parts[i];
        const uint8_t subparts_count = cp->subparts_count;

        StringSlice* slice = &instance->slices[cp->part];

        for(uint32_t j = 0; j < subparts_count; ++j) {
            const UrlParseStep* subpart = &url_parse_steps[cp->subparts[j]];
            const StringSlice* subpart_slice = &instance->slices[subpart->part];

            if(subpart_slice->length == 0) {
                continue;
            }

            if(slice->length == 0) {
                slice->first_char = subpart_slice->first_char;
            }

            slice->length += subpart_slice->length;

            if(j < (subparts_count - 1U)) {
                slice->length += strlen(subpart->delim);
            }
        }
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
        url_calc_compound_parts(instance);
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
