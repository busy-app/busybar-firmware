#include "http_headers.h"

#define HTTP_MIN_HEADERS_LENGTH strlen("HTTP/1.0 XXX \r\n")
#define HTTP_NAME               "HTTP/"
#define HTTP_NAME_LEN           5

void http_header_free(HttpHeader header);

M_ARRAY_DEF(HttpHeaderArray, HttpHeader, M_OPEXTEND(M_POD_OPLIST, CLEAR(http_header_free)));

typedef struct HttpHeaders {
    uint32_t status;
    FuriString* status_text;

    HttpHeaderArray_t headers;
} HttpHeaders;

void http_header_free(HttpHeader header) {
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

static ssize_t parse_status_line(HttpHeaders* headers, const char* data, size_t size) {
    ssize_t result = -1;
    do {
        ssize_t i = 0;
        // Guarantees the following index increments won't go out of bounds
        if(size < HTTP_MIN_HEADERS_LENGTH) {
            break;
        }

        // "HTTP" "/"
        if(memcmp(data, HTTP_NAME, HTTP_NAME_LEN) != 0) {
            break;
        }
        i += HTTP_NAME_LEN;

        // HTTP version: DIGIT "." DIGIT
        if(!isdigit((int)data[i + 0]) || data[i + 1] != '.' || !isdigit((int)data[i + 2]) ||
           data[i + 3] != ' ') {
            break;
        }
        i += 4;

        // status-code: 3DIGIT SP
        char status_code[4];
        for(ssize_t j = 0; j != 3; ++j) {
            if(!isdigit((int)data[i + j])) {
                break;
            }
            status_code[j] = data[i + j];
        }
        if(data[i + 3] != ' ') {
            break;
        }
        status_code[3] = 0;
        headers->status = atoi(status_code);
        i += 4;

        // reason-phrase
        const char* cr = memchr(data + i, '\r', size - i);
        if(!cr || (size_t)(cr - data + 1) >= size || cr[1] != '\n') {
            break;
        }
        size_t reason_phrase_len = cr - data - i;
        headers->status_text = furi_string_alloc_printf("%.*s", reason_phrase_len, data + i);
        result = cr - data + 2;
    } while(false);
    return result;
}

HttpHeaders* http_headers_alloc(void) {
    HttpHeaders* instance = malloc(sizeof(HttpHeaders));
    HttpHeaderArray_init(instance->headers);

    return instance;
}

bool http_headers_parse(HttpHeaders* instance, const char* data, size_t size) {
    bool success = false;

    do {
        ssize_t headers_offset = parse_status_line(instance, data, size);
        if(headers_offset < 0) {
            break;
        }

        success = parse_headers_list(instance, data + headers_offset, size - headers_offset);
    } while(false);

    return success;
}

void http_headers_free(HttpHeaders* headers) {
    HttpHeaderArray_clear(headers->headers);
    if(headers->status_text) {
        furi_string_free(headers->status_text);
    }
    free(headers);
}

uint32_t http_headers_get_status(const HttpHeaders* instance) {
    return instance->status;
}

const char* http_headers_get_status_text(const HttpHeaders* instance) {
    return furi_string_get_cstr(instance->status_text);
}

size_t http_headers_get_header_count(const HttpHeaders* instance) {
    return HttpHeaderArray_size(instance->headers);
}

const HttpHeader* http_headers_get_header(const HttpHeaders* instance, size_t index) {
    return HttpHeaderArray_cget(instance->headers, index);
}
