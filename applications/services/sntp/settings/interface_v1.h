#pragma once

#include <toolbox/setting_provider.h>

#define SNTP_V1_TIMEZONE_OFFSET_MIN     (-12 * 60)
#define SNTP_V1_TIMEZONE_OFFSET_MAX     (+14 * 60)
#define SNTP_V1_TIMEZONE_OFFSET_DEFAULT (+4 * 60)

#define SNTP_V1_BACKGROUND_SYNC_INTERVAL_MIN     (2 * 60 * 60)
#define SNTP_V1_BACKGROUND_SYNC_INTERVAL_MAX     (48 * 60 * 60)
#define SNTP_V1_BACKGROUND_SYNC_INTERVAL_DEFAULT (3 * 60 * 60)

#define SNTP_V1_RETRY_SYNC_INTERVAL_MIN     (5 * 60)
#define SNTP_V1_RETRY_SYNC_INTERVAL_MAX     (60 * 60)
#define SNTP_V1_RETRY_SYNC_INTERVAL_DEFAULT (10 * 60)

#define SNTP_V1_BOOT_DELAY_MIN     (1 * 60)
#define SNTP_V1_BOOT_DELAY_MAX     (10 * 60)
#define SNTP_V1_BOOT_DELAY_DEFAULT (5 * 60)

#define SNTP_V1_IS_ENABLED_DEFAULT true

typedef enum {
    SntpSettingV1IdxTimezone,
    SntpSettingV1IdxBackgroundSyncInterval,
    SntpSettingV1IdxRetrySyncInterval,
    SntpSettingV1IdxBootDelay,
    SntpSettingV1IdxServerAddress,
    SntpSettingV1IdxIsEnabled,

    SntpSettingV1IdxsCount,
} SntpSettingV1Idx;

typedef struct {
    int timezone_offset;
    int background_sync_interval;
    int retry_sync_interval;
    int boot_delay;
    bool is_enabled;
} SntpSettingsV1;

bool sntp_settings_v1_migrate_from_v0(SettingProvider* provider);

extern const SettingProviderSetting sntp_v1_settings[];
