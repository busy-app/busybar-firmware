#include "http_response.h"

#include <core/check.h>

#define HTTP_NAME     "HTTP/"
#define HTTP_NAME_LEN (sizeof(HTTP_NAME) - 1)

#define HTTP_STATUS_TEMPLATE HTTP_NAME "1.0 XXX \r\n"
#define HTTP_STATUS_LEN_MIN  (sizeof(HTTP_STATUS_TEMPLATE) - 1)

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
        instance->status = atoi(status_code);
        i += 4;

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
