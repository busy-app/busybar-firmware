/**
 * @file js_app_settings_geo.h
 * @brief Geolocation type for application settings.
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JS_APP_SETTINGS_GEO_NAME_MAX_LENGTH (128u)

/**
 * @brief Geolocation mode.
 */
typedef enum {
    JsAppSettingsGeoModeAuto, ///< Derive coordinates from the device.
    JsAppSettingsGeoModeFixed, ///< Use the given coordinates.

    JsAppSettingsGeoModesCount,
} JsAppSettingsGeoMode;

/**
 * @brief Geolocation value.
 */
typedef struct {
    JsAppSettingsGeoMode mode; ///< Coordinate source.
    char name[JS_APP_SETTINGS_GEO_NAME_MAX_LENGTH + 1]; ///< Location name.
    float latitude; ///< Degrees within [-90, 90], NAN in auto mode.
    float longitude; ///< Degrees within [-180, 180], NAN in auto mode.
} JsAppSettingsGeoValue;

/**
 * @brief Parse a geolocation mode string.
 *
 * @param[in] string "auto" or "fixed".
 * @param[out] mode Parsed mode.
 * @return true if the string names a mode, false otherwise.
 */
bool js_app_settings_geo_mode_parse(const char* string, JsAppSettingsGeoMode* mode);

/**
 * @brief Format a geolocation mode as a string.
 *
 * @param[in] mode Mode.
 * @return Mode name, "auto" or "fixed".
 */
const char* js_app_settings_geo_mode_format(JsAppSettingsGeoMode mode);

#ifdef __cplusplus
}
#endif
