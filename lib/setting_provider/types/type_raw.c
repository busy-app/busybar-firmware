#include "type_raw.h"
#include "common.h"

SETTING_SAVE_DECLARATION(type_raw, json_node, setting, value) {
    const SettingProviderRawInterface* interface = setting->interface;
    furi_check(interface->serialize_callback);

    cJSON* _json_node = cJSON_CreateObject();
    if(!interface->serialize_callback(setting, value, _json_node)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" raw value save attempt.", setting->name);
        cJSON_Delete(_json_node);
        return false;
    }

    json_write_any(json_node, setting->name, _json_node);
    return true;
}

SETTING_LOAD_DECLARATION(type_raw, json_node, setting, value) {
    const SettingProviderRawInterface* interface = setting->interface;
    furi_check(interface->deserialize_callback);

    cJSON* _json_node;
    if(!json_read_any(json_node, setting->name, &_json_node)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as raw.", setting->name);
        return SettingLoadResultFailure;
    }

    if(!interface->deserialize_callback(setting, _json_node, value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" raw value.", setting->name);
        return SettingLoadResultFailure;
    }

    return SettingLoadResultOk;
}

SETTING_RESET_DECLARATION(type_raw, json_node, setting, value) {
    const SettingProviderRawInterface* interface = setting->interface;
    furi_check(interface->serialize_callback);
    furi_check(interface->default_value_size > 0);

    const void* default_value = interface->default_value;
    furi_check(default_value);

    cJSON* _json_node = cJSON_CreateObject();
    furi_check(interface->serialize_callback(setting, default_value, _json_node));

    FURI_LOG_T(TAG, "Loading default for \"%s\" raw...", setting->name);

    json_write_any(json_node, setting->name, _json_node);
    if(value) memcpy(value, default_value, interface->default_value_size);
}

SETTING_VALIDATE_DECLARATION(type_raw, setting, value) {
    const SettingProviderRawInterface* interface = setting->interface;
    return !interface->is_valid_callback || interface->is_valid_callback(setting, value);
}
