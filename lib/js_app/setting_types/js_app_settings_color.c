#include "js_app_settings_color.h"

bool js_app_settings_color_parse(const char* string, Color* value) {
    furi_check(string);
    furi_check(value);

    return color_parse_hex_string(string, value) || color_parse_hexa_string(string, value);
}

void js_app_settings_color_format(const Color* value, FuriString* string) {
    furi_check(value);
    furi_check(string);

    uint8_t alpha = value->a;
    furi_string_printf(
        string,
        (alpha == 0xFF) ? "#%02X%02X%02X" : "#%02X%02X%02X%02X",
        value->r,
        value->g,
        value->b,
        alpha);
}
