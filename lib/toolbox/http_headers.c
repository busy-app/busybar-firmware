#include "http_headers.h"

#include <m-array.h>

#include <core/check.h>

static void http_header_clear(HttpHeader header);

M_ARRAY_DEF(HttpHeaderArray, HttpHeader, M_OPEXTEND(M_POD_OPLIST, CLEAR(http_header_clear)));

typedef struct HttpHeaders {
    HttpHeaderArray_t headers;
} HttpHeaders;

static void http_header_clear(HttpHeader header) {
    furi_string_free(header.key);
    furi_string_free(header.value);
}

typedef enum ParseState {
    ParseStateKey,
    ParseStateColon,
    ParseStateValue,
    ParseStateValueSpace,
    ParseStateNewline,
} ParseState;

static bool parse_headers_list(HttpHeaders* headers, const char* data, size_t data_size) {
    ParseState state = ParseStateKey;
    size_t key_begin_i = 0;
    size_t key_end_i = 0;
    size_t value_begin_i = 0;
    size_t value_end_i = 0;
    for(size_t i = 0; i != data_size; ++i) {
        char c = data[i];
        switch(state) {
        case ParseStateKey: {
            if(c == ':') {
                if(key_begin_i == i) {
                    // Line starts with a colon
                    return false;
                } else {
                    key_end_i = i;
                    state = ParseStateColon;
                }
            } else if(c == '\r' && key_begin_i == i) {
                // End of headers
                return true;
            } else if(isspace((int)c)) {
                return false;
            }
            break;
        }
        case ParseStateColon: {
            if(c != ' ' && c != '\t') {
                if(isspace((int)c)) {
                    return false;
                }
                state = ParseStateValue;
                value_begin_i = i;
            }
            break;
        }
        case ParseStateValue: {
            if(c == '\r') {
                value_end_i = i;
                state = ParseStateNewline;
            } else if(c == ' ' || c == '\t') {
                value_end_i = i;
                state = ParseStateValueSpace;
            }
            break;
        }
        case ParseStateValueSpace: {
            if(c == '\r') {
                state = ParseStateNewline;
            } else if(c != ' ' && c != '\t') {
                state = ParseStateValue;
            }
            break;
        }
        case ParseStateNewline: {
            if(c == '\n') {
                HttpHeader header = {
                    .key = furi_string_alloc_printf(
                        "%.*s", key_end_i - key_begin_i, data + key_begin_i),
                    .value = furi_string_alloc_printf(
                        "%.*s", value_end_i - value_begin_i, data + value_begin_i),
                };
                HttpHeaderArray_push_back(headers->headers, header);

                key_begin_i = i + 1;
                state = ParseStateKey;
            } else {
                return false;
            }
            break;
        }
        }
    }
    return true;
}

static HttpHeader* http_headers_find_by_key(const HttpHeaders* instance, const char* key) {
    HttpHeader* result = NULL;

    for(uint32_t i = 0; i < HttpHeaderArray_size(instance->headers); ++i) {
        HttpHeader* hdr = HttpHeaderArray_get(instance->headers, i);
        if(furi_string_cmpi(hdr->key, key) == 0) {
            result = hdr;
        }
    }

    return result;
}

HttpHeaders* http_headers_alloc(void) {
    HttpHeaders* instance = malloc(sizeof(HttpHeaders));
    HttpHeaderArray_init(instance->headers);
    return instance;
}

void http_headers_free(HttpHeaders* instance) {
    furi_check(instance);

    HttpHeaderArray_clear(instance->headers);
    free(instance);
}

bool http_headers_parse(HttpHeaders* instance, const char* data, size_t data_len) {
    furi_check(instance);
    furi_check(data);

    return parse_headers_list(instance, data, data_len);
}

size_t http_headers_get_header_count(const HttpHeaders* instance) {
    furi_check(instance);
    return HttpHeaderArray_size(instance->headers);
}

const HttpHeader* http_headers_get_header(const HttpHeaders* instance, size_t index) {
    furi_check(instance);
    return HttpHeaderArray_cget(instance->headers, index);
}

const HttpHeader* http_headers_get(const HttpHeaders* instance, const char* key) {
    furi_check(instance);
    furi_check(key);
    return http_headers_find_by_key(instance, key);
}

void http_headers_set(HttpHeaders* instance, const char* key, const char* value) {
    furi_check(instance);
    furi_check(key);
    furi_check(value);

    HttpHeader* header = http_headers_find_by_key(instance, key);
    if(header == NULL) {
        const HttpHeader new_header = {
            .key = furi_string_alloc_set(key),
            .value = furi_string_alloc_set(value),
        };
        HttpHeaderArray_push_back(instance->headers, new_header);

    } else {
        furi_string_set(header->key, key);
        furi_string_set(header->value, value);
    }
}
