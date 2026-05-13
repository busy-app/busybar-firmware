#include "interface_v2.h"

typedef struct {
    int min;
    int max;
} IntMinMaxValidationContext;

static bool int_min_max_validate(const SettingProviderSetting* setting, int value) {
    const IntMinMaxValidationContext* context = setting->context;
    return value >= context->min && value <= context->max;
}

const SettingProviderSetting updater_v2_settings[] = {
    [UpdaterSettingV2IdxCheckUrl] =
        {
            .name = "check_url",
            .interface =
                &(const SettingProviderStringInterface){
                    .is_valid_callback = NULL,
                    .default_value = UPDATER_SETTINGS_V2_CHECK_URL_DEFAULT_ALIAS,
                    .max_size = SIZEOF_MEMBER(UpdaterSettingsV2, check_url),
                },
            .field_offset = offsetof(UpdaterSettingsV2, check_url),
            .type = SettingProviderSettingTypeString,
        },
    [UpdaterSettingV2IdxCheckChannelId] =
        {
            .name = "check_channel_id",
            .interface =
                &(const SettingProviderStringInterface){
                    .is_valid_callback = NULL,
                    .default_value = UPDATER_SETTINGS_V2_CHECK_CHANNEL_ID_DEFAULT,
                    .max_size = SIZEOF_MEMBER(UpdaterSettingsV2, check_channel_id),
                },
            .field_offset = offsetof(UpdaterSettingsV2, check_channel_id),
            .type = SettingProviderSettingTypeString,
        },
    [UpdaterSettingV2IdxCheckStartupInterval] =
        {
            .name = "check_startup_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = UPDATER_SETTINGS_V2_CHECK_STARTUP_INTERVAL_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = UPDATER_SETTINGS_V2_CHECK_STARTUP_INTERVAL_MIN,
                    .max = UPDATER_SETTINGS_V2_CHECK_STARTUP_INTERVAL_MAX,
                },
            .field_offset = offsetof(UpdaterSettingsV2, check_startup_interval),
            .type = SettingProviderSettingTypeInt,
        },
    [UpdaterSettingV2IdxCheckInterval] =
        {
            .name = "check_interval",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = UPDATER_SETTINGS_V2_CHECK_INTERVAL_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = UPDATER_SETTINGS_V2_CHECK_INTERVAL_MIN,
                    .max = UPDATER_SETTINGS_V2_CHECK_INTERVAL_MAX,
                },
            .field_offset = offsetof(UpdaterSettingsV2, check_interval),
            .type = SettingProviderSettingTypeInt,
        },
    [UpdaterSettingV2IdxAutoupdateEnabled] =
        {
            .name = "autoupdate_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = UPDATER_SETTINGS_V2_AUTOUPDATE_ENABLED_DEFAULT,
                },
            .field_offset = offsetof(UpdaterSettingsV2, autoupdate_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [UpdaterSettingV2IdxAutoupdateIntervalStart] =
        {
            .name = "autoupdate_interval_start",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_START_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_START_MIN,
                    .max = UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_START_MAX,
                },
            .field_offset = offsetof(UpdaterSettingsV2, autoupdate_interval_start),
            .type = SettingProviderSettingTypeInt,
        },
    [UpdaterSettingV2IdxAutoupdateIntervalEnd] =
        {
            .name = "autoupdate_interval_end",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_END_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_END_MIN,
                    .max = UPDATER_SETTINGS_V2_AUTOUPDATE_INTERVAL_END_MAX,
                },
            .field_offset = offsetof(UpdaterSettingsV2, autoupdate_interval_end),
            .type = SettingProviderSettingTypeInt,
        },
    [UpdaterSettingV2IdxAutoupdateAttemptDelay] =
        {
            .name = "autoupdate_attempt_delay",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = int_min_max_validate,
                    .default_value = UPDATER_SETTINGS_V2_AUTOUPDATE_ATTEMPT_DELAY_DEFAULT,
                },
            .context =
                &(const IntMinMaxValidationContext){
                    .min = UPDATER_SETTINGS_V2_AUTOUPDATE_ATTEMPT_DELAY_MIN,
                    .max = UPDATER_SETTINGS_V2_AUTOUPDATE_ATTEMPT_DELAY_MAX,
                },
            .field_offset = offsetof(UpdaterSettingsV2, autoupdate_attempt_delay),
            .type = SettingProviderSettingTypeInt,
        },
};

const SettingProviderSetting updater_v2_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = updater_v2_settings,
            .inner_settings_count = COUNT_OF(updater_v2_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(updater_v2_settings) == UpdaterSettingV2IdxsCount);
