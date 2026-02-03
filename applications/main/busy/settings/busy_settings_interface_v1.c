#include "busy_settings_interface_v1.h"

static const SettingProviderStringInterface busy_setting_v1_string_interface = {
    .default_value = "",
    .max_length = BUSY_SETTINGS_V1_THEME_NAME_LEN,
};

static const SettingProviderBoolInterface busy_setting_v1_bool_interface = {
    .default_value = false,
};

static const SettingProviderSetting busy_settings_v1[] = {
    [BusySettingsV1IdxThemeName] =
        {
            .name = "theme_name",
            .interface = &busy_setting_v1_string_interface,
            .field_offset = offsetof(BusySettingsV1, theme_name),
            .type = SettingProviderSettingTypeString,
        },
    [BusySettingsV1IdxSmartHome] =
        {
            .name = "is_smart_home_enabled",
            .interface = &busy_setting_v1_bool_interface,
            .field_offset = offsetof(BusySettingsV1, is_smart_home_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [BusySettingsV1IdxWorkOnly] =
        {
            .name = "is_show_work_only_enabled",
            .interface = &busy_setting_v1_bool_interface,
            .field_offset = offsetof(BusySettingsV1, is_show_work_only_enabled),
            .type = SettingProviderSettingTypeBool,
        },
};

static const SettingProviderStructureInterface busy_settings_v1_root_interface = {
    .inner_settings = busy_settings_v1,
    .inner_settings_count = COUNT_OF(busy_settings_v1),
};

const SettingProviderSetting busy_settings_v1_root = {
    .interface = &busy_settings_v1_root_interface,
    .type = SettingProviderSettingTypeStructure,
};

static_assert(COUNT_OF(busy_settings_v1) == BusySettingsV1IdxMax);
