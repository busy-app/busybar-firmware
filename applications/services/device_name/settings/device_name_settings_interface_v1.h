#pragma once

#include <setting_provider.h>
#include "../device_name_common.h"

typedef enum {
    DeviceNameSettingsV1IdxName,
    DeviceNameSettingsV1IdxMax,
} DeviceNameSettingsV1Idx;

typedef struct {
    char name[DEVICE_NAME_MAX_SIZE];
} DeviceNameSettingsV1;

extern const SettingProviderSetting device_name_settings_v1[];
extern const SettingProviderSetting device_name_settings_v1_root;

void device_name_settings_v1_init(DeviceNameSettingsV1* settings_v1);
