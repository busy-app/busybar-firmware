#include "interface.h"
#include "type_bool.h"
#include "type_int.h"
#include "type_float.h"
#include "type_string.h"
#include "type_custom.h"
#include "type_enum.h"
#include "type_union.h"
#include "type_struct.h"

#define IS_VALID_SETTING_TYPE(type) ((type) < SettingProviderSettingTypesCount)

#define SETTING_TYPE_ACTIONS(type)                                                            \
    (const SettingTypeActions) {                                                              \
        .save = SETTING_SAVE(type), .load = SETTING_LOAD(type), .reset = SETTING_RESET(type), \
    }

typedef struct {
    SETTING_SAVE_POINTER(save);
    SETTING_LOAD_POINTER(load);
    SETTING_RESET_POINTER(reset);
} SettingTypeActions;

static const SettingTypeActions* const setting_type_actions[] = {
    [SettingProviderSettingTypeBool] = &SETTING_TYPE_ACTIONS(type_bool),
    [SettingProviderSettingTypeInt] = &SETTING_TYPE_ACTIONS(type_int),
    [SettingProviderSettingTypeFloat] = &SETTING_TYPE_ACTIONS(type_float),
    [SettingProviderSettingTypeString] = &SETTING_TYPE_ACTIONS(type_string),
    [SettingProviderSettingTypeCustom] = &SETTING_TYPE_ACTIONS(type_custom),
    [SettingProviderSettingTypeEnum] = &SETTING_TYPE_ACTIONS(type_enum),
    [SettingProviderSettingTypeUnion] = &SETTING_TYPE_ACTIONS(type_union),
    [SettingProviderSettingTypeStruct] = &SETTING_TYPE_ACTIONS(type_struct),
};

static_assert(COUNT_OF(setting_type_actions) == SettingProviderSettingTypesCount);

void setting_provider_internal_reset(
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(setting);
    furi_check(setting->interface);
    furi_check(IS_VALID_SETTING_TYPE(setting->type));
    furi_check(
        setting->name || setting->type == SettingProviderSettingTypeStruct ||
        setting->type == SettingProviderSettingTypeEnum);

    const SettingTypeActions* type_actions = setting_type_actions[setting->type];
    type_actions->reset(json_node, setting, value ? value + setting->field_offset : NULL);
}

bool setting_provider_internal_load(
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value) {
    furi_check(setting);
    furi_check(setting->interface);
    furi_check(IS_VALID_SETTING_TYPE(setting->type));
    furi_check(
        setting->name || setting->type == SettingProviderSettingTypeStruct ||
        setting->type == SettingProviderSettingTypeEnum);

    const SettingTypeActions* type_actions = setting_type_actions[setting->type];
    void* _value = value + setting->field_offset;
    return (type_actions->load(json_node, setting, _value)) ?
               false :
               (type_actions->reset(json_node, setting, _value), true);
}

bool setting_provider_internal_save(
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value) {
    furi_check(setting);
    furi_check(setting->interface);
    furi_check(IS_VALID_SETTING_TYPE(setting->type));
    furi_check(
        setting->name || setting->type == SettingProviderSettingTypeStruct ||
        setting->type == SettingProviderSettingTypeEnum);

    const SettingTypeActions* type_actions = setting_type_actions[setting->type];
    return type_actions->save(json_node, setting, value + setting->field_offset);
}
