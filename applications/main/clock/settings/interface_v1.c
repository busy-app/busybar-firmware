#include "interface_v1.h"

#include <furi/core/core_defines.h>

#define SHOW_DATE_DEFAULT    true
#define SHOW_SECONDS_DEFAULT false

const SettingProviderSetting clock_v1_settings[] = {
    [ClockSettingV1IdxShowDate] =
        {
            .name = "show_date",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = SHOW_DATE_DEFAULT,
                },
            .context = NULL,
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
            .context = NULL,
            .field_offset = offsetof(ClockSettingsV1, show_seconds),
            .type = SettingProviderSettingTypeBool,
        },
};

const SettingProviderSetting clock_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructureInterface){
            .is_valid_callback = NULL,
            .inner_settings = clock_v1_settings,
            .inner_settings_count = COUNT_OF(clock_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStructure,
};

static_assert(COUNT_OF(clock_v1_settings) == ClockSettingV1IdxsCount);
