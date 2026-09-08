#include "http_response.h"

#include <core/check.h>
#include <toolbox/strint.h>

#define HTTP_NAME     "HTTP/"
#define HTTP_NAME_LEN (sizeof(HTTP_NAME) - 1)

#define HTTP_STATUS_TEMPLATE HTTP_NAME "1.0 XXX \r\n"
#define HTTP_STATUS_LEN_MIN  (sizeof(HTTP_STATUS_TEMPLATE) - 1)

#define HTTP_STATUS_CODE_LEN (3)

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
        char* status_code_end;
        if(strint_to_uint32(&data[i], &status_code_end, &instance->status, 10) !=
           StrintParseNoError) {
            break;
        }
        if(status_code_end - &data[i] != HTTP_STATUS_CODE_LEN) {
            break;
        }
        i += HTTP_STATUS_CODE_LEN;

        if(data[i] != ' ') {
            break;
        }
        i += 1;

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
