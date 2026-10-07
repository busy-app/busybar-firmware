#include "interface_v1.h"

#define ACTIVE_APP_DEFAULT ""

const SettingProviderSetting apps_menu_v1_settings[] = {
    [AppsMenuSettingV1IdxActiveApp] =
        {
            .name = "active_application",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = ACTIVE_APP_DEFAULT,
                    .max_size = SIZEOF_MEMBER(AppsMenuSettingsV1, active_application),
                },
            .field_offset = offsetof(AppsMenuSettingsV1, active_application),
            .type = SettingProviderSettingTypeString,
        },
};

const SettingProviderSetting apps_menu_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = apps_menu_v1_settings,
            .inner_settings_count = COUNT_OF(apps_menu_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(apps_menu_v1_settings) == AppsMenuSettingV1IdxsCount);
