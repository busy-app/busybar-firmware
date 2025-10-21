#include "interface_v1.h"
#include "interface_v0.h"

static bool is_timezone_offset_valid(int timezone_offset) {
    return timezone_offset >= SNTP_V1_TIMEZONE_OFFSET_MIN &&
           timezone_offset <= SNTP_V1_TIMEZONE_OFFSET_MAX;
}

static bool is_background_sync_interval_valid(int background_sync_interval) {
    return background_sync_interval >= SNTP_V1_BACKGROUND_SYNC_INTERVAL_MIN &&
           background_sync_interval <= SNTP_V1_BACKGROUND_SYNC_INTERVAL_MAX;
}

static bool is_retry_sync_interval_valid(int retry_sync_interval) {
    return retry_sync_interval >= SNTP_V1_RETRY_SYNC_INTERVAL_MIN &&
           retry_sync_interval <= SNTP_V1_RETRY_SYNC_INTERVAL_MAX;
}

static bool is_boot_delay_valid(int boot_delay) {
    return boot_delay >= SNTP_V1_BOOT_DELAY_MIN && boot_delay <= SNTP_V1_BOOT_DELAY_MAX;
}

bool sntp_settings_v1_migrate_from_v0(SettingProvider* provider) {
    const SettingProviderSetting* setting_v0 = &sntp_v0_settings[SntpSettingV0IdxTimezone];
    const SettingProviderSetting* setting_v1 = &sntp_v1_settings[SntpSettingV1IdxTimezone];

    int timezone;
    setting_provider_load(provider, setting_v0, &timezone);
    setting_provider_drop(provider, setting_v0);

    timezone *= 60;
    setting_provider_save(provider, setting_v1, &timezone);

    return true;
}

const SettingProviderSetting sntp_v1_settings[] = {
    [SntpSettingV1IdxTimezone] =
        {
            .name = "timezone",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = SNTP_V1_TIMEZONE_OFFSET_DEFAULT,
                    .is_valid_callback = is_timezone_offset_valid,
                },
            .context = &(const size_t){offsetof(SntpSettingsV1, timezone_offset)},
            .type = SettingProviderSettingTypeInt,
        },
    [SntpSettingV1IdxBackgroundSyncInterval] =
        {
            .name = "background_sync_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = SNTP_V1_BACKGROUND_SYNC_INTERVAL_DEFAULT,
                    .is_valid_callback = is_background_sync_interval_valid,
                },
            .context = &(const size_t){offsetof(SntpSettingsV1, background_sync_interval)},
            .type = SettingProviderSettingTypeInt,
        },
    [SntpSettingV1IdxRetrySyncInterval] =
        {
            .name = "retry_sync_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = SNTP_V1_RETRY_SYNC_INTERVAL_DEFAULT,
                    .is_valid_callback = is_retry_sync_interval_valid,
                },
            .context = &(const size_t){offsetof(SntpSettingsV1, retry_sync_interval)},
            .type = SettingProviderSettingTypeInt,
        },
    [SntpSettingV1IdxBootDelay] =
        {
            .name = "boot_delay",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = SNTP_V1_BOOT_DELAY_DEFAULT,
                    .is_valid_callback = is_boot_delay_valid,
                },
            .context = &(const size_t){offsetof(SntpSettingsV1, boot_delay)},
            .type = SettingProviderSettingTypeInt,
        },
    [SntpSettingV1IdxIsEnabled] =
        {
            .name = "is_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = SNTP_V1_IS_ENABLED_DEFAULT,
                },
            .context = &(const size_t){offsetof(SntpSettingsV1, is_enabled)},
            .type = SettingProviderSettingTypeBool,
        },
};

static_assert(COUNT_OF(sntp_v1_settings) == SntpSettingV1IdxsCount);
