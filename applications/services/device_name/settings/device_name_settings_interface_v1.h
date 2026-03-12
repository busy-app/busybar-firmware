#pragma once

#include <setting_provider.h>

#define DEVICE_NAME_MAX_LENGTH (20U)

typedef enum {
    DeviceNameSettingsV1IdxName,
    DeviceNameSettingsV1IdxMax,
} DeviceNameSettingsV1Idx;

typedef struct {
    char name[DEVICE_NAME_MAX_LENGTH];
} DeviceNameSettingsV1;

extern const SettingProviderSetting device_name_settings_v1[];
extern const SettingProviderSetting device_name_settings_v1_root;

void device_name_settings_v1_init(DeviceNameSettingsV1* settings_v1);
