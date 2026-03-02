#include "interface_v1.h"

#define ACTIVE_APP_DEFAULT ""

const SettingProviderSetting apps_menu_v1_settings[] = {
    [AppsMenuSettingV1IdxActiveApp] =
        {
            .name = "active_application",
            .interface =
                &(const SettingProviderFuriStringInterface){
                    .default_value = ACTIVE_APP_DEFAULT,
                    .is_valid_callback = NULL,
                },
            .field_offset = offsetof(AppsMenuSettingsV1, active_application),
            .type = SettingProviderSettingTypeFuriString,
        },
};

const SettingProviderSetting apps_menu_v1_settings_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructureInterface){
            .is_valid_callback = NULL,
            .inner_settings = apps_menu_v1_settings,
            .inner_settings_count = COUNT_OF(apps_menu_v1_settings),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStructure,
};

static_assert(COUNT_OF(apps_menu_v1_settings) == AppsMenuSettingV1IdxsCount);
