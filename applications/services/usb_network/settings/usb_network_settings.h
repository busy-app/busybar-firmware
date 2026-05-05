#pragma once

#include "usb_network_settings_interface_v1.h"

typedef UsbNetworkSettingsV1 UsbNetworkSettings;

void usb_network_settings_load(UsbNetworkSettings* settings);
void usb_network_settings_save(const UsbNetworkSettings* settings);
