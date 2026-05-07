#include "interface_v1.h"

const SettingProviderSetting updater_v1_settings[] = {
    [UpdaterSettingV1IdxCheckUrl] =
        {
            .name = "check_url",
            .interface =
                &(const SettingProviderStringInterface){
                    .is_valid_callback = NULL,
                    .default_value = UPDATER_SETTINGS_V1_CHECK_URL_DEFAULT,
                    .max_size = SIZEOF_MEMBER(UpdaterSettingsV1, check_url),
                },
            .field_offset = offsetof(UpdaterSettingsV1, check_url),
            .type = SettingProviderSettingTypeString,
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
