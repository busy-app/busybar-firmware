#include "js_app_settings_geo.h"

#include <furi.h>

#include <string.h>

static const char* const js_app_settings_geo_mode_map[] = {
    [JsAppSettingsGeoModeAuto] = "auto",
    [JsAppSettingsGeoModeFixed] = "fixed",
};

static_assert(COUNT_OF(js_app_settings_geo_mode_map) == JsAppSettingsGeoModesCount);

bool js_app_settings_geo_mode_parse(const char* string, JsAppSettingsGeoMode* mode) {
    furi_check(string);
    furi_check(mode);

    bool is_mode_valid = false;
    for(JsAppSettingsGeoMode i = 0; i < JsAppSettingsGeoModesCount; i++) {
        if(strcmp(js_app_settings_geo_mode_map[i], string) == 0) {
            *mode = i;
            is_mode_valid = true;
            break;
        }
    }

    return is_mode_valid;
}

const char* js_app_settings_geo_mode_format(JsAppSettingsGeoMode mode) {
    return (mode < JsAppSettingsGeoModesCount) ? js_app_settings_geo_mode_map[mode] : "unknown";
}
