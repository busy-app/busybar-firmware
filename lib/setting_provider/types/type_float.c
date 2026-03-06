#include "type_float.h"
#include "common.h"

static inline bool is_value_valid(const SettingProviderSetting* setting, float value) {
    const SettingProviderFloatInterface* interface = setting->interface;
    return !interface->is_valid_callback || interface->is_valid_callback(setting, value);
}

SETTING_SAVE_DECLARATION(type_float, json_node, setting, value) {
    float _value = *(const float*)value;
    if(!is_value_valid(setting, _value)) {
        FURI_LOG_W(
            TAG, "Invalid \"%s\" float save attempt with value: \"%.2f\".", setting->name, _value);
        return false;
    }

    json_write_float(json_node, setting->name, _value);
    return true;
}

SETTING_LOAD_DECLARATION(type_float, json_node, setting, value) {
    float json_value;
    if(!json_read_float(json_node, setting->name, &json_value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as float.", setting->name);
        return SettingLoadResultFailure;
    }

    if(!is_value_valid(setting, json_value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" float value: \"%.2f\".", setting->name, json_value);
        return SettingLoadResultFailure;
    }

    *(float*)value = json_value;
    return SettingLoadResultOk;
}

SETTING_RESET_DECLARATION(type_float, json_node, setting, value) {
    const SettingProviderFloatInterface* interface = setting->interface;

    float default_value = interface->default_value;
    furi_check(is_value_valid(setting, default_value));

    FURI_LOG_T(TAG, "Loading default for \"%s\" float: \"%.2f\"...", setting->name, default_value);

    json_write_float(json_node, setting->name, default_value);
    if(value) *(float*)value = default_value;
}

SETTING_VALIDATE_DECLARATION(type_float, setting, value) {
    float _value = *(const float*)value;
    return is_value_valid(setting, _value);
}
