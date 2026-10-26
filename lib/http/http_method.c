#include "http_method.h"

#include <core/check.h>

bool http_method_is_valid(const char* method_str) {
    bool is_valid = true;

    do {
        const size_t method_len = strlen(method_str);
        if(method_len == 0) {
            is_valid = false;
            break;
        }

        for(size_t i = 0; i < method_len; ++i) {
            const unsigned char c = method_str[i];
            if(c > 0x7F || !isalnum(c)) {
                is_valid = false;
                break;
            }
        }
    } while(false);

    return is_valid;
}

/* https://fetch.spec.whatwg.org/#concept-method-normalize */
void http_method_normalize(char* method_str) {
    furi_check(method_str);

    static const char* const normalized_methods[] = {
        "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS"};

    for(uint8_t i = 0; i < COUNT_OF(normalized_methods); i++) {
        const char* reference = normalized_methods[i];
        if(strcasecmp(method_str, reference) == 0) {
            strcpy(method_str, reference);
            break;
        }
    }
}
