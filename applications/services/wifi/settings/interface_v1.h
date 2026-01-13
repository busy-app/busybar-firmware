#include "../wifi_common.h"

#include <toolbox/setting_provider.h>

typedef enum {
    WifiSettingV1IdxCredentials,
    WifiSettingV1IdxIpConfig,

    WifiSettingV1IdxsCount,
} WifiSettingV1Idx;

typedef struct {
    WifiCredentials credentials;
    WifiIpConfig ip_config;
} WifiSettingsV1;

extern const SettingProviderSetting wifi_v1_settings[];
extern const SettingProviderSetting wifi_v1_settings_root;
