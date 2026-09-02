#include "url.h"

#include <core/check.h>
#include <core/string.h>

struct Url {
    FuriString* source;
    StringSlice parts[UrlPartIdMax];
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
    UrlPartId part_id;
    bool is_required;
    bool is_include_delim;
    const char* delim;
    const UrlParseStepIdx* next_step_idxs;
} UrlParseStep;

typedef struct {
    UrlPartId part_id;
    uint8_t subparts_count;
    const UrlParseStepIdx* subparts;
} UrlCompoundPart;

/* clang-format off */

static const UrlParseStep url_parse_steps[] = {
    [UrlParseStepIdxProtocol] = {
        .part_id = UrlPartIdProtocol,
        .is_required = true,
        .delim = NULL,
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxHostname,
            UrlParseStepIdxMax,
        },
    },
    [UrlParseStepIdxHostname] = {
        .part_id = UrlPartIdHostname,
        .is_required = true,
        .delim = "//",
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxPort,
            UrlParseStepIdxPathname,
            UrlParseStepIdxMax,
        },
    },
    [UrlParseStepIdxPort] = {
        .part_id = UrlPartIdPort,
        .delim = ":",
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxPathname,
            UrlParseStepIdxMax,
        },
    },
    [UrlParseStepIdxPathname] = {
        .part_id = UrlPartIdPathname,
        .is_include_delim = true,
        .delim = "/",
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxSearch,
            UrlParseStepIdxMax,
        },
    },
    [UrlParseStepIdxSearch] = {
        .part_id = UrlPartIdSearch,
        .is_include_delim = true,
        .delim = "?",
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxMax,
        },
    },
};

static const UrlCompoundPart url_compound_parts[] = {
    {
        .part_id = UrlPartIdHref,
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
        .part_id = UrlPartIdOrigin,
        .subparts_count = 3,
        .subparts = (const UrlParseStepIdx[3]) {
            UrlParseStepIdxProtocol,
            UrlParseStepIdxHostname,
            UrlParseStepIdxPort,
        },
    },
    {
        .part_id = UrlPartIdHost,
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
    for(uint32_t i = 0; i < COUNT_OF(instance->parts); ++i) {
        instance->parts[i] = (const StringSlice){
            .first_char = "",
            .length = 0,
        };
    }
}

static void url_calc_compound_parts(Url* instance) {
    for(uint32_t i = 0; i < COUNT_OF(url_compound_parts); ++i) {
        const UrlCompoundPart* cp = &url_compound_parts[i];
        const uint8_t subparts_count = cp->subparts_count;

        StringSlice* slice = &instance->parts[cp->part_id];

        for(uint32_t j = 0; j < subparts_count; ++j) {
            const UrlParseStep* subpart = &url_parse_steps[cp->subparts[j]];
            const StringSlice* subpart_slice = &instance->parts[subpart->part_id];

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

    for(UrlParseStepIdx step_idx = UrlParseStepIdxProtocol; step_idx != UrlParseStepIdxMax;) {
        const UrlParseStep* step = &url_parse_steps[step_idx];
        const UrlParseStepIdx* next_step_idxs = step->next_step_idxs;

        StringSlice* part = &instance->parts[step->part_id];

        for(uint32_t i = 0;; ++i) {
            const UrlParseStepIdx next_step_idx = next_step_idxs[i];
            step_idx = next_step_idx;

            size_t part_idx, next_delim_len;

            if(next_step_idx != UrlParseStepIdxMax) {
                const UrlParseStep* next_step = &url_parse_steps[next_step_idx];
                const char* next_delim = next_step->delim;

                part_idx = furi_string_search(source, next_delim, offset);
                if(part_idx == FURI_STRING_FAILURE) {
                    continue;
                }

                next_delim_len = strlen(next_delim);

            } else {
                part_idx = source_len;
                next_delim_len = 0;
            }

            part->first_char = furi_string_get_cstr(source) + offset;
            part->length = part_idx - offset;

            offset = part_idx + next_delim_len;
            break;
        }

        if(step->is_include_delim) {
            const uint8_t delim_len = strlen(step->delim);
            part->first_char -= delim_len;
            part->length += delim_len;
        }
    }

    if(offset == source_len) {
        // url_calc_compound_parts(instance);
        UNUSED(url_calc_compound_parts);
        success = true;
    }

    return success;
}

void url_get_part(const Url* instance, UrlPartId part_id, StringSlice* part) {
    furi_check(instance);
    furi_check(part_id < UrlPartIdMax);
    furi_check(part);

    *part = instance->parts[part_id];
}
