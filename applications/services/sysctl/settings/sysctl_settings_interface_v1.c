#include "sysctl_settings_interface_v1.h"

static bool websrv_accesslog_level_is_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);
    return value >= 0 && value <= 3;
}

const SettingProviderSetting sysctl_settings_v1[] = {
    [SysctlSettingsV1IdxCliWifiEnabled] =
        {
            .name = "cli_wifi_enabled",
            .interface =
                &(const SettingProviderBoolInterface){
                    .default_value = false,
                },
            .field_offset = offsetof(SysctlSettingsV1, cli_wifi_enabled),
            .type = SettingProviderSettingTypeBool,
        },
    [SysctlSettingsV1IdxWebsrvAccesslogLevel] =
        {
            .name = "websrv_accesslog_level",
            .interface =
                &(const SettingProviderIntInterface){
                    .is_valid_callback = websrv_accesslog_level_is_valid,
                    .default_value = 0,
                },
            .field_offset = offsetof(SysctlSettingsV1, websrv_accesslog_level),
            .type = SettingProviderSettingTypeInt,
        },
};

static_assert(COUNT_OF(sysctl_settings_v1) == SysctlSettingsV1IdxMax);

const SettingProviderSetting sysctl_settings_v1_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = sysctl_settings_v1,
            .inner_settings_count = COUNT_OF(sysctl_settings_v1),
        },
    .field_offset = 0,
    .type = SettingProviderSettingTypeStruct,
};
