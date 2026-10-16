#include "url.h"

#include <core/check.h>
#include <core/string.h>

#define URL_HOSTNAME_CHARS "-._"
#define URL_SPECIAL_CHARS  URL_HOSTNAME_CHARS "~:/?#[]@!$&'()*+,;=%"

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

static bool url_is_delim_at(const char* str, const char* delim) {
    const size_t delim_len = strlen(delim);
    return (delim_len != 0) && (strncmp(str, delim, delim_len) == 0);
}

static bool url_is_any_step_required(const UrlParseStepIdx* step_idxs) {
    for(uint32_t i = 0; step_idxs[i] != UrlParseStepIdxMax; ++i) {
        if(url_parse_steps[step_idxs[i]].is_required) {
            return true;
        }
    }

    return false;
}

static UrlParseStepIdx url_find_next_step(
    const UrlParseStepIdx* candidate_idxs,
    const char* source_str,
    size_t offset,
    size_t* delim_idx) {
    const char* p;

    for(p = source_str + offset; *p != '\0'; ++p) {
        for(uint32_t i = 0; candidate_idxs[i] != UrlParseStepIdxMax; ++i) {
            if(url_is_delim_at(p, url_parse_steps[candidate_idxs[i]].delim)) {
                *delim_idx = p - source_str;
                return candidate_idxs[i];
            }
        }
    }

    *delim_idx = p - source_str;
    return UrlParseStepIdxMax;
}

static bool url_parse_parts(StringSlice* parts, const char* source_str, size_t source_len) {
    bool success = true;

    size_t offset = 0;

    for(UrlParseStepIdx step_idx = UrlParseStepIdxProtocol; step_idx < UrlParseStepIdxMax;) {
        const UrlParseStep* step = &url_parse_steps[step_idx];
        StringSlice* part = &parts[step->part_id];

        size_t part_idx;
        const UrlParseStepIdx next_step_idx =
            url_find_next_step(step->next_step_idxs, source_str, offset, &part_idx);

        size_t new_offset = part_idx;

        if(next_step_idx != UrlParseStepIdxMax) {
            const UrlParseStep* next_step = &url_parse_steps[next_step_idx];
            new_offset += strlen(next_step->delim);

            if(next_step->is_required && (new_offset == source_len)) {
                success = false;
                break;
            }

        } else if(url_is_any_step_required(step->next_step_idxs)) {
            success = false;
            break;
        }

        if(step->is_required && (part_idx == offset)) {
            success = false;
            break;
        }

        part->first_char = source_str + offset;
        part->length = part_idx - offset;

        if(step->is_include_delim) {
            const size_t delim_len = strlen(step->delim);
            part->first_char -= delim_len;
            part->length += delim_len;
        }

        offset = new_offset;
        step_idx = next_step_idx;
    }

    return success;
}

static bool url_encode_protocol(const StringSlice* part, FuriString* out_str) {
    bool success = true;

    do {
        const char* part_str = part->first_char;
        const size_t part_len_minus_one = part->length - 1;

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

    const char* part_str = part->first_char;
    const size_t part_len = part->length;

    for(size_t i = 0; i < part_len; ++i) {
        const unsigned char c = part_str[i];

        if((c > 0x7F) || !isdigit(c)) {
            success = false;
            break;
        }

        furi_string_push_back(out_str, c);
    }

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

static bool url_encode_parts(const StringSlice* parts, FuriString* out_str) {
    bool success = true;

    for(UrlParseStepIdx step_idx = UrlParseStepIdxProtocol; step_idx < UrlParseStepIdxMax;
        ++step_idx) {
        const UrlParseStep* step = &url_parse_steps[step_idx];
        const StringSlice* part = &parts[step->part_id];

        if(part->first_char == NULL) {
            if(step->is_required) {
                success = false;
                break;
            }

            continue;
        }

        if(!step->is_include_delim) {
            furi_string_cat(out_str, step->delim);
        }

        if(!step->encode_callback(part, out_str)) {
            success = false;
            break;
        }
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

        if(!url_parse_parts(tmp_parts, source_str, source_len)) {
            break;
        }

        if(!url_encode_parts(tmp_parts, instance->source)) {
            break;
        }

        const char* encoded_str = furi_string_get_cstr(instance->source);
        const size_t encoded_len = furi_string_size(instance->source);

        if(!url_parse_parts(instance->parts, encoded_str, encoded_len)) {
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
