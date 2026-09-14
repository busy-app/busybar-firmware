#include "url_utils.h"

static bool is_url_safe(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           c == '.' || c == '_' || c == '-' || c == '~';
}

FuriString* url_utils_encode(const char* str) {
    FuriString* result = furi_string_alloc();

    while(*str) {
        char c = *str;
        if(is_url_safe(c)) {
            furi_string_push_back(result, c);
        } else {
            furi_string_cat_printf(result, "%%%02hhX", c);
        }
        ++str;
    }
    return result;
}
