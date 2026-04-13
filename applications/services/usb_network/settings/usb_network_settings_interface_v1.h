#pragma once

#include "../usb_network_common.h"

#include <setting_provider.h>

typedef enum {
    UsbNetworkSettingsV1IdxIpConfig,
    UsbNetworkSettingsV1IdxMax,
} UsbNetworkSettingsV1Idx;

typedef struct {
    UsbNetworkIpConfig ip_config;
} UsbNetworkSettingsV1;

extern const SettingProviderSetting usb_network_settings_v1[];
extern const SettingProviderSetting usb_network_settings_v1_root;
