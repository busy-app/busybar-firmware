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

    cJSON* inner_json_node = cJSON_CreateObject();
    for(size_t i = 0; i < interface->inner_settings_count; i++) {
        if(!setting_provider_internal_save(inner_json_node, &interface->inner_settings[i], value)) {
            cJSON_Delete(inner_json_node);
            return false;
        }
    }

    if(setting->name) {
        json_write_object(json_node, setting->name, inner_json_node);
    } else {
        cJSON_Delete(json_node->child);
        json_node->child = inner_json_node->child;
        cJSON_free(inner_json_node);
    }

    return true;
}

SETTING_LOAD_DECLARATION(type_struct, json_node, setting, value) {
    const SettingProviderStructInterface* interface = setting->interface;

    furi_check(interface->inner_settings || interface->inner_settings_count == 0);

    cJSON* inner_json_node;
    if(setting->name) {
        if(!json_read_object(json_node, setting->name, &inner_json_node)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as struct.", setting->name ?: "<anonymous>");
            return false;
        }
    } else {
        inner_json_node = json_node;
    }

    for(size_t i = 0; i < interface->inner_settings_count; i++) {
        setting_provider_internal_load(inner_json_node, &interface->inner_settings[i], value);
    }

    if(!is_value_valid(setting, value)) {
        FURI_LOG_W(TAG, "Invalid \"%s\" struct value.", setting->name ?: "<anonymous>");
        return false;
    }

    return true;
}

SETTING_RESET_DECLARATION(type_struct, json_node, setting, value) {
    const SettingProviderStructInterface* interface = setting->interface;

    furi_check(interface->inner_settings || interface->inner_settings_count == 0);

    FURI_LOG_D(TAG, "Loading default for \"%s\" struct...", setting->name ?: "<anonymous>");

    cJSON* inner_json_node;
    if(setting->name) {
        if(!json_read_object(json_node, setting->name, &inner_json_node)) {
            inner_json_node = cJSON_CreateObject();
            json_write_object(json_node, setting->name, inner_json_node);
        }
    } else {
        inner_json_node = json_node;
    }

    for(size_t i = 0; i < interface->inner_settings_count; i++) {
        setting_provider_internal_reset(inner_json_node, &interface->inner_settings[i], value);
    }
}
