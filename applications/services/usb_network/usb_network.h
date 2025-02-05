#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UsbNetwork UsbNetwork;
#define RECORD_USB_NETWORK "usb_network"

void usb_network_thread_init(UsbNetwork* usb_network);

void usb_network_thread_cleanup(UsbNetwork* usb_network);

#ifdef __cplusplus
}
#endif
