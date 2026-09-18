#include "http_response.h"

#include <core/check.h>

#define HTTP_NAME     "HTTP/"
#define HTTP_NAME_LEN (sizeof(HTTP_NAME) - 1)

#define HTTP_STATUS_TEMPLATE HTTP_NAME "1.0 XXX \r\n"
#define HTTP_STATUS_LEN_MIN  (sizeof(HTTP_STATUS_TEMPLATE) - 1)

#define HTTP_VERSION_LEN     (3)
#define HTTP_STATUS_CODE_LEN (3)

#define SPACE_LEN (1)

static bool data_contains_only_digits(const char* data, size_t data_len) {
    bool contains_digits = true;

    for(uint32_t i = 0; i < data_len; ++i) {
        if(!isdigit((int)data[i])) {
            contains_digits = false;
            break;
        }
    }

    return contains_digits;
}

static bool http_response_is_version_valid(const char* data) {
    return isdigit((int)data[0]) && ('.' == data[1]) && isdigit((int)data[2]);
}

static bool http_response_parse_status_code(const char* data, uint32_t* out) {
    bool success = false;

    do {
        if(!data_contains_only_digits(data, HTTP_STATUS_CODE_LEN)) {
            break;
        }

        char tmp[HTTP_STATUS_CODE_LEN + 1];
        memcpy(tmp, data, HTTP_STATUS_CODE_LEN);
        tmp[HTTP_STATUS_CODE_LEN] = '\0';

        *out = atoi(tmp);
        success = true;
    } while(false);

    return success;
}

static ssize_t
    http_response_parse_status_line(HttpResponse* instance, const char* data, size_t data_len) {
    ssize_t result = -1;
    do {
        ssize_t i = 0;
        // Guarantees the following index increments won't go out of bounds
        if(data_len < HTTP_STATUS_LEN_MIN) {
            break;
        }

        // "HTTP" "/"
        if(strncmp(data, HTTP_NAME, HTTP_NAME_LEN) != 0) {
            break;
        }
        i += HTTP_NAME_LEN;

        // HTTP version: DIGIT "." DIGIT
        if(!http_response_is_version_valid(&data[i])) {
            break;
        }
        i += HTTP_VERSION_LEN;

        if(data[i] != ' ') {
            break;
        }
        i += SPACE_LEN;

        // status-code: 3DIGIT SP
        if(!http_response_parse_status_code(&data[i], &instance->status)) {
            break;
        }
        i += HTTP_STATUS_CODE_LEN;

        if(data[i] != ' ') {
            break;
        }
        i += SPACE_LEN;

        // reason-phrase
        const char* cr = memchr(data + i, '\r', data_len - i);
        if(!cr || (size_t)(cr - data + 1) >= data_len || cr[1] != '\n') {
            break;
        }

        StringSlice* status_text = &instance->status_text;
        status_text->first_char = data + i;
        status_text->length = cr - data - i;

        result = cr - data + 2;
    } while(false);
    return result;
}

void http_response_init(HttpResponse* instance) {
    instance->status = 0;
    string_slice_reset(&instance->status_text);
    string_slice_reset(&instance->headers);
}

bool http_response_parse(HttpResponse* instance, const char* data, size_t data_len) {
    furi_check(instance);
    furi_check(data);

    bool success = false;

    do {
        const ssize_t status_offset = http_response_parse_status_line(instance, data, data_len);
        if(status_offset < 0) {
            break;
        }

        StringSlice* headers = &instance->headers;
        headers->first_char = data + status_offset;
        headers->length = data_len - status_offset;

        success = true;
    } while(false);

    return success;
}
