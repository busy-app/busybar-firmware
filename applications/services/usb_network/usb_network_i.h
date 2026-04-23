#pragma once

#include "usb_network.h"

#include <lwip/api.h>
#include <lwip/netif.h>

#include <dhserver.h>

#include "settings/usb_network_settings.h"

#define USB_NET_IPERF

struct UsbNetwork {
    struct netif netif;
    DhcpServerConfig dhcp_config;
    UsbNetworkSettings settings;
};

void usb_network_up(void);

void usb_network_down(void);

bool usb_network_rx(const uint8_t* data, uint16_t data_size);

uint16_t usb_network_tx(uint8_t* data, void* context);
