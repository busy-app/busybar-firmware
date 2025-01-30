#pragma once

// TODO: furi_hal_version
#define USB_NETWORK_MAC      {0x0C, 0xFA, 0x22, 0x01, 0x23, 0x45}
#define USB_NETWORK_IP       LWIP_MAKEU32(10, 12, 34, 1)
#define USB_NETWORK_HOSTNAME "busybar"
#define WEBUSB_URL           USB_NETWORK_HOSTNAME ".local"

void usb_network_init(FuriEventLoop* usb_loop);

const uint8_t* usb_network_get_mac_address(void);
