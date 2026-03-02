#include "type_float.h"
#include "common.h"

static inline bool is_value_valid(const SettingProviderSetting* setting, float value) {
    const SettingProviderFloatInterface* interface = setting->interface;
    return !interface->is_valid_callback || interface->is_valid_callback(setting, value);
}

SETTING_SAVE_DECLARATION(type_float, json_node, setting, value) {
    float _value;
    memcpy(&_value, value, sizeof(_value));
    if(!is_value_valid(setting, _value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" save attempt with value: \"%f\".", setting->name, _value);
        return false;
    }

    json_write_float(json_node, setting->name, _value);
    return true;
}

SETTING_LOAD_DECLARATION(type_float, json_node, setting, value) {
    float _value;
    if(!json_read_float(json_node, setting->name, &_value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as float.", setting->name);
        return false;
    }

    if(!is_value_valid(setting, _value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" value: \"%f\".", setting->name, _value);
        return false;
    }

    memcpy(value, &_value, sizeof(_value));
    return true;
}

SETTING_RESET_DECLARATION(type_float, json_node, setting, value) {
    const SettingProviderFloatInterface* interface = setting->interface;

    FURI_LOG_D(
        TAG, "Loading default for \"%s\": \"%f\"...", setting->name, interface->default_value);

    json_write_float(json_node, setting->name, interface->default_value);
    if(value) memcpy(value, &interface->default_value, sizeof(interface->default_value));
}
