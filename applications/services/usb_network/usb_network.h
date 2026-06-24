#pragma once

#include <furi.h>

#define RECORD_USB_NETWORK "usb_network"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UsbNetwork UsbNetwork;

typedef enum {
    UsbNetworkStateUp,
    UsbNetworkStateDown,
    UsbNetworkStateMAX,
} UsbNetworkState;

typedef struct {
    UsbNetworkState state;
} UsbNetworkInfo;

bool usb_network_is_dhcp_addr(UsbNetwork* usb_network, const uint8_t* addr);

/**
 * @brief Gets the `UsbNetwork` dynamic info object
 * 
 * The enclosed type is `UsbNetworkInfo`
 * 
 * @param[inout] usb_network Service instance
 */
FuriState* usb_network_get_state(UsbNetwork* usb_network);

#ifdef __cplusplus
}
#endif
