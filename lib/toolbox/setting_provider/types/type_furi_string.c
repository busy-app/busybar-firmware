#include "type_furi_string.h"
#include "common.h"

static inline bool is_value_valid(const SettingProviderSetting* setting, const FuriString* value) {
    const SettingProviderFuriStringInterface* interface = setting->interface;
    return !interface->is_valid_callback || interface->is_valid_callback(setting, value);
}

SETTING_SAVE_DECLARATION(type_furi_string, json_node, setting, value) {
    const FuriString* _value = *(const FuriString**)value;

    furi_check(_value);

    if(!is_value_valid(setting, _value)) {
        FURI_LOG_W(
            TAG,
            "Invalid \"%s\" save attempt with value: \"%s\".",
            setting->name,
            furi_string_get_cstr(_value));

        return false;
    }

    json_write_string(json_node, setting->name, furi_string_get_cstr(_value));
    return true;
}

SETTING_LOAD_DECLARATION(type_furi_string, json_node, setting, value) {
    FuriString* _value = *(FuriString**)value;

    furi_check(_value);

    if(!json_read_string(json_node, setting->name, _value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as furi string.", setting->name);
        return false;
    }

    if(!is_value_valid(setting, _value)) {
        FURI_LOG_W(
            TAG, "Invalid \"%s\" value: \"%s\".", setting->name, furi_string_get_cstr(_value));
        return false;
    }

    return true;
}

SETTING_RESET_DECLARATION(type_furi_string, json_node, setting, value) {
    const SettingProviderFuriStringInterface* interface = setting->interface;

    furi_check(interface->default_value);

    FURI_LOG_D(
        TAG, "Loading default for \"%s\": \"%s\"...", setting->name, interface->default_value);

    FuriString* _value = *(FuriString**)value;

    furi_check(_value);

    json_write_string(json_node, setting->name, interface->default_value);
    if(_value) furi_string_set(_value, interface->default_value);
}
