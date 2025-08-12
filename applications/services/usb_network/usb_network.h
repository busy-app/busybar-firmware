#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RECORD_USB_NETWORK "usb_network"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UsbNetwork UsbNetwork;

bool usb_network_is_dhcp_addr(UsbNetwork* usb_network, uint8_t* addr);

#ifdef __cplusplus
}
#endif
