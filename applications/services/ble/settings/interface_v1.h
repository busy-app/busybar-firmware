#pragma once

#include <toolbox/setting_provider.h>

typedef enum {
    BleSettingsV1IdxEnabled,

    BleSettingsV1IdxsCount,
} BleSettingsV1Idx;

typedef struct {
    bool enabled;
} BleSettingsV1;

extern const SettingProviderSetting ble_v1_settings[];
extern const SettingProviderSetting ble_v1_settings_root;
