#pragma once
#include <stddef.h>
typedef struct SettingProvider SettingProvider;
typedef struct {
    int unused;
} SettingProviderSetting;
SettingProvider* setting_provider_alloc(const char*, unsigned, void*, size_t);
void setting_provider_load(SettingProvider*, const SettingProviderSetting*, void*);
void setting_provider_save(SettingProvider*, const SettingProviderSetting*, const void*);
