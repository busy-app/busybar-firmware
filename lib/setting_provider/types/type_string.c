#include "type_string.h"
#include "common.h"

static inline bool is_value_valid(const SettingProviderSetting* setting, const char* value) {
    const SettingProviderStringInterface* interface = setting->interface;
    return strlen(value) < interface->max_size &&
           (!interface->is_valid_callback || interface->is_valid_callback(setting, value));
}

SETTING_SAVE_DECLARATION(type_string, json_node, setting, value) {
    const char* _value = value;

    if(!is_value_valid(setting, _value)) {
        FURI_LOG_W(
            TAG, "Invalid \"%s\" string save attempt with value: \"%s\".", setting->name, _value);
        return false;
    }

    json_write_string(json_node, setting->name, _value);
    return true;
}

SETTING_LOAD_DECLARATION(type_string, json_node, setting, value) {
    const char* json_value;
    if(!json_read_string(json_node, setting->name, &json_value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as string.", setting->name);
        return SettingLoadResultFailure;
    }

    if(!is_value_valid(setting, json_value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" string value: \"%s\".", setting->name, json_value);
        return SettingLoadResultFailure;
    }

    strcpy(value, json_value);
    return SettingLoadResultOk;
}

SETTING_RESET_DECLARATION(type_string, json_node, setting, value) {
    const SettingProviderStringInterface* interface = setting->interface;

    const char* default_value = interface->default_value;
    furi_check(default_value);
    furi_check(is_value_valid(setting, default_value));

    FURI_LOG_T(TAG, "Loading default for \"%s\" string: \"%s\"...", setting->name, default_value);

    json_write_string(json_node, setting->name, default_value);
    if(value) strcpy(value, default_value);
}

SETTING_VALIDATE_DECLARATION(type_string, setting, value) {
    return is_value_valid(setting, value);
}
