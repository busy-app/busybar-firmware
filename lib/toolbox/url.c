#include "url.h"

#include <core/check.h>
#include <core/string.h>

#define URL_HOSTNAME_CHARS "-._"
#define URL_SPECIAL_CHARS  URL_HOSTNAME_CHARS "~:/?#[]@!$&'()*+,;=%"

#define URL_PORT_LEN_MAX (sizeof("65535") - 1)
#define URL_PORT_VAL_MAX (UINT16_MAX)

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

typedef bool (*UrlEncodeCallback)(const StringSlice* part, FuriString* out_str);

typedef struct {
    UrlPartId part_id;
    bool is_required;
    bool is_include_delim;
    bool is_default_delim;
    const char* delim;
    const UrlParseStepIdx* next_step_idxs;
    UrlEncodeCallback encode_callback;
} UrlParseStep;

typedef struct {
    UrlPartId part_id;
    const UrlParseStepIdx* subparts;
} UrlCompoundPart;

static bool url_encode_protocol(const StringSlice* part, FuriString* out_str);
static bool url_encode_hostname(const StringSlice* part, FuriString* out_str);
static bool url_encode_port(const StringSlice* part, FuriString* out_str);
static bool url_encode_percent(const StringSlice* part, FuriString* out_str);

/* clang-format off */

static const UrlParseStep url_parse_steps[] = {
    [UrlParseStepIdxProtocol] = {
        .part_id = UrlPartIdProtocol,
        .is_required = true,
        .delim = "",
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxHostname,
            UrlParseStepIdxMax,
        },
        .encode_callback = url_encode_protocol,
    },
    [UrlParseStepIdxHostname] = {
        .part_id = UrlPartIdHostname,
        .is_required = true,
        .delim = "//",
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxPort,
            UrlParseStepIdxPathname,
            UrlParseStepIdxSearch,
            UrlParseStepIdxMax,
        },
        .encode_callback = url_encode_hostname,
    },
    [UrlParseStepIdxPort] = {
        .part_id = UrlPartIdPort,
        .delim = ":",
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxPathname,
            UrlParseStepIdxSearch,
            UrlParseStepIdxMax,
        },
        .encode_callback = url_encode_port,
    },
    [UrlParseStepIdxPathname] = {
        .part_id = UrlPartIdPathname,
        .is_include_delim = true,
        .is_default_delim = true,
        .delim = "/",
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxSearch,
            UrlParseStepIdxMax,
        },
        .encode_callback = url_encode_percent,
    },
    [UrlParseStepIdxSearch] = {
        .part_id = UrlPartIdSearch,
        .is_include_delim = true,
        .delim = "?",
        .next_step_idxs = (const UrlParseStepIdx[]) {
            UrlParseStepIdxMax,
        },
        .encode_callback = url_encode_percent,
    },
};

static const UrlCompoundPart url_compound_parts[] = {
    {
        .part_id = UrlPartIdHref,
        .subparts = (const UrlParseStepIdx[]) {
            UrlParseStepIdxProtocol,
            UrlParseStepIdxHostname,
            UrlParseStepIdxPort,
            UrlParseStepIdxPathname,
            UrlParseStepIdxSearch,
            UrlParseStepIdxMax,
        },
    },
    {
        .part_id = UrlPartIdOrigin,
        .subparts = (const UrlParseStepIdx[]) {
            UrlParseStepIdxProtocol,
            UrlParseStepIdxHostname,
            UrlParseStepIdxPort,
            UrlParseStepIdxMax,
        },
    },
    {
        .part_id = UrlPartIdHost,
        .subparts = (const UrlParseStepIdx[]) {
            UrlParseStepIdxHostname,
            UrlParseStepIdxPort,
            UrlParseStepIdxMax,
        },
    },
};

/* clang-format on */

static void url_reset_parts(Url* instance) {
    for(uint32_t i = 0; i < COUNT_OF(instance->parts); ++i) {
        string_slice_reset(&instance->parts[i]);
    }
}

Url* url_alloc(void) {
    Url* instance = malloc(sizeof(Url));

    instance->source = furi_string_alloc();
    url_reset_parts(instance);

    return instance;
}

void url_free(Url* instance) {
    furi_check(instance);

    furi_string_free(instance->source);
    free(instance);
}

