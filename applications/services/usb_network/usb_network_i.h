#pragma once

#include "usb_network.h"

#include <lwip/api.h>
#include <lwip/netif.h>

#include <dhserver.h>

#include "settings/usb_network_settings.h"

#define USB_NET_IPERF
#define DHCP_ENTRIES_MAX   3
#define DHCP_LEASE_DEFAULT (24 * 60 * 60)

struct UsbNetwork {
    struct netif netif;
    dhcp_config_t dhcp_config;
    dhcp_entry_t dhcp_entries[DHCP_ENTRIES_MAX];
    UsbNetworkSettings settings;
};

extern UsbNetwork* usb_network;
