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

static void url_reset(Url* instance) {
    for(uint32_t i = 0; i < COUNT_OF(instance->parts); ++i) {
        string_slice_reset(&instance->parts[i]);
    }
}

Url* url_alloc(void) {
    Url* instance = malloc(sizeof(Url));

    instance->source = furi_string_alloc();
    url_reset(instance);

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

static bool url_parse_parts(StringSlice* parts, const char* source_str, size_t source_len) {
    bool success = true;

    size_t offset = 0;

    for(UrlParseStepIdx step_idx = UrlParseStepIdxProtocol; step_idx != UrlParseStepIdxMax;) {
        const UrlParseStep* step = &url_parse_steps[step_idx];
        const UrlParseStepIdx* next_step_idxs = step->next_step_idxs;

        StringSlice* part = &parts[step->part_id];

        for(uint32_t i = 0;; ++i) {
            const UrlParseStepIdx next_step_idx = next_step_idxs[i];
            step_idx = next_step_idx;

            size_t part_idx, new_offset;

            if(next_step_idx != UrlParseStepIdxMax) {
                const UrlParseStep* next_step = &url_parse_steps[next_step_idx];
                const char* next_delim = next_step->delim;

                const char* delim_p = strstr(source_str + offset, next_delim);
                if(delim_p == NULL) {
                    if(next_step->is_required) {
                        step_idx = UrlParseStepIdxMax;
                        success = false;
                        break;
                    }

                    continue;
                }

                part_idx = delim_p - source_str;
                new_offset = part_idx + strlen(next_delim);

                if(new_offset == source_len) {
                    if(next_step->is_required) {
                        step_idx = UrlParseStepIdxMax;
                        success = false;
                        break;
                    }
                }

            } else {
                part_idx = source_len;
                new_offset = source_len;
            }

            if(part_idx == offset) {
                if(step->is_required) {
                    step_idx = UrlParseStepIdxMax;
                    success = false;
                    break;
                }
            }

            part->first_char = source_str + offset;
            part->length = part_idx - offset;

            offset = new_offset;
            break;
        }

        if(step->is_include_delim) {
            const uint8_t delim_len = strlen(step->delim);
            part->first_char -= delim_len;
            part->length += delim_len;
        }
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
    url_reset(instance);

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
