#include "busy_settings_interface_v1.h"

static bool busy_settings_v1_theme_name_is_valid_callback(
    const SettingProviderSetting* setting,
    const char* value) {
    UNUSED(setting);
    return strlen(value) > 0;
}

const void* busy_settings_v1_interfaces[BusySettingsProfileIdMax][BusySettingsV1IdxMax] = {
    [BusySettingsProfileIdBusy] =
        {
            [BusySettingsV1IdxThemeName] =
                &(const SettingProviderStringInterface){
                    .default_value = "default",
                    .is_valid_callback = busy_settings_v1_theme_name_is_valid_callback,
                    .max_length = BUSY_SETTINGS_V1_THEME_NAME_LEN,
                },
            [BusySettingsV1IdxSmartHome] =
                &(const SettingProviderBoolInterface){
                    .default_value = true,
                },
            [BusySettingsV1IdxWorkOnly] =
                &(const SettingProviderBoolInterface){
                    .default_value = false,
                },
        },
    [BusySettingsProfileIdCustom] =
        {
            [BusySettingsV1IdxThemeName] =
                &(const SettingProviderStringInterface){
                    .default_value = "keep_out",
                    .max_length = BUSY_SETTINGS_V1_THEME_NAME_LEN,
                },
            [BusySettingsV1IdxSmartHome] =
                &(const SettingProviderBoolInterface){
                    .default_value = true,
                },
            [BusySettingsV1IdxWorkOnly] =
                &(const SettingProviderBoolInterface){
                    .default_value = true,
                },
        },
};

const SettingProviderSetting busy_settings_v1[BusySettingsV1IdxMax] = {
    [BusySettingsV1IdxThemeName] =
        {
            .name = "theme_name",
            .field_offset = offsetof(BusySettingsV1, theme_name),
            .type = SettingProviderSettingTypeString,
        },
    [BusySettingsV1IdxSmartHome] =
        {
            .name = "is_smart_home_enabled",
            .field_offset = offsetof(BusySettingsV1, is_smart_home_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [BusySettingsV1IdxWorkOnly] =
        {
            .name = "is_show_work_only_enabled",
            .field_offset = offsetof(BusySettingsV1, is_show_work_only_enabled),
            .type = SettingProviderSettingTypeBool,
        },
};
