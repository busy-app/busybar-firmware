#include "js_app_settings_time.h"

#include <inttypes.h>

static bool js_app_settings_time_parse_digits(const char* digits, unsigned int* value) {
    unsigned char high_digit = digits[0];
    unsigned char low_digit = digits[1];

    bool is_valid;
    if((is_valid = isdigit(high_digit) && isdigit(low_digit))) {
        *value = (high_digit - '0') * 10 + (low_digit - '0');
    }

    return is_valid;
}

static bool js_app_settings_time_parse_fields(const char* string, unsigned int* fields) {
    bool is_valid = false;
    for(;; fields++, string += strlen(":")) {
        if(!js_app_settings_time_parse_digits(string, fields)) {
            break;
        }

        string += strlen("XX");
        if(*string == '\0') {
            is_valid = true;
            break;
        }

        if(*string != ':') {
            break;
        }
    }

    return is_valid;
}

bool js_app_settings_time_parse(const char* string, JsAppSettingsTimeValue* value) {
    furi_check(string);
    furi_check(value);

    bool is_format_valid = false;

    do {
        size_t length = strlen(string);
        bool has_seconds = length == strlen("HH:MM:SS");
        if(!has_seconds && length != strlen("HH:MM")) {
            break;
        }

        /* hours, minutes, seconds */
        unsigned int fields[3] = {0};
        if(!js_app_settings_time_parse_fields(string, fields)) {
            break;
        }

        if(utz_time_init_checked(fields[0], fields[1], fields[2], &value->time)) {
            value->has_seconds = has_seconds;
            is_format_valid = true;
        }
    } while(false);

    return is_format_valid;
}

void js_app_settings_time_format(const JsAppSettingsTimeValue* value, FuriString* string) {
    furi_check(string);
    furi_check(value);

    furi_string_printf(
        string,
        value->has_seconds ? "%02" PRIu8 ":%02" PRIu8 ":%02" PRIu8 : "%02" PRIu8 ":%02" PRIu8,
        value->time.hour,
        value->time.minute,
        value->time.second);
}
