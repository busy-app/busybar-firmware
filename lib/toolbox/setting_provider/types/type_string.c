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
    const SettingProviderStringInterface* interface = setting->interface;

    FuriString* value_buffer = furi_string_alloc();
    bool is_load_successful = false;

    do {
        if(!json_read_string(json_node, setting->name, value_buffer)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as string.", setting->name);
            break;
        }

        const char* _value = furi_string_get_cstr(value_buffer);
        if(!is_value_valid(setting, _value)) {
            FURI_LOG_W(TAG, "Invalid \"%s\" string value: \"%s\".", setting->name, _value);
            break;
        }

        strncpy(value, _value, interface->max_size);
        is_load_successful = true;
    } while(false);

    furi_string_free(value_buffer);
    return is_load_successful;
}

SETTING_RESET_DECLARATION(type_string, json_node, setting, value) {
    const SettingProviderStringInterface* interface = setting->interface;

    furi_check(interface->default_value);
    furi_check(strlen(interface->default_value) < interface->max_size);

    FURI_LOG_D(
        TAG,
        "Loading default for \"%s\" string: \"%s\"...",
        setting->name,
        interface->default_value);

    json_write_string(json_node, setting->name, interface->default_value);
    if(value) strncpy(value, interface->default_value, interface->max_size);
}
