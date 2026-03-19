#include "device_name_validator.h"

#define DEVICE_NAME_SET_ERROR(error, format, ...)                   \
    ({                                                              \
        if(error) furi_string_printf(error, format, ##__VA_ARGS__); \
    })

static bool device_name_validate_char(char c) {
    static const char* const allowed_special_chars = " !()-_=+;:,.?'|@#$%^&*[]{}/\\\"<>";

    bool allowed_ascii = isalnum(c) || strchr(allowed_special_chars, c);
    bool utf8 = c >= 128;
    bool null = c == 0;
    return allowed_ascii && !utf8 && !null;
}

bool device_name_validate_cstr(const char* name, FuriString* error) {
    furi_assert(name);

    if(strlen(name) == 0) {
        DEVICE_NAME_SET_ERROR(error, "Name is empty");
        return false;
    }

    if(strlen(name) > DEVICE_NAME_MAX_LENGTH) {
        DEVICE_NAME_SET_ERROR(error, "Name exceeds %d characters", DEVICE_NAME_MAX_LENGTH);
        return false;
    }

    bool only_contains_spaces = true;

    for(size_t i = 0; i < strlen(name); i++) {
        char c = name[i];

        if(c != ' ') only_contains_spaces = false;

        if(!device_name_validate_char(c)) {
            DEVICE_NAME_SET_ERROR(error, "Disallowed character: %c", c);
            return false;
        }
    }

    if(only_contains_spaces) {
        DEVICE_NAME_SET_ERROR(error, "Name can't consist of only spaces");
        return false;
    }

    return true;
}

bool device_name_validate(FuriString* name, FuriString* error) {
    furi_assert(name);

    return device_name_validate_cstr(furi_string_get_cstr(name), error);
}
