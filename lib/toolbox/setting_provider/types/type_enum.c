#include "type_enum.h"
#include "common.h"

SETTING_SAVE_DECLARATION(type_enum, json_node, setting, value) {
    const SettingProviderEnumInterface* interface = setting->interface;

    furi_check(interface->string_map);
    furi_check(interface->string_map_length > 0);

    int _value;
    memcpy(&_value, value, sizeof(_value));
    if(_value < 0 || _value >= interface->string_map_length) {
        FURI_LOG_W(
            TAG, "Invalid \"%s\" enum save attempt with value: \"%d\".", setting->name, _value);
        return false;
    }

    const char* value_string = interface->string_map[_value];

    furi_check(value_string);

    json_write_string(json_node, setting->name, value_string);
    return true;
}

SETTING_LOAD_DECLARATION(type_enum, json_node, setting, value) {
    const SettingProviderEnumInterface* interface = setting->interface;

    furi_check(interface->string_map);
    furi_check(interface->string_map_length > 0);

    FuriString* value_string = furi_string_alloc();
    if(!json_read_string(json_node, setting->name, value_string)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as enum.", setting->name);
        return false;
    }

    for(int _value = 0; _value < interface->string_map_length; _value++) {
        const char* _value_string = interface->string_map[_value];

        furi_check(_value_string);

        if(furi_string_cmp(value_string, _value_string)) {
            memcpy(value, &_value, sizeof(_value));
            furi_string_free(value_string);
            return true;
        }
    }

    FURI_LOG_W(
        TAG,
        "Invalid \"%s\" enum value: \"%s\".",
        setting->name,
        furi_string_get_cstr(value_string));

    furi_string_free(value_string);
    return false;
}

SETTING_RESET_DECLARATION(type_enum, json_node, setting, value) {
    const SettingProviderEnumInterface* interface = setting->interface;

    furi_check(interface->string_map);
    furi_check(interface->string_map_length > 0);
    furi_check(interface->default_value < interface->string_map_length);

    const char* value_string = interface->string_map[interface->default_value];

    furi_check(value_string);

    FURI_LOG_D(TAG, "Loading default for \"%s\" enum: \"%s\"...", setting->name, value_string);

    json_write_string(json_node, setting->name, value_string);
    if(value) memcpy(value, &interface->default_value, sizeof(interface->default_value));
}
