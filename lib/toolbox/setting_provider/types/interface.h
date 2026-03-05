#pragma once

#include "../setting_provider_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SettingLoadResultOk,
    SettingLoadResultFallback,
    SettingLoadResultFailure,
} SettingLoadResult;

#define SETTING_RESET_RETURN_TYPE void
#define SETTING_RESET_PARAMETERS(json_node, setting, value) \
    (cJSON * json_node, const SettingProviderSetting* setting, void* value)
#define SETTING_RESET_POINTER(name) SETTING_RESET_RETURN_TYPE(*name) SETTING_RESET_PARAMETERS(, , )
#define SETTING_RESET(type)         setting_provider_internal_reset_##type
#define SETTING_RESET_DECLARATION(type, json_node, setting, value) \
    SETTING_RESET_RETURN_TYPE                                      \
    SETTING_RESET(type)                                            \
    SETTING_RESET_PARAMETERS(json_node, setting, value)

#define SETTING_LOAD_RETURN_TYPE SettingLoadResult
#define SETTING_LOAD_PARAMETERS(json_node, setting, value) \
    (cJSON * json_node, const SettingProviderSetting* setting, void* value)
#define SETTING_LOAD_POINTER(name) SETTING_LOAD_RETURN_TYPE(*name) SETTING_LOAD_PARAMETERS(, , )
#define SETTING_LOAD(type)         setting_provider_internal_load_##type
#define SETTING_LOAD_DECLARATION(type, json_node, setting, value) \
    SETTING_LOAD_RETURN_TYPE                                      \
    SETTING_LOAD(type)                                            \
    SETTING_LOAD_PARAMETERS(json_node, setting, value)

#define SETTING_SAVE_RETURN_TYPE bool
#define SETTING_SAVE_PARAMETERS(json_node, setting, value) \
    (cJSON * json_node, const SettingProviderSetting* setting, const void* value)
#define SETTING_SAVE_POINTER(name) SETTING_SAVE_RETURN_TYPE(*name) SETTING_SAVE_PARAMETERS(, , )
#define SETTING_SAVE(type)         setting_provider_internal_save_##type
#define SETTING_SAVE_DECLARATION(type, json_node, setting, value) \
    SETTING_SAVE_RETURN_TYPE                                      \
    SETTING_SAVE(type)                                            \
    SETTING_SAVE_PARAMETERS(json_node, setting, value)

#define SETTING_VALIDATE_RETURN_TYPE bool
#define SETTING_VALIDATE_PARAMETERS(setting, value) \
    (const SettingProviderSetting* setting, const void* value)
#define SETTING_VALIDATE_POINTER(name) \
    SETTING_VALIDATE_RETURN_TYPE(*name) SETTING_VALIDATE_PARAMETERS(, )
#define SETTING_VALIDATE(type) setting_provider_internal_validate_##type
#define SETTING_VALIDATE_DECLARATION(type, setting, value) \
    SETTING_VALIDATE_RETURN_TYPE                           \
    SETTING_VALIDATE(type)                                 \
    SETTING_VALIDATE_PARAMETERS(setting, value)

void setting_provider_internal_reset(
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value);

bool setting_provider_internal_load(
    cJSON* json_node,
    const SettingProviderSetting* setting,
    void* value);

bool setting_provider_internal_save(
    cJSON* json_node,
    const SettingProviderSetting* setting,
    const void* value);

bool setting_provider_internal_validate(const SettingProviderSetting* setting, const void* value);

#ifdef __cplusplus
}
#endif
