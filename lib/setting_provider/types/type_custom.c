#include "type_custom.h"
#include "common.h"

SETTING_SAVE_DECLARATION(type_custom, json_node, setting, value) {
    const SettingProviderCustomInterface* interface = setting->interface;
    furi_check(interface->serialize_callback);

    FuriString* string_value = furi_string_alloc();
    if(!interface->serialize_callback(setting, value, string_value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" custom value save attempt.", setting->name);
        furi_string_free(string_value);
        return false;
    }

    json_write_string(json_node, setting->name, furi_string_get_cstr(string_value));
    furi_string_free(string_value);

    return true;
}

SETTING_LOAD_DECLARATION(type_custom, json_node, setting, value) {
    const SettingProviderCustomInterface* interface = setting->interface;
    furi_check(interface->deserialize_callback);

    const char* json_string_value;
    if(!json_read_string(json_node, setting->name, &json_string_value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as custom.", setting->name);
        return SettingLoadResultFailure;
    }

    if(!interface->deserialize_callback(setting, json_string_value, value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" custom value.", setting->name);
        return SettingLoadResultFailure;
    }

    return SettingLoadResultOk;
}

SETTING_RESET_DECLARATION(type_custom, json_node, setting, value) {
    const SettingProviderCustomInterface* interface = setting->interface;
    furi_check(interface->default_value_size > 0);
    furi_check(interface->serialize_callback);

    uint8_t default_value[interface->default_value_size];
    bool use_static_default = !!interface->default_value;
    bool use_dynamic_default = !!interface->default_value_callback;

    if(!(use_static_default ^ use_dynamic_default)) {
        furi_crash(
            "Exactly one of `default_value` or `default_value_callback` must be non-NULL in `SettingProviderCustomInterface`.");
    }

    if(use_static_default) memcpy(default_value, interface->default_value, sizeof(default_value));
    if(use_dynamic_default) interface->default_value_callback(default_value);

    FuriString* default_string_value = furi_string_alloc();
    furi_check(interface->serialize_callback(setting, default_value, default_string_value));

    FURI_LOG_T(
        TAG,
        "Loading default for \"%s\" custom: \"%s\"...",
        setting->name,
        furi_string_get_cstr(default_string_value));

    json_write_string(json_node, setting->name, furi_string_get_cstr(default_string_value));
    furi_string_free(default_string_value);

    if(value) memcpy(value, default_value, interface->default_value_size);
}

SETTING_VALIDATE_DECLARATION(type_custom, setting, value) {
    const SettingProviderCustomInterface* interface = setting->interface;
    return !interface->is_valid_callback || interface->is_valid_callback(setting, value);
}
