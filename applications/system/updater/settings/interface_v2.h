#pragma once

#include "interface_v1.h"

#define UPDATER_SETTINGS_V2_CHECK_URL_MAX_SIZE        UPDATER_SETTINGS_V1_CHECK_URL_MAX_SIZE
#define UPDATER_SETTINGS_V2_CHECK_CHANNEL_ID_MAX_SIZE UPDATER_SETTINGS_V1_CHECK_CHANNEL_ID_MAX_SIZE

#define UPDATER_SETTINGS_V2_CHECK_URL_DEFAULT \
    "https://update.busy.app/busybar-firmware/directory.json"

#define UPDATER_SETTINGS_V2_CHECK_CHANNEL_ID_DEFAULT "release"

#define UPDATER_SETTINGS_V2_CHECK_STARTUP_INTERVAL_MIN     (1 * 1000 * 60)
#define UPDATER_SETTINGS_V2_CHECK_STARTUP_INTERVAL_MAX     (20 * 1000 * 60)
#define UPDATER_SETTINGS_V2_CHECK_STARTUP_INTERVAL_DEFAULT (10 * 1000 * 60)

#define UPDATER_SETTINGS_V2_CHECK_INTERVAL_MIN     (1 * 1000 * 60 * 60)
#define UPDATER_SETTINGS_V2_CHECK_INTERVAL_MAX     (20 * 1000 * 60 * 60)
#define UPDATER_SETTINGS_V2_CHECK_INTERVAL_DEFAULT (5 * 1000 * 60 * 60)

#define UPDATER_SETTINGS_V2_AUTOUPDATE_ENABLED_DEFAULT true

#define UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_START_MIN     (0 * 60)
#define UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_START_MAX     (24 * 60 - 1)
#define UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_START_DEFAULT (2 * 60)

#define UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_END_MIN     (0 * 60)
#define UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_END_MAX     (24 * 60 - 1)
#define UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_END_DEFAULT (5 * 60)

typedef enum {
    UpdaterSettingV2IdxCheckUrl,
    UpdaterSettingV2IdxCheckChannelId,
    UpdaterSettingV2IdxCheckStartupInterval,
    UpdaterSettingV2IdxCheckInterval,

    UpdaterSettingV2IdxAutoupdateEnabled,
    UpdaterSettingV2IdxAutoupdateIntervalStart,
    UpdaterSettingV2IdxAutoupdateIntervalEnd,

    UpdaterSettingV2IdxsCount,
} UpdaterSettingV2Idx;

typedef struct {
    char check_url[UPDATER_SETTINGS_V2_CHECK_URL_MAX_SIZE];
    char check_channel_id[UPDATER_SETTINGS_V2_CHECK_CHANNEL_ID_MAX_SIZE];
    int check_startup_interval;
    int check_interval;

    bool autoupdate_enabled;
    int autoupdate_interval_start; /* minutes since midnight */
    int autoupdate_interval_end; /* minutes since midnight */
} UpdaterSettingsV2;

bool updater_settings_v2_migrate(SettingProvider* provider);

extern const SettingProviderSetting updater_v2_settings[];
extern const SettingProviderSetting updater_v2_settings_root;