static void url_calc_compound_parts(Url* instance) {
    for(uint32_t i = 0; i < COUNT_OF(url_compound_parts); ++i) {
        const UrlCompoundPart* cp = &url_compound_parts[i];
        StringSlice* part = &instance->parts[cp->part_id];

        for(uint32_t j = 0, n = 0;; ++j) {
            const UrlParseStepIdx subpart_step_idx = cp->subparts[j];
            if(subpart_step_idx == UrlParseStepIdxMax) {
                break;
            }

            const UrlParseStep* subpart_step = &url_parse_steps[subpart_step_idx];
            const StringSlice* subpart = &instance->parts[subpart_step->part_id];

            if(subpart->length == 0) {
                continue;
            }

            if(n == 0) {
                part->first_char = subpart->first_char;
            }

            part->length += subpart->length;

            if(n != 0 && !subpart_step->is_include_delim) {
                part->length += strlen(subpart_step->delim);
            }

            ++n;
        }
    }
}

static bool url_find_delim(const char* str, const char* delim) {
    const size_t delim_len = strlen(delim);
    return (delim_len != 0) && (strncmp(str, delim, delim_len) == 0);
}

static bool url_has_required_step(const UrlParseStepIdx* step_idxs) {
    bool is_required = false;

    for(uint32_t i = 0;; ++i) {
        const UrlParseStepIdx step_idx = step_idxs[i];
        if(step_idx == UrlParseStepIdxMax) {
            break;
        }

        const UrlParseStep* step = &url_parse_steps[step_idx];
        if(step->is_required) {
            is_required = true;
            break;
        }
    }

    return is_required;
}

static UrlParseStepIdx
    url_find_step_idx(const char* source_str, const UrlParseStepIdx* step_idxs) {
    UrlParseStepIdx step_idx = UrlParseStepIdxMax;

    for(uint32_t i = 0;; ++i) {
        step_idx = step_idxs[i];
        if(step_idx == UrlParseStepIdxMax) {
            break;
        }

        const UrlParseStep* step = &url_parse_steps[step_idx];
        if(url_find_delim(source_str, step->delim)) {
            break;
        }
    }

    return step_idx;
}

typedef struct {
    UrlParseStepIdx step_idx;
    size_t part_offset;
} UrlFindNextStepResult;

static void url_find_next_step(
    const char* source_str,
    size_t offset,
    const UrlParseStepIdx* step_idxs,
    UrlFindNextStepResult* result) {
    UrlParseStepIdx step_idx = UrlParseStepIdxMax;

    const char* p;
    for(p = source_str + offset; *p != '\0'; ++p) {
        step_idx = url_find_step_idx(p, step_idxs);
        if(step_idx != UrlParseStepIdxMax) {
            break;
        }
    }

    result->step_idx = step_idx;
    result->part_offset = p - source_str;
}

static bool url_split_to_parts(StringSlice* parts, const char* source_str, size_t source_len) {
    bool success = true;

    size_t source_offset = 0;

    for(UrlParseStepIdx step_idx = UrlParseStepIdxProtocol; step_idx < UrlParseStepIdxMax;) {
        const UrlParseStep* step = &url_parse_steps[step_idx];

        UrlFindNextStepResult find_result;
        url_find_next_step(source_str, source_offset, step->next_step_idxs, &find_result);

        const StringSliceOffset part_offset = {
            .start = source_offset - (step->is_include_delim ? strlen(step->delim) : 0),
            .end = find_result.part_offset,
        };

        if(part_offset.start == part_offset.end) {
            success = false;
            break;
        }

        UrlParseStepIdx next_step_idx = find_result.step_idx;
        size_t new_source_offset = part_offset.end;

        if(next_step_idx != UrlParseStepIdxMax) {
            const UrlParseStep* next_step = &url_parse_steps[next_step_idx];
            new_source_offset += strlen(next_step->delim);

            if(next_step->is_required && (new_source_offset == source_len)) {
                success = false;
                break;
            }

        } else if(url_has_required_step(step->next_step_idxs)) {
            success = false;
            break;
        }

        StringSlice* part = &parts[step->part_id];
        string_slice_from_offset(part, &part_offset, source_str);

        source_offset = new_source_offset;
        step_idx = next_step_idx;
    }

    return success;
}

static bool url_encode_protocol(const StringSlice* part, FuriString* out_str) {
    bool success = true;

    do {
        const size_t part_len = part->length;
        if(part_len < 2) {
            success = false;
            break;
        }

        const char* part_str = part->first_char;
        const size_t part_len_minus_one = part_len - 1;

        if(part_str[part_len_minus_one] != ':') {
            success = false;
            break;
        }

        for(size_t i = 0; i < part_len_minus_one; ++i) {
            const unsigned char c = part_str[i];

            if((c > 0x7F) || (!isalpha(c))) {
                success = false;
                break;
            }

            furi_string_push_back(out_str, tolower(c));
        }

        if(success) {
            furi_string_push_back(out_str, ':');
        }

    } while(false);

    return success;
}

