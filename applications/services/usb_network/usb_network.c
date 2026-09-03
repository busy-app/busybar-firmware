#include "usb_network_i.h"

#include <furi_hal_version.h>

#include <furi.h>

#include <tusb.h>

#include <lwip/udp.h>
#include <lwip/tcpip.h>
#include <lwip/etharp.h>

#include <lwip/apps/mdns.h>
#include <lwip/apps/lwiperf.h>

#include <network/network.h>

#ifndef ETH_PAD_SIZE
#define ETH_PAD_SIZE 0
#endif // ETH_PAD_SIZE

#if(ETH_PAD_SIZE != 0)
#define PBUF_ADD_PADDING(p)  pbuf_header((p), ETH_PAD_SIZE)
#define PBUF_DROP_PADDING(p) pbuf_header((p), -ETH_PAD_SIZE)
#else // ETH_PAD_SIZE != 0
#define PBUF_ADD_PADDING(p)
#define PBUF_DROP_PADDING(p)
#endif // ETH_PAD_SIZE != 0

#define DHCP_INIT_ATTEMPTS (10)

#define TAG "UsbNet"

static UsbNetwork* usb_network = NULL;

static err_t usb_network_link_output_callback(struct netif* netif, struct pbuf* p) {
    UNUSED(netif);

    err_t ret = ERR_IF;

    PBUF_DROP_PADDING(p);

    do {
        if(!tud_mounted()) {
            break;
        }

        if(!tud_network_can_xmit(p->tot_len)) {
            break;
        }

        tud_network_xmit(p, 0);

        ret = ERR_OK;
    } while(false);

    PBUF_ADD_PADDING(p);

    return ret;
}

static err_t usb_network_netif_init_callback(struct netif* netif) {
    furi_assert(netif);

    netif->mtu = CFG_TUD_NET_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;
    network_netif_assign_name(netif, NetworkNetifUsb);
    netif->output = etharp_output;
    netif->linkoutput = usb_network_link_output_callback;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    return ERR_OK;
}

static void usb_network_dhcp_init(UsbNetwork* instance) {
    DhcpServerConfig* dhcp_config = &instance->dhcp_config;

    dhcp_config->netif = &instance->netif;
    dhcp_config->router.addr = 0;
    dhcp_config->port = 67;
    dhcp_config->dns.addr = 0;
    dhcp_config->domain = "usb";
    dhcp_config->max_lease_count = 3;
}

static void usb_network_dhcp_start(UsbNetwork* instance) {
    uint32_t num_attempts;

    for(num_attempts = 0; num_attempts < DHCP_INIT_ATTEMPTS; ++num_attempts) {
        if(dhserv_init(&instance->dhcp_config)) {
            break;
        }
    }

    if(num_attempts == DHCP_INIT_ATTEMPTS) {
        FURI_LOG_E(TAG, "Failed to start DHCP server");
    }
}

static void usb_network_dhcp_stop(UsbNetwork* instance) {
    UNUSED(instance);
    dhserv_deinit();
}

static void usb_network_set_state(UsbNetwork* instance, UsbNetworkState state) {
    with_furi_state(instance->state, UsbNetworkInfo * info, { info->state = state; });
}

static void usb_network_netif_set_hw_address(struct netif* netif) {
    memcpy(netif->hwaddr, furi_hal_version_get_usb_mac(), ETH_HWADDR_LEN);
    netif->hwaddr[5] ^= 0x01;
    netif->hwaddr_len = ETH_HWADDR_LEN;
}

