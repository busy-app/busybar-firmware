#include "interface_v1.h"

#include <furi/core/core_defines.h>

#define SHOW_DATE_DEFAULT    true
#define SHOW_SECONDS_DEFAULT false
#define BLINK_COLONS_DEFAULT true

const SettingProviderSetting clock_v1_settings[] = {
    [ClockSettingV1IdxShowDate] =
        {
            .name = "show_date",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = SHOW_DATE_DEFAULT,
                },
            .field_offset = offsetof(ClockSettingsV1, show_date),
            .type = SettingProviderSettingTypeBool,
        },
    [ClockSettingV1IdxShowSeconds] =
        {
            .name = "show_seconds",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = SHOW_SECONDS_DEFAULT,
                },
            .field_offset = offsetof(ClockSettingsV1, show_seconds),
            .type = SettingProviderSettingTypeBool,
        },
    [ClockSettingV1IdxBlinkColons] =
        {
            .name = "blink_colons",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = BLINK_COLONS_DEFAULT,
                },
            .field_offset = offsetof(ClockSettingsV1, blink_colons),
            .type = SettingProviderSettingTypeBool,
        },
};

const SettingProviderSetting clock_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = clock_v1_settings,
            .inner_settings_count = COUNT_OF(clock_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(clock_v1_settings) == ClockSettingV1IdxsCount);