static bool url_encode_hostname(const StringSlice* part, FuriString* out_str) {
    bool success = true;

    const char* part_str = part->first_char;
    const size_t part_len = part->length;

    for(size_t i = 0; i < part_len; ++i) {
        const unsigned char c = part_str[i];
        if((c == '\0') || (c > 0x7F) || !(isalnum(c) || (strchr(URL_HOSTNAME_CHARS, c) != NULL))) {
            success = false;
            break;
        }

        furi_string_push_back(out_str, tolower(c));
    }

    return success;
}

static bool url_encode_port(const StringSlice* part, FuriString* out_str) {
    bool success = true;

    do {
        const char* part_str = part->first_char;
        const size_t part_len = part->length;

        if(part_len > URL_PORT_LEN_MAX) {
            success = false;
            break;
        }

        for(size_t i = 0; i < part_len; ++i) {
            const unsigned char c = part_str[i];

            if((c > 0x7F) || !isdigit(c)) {
                success = false;
                break;
            }

            furi_string_push_back(out_str, c);
        }

        if(!success) {
            break;
        }

        char tmp[part_len + 1];
        memcpy(tmp, part_str, part_len);
        tmp[part_len] = '\0';

        if(atoi(tmp) > URL_PORT_VAL_MAX) {
            success = false;
        }

    } while(false);

    return success;
}

static bool url_encode_percent(const StringSlice* part, FuriString* out_str) {
    const char* part_str = part->first_char;
    const size_t part_len = part->length;

    for(size_t i = 0; i < part_len; ++i) {
        const unsigned char c = part_str[i];

        if((c == '\0') || (c > 0x7F) || !(isalnum(c) || (strchr(URL_SPECIAL_CHARS, c) != NULL))) {
            furi_string_cat_printf(out_str, "%%%02X", c);
        } else {
            furi_string_push_back(out_str, c);
        }
    }

    return true;
}

static void url_calc_parts_from_offsets(Url* instance, const StringSliceOffset* spans) {
    const char* source_str = furi_string_get_cstr(instance->source);

    for(UrlParseStepIdx step_idx = UrlParseStepIdxProtocol; step_idx < UrlParseStepIdxMax;
        ++step_idx) {
        const UrlParseStep* step = &url_parse_steps[step_idx];
        StringSlice* part = &instance->parts[step->part_id];

        const StringSliceOffset* offset = &spans[step_idx];
        string_slice_from_offset(part, offset, source_str);
    }
}

static bool url_encode_from_parts(Url* instance, const StringSlice* parts) {
    bool success = true;

    FuriString* source = instance->source;
    StringSliceOffset offsets[UrlParseStepIdxMax] = {0};

    for(UrlParseStepIdx step_idx = UrlParseStepIdxProtocol; step_idx < UrlParseStepIdxMax;
        ++step_idx) {
        const UrlParseStep* step = &url_parse_steps[step_idx];
        const StringSlice* part = &parts[step->part_id];
        bool is_step_default_value = false;

        if(part->first_char == NULL) {
            if(step->is_required) {
                success = false;
                break;
            }

            if(!step->is_default_delim) {
                continue;
            }

            is_step_default_value = true;
        }

        if(!step->is_include_delim) {
            furi_string_cat(source, step->delim);
        }

        StringSliceOffset* offset = &offsets[step_idx];
        offset->start = furi_string_size(source);

        if(is_step_default_value) {
            furi_assert(step->is_include_delim);
            furi_string_cat(source, step->delim);
        }

        if(!step->encode_callback(part, source)) {
            success = false;
            break;
        }

        offset->end = furi_string_size(source);
    }

    if(success) {
        url_calc_parts_from_offsets(instance, offsets);
    }

    return success;
}

bool url_parse(Url* instance, const char* source_str) {
    furi_check(instance);
    furi_check(source_str);

    bool success = false;

    furi_string_reset(instance->source);
    url_reset_parts(instance);

    do {
        StringSlice tmp_parts[UrlPartIdMax] = {0};
        const size_t source_len = strlen(source_str);

        if(!url_split_to_parts(tmp_parts, source_str, source_len)) {
            break;
        }

        furi_string_reserve(instance->source, source_len);

        if(!url_encode_from_parts(instance, tmp_parts)) {
            break;
        }

        url_calc_compound_parts(instance);

        success = true;
    } while(false);

    return success;
}

const StringSlice* url_get_part(const Url* instance, UrlPartId part_id) {
    furi_check(instance);
    furi_check(part_id < UrlPartIdMax);

    return &instance->parts[part_id];
}
