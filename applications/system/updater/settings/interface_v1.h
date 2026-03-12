#pragma once

#include <setting_provider/setting_provider.h>

#define UPDATER_CHECK_URL_MAX_SIZE        (512 + 1)
#define UPDATER_CHECK_CHANNEL_ID_MAX_SIZE (32 + 1)

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
    char check_url[UPDATER_CHECK_URL_MAX_SIZE];
    char check_channel_id[UPDATER_CHECK_CHANNEL_ID_MAX_SIZE];
    int check_startup_interval;
    int check_interval;

    bool autoupdate_enabled;
    int autoupdate_interval_start; /* minutes since midnight */
    int autoupdate_interval_end; /* minutes since midnight */
} UpdaterSettingsV1;

extern const SettingProviderSetting updater_v1_settings[];
extern const SettingProviderSetting updater_v1_settings_root;
