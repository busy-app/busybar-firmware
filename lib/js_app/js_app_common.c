#include "js_app_common.h"
#include <stddef.h>
#include <string.h>

bool js_app_registry_is_valid_app_id(const char* app_id) {
    size_t len = strnlen(app_id, JS_APP_ID_LEN_MAX + 1);
    if(len == 0 || len > JS_APP_ID_LEN_MAX) {
        return false;
    }
    for(size_t i = 0; i != len; ++i) {
        char c = app_id[i];
        bool is_word = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                       (c >= '0' && c <= '9') || c == '_' || c == '-';
        bool is_dot = c == '.';
        if(i == 0) {
            if(!is_word) {
                return false;
            }
        } else {
            if(!is_word && !is_dot) {
                return false;
            }
        }
    }
    return true;
}
