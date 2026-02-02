#pragma once

#include <toolbox/setting_provider.h>

typedef enum {
    UpdaterSettingV1IdxCheckUrl,
    UpdaterSettingV1IdxCheckChannelId,
    UpdaterSettingV1IdxCheckStartupInterval,
    UpdaterSettingV1IdxCheckInterval,

    UpdaterSettingV1IdxAutoupdateEnabled,
    UpdaterSettingV1IdxAutoupdateIntervalStart,
    UpdaterSettingV1IdxAutoupdateIntervalEnd,

    UpdaterSettingV1IdxsCount,
} UpdaterSettingV1Idx;

typedef struct {
    FuriString* check_url;
    FuriString* check_channel_id;
    int check_startup_interval;
    int check_interval;

    bool autoupdate_enabled;
    int autoupdate_interval_start; /* minutes since midnight */
    int autoupdate_interval_end; /* minutes since midnight */
} UpdaterSettingsV1;

extern const SettingProviderSetting updater_v1_settings[];
extern const SettingProviderSetting updater_v1_settings_root;
