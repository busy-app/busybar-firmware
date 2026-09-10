/**
 * @file js_app_settings_color.h
 * @brief Color type for application settings.
 */
#pragma once

#include <furi.h>
#include <color.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse a color string.
 *
 * @param[in] string "#RRGGBB" or "#RRGGBBAA"; the short form is opaque.
 * @param[out] value Parsed color.
 * @return true if the string is a valid color, false otherwise.
 */
bool js_app_settings_color_parse(const char* string, Color* value);

/**
 * @brief Format a color as a string.
 *
 * @param[in] value Color.
 * @param[out] string Output, "#RRGGBB" when opaque, "#RRGGBBAA" otherwise.
 */
void js_app_settings_color_format(const Color* value, FuriString* string);

#ifdef __cplusplus
}
#endif
