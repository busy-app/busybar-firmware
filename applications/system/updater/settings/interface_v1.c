#include "interface_v1.h"

#define CHECK_URL_DEFAULT "https://update.flipperzero.one/busybar-firmware/directory.json"

#define CHECK_CHANNEL_ID_DEFAULT "release"

#define CHECK_STARTUP_INTERVAL_MIN     (1 * 1000 * 60)
#define CHECK_STARTUP_INTERVAL_MAX     (20 * 1000 * 60)
#define CHECK_STARTUP_INTERVAL_DEFAULT (10 * 1000 * 60)

#define CHECK_INTERVAL_MIN     (1 * 1000 * 60 * 60)
#define CHECK_INTERVAL_MAX     (20 * 1000 * 60 * 60)
#define CHECK_INTERVAL_DEFAULT (5 * 1000 * 60 * 60)

#define AUTOUPDATE_ENABLED_DEFAULT true

#define AUTOUPDATE_INTERVAL_START_MIN     (0 * 60)
#define AUTOUPDATE_INTERVAL_START_MAX     (24 * 60 - 1)
#define AUTOUPDATE_INTERVAL_START_DEFAULT (2 * 60)

#define AUTOUPDATE_INTERVAL_END_MIN     (0 * 60)
#define AUTOUPDATE_INTERVAL_END_MAX     (24 * 60 - 1)
#define AUTOUPDATE_INTERVAL_END_DEFAULT (5 * 60)

typedef struct {
    int min;
    int max;
} IntMinMaxValidationContext;

static bool int_min_max_validate(const SettingProviderSetting* setting, int value) {
    const IntMinMaxValidationContext* context = setting->context;
    return value >= context->min && value <= context->max;
}

const SettingProviderSetting updater_v1_settings[] = {
    [UpdaterSettingV1IdxCheckUrl] =
        {
            .name = "check_url",
            .interface =
                &(const SettingProviderStringInterface){
                    .is_valid_callback = NULL,
                    .default_value = CHECK_URL_DEFAULT,
                    .max_size = SIZEOF_MEMBER(UpdaterSettingsV1, check_url),
                },
            .field_offset = offsetof(UpdaterSettingsV1, check_url),
            .type = SettingProviderSettingTypeString,
        },
    [UpdaterSettingV1IdxCheckChannelId] =
        {
            .name = "check_channel_id",
            .interface =
                &(const SettingProviderStringInterface){
                    .is_valid_callback = NULL,
                    .default_value = CHECK_CHANNEL_ID_DEFAULT,
                    .max_size = SIZEOF_MEMBER(UpdaterSettingsV1, check_channel_id),
                },
            .field_offset = offsetof(UpdaterSettingsV1, check_channel_id),
            .type = SettingProviderSettingTypeString,
        },
    [UpdaterSettingV1IdxCheckStartupInterval] =
        {
            .name = "check_startup_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = CHECK_STARTUP_INTERVAL_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = CHECK_STARTUP_INTERVAL_MIN,
                    .max = CHECK_STARTUP_INTERVAL_MAX,
                },
            .field_offset = offsetof(UpdaterSettingsV1, check_startup_interval),
            .type = SettingProviderSettingTypeInt,
        },
    [UpdaterSettingV1IdxCheckInterval] =
        {
            .name = "check_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = CHECK_INTERVAL_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = CHECK_INTERVAL_MIN,
                    .max = CHECK_INTERVAL_MAX,
                },
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
            .context =
                &(const IntMinMaxValidationContext){
                    .min = CHECK_STARTUP_INTERVAL_MIN,
                    .max = CHECK_STARTUP_INTERVAL_MAX,
                },
            .field_offset = offsetof(UpdaterSettingsV1, autoupdate_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [UpdaterSettingV1IdxAutoupdateIntervalStart] =
        {
            .name = "autoupdate_interval_start",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = AUTOUPDATE_INTERVAL_START_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = AUTOUPDATE_INTERVAL_START_MIN,
                    .max = AUTOUPDATE_INTERVAL_START_MAX,
                },
            .field_offset = offsetof(UpdaterSettingsV1, autoupdate_interval_start),
            .type = SettingProviderSettingTypeInt,
        },
    [UpdaterSettingV1IdxAutoupdateIntervalEnd] =
        {
            .name = "autoupdate_interval_end",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = AUTOUPDATE_INTERVAL_END_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = AUTOUPDATE_INTERVAL_END_MIN,
                    .max = AUTOUPDATE_INTERVAL_END_MAX,
                },
            .field_offset = offsetof(UpdaterSettingsV1, autoupdate_interval_end),
            .type = SettingProviderSettingTypeInt,
        },
};

const SettingProviderSetting updater_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = updater_v1_settings,
            .inner_settings_count = COUNT_OF(updater_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(updater_v1_settings) == UpdaterSettingV1IdxsCount);
