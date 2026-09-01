/**
 * @file js_app_settings.h
 * @brief Declarative settings for JavaScript applications.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <cjson/cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsAppSettings JsAppSettings;
typedef struct JsAppSetting JsAppSetting;

typedef enum {
    JsAppSettingTypeGroup,
    JsAppSettingTypeSwitch,
    JsAppSettingTypeSelector,
    JsAppSettingTypeSpinbox,
    JsAppSettingTypeTimebox,
    JsAppSettingTypeMax,
} JsAppSettingType;

JsAppSettings* js_app_settings_alloc(void);
void js_app_settings_free(JsAppSettings* instance);

bool js_app_settings_load(JsAppSettings* instance, const char* app_id, const char* schema_path);
bool js_app_settings_remove(const char* app_id);

/**
 * @brief Re-read the stored values for an already loaded instance.
 *
 * Keeps the parsed schema and re-points every setting at freshly read values,
 * so a caller holding a long-lived instance picks up changes made elsewhere
 * (the Setup scene, the HTTP API, or another running copy of the app).
 *
 * @param instance loaded settings instance
 * @returns true if the values were re-read
 */
bool js_app_settings_reload(JsAppSettings* instance);

size_t js_app_settings_get_count(const JsAppSettings* instance, const JsAppSetting* group);
const JsAppSetting*
    js_app_settings_get(const JsAppSettings* instance, const JsAppSetting* group, size_t index);
const JsAppSetting* js_app_settings_find(const JsAppSettings* instance, const char* path);

JsAppSettingType js_app_setting_get_type(const JsAppSetting* setting);
const char* js_app_setting_get_name(const JsAppSetting* setting);
const char* js_app_setting_get_label(const JsAppSetting* setting);
const char* js_app_setting_get_suffix(const JsAppSetting* setting);
const JsAppSetting* js_app_setting_get_sub_label_setting(const JsAppSetting* setting);
bool js_app_setting_is_visible(const JsAppSetting* setting);

int32_t js_app_setting_get_min(const JsAppSetting* setting);
int32_t js_app_setting_get_max(const JsAppSetting* setting);
int32_t js_app_setting_get_step(const JsAppSetting* setting);

size_t js_app_setting_get_option_count(const JsAppSetting* setting);
const char* js_app_setting_get_option_value(const JsAppSetting* setting, size_t index);
const char* js_app_setting_get_option_label(const JsAppSetting* setting, size_t index);

bool js_app_setting_get_bool(const JsAppSetting* setting);
int32_t js_app_setting_get_int(const JsAppSetting* setting);
const char* js_app_setting_get_string(const JsAppSetting* setting);

/**
 * @name Value setters
 *
 * These change the in-memory value only and return whether the value was
 * accepted. Call js_app_settings_commit() to write them out — the setters are
 * safe to call from a UI callback, a storage write is not.
 * @{
 */
bool js_app_settings_set_bool(JsAppSettings* instance, const JsAppSetting* setting, bool value);
bool js_app_settings_set_int(JsAppSettings* instance, const JsAppSetting* setting, int32_t value);
bool js_app_settings_set_string(
    JsAppSettings* instance,
    const JsAppSetting* setting,
    const char* value);
/** @} */

/**
 * @brief Write out any values changed by the setters.
 *
 * Does nothing and succeeds when there is nothing pending.
 *
 * @param instance loaded settings instance
 * @returns true if the stored values are up to date
 */
bool js_app_settings_commit(JsAppSettings* instance);

const cJSON* js_app_settings_get_schema_json(const JsAppSettings* instance);
const cJSON* js_app_settings_get_config_json(const JsAppSettings* instance);
const cJSON* js_app_settings_get_values_json(const JsAppSettings* instance);

/**
 * @brief Validate and apply a set of values, then persist them.
 *
 * Either the whole update is applied and written, or nothing changes: a failed
 * write rolls the in-memory values back.
 */
bool js_app_settings_update_json(JsAppSettings* instance, const cJSON* values);

#ifdef __cplusplus
}
#endif
