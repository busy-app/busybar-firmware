#pragma once

#include <setting_provider/setting_provider.h>

#define UPDATER_SETTINGS_V1_CHECK_URL_MAX_SIZE        (512 + 1)
#define UPDATER_SETTINGS_V1_CHECK_CHANNEL_ID_MAX_SIZE (32 + 1)

#define UPDATER_SETTINGS_V1_CHECK_URL_DEFAULT \
    "https://update.flipperzero.one/busybar-firmware/directory.json"

typedef enum {
    UpdaterSettingV1IdxCheckUrl,

    UpdaterSettingV1IdxsCount,
} UpdaterSettingV1Idx;

typedef struct {
    char check_url[UPDATER_SETTINGS_V1_CHECK_URL_MAX_SIZE];
} UpdaterSettingsV1;

extern const SettingProviderSetting updater_v1_settings[];
extern const SettingProviderSetting updater_v1_settings_root;
