#pragma once

#include <setting_provider.h>

#include "../busy_timer_profile.h"

typedef struct {
    BusyTimerProfile profile;
    bool is_demo_mode_enabled;
} BusyTimerSettingsV1;

extern const SettingProviderSetting busy_timer_settings_v1_root;

bool busy_timer_settings_v1_apply_defaults(
    BusyTimerSettingsV1* settings_v1,
    BusyTimerProfileId profile_id);
