#include "interface_v1.h"

#define CHECK_URL_DEFAULT "https://update.flipperzero.one/busybar-firmware/directory.json"

#define CHECK_CHANNEL_ID_DEFAULT "development"

#define CHECK_STARTUP_INTERVAL_MIN     (1 * 1000 * 60)
#define CHECK_STARTUP_INTERVAL_MAX     (20 * 1000 * 60)
#define CHECK_STARTUP_INTERVAL_DEFAULT (10 * 1000 * 60)

#define CHECK_INTERVAL_MIN     (1 * 1000 * 60 * 60)
#define CHECK_INTERVAL_MAX     (20 * 1000 * 60 * 60)
#define CHECK_INTERVAL_DEFAULT (5 * 1000 * 60 * 60)

#define AUTOUPDATE_ENABLED_DEFAULT false

#define AUTOUPDATE_INTERVAL_START_MIN     (0 * 60)
#define AUTOUPDATE_INTERVAL_START_MAX     (24 * 60 - 1)
#define AUTOUPDATE_INTERVAL_START_DEFAULT (2 * 60)

#define AUTOUPDATE_INTERVAL_END_MIN     (0 * 60)
#define AUTOUPDATE_INTERVAL_END_MAX     (24 * 60 - 1)
#define AUTOUPDATE_INTERVAL_END_DEFAULT (5 * 60)

/* validation callbacks */
static bool is_check_startup_interval_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);

    return (value >= CHECK_STARTUP_INTERVAL_MIN && value <= CHECK_STARTUP_INTERVAL_MAX);
}

static bool is_check_interval_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);

    return (value >= CHECK_INTERVAL_MIN && value <= CHECK_INTERVAL_MAX);
}

static bool is_autoupdate_interval_start_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);

    return (value >= AUTOUPDATE_INTERVAL_START_MIN && value <= AUTOUPDATE_INTERVAL_START_MAX);
}

static bool is_autoupdate_interval_end_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);

    return (value >= AUTOUPDATE_INTERVAL_END_MIN && value <= AUTOUPDATE_INTERVAL_END_MAX);
}

const SettingProviderSetting updater_v1_settings[] = {
    [UpdaterSettingV1IdxCheckUrl] =
        {
            .name = "check_url",
            .interface =
                &(const SettingProviderFuriStringInterface){
                    .default_value = CHECK_URL_DEFAULT,
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = offsetof(UpdaterSettingsV1, check_url),
            .type = SettingProviderSettingTypeFuriString,
        },
    [UpdaterSettingV1IdxCheckChannelId] =
        {
            .name = "check_channel_id",
            .interface =
                &(const SettingProviderFuriStringInterface){
                    .default_value = CHECK_CHANNEL_ID_DEFAULT,
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = offsetof(UpdaterSettingsV1, check_channel_id),
            .type = SettingProviderSettingTypeFuriString,
        },
    [UpdaterSettingV1IdxCheckStartupInterval] =
        {
            .name = "check_startup_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = CHECK_STARTUP_INTERVAL_DEFAULT,
                    .is_valid_callback = is_check_startup_interval_valid,
                },
            .context = NULL,
            .field_offset = offsetof(UpdaterSettingsV1, check_startup_interval),
            .type = SettingProviderSettingTypeInt,
        },
    [UpdaterSettingV1IdxCheckInterval] =
        {
            .name = "check_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = CHECK_INTERVAL_DEFAULT,
                    .is_valid_callback = is_check_interval_valid,
                },
            .context = NULL,
            .field_offset = offsetof(UpdaterSettingsV1, check_interval),
            .type = SettingProviderSettingTypeInt,
        },
    [UpdaterSettingV1IdxAutoupdateEnabled] =
        {
            .name = "autoupdate_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = AUTOUPDATE_ENABLED_DEFAULT,
                },
            .context = NULL,
            .field_offset = offsetof(UpdaterSettingsV1, autoupdate_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [UpdaterSettingV1IdxAutoupdateIntervalStart] =
        {
            .name = "autoupdate_interval_start",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = AUTOUPDATE_INTERVAL_START_DEFAULT,
                    .is_valid_callback = is_autoupdate_interval_start_valid,
                },
            .context = NULL,
            .field_offset = offsetof(UpdaterSettingsV1, autoupdate_interval_start),
            .type = SettingProviderSettingTypeInt,
        },
    [UpdaterSettingV1IdxAutoupdateIntervalEnd] =
        {
            .name = "autoupdate_interval_end",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = AUTOUPDATE_INTERVAL_END_DEFAULT,
                    .is_valid_callback = is_autoupdate_interval_end_valid,
                },
            .context = NULL,
            .field_offset = offsetof(UpdaterSettingsV1, autoupdate_interval_end),
            .type = SettingProviderSettingTypeInt,
        },
};

const SettingProviderSetting updater_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructureInterface){
            .is_valid_callback = NULL,
            .inner_settings = updater_v1_settings,
            .inner_settings_count = COUNT_OF(updater_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStructure,
};

static_assert(COUNT_OF(updater_v1_settings) == UpdaterSettingV1IdxsCount);
