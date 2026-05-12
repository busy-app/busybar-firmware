#pragma once

#include <setting_provider.h>

typedef enum {
    SysctlSettingsV1IdxCliWifiEnabled,
    SysctlSettingsV1IdxWebsrvAccesslogLevel,
    SysctlSettingsV1IdxMax,
} SysctlSettingsV1Idx;

typedef struct {
    bool cli_wifi_enabled;
    int websrv_accesslog_level;
} SysctlSettingsV1;

extern const SettingProviderSetting sysctl_settings_v1[];
extern const SettingProviderSetting sysctl_settings_v1_root;
