#include "type_union.h"
#include "type_enum.h"
#include "common.h"

SETTING_SAVE_DECLARATION(type_union, json_node, setting, value) {
    const SettingProviderUnionInterface* interface = setting->interface;

    furi_check(interface->tag_setting);
    furi_check(interface->tag_setting->type == SettingProviderSettingTypeEnum);
    furi_check(interface->tag_setting->interface);
    furi_check(interface->inner_settings);

    int tag;
    memcpy(&tag, value + interface->tag_setting->field_offset, sizeof(tag));

    cJSON* _json_node = cJSON_CreateObject();
    if(!SETTING_SAVE(type_enum)(_json_node, interface->tag_setting, &tag)) {
        cJSON_Delete(_json_node);
        return false;
    }

    const SettingProviderSetting* inner_setting = &interface->inner_settings[tag];
    if(!setting_provider_internal_save(_json_node, inner_setting, value)) {
        cJSON_Delete(_json_node);
        return false;
    }

    if(setting->name) {
        json_write_object(json_node, setting->name, _json_node);
    } else {
        for(cJSON* json_item = _json_node->child; json_item;) {
            cJSON* next_json_item = json_item->next;
            json_write_object(json_node, json_item->string, json_item);
            json_item = next_json_item;
        }

        cJSON_free(_json_node);
    }

    return true;
}

SETTING_LOAD_DECLARATION(type_union, json_node, setting, value) {
    const SettingProviderUnionInterface* interface = setting->interface;

    furi_check(interface->tag_setting);
    furi_check(interface->tag_setting->type == SettingProviderSettingTypeEnum);
    furi_check(interface->tag_setting->interface);
    furi_check(interface->inner_settings);

    cJSON* _json_node;
    if(setting->name) {
        if(!json_read_object(json_node, setting->name, &_json_node)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as union.", setting->name);
            return false;
        }
    } else {
        _json_node = json_node;
    }

    int tag;
    if(!SETTING_LOAD(type_enum)(_json_node, interface->tag_setting, &tag)) return false;

    const SettingProviderSetting* inner_setting = &interface->inner_settings[tag];
    if(!setting_provider_internal_load(_json_node, inner_setting, value)) return false;

    memcpy(value + interface->tag_setting->field_offset, &tag, sizeof(tag));

    return true;
}

SETTING_RESET_DECLARATION(type_union, json_node, setting, value) {
    const SettingProviderUnionInterface* interface = setting->interface;

    furi_check(interface->tag_setting);
    furi_check(interface->tag_setting->type == SettingProviderSettingTypeEnum);
    furi_check(interface->tag_setting->interface);
    furi_check(interface->inner_settings);

    FURI_LOG_D(TAG, "Loading default for \"%s\" union...", setting->name ?: "<anonymous>");

    cJSON* _json_node;
    if(setting->name) {
        if(!json_read_object(json_node, setting->name, &_json_node)) {
            _json_node = cJSON_CreateObject();
            json_write_object(json_node, setting->name, _json_node);
        }
    } else {
        _json_node = json_node;
    }

    int tag;
    SETTING_RESET(type_enum)(_json_node, interface->tag_setting, &tag);

    const SettingProviderSetting* inner_setting = &interface->inner_settings[tag];
    setting_provider_internal_reset(_json_node, inner_setting, value);

    if(value) memcpy(value + interface->tag_setting->field_offset, &tag, sizeof(tag));
}
