#include "type_bool.h"
#include "common.h"

SETTING_SAVE_DECLARATION(type_bool, json_node, setting, value) {
    bool _value;
    memcpy(&_value, value, sizeof(_value));

    json_write_bool(json_node, setting->name, _value);
    return true;
}

SETTING_LOAD_DECLARATION(type_bool, json_node, setting, value) {
    bool _value;
    if(!json_read_bool(json_node, setting->name, &_value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as bool.", setting->name);
        return false;
    }

    memcpy(value, &_value, sizeof(_value));
    return true;
}

SETTING_RESET_DECLARATION(type_bool, json_node, setting, value) {
    const SettingProviderBoolInterface* interface = setting->interface;

    FURI_LOG_D(
        TAG,
        "Loading default for \"%s\": \"%s\"...",
        setting->name,
        interface->default_value ? "true" : "false");

    json_write_bool(json_node, setting->name, interface->default_value);
    if(value) memcpy(value, &interface->default_value, sizeof(interface->default_value));
}
