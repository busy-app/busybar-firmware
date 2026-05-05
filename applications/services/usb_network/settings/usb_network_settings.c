#include "usb_network_settings.h"

#include <storage/storage.h>

#define USB_NETWORK_SETTINGS_FILE_PATH APP_DATA_PATH("settings.json")
#define USB_NETWORK_SETTINGS_VERSION   1
#define USB_NETWORK_SETTINGS_ROOT      usb_network_settings_v1_root

void usb_network_settings_load(UsbNetworkSettings* settings) {
    furi_assert(settings);

    SettingProvider* provider = setting_provider_alloc(
        USB_NETWORK_SETTINGS_FILE_PATH, USB_NETWORK_SETTINGS_VERSION, NULL, 0);
    setting_provider_load(provider, &USB_NETWORK_SETTINGS_ROOT, settings);
    setting_provider_free(provider);
}

void usb_network_settings_save(const UsbNetworkSettings* settings) {
    furi_assert(settings);

    SettingProvider* provider = setting_provider_alloc(
        USB_NETWORK_SETTINGS_FILE_PATH, USB_NETWORK_SETTINGS_VERSION, NULL, 0);
    setting_provider_save(provider, &USB_NETWORK_SETTINGS_ROOT, settings);
    setting_provider_free(provider);
}
