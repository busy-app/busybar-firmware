#include "type_bool.h"
#include "common.h"

SETTING_SAVE_DECLARATION(type_bool, json_node, setting, value) {
    bool _value = *(const bool*)value;
    json_write_bool(json_node, setting->name, _value);
    return true;
}

SETTING_LOAD_DECLARATION(type_bool, json_node, setting, value) {
    bool json_value;
    if(!json_read_bool(json_node, setting->name, &json_value)) {
        FURI_LOG_W(TAG, "Failed to load \"%s\" as bool.", setting->name);
        return SettingLoadResultFailure;
    }

    *(bool*)value = json_value;
    return SettingLoadResultOk;
}

SETTING_RESET_DECLARATION(type_bool, json_node, setting, value) {
    const SettingProviderBoolInterface* interface = setting->interface;
    bool default_value = interface->default_value;

    FURI_LOG_T(
        TAG,
        "Loading default for \"%s\" bool: \"%s\"...",
        setting->name,
        default_value ? "true" : "false");

    json_write_bool(json_node, setting->name, default_value);
    if(value) *(bool*)value = default_value;
}

SETTING_VALIDATE_DECLARATION(type_bool, setting, value) {
    UNUSED(setting);
    UNUSED(value);

    return true;
}
