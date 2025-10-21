#pragma once

#include <furi.h>

#include <cjson/cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SettingProvider SettingProvider;

typedef enum {
    SettingProviderSettingTypeBool,
    SettingProviderSettingTypeInt,
    SettingProviderSettingTypeFloat,
    SettingProviderSettingTypeString,
    /* SettingProviderSettingTypeCustom, */

    SettingProviderSettingTypesCount
} SettingProviderSettingType;

typedef struct {
    bool default_value;
} SettingProviderBoolInterface;

typedef struct {
    int default_value;
    bool (*is_valid_callback)(int value);
} SettingProviderIntInterface;

typedef struct {
    float default_value;
    bool (*is_valid_callback)(float value);
} SettingProviderFloatInterface;

typedef struct {
    const char* default_value;
    bool (*is_valid_callback)(const FuriString* value);
} SettingProviderStringInterface;

/*
typedef struct {
    const void* default_value;
    bool (*is_valid_callback)(const void* value);
    void (*deep_copy_callback)(const void* from, void* to);
    void (*serialize_callback)(const void* value, cJSON* json);
    void (*deserialize_callback)(const cJSON* json, void* value);
} SettingProviderCustomInterface;
*/

typedef struct {
    const char* name;
    const void* interface;
    const void* context;
    SettingProviderSettingType type;
} SettingProviderSetting;

SettingProvider* setting_provider_alloc(const char* file_path);

void setting_provider_free(SettingProvider* provider);

bool setting_provider_open(SettingProvider* provider);

void setting_provider_load(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value);

void setting_provider_save(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value);

void setting_provider_drop(SettingProvider* provider, const SettingProviderSetting* setting);

#ifdef __cplusplus
}
#endif
