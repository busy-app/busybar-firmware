#include "type_enum.h"
#include "common.h"

static inline bool is_value_valid(const SettingProviderSetting* setting, int value) {
    const SettingProviderEnumInterface* interface = setting->interface;
    return value >= 0 && value < interface->string_map_length;
}

SETTING_SAVE_DECLARATION(type_enum, json_node, setting, value) {
    const SettingProviderEnumInterface* interface = setting->interface;
    furi_check(interface->type_size > 0 && interface->type_size <= sizeof(int));
    furi_check(interface->string_map);
    furi_check(interface->string_map_length > 0);

    int _value = 0;
    memcpy(&_value, value, interface->type_size);
    if(!is_value_valid(setting, _value)) {
        FURI_LOG_W(
            TAG, "Invalid \"%s\" enum save attempt with value: \"%d\".", setting->name, _value);
        return false;
    }

    const char* string_value = interface->string_map[_value];
    furi_check(string_value);

    json_write_string(json_node, setting->name, string_value);
    return true;
}

SETTING_LOAD_DECLARATION(type_enum, json_node, setting, value) {
    const SettingProviderEnumInterface* interface = setting->interface;
    furi_check(interface->type_size > 0 && interface->type_size <= sizeof(int));
    furi_check(interface->string_map);
    furi_check(interface->string_map_length > 0);

    const char* json_string_value;
    if(!json_read_string(json_node, setting->name, &json_string_value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as enum.", setting->name);
        return SettingLoadResultFailure;
    }

    for(int json_value = 0; json_value < interface->string_map_length; json_value++) {
        const char* string_value = interface->string_map[json_value];
        furi_check(string_value);

        if(strcmp(json_string_value, string_value) == 0) {
            memcpy(value, &json_value, interface->type_size);
            return SettingLoadResultOk;
        }
    }

    FURI_LOG_W(TAG, "Invalid \"%s\" enum value: \"%s\".", setting->name, json_string_value);

    return SettingLoadResultFailure;
}

SETTING_RESET_DECLARATION(type_enum, json_node, setting, value) {
    const SettingProviderEnumInterface* interface = setting->interface;
    furi_check(interface->type_size > 0 && interface->type_size <= sizeof(int));
    furi_check(interface->string_map);
    furi_check(interface->string_map_length > 0);
    furi_check(interface->default_value);

    int default_value = 0;
    memcpy(&default_value, interface->default_value, interface->type_size);
    furi_check(is_value_valid(setting, default_value));

    const char* default_string_value = interface->string_map[default_value];
    furi_check(default_string_value);

    FURI_LOG_T(
        TAG, "Loading default for \"%s\" enum: \"%s\"...", setting->name, default_string_value);

    json_write_string(json_node, setting->name, default_string_value);
    if(value) memcpy(value, &default_value, interface->type_size);
}

SETTING_VALIDATE_DECLARATION(type_enum, setting, value) {
    const SettingProviderEnumInterface* interface = setting->interface;
    furi_check(interface->type_size > 0 && interface->type_size <= sizeof(int));

    int _value = 0;
    memcpy(&_value, value, interface->type_size);
    return is_value_valid(setting, _value);
}
