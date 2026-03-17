#pragma once

#include <setting_provider.h>

typedef enum {
    ClockSettingV1IdxShowDate,
    ClockSettingV1IdxShowSeconds,

    ClockSettingV1IdxsCount,
} ClockSettingV1Idx;

typedef struct {
    bool show_date;
    bool show_seconds;
} ClockSettingsV1;

extern const SettingProviderSetting clock_v1_settings[];
extern const SettingProviderSetting clock_v1_settings_root;
