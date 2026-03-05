#include "type_union.h"
#include "type_enum.h"
#include "common.h"

SETTING_SAVE_DECLARATION(type_union, json_node, setting, value) {
    const SettingProviderUnionInterface* interface = setting->interface;
    furi_check(interface->inner_settings);

    const SettingProviderSetting* tag_setting = interface->tag_setting;
    furi_check(tag_setting);
    furi_check(tag_setting->type == SettingProviderSettingTypeEnum);
    furi_check(tag_setting->interface);

    const SettingProviderEnumInterface* tag_interface = tag_setting->interface;
    furi_check(tag_interface->type_size > 0 && tag_interface->type_size <= sizeof(int));

    int tag = 0;
    memcpy(&tag, value + tag_setting->field_offset, tag_interface->type_size);
    cJSON* _json_node = cJSON_CreateObject();
    if(!SETTING_SAVE(type_enum)(_json_node, tag_setting, &tag)) {
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
            json_write_object(
                json_node, json_item->string, cJSON_DetachItemViaPointer(_json_node, json_item));
            json_item = next_json_item;
        }

        cJSON_Delete(_json_node);
    }

    return true;
}

SETTING_LOAD_DECLARATION(type_union, json_node, setting, value) {
    const SettingProviderUnionInterface* interface = setting->interface;
    furi_check(interface->inner_settings);

    const SettingProviderSetting* tag_setting = interface->tag_setting;
    furi_check(tag_setting);
    furi_check(tag_setting->type == SettingProviderSettingTypeEnum);
    furi_check(tag_setting->interface);

    const SettingProviderEnumInterface* tag_interface = tag_setting->interface;
    furi_check(tag_interface->type_size > 0 && tag_interface->type_size <= sizeof(int));

    cJSON* _json_node;
    if(setting->name) {
        if(!json_read_object(json_node, setting->name, &_json_node)) {
            FURI_LOG_W(TAG, "Failed to load \"%s\" as union.", setting->name);
            return SettingLoadResultFailure;
        }
    } else {
        _json_node = json_node;
    }

    int tag = 0;
    if(SETTING_LOAD(type_enum)(_json_node, tag_setting, &tag) != SettingLoadResultOk) {
        return SettingLoadResultFailure;
    }

    const SettingProviderSetting* inner_setting = &interface->inner_settings[tag];
    bool was_inner_fallback_invoked =
        setting_provider_internal_load(_json_node, inner_setting, value);

    memcpy(value + tag_setting->field_offset, &tag, tag_interface->type_size);
    return was_inner_fallback_invoked ? SettingLoadResultFallback : SettingLoadResultOk;
}

SETTING_RESET_DECLARATION(type_union, json_node, setting, value) {
    const SettingProviderUnionInterface* interface = setting->interface;
    furi_check(interface->inner_settings);

    const SettingProviderSetting* tag_setting = interface->tag_setting;
    furi_check(tag_setting);
    furi_check(tag_setting->type == SettingProviderSettingTypeEnum);
    furi_check(tag_setting->interface);

    const SettingProviderEnumInterface* tag_interface = tag_setting->interface;
    furi_check(tag_interface->type_size > 0 && tag_interface->type_size <= sizeof(int));

    FURI_LOG_T(TAG, "Loading default for \"%s\" union...", setting->name ?: "<anonymous>");

    cJSON* _json_node;
    if(setting->name) {
        _json_node = cJSON_CreateObject();
        json_write_object(json_node, setting->name, _json_node);
    } else {
        _json_node = json_node;
    }

    int tag = 0;
    SETTING_RESET(type_enum)(_json_node, tag_setting, &tag);

    const SettingProviderSetting* inner_setting = &interface->inner_settings[tag];
    setting_provider_internal_reset(_json_node, inner_setting, value);

    if(value) memcpy(value + tag_setting->field_offset, &tag, tag_interface->type_size);
}

SETTING_VALIDATE_DECLARATION(type_union, setting, value) {
    const SettingProviderUnionInterface* interface = setting->interface;
    furi_check(interface->inner_settings);

    const SettingProviderSetting* tag_setting = interface->tag_setting;
    furi_check(tag_setting);
    furi_check(tag_setting->type == SettingProviderSettingTypeEnum);
    furi_check(tag_setting->interface);

    const SettingProviderEnumInterface* tag_interface = tag_setting->interface;
    furi_check(tag_interface->type_size > 0 && tag_interface->type_size <= sizeof(int));

    int tag = 0;
    memcpy(&tag, value + tag_setting->field_offset, tag_interface->type_size);
    if(!SETTING_VALIDATE(type_enum)(tag_setting, &tag)) return false;

    const SettingProviderSetting* inner_setting = &interface->inner_settings[tag];
    return setting_provider_internal_validate(inner_setting, value);
}
