#include <stdint.h>
#include <furi.h>
#include "usb_network_settings.h"

// TODO: furi_hal_version
#define DEFAULT_MAC         {0x0C, 0xFA, 0x22, 0x01, 0x23, 0x45}
#define DEFAULT_HOSTNAME    "busybar"
#define DEFAULT_WEBUSB_ZONE ".local"

static FuriString* hostname = NULL;
static FuriString* webusb_url = NULL;

static UsbNetworkAddress address = {
    .ip = {10, 12, 34, 1},
    .netmask = {255, 255, 255, 0},
    .gateway = {10, 12, 34, 1},
};

static uint8_t mac_address[6] = DEFAULT_MAC;

UsbNetworkAddress usb_network_settings_get_address(void) {
    return address;
}

const uint8_t* usb_network_settings_get_mac_address(void) {
    return mac_address;
}

const char* usb_network_settings_get_hostname(void) {
    furi_check(hostname);
    return furi_string_get_cstr(hostname);
}

const char* usb_network_settings_get_webusb_url(void) {
    furi_check(webusb_url);
    return furi_string_get_cstr(webusb_url);
}

void usb_network_settings_init(void) {
    hostname = furi_string_alloc_set(DEFAULT_HOSTNAME);
    webusb_url = furi_string_alloc_set(DEFAULT_HOSTNAME DEFAULT_WEBUSB_ZONE);
}
