#include "type_struct.h"
#include "common.h"

static inline bool is_value_valid(const SettingProviderSetting* setting, const void* value) {
    const SettingProviderStructInterface* interface = setting->interface;
    return !interface->is_valid_callback || interface->is_valid_callback(setting, value);
}

SETTING_SAVE_DECLARATION(type_struct, json_node, setting, value) {
    const SettingProviderStructInterface* interface = setting->interface;
    furi_check(interface->inner_settings || interface->inner_settings_count == 0);

    if(!is_value_valid(setting, value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" struct save attempt.", setting->name ?: "<anonymous>");
        return false;
    }

    cJSON* _json_node = cJSON_CreateObject();
    for(size_t i = 0; i < interface->inner_settings_count; i++) {
        if(!setting_provider_internal_save(_json_node, &interface->inner_settings[i], value)) {
            cJSON_Delete(_json_node);
            return false;
        }
    }

    if(setting->name) {
        json_write_object(json_node, setting->name, _json_node);
    } else {
        for(cJSON* json_item = _json_node->child; json_item;) {
            cJSON* next_json_item = json_item->next;
            json_write_object(
                json_node, json_item->string, cJSON_DetachItemViaPointer(_json_node, json_item));
            json_item = next_json_item;
        }

        cJSON_Delete(_json_node);
    }

    return true;
}

SETTING_LOAD_DECLARATION(type_struct, json_node, setting, value) {
    const SettingProviderStructInterface* interface = setting->interface;
    furi_check(interface->inner_settings || interface->inner_settings_count == 0);

    cJSON* _json_node;
    if(setting->name) {
        if(!json_read_object(json_node, setting->name, &_json_node)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as struct.", setting->name ?: "<anonymous>");
            return SettingLoadResultFailure;
        }
    } else {
        _json_node = json_node;
    }

    bool was_inner_fallback_invoked = false;
    for(size_t i = 0; i < interface->inner_settings_count; i++) {
        was_inner_fallback_invoked |=
            setting_provider_internal_load(_json_node, &interface->inner_settings[i], value);
    }

    if(!is_value_valid(setting, value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" struct value.", setting->name ?: "<anonymous>");
        return SettingLoadResultFailure;
    }

    return was_inner_fallback_invoked ? SettingLoadResultFallback : SettingLoadResultOk;
}

SETTING_RESET_DECLARATION(type_struct, json_node, setting, value) {
    const SettingProviderStructInterface* interface = setting->interface;
    furi_check(interface->inner_settings || interface->inner_settings_count == 0);

    FURI_LOG_T(TAG, "Loading default for \"%s\" struct...", setting->name ?: "<anonymous>");

    cJSON* _json_node;
    if(setting->name) {
        _json_node = cJSON_CreateObject();
        json_write_object(json_node, setting->name, _json_node);
    } else {
        _json_node = json_node;
    }

    for(size_t i = 0; i < interface->inner_settings_count; i++) {
        setting_provider_internal_reset(_json_node, &interface->inner_settings[i], value);
    }
}

SETTING_VALIDATE_DECLARATION(type_struct, setting, value) {
    const SettingProviderStructInterface* interface = setting->interface;
    furi_check(interface->inner_settings || interface->inner_settings_count == 0);

    for(size_t i = 0; i < interface->inner_settings_count; i++) {
        if(!setting_provider_internal_validate(&interface->inner_settings[i], value)) {
            return false;
        }
    }

    return is_value_valid(setting, value);
}
