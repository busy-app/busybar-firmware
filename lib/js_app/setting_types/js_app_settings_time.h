/**
 * @file js_app_settings_time.h
 * @brief Time of day type for application settings.
 */
#pragma once

#include <furi.h>
#include <utz/utz.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Time of day value.
 */
typedef struct {
    utz_time_t time; ///< Time of day.
    bool has_seconds; ///< Whether seconds are part of the value.
} JsAppSettingsTimeValue;

/**
 * @brief Parse a time of day string.
 *
 * @param[in] string "HH:MM" or "HH:MM:SS".
 * @param[out] value Parsed time of day; has_seconds is set for the long form.
 * @return true if the string is a valid time, false otherwise.
 */
bool js_app_settings_time_parse(const char* string, JsAppSettingsTimeValue* value);

/**
 * @brief Format a time of day as a string.
 *
 * @param[in] value Time of day.
 * @param[out] string Output, zero-padded "HH:MM" or "HH:MM:SS".
 */
void js_app_settings_time_format(const JsAppSettingsTimeValue* value, FuriString* string);

#ifdef __cplusplus
}
#endif
