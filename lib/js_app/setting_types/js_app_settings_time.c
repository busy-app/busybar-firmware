#include "js_app_settings_time.h"

#include <inttypes.h>

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

        unsigned int hours = 0;
        unsigned int minutes = 0;
        unsigned int seconds = 0;

        int parsed_length = 0;
        if(has_seconds) {
            sscanf(string, "%2u:%2u:%2u%n", &hours, &minutes, &seconds, &parsed_length);
        } else {
            sscanf(string, "%2u:%2u%n", &hours, &minutes, &parsed_length);
        }

        if((size_t)parsed_length == length) {
            if(utz_time_init_checked(hours, minutes, seconds, &value->time)) {
                value->has_seconds = has_seconds;
                is_format_valid = true;
            }
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
