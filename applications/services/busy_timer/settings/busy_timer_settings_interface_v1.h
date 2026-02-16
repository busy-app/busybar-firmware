#pragma once

#include <setting_provider.h>

#include <busy/busy.h>
#include <busy_timer/busy_timer.h>

typedef struct {
    BusyAppConfig busy_bar_settings;
    BusyTimerConfig timer_settings;
    BusyTimerMetadata metadata;
    time_t timestamp_ms;
} BusyTimerSettingsV1;

extern const SettingProviderSetting busy_timer_settings_v1_root;

bool busy_timer_settings_v1_apply_defaults(
    BusyTimerSettingsV1* settings_v1,
    BusyTimerProfileId profile_id);
