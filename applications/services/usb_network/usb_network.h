/**
 * @file usb_network.h
 * @brief USB Network (virtual Ethernet) API.
 */
#pragma once

#include <core/state.h>

#define RECORD_USB_NETWORK "usb_network"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UsbNetwork opaque type.
 */
typedef struct UsbNetwork UsbNetwork;

/**
 * @brief Enumeration of possible UsbNetwork states.
 */
typedef enum {
    UsbNetworkStateUnknown, /**< Unknown state (e.g. service is starting up) */
    UsbNetworkStateDown, /**< USB Network is down (e.g. cable disconnected) */
    UsbNetworkStateUp, /**< USB Network is up (normal operation) */
} UsbNetworkState;

/**
 * @brief USB Network information structure.
 */
typedef struct {
    UsbNetworkState state; /**< Current USB Network state */
} UsbNetworkInfo;

/**
 * @brief Check whether the provided IPv4 address was configured via DHCP.
 *
 * @param[in] usb_network Service instance
 * @param[in] addr IPv4 address to be checked
 */
bool usb_network_is_dhcp_addr(UsbNetwork* usb_network, const uint8_t* addr);

/**
 * @brief Get the `UsbNetwork` dynamic info object
 *
 * The enclosed type is @ref UsbNetworkInfo
 *
 * @param[in] usb_network Service instance
 * @returns pointer to the state object
 */
FuriState* usb_network_get_state(UsbNetwork* usb_network);

#ifdef __cplusplus
}
#endif
