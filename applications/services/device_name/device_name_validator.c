#include "device_name_validator.h"

static bool device_name_validate_char(char c) {
    static const char* const allowed_special_chars = " !()-_=+;:,.?'|@#$%^&*[]{}/\\\"<>";

    bool allowed_ascii = isalnum(c) || strchr(allowed_special_chars, c);
    bool utf8 = c >= 128;
    bool null = c == 0;
    return allowed_ascii && !utf8 && !null;
}

DeviceNameValidationStatus device_name_validate(const char* name) {
    furi_assert(name);

    if(strnlen(name, DEVICE_NAME_MAX_SIZE) == 0) {
        return DeviceNameValidationStatusEmpty;
    }

    if(strnlen(name, DEVICE_NAME_MAX_SIZE) > DEVICE_NAME_MAX_LENGTH) {
        return DeviceNameValidationStatusTooLong;
    }

    bool only_contains_spaces = true;

    for(size_t i = 0; i < strlen(name); i++) {
        char c = name[i];

        if(c != ' ') only_contains_spaces = false;

        if(!device_name_validate_char(c)) {
            return DeviceNameValidationStatusDisallowedChar;
        }
    }

    if(only_contains_spaces) {
        return DeviceNameValidationStatusOnlySpaces;
    }

    return DeviceNameValidationStatusOk;
}
