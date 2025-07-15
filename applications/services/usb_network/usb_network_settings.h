#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
} UsbNetworkIp;

typedef struct {
    UsbNetworkIp ip;
    UsbNetworkIp netmask;
    UsbNetworkIp gateway;
} UsbNetworkAddress;

void usb_network_settings_init(void);

UsbNetworkAddress usb_network_settings_get_address(void);

const uint8_t* usb_network_settings_get_mac_address(void);

const char* usb_network_settings_get_hostname(void);

#ifdef __cplusplus
}
#endif
