#pragma once

#include <setting_provider.h>
#include "../low_power.h"

typedef enum {
    LowPowerSleepSettingsV1IdxSleepMode,
    LowPowerSleepSettingsV1IdxSleepHoldoffDuration,

    LowPowerSleepSettingsV1IdxMAX,
} LowPowerSleepSettingsV1Idx;

typedef LowPowerSleepSettings LowPowerSleepSettingsV1;

typedef enum {
    LowPowerSettingV1IdxSleep,

    LowPowerSettingV1IdxMAX,
} LowPowerSettingV1Idx;

typedef struct {
    LowPowerSleepSettingsV1 sleep;
} LowPowerSettingsV1;

extern const SettingProviderSetting low_power_v1_settings[];
extern const SettingProviderSetting low_power_v1_settings_root;
