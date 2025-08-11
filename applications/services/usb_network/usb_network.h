#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UsbNetwork UsbNetwork;
#define RECORD_USB_NETWORK "usb_network"

void usb_network_thread_init(UsbNetwork* usb_network);

void usb_network_thread_cleanup(UsbNetwork* usb_network);

bool usb_network_is_dhcp_addr(UsbNetwork* usb_network, uint8_t* addr);

#ifdef __cplusplus
}
#endif