static void usb_network_init_netif(UsbNetwork* instance) {
    LOCK_TCPIP_CORE();

    struct netif* netif = &instance->netif;
    usb_network_netif_set_hw_address(netif);

    const UsbNetworkIpConfig* ip_config = &instance->settings.ip_config;

    const ip4_addr_t ip = {ip_config->address.val};
    const ip4_addr_t gateway = {ip_config->gateway.val};
    const ip4_addr_t netmask = {ip_config->netmask.val};

    netif_add(netif, &ip, &netmask, &gateway, NULL, usb_network_netif_init_callback, tcpip_input);
#if LWIP_IPV6
    netif_create_ip6_linklocal_address(netif, 1);
#endif

    usb_network_dhcp_init(instance);

#ifdef USB_NET_IPERF
    lwiperf_start_tcp_server_default(NULL, NULL);
#endif

    UNLOCK_TCPIP_CORE();

    usb_network_set_state(instance, UsbNetworkStateDown);
}

void usb_network_up(void) {
    furi_assert(usb_network);
    struct netif* netif = &usb_network->netif;

    LOCK_TCPIP_CORE();
    netif_set_up(netif);
    netif_set_link_up(netif);

    usb_network_dhcp_start(usb_network);
    UNLOCK_TCPIP_CORE();

    usb_network_set_state(usb_network, UsbNetworkStateUp);
}

void usb_network_down(void) {
    furi_assert(usb_network);
    struct netif* netif = &usb_network->netif;

    LOCK_TCPIP_CORE();
    usb_network_dhcp_stop(usb_network);

    netif_set_link_down(netif);
    netif_set_down(netif);
    UNLOCK_TCPIP_CORE();

    usb_network_set_state(usb_network, UsbNetworkStateDown);
}

bool usb_network_rx(const uint8_t* data, uint16_t data_size) {
    furi_assert(usb_network);

    bool success = false;
    struct pbuf* pbuf = NULL;

    do {
        // TODO: Is this really possible?
        if(data_size == 0) {
            FURI_LOG_W(TAG, "data_size == 0");
            break;
        }

        pbuf = pbuf_alloc(PBUF_RAW, data_size + ETH_PAD_SIZE, PBUF_POOL);

        if(!pbuf) {
            FURI_LOG_T(TAG, "pbuf_alloc() failed");
            break;
        }

        err_t lwip_err;

        PBUF_DROP_PADDING(pbuf);
        lwip_err = pbuf_take(pbuf, data, data_size);
        PBUF_ADD_PADDING(pbuf);

        if(lwip_err != ERR_OK) {
            FURI_LOG_D(TAG, "pbuf_take() failed with error: %d", lwip_err);
            break;
        }

        struct netif* netif = &usb_network->netif;
        lwip_err = netif->input(pbuf, netif);

        if(lwip_err != ERR_OK) {
            FURI_LOG_D(TAG, "netif->input() failed with error: %d", lwip_err);
            break;
        }

        success = true;
    } while(false);

    if(!success && pbuf) {
        pbuf_free(pbuf);
    }

    tud_network_recv_renew();

    return true;
}

uint16_t usb_network_tx(uint8_t* data, void* context) {
    struct pbuf* pbuf = context;
    return pbuf_copy_partial(pbuf, data, pbuf->tot_len, 0);
}

bool usb_network_is_dhcp_addr(UsbNetwork* instance, const uint8_t* addr) {
    furi_assert(instance);
    furi_assert(addr);

    ip4_addr_t ip4_addr;
    memcpy(&ip4_addr, addr, sizeof(ip4_addr));

    LOCK_TCPIP_CORE();
    const bool result = dhserv_has_lease(ip4_addr);
    UNLOCK_TCPIP_CORE();

    return result;
}

FuriState* usb_network_get_state(UsbNetwork* usb_network) {
    furi_check(usb_network);
    return usb_network->state;
}

static UsbNetwork* usb_network_alloc(void) {
    UsbNetwork* instance = malloc(sizeof(UsbNetwork));

    instance->state = furi_state_alloc(sizeof(UsbNetworkInfo));

    usb_network_settings_load(&instance->settings);
    usb_network_init_netif(instance);

    return instance;
}

void usb_network_init(void) {
    furi_record_open(RECORD_NETWORK);

    furi_check(usb_network == NULL);
    usb_network = usb_network_alloc();

    furi_record_create(RECORD_USB_NETWORK, usb_network);
}
