#include "interface_v1.h"

static const LowPowerMode default_sleep_mode = LowPowerModeSleepAfterHoldoff;
#define DEFAULT_SLEEP_HOLDOFF_DURATION 60000

static bool
    low_power_v1_is_sleep_holdoff_duration_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);
    return value >= 0;
}

static const char* const low_power_v1_sleep_settings_mode_string_map[LowPowerModeMAX] = {
    [LowPowerModeSleepAfterHoldoff] = "after_holdoff",
    [LowPowerModeDisabled] = "disabled",
};

const SettingProviderSetting low_power_v1_sleep_settings[] = {
    [LowPowerSleepSettingsV1IdxSleepMode] =
        {
            .name = "mode",
            .type = SettingProviderSettingTypeEnum,
            .interface =
                &(const SettingProviderEnumInterface){
                    .default_value = &default_sleep_mode,
                    .string_map = low_power_v1_sleep_settings_mode_string_map,
                    .string_map_length = COUNT_OF(low_power_v1_sleep_settings_mode_string_map),
                    .type_size = sizeof(LowPowerMode),
                },
            .field_offset = offsetof(LowPowerSleepSettingsV1, mode),
        },
    [LowPowerSleepSettingsV1IdxSleepHoldoffDuration] =
        {
            .name = "sleep_holdoff_duration",
            .type = SettingProviderSettingTypeInt,
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = low_power_v1_is_sleep_holdoff_duration_valid,
                    .default_value = DEFAULT_SLEEP_HOLDOFF_DURATION,
                },
            .field_offset = offsetof(LowPowerSleepSettingsV1, holdoff_duration),
        },
};

const SettingProviderSetting low_power_v1_settings[] = {
    [LowPowerSettingV1IdxSleep] =
        {
            .name = "sleep",
            .type = SettingProviderSettingTypeStruct,
            .interface =
                &(const SettingProviderStructInterface){
                    .inner_settings = low_power_v1_sleep_settings,
                    .inner_settings_count = COUNT_OF(low_power_v1_sleep_settings),
                },
            .field_offset = offsetof(LowPowerSleepSettingsV1, mode),
        },
};

const SettingProviderSetting low_power_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = low_power_v1_settings,
            .inner_settings_count = COUNT_OF(low_power_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(low_power_v1_settings) == LowPowerSettingV1IdxMAX);
