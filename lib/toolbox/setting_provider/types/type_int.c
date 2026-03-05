#include "type_int.h"
#include "common.h"

static inline bool is_value_valid(const SettingProviderSetting* setting, int value) {
    const SettingProviderIntInterface* interface = setting->interface;
    return !interface->is_valid_callback || interface->is_valid_callback(setting, value);
}

SETTING_SAVE_DECLARATION(type_int, json_node, setting, value) {
    int _value = *(const int*)value;
    if(!is_value_valid(setting, _value)) {
        FURI_LOG_W(
            TAG, "Invalid \"%s\" int save attempt with value: \"%d\".", setting->name, _value);
        return false;
    }

    json_write_int(json_node, setting->name, _value);
    return true;
}

SETTING_LOAD_DECLARATION(type_int, json_node, setting, value) {
    int json_value;
    if(!json_read_int(json_node, setting->name, &json_value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as int.", setting->name);
        return SettingLoadResultFailure;
    }

    if(!is_value_valid(setting, json_value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" int value: \"%d\".", setting->name, json_value);
        return SettingLoadResultFailure;
    }

    *(int*)value = json_value;
    return SettingLoadResultOk;
}

SETTING_RESET_DECLARATION(type_int, json_node, setting, value) {
    const SettingProviderIntInterface* interface = setting->interface;

    int default_value = interface->default_value;
    furi_check(is_value_valid(setting, default_value));

    FURI_LOG_T(TAG, "Loading default for \"%s\" int: \"%d\"...", setting->name, default_value);

    json_write_int(json_node, setting->name, default_value);
    if(value) *(int*)value = default_value;
}

SETTING_VALIDATE_DECLARATION(type_int, setting, value) {
    int _value = *(const int*)value;
    return is_value_valid(setting, _value);
}
