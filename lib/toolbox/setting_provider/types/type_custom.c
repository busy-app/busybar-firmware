#include "type_custom.h"
#include "common.h"

SETTING_SAVE_DECLARATION(type_custom, json_node, setting, value) {
    const SettingProviderCustomInterface* interface = setting->interface;

    furi_check(interface->serialize_callback);

    FuriString* _value = furi_string_alloc();
    bool is_save_successful = false;

    do {
        if(!interface->serialize_callback(setting, _value, value)) {
            FURI_LOG_W(TAG, "Invalid \"%s\" value save attempt.", setting->name);
            break;
        }

        json_write_string(json_node, setting->name, furi_string_get_cstr(_value));
        is_save_successful = true;
    } while(false);

    furi_string_free(_value);
    return is_save_successful;
}

SETTING_LOAD_DECLARATION(type_custom, json_node, setting, value) {
    const SettingProviderCustomInterface* interface = setting->interface;

    furi_check(interface->deserialize_callback);

    FuriString* _value = furi_string_alloc();
    bool is_load_successful = false;

    do {
        if(!json_read_string(json_node, setting->name, _value)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as custom.", setting->name);
            break;
        }

        if(!interface->deserialize_callback(setting, value, _value)) {
            FURI_LOG_W(TAG, "Invalid \"%s\" value.", setting->name);
            break;
        }

        is_load_successful = true;
    } while(false);

    furi_string_free(_value);
    return is_load_successful;
}

SETTING_RESET_DECLARATION(type_custom, json_node, setting, value) {
    const SettingProviderCustomInterface* interface = setting->interface;

    furi_check(interface->serialize_callback);
    furi_check(interface->default_value);
    furi_check(interface->default_value_size > 0);

    FuriString* _value = furi_string_alloc();
    interface->serialize_callback(setting, _value, interface->default_value);

    FURI_LOG_D(
        TAG, "Loading default for \"%s\": \"%s\"...", setting->name, furi_string_get_cstr(_value));

    json_write_string(json_node, setting->name, furi_string_get_cstr(_value));
    furi_string_free(_value);

    if(value) memcpy(value, interface->default_value, interface->default_value_size);
}
