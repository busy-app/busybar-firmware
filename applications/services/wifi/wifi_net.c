#include "wifi_i.h"

#include <network/network.h>
#include <lwip/tcpip.h>
#include <lwip/etharp.h>
#include <lwip/dns.h>
#include <lwip/dhcp.h>

#include <usb_network/usb_network.h>

#define MAX_DATA_LEN             (1019UL) // Limited by Intercom
#define WIRELESS_MTU             (MAX_DATA_LEN - SIZEOF_ETH_HDR + ETH_PAD_SIZE)
#define DHCP_WAIT_MS             (30 * 1000)
#define DNS_PRIMARY_SERVER_INDEX (0)

#define INTERCOM_TX_TIMEOUT_MS (500)

static err_t wifi_link_output_callback(struct netif* netif, struct pbuf* p) {
    Wifi* instance = netif->state;
    furi_assert(instance);

#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    const size_t tx_size =
        intercom_tx(instance->intercom_ch_data, p->payload, p->len, INTERCOM_TX_TIMEOUT_MS);

    const bool success = (tx_size == p->len);

#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    if(!success) {
        FURI_LOG_W(TAG, "intercom_tx timeout or incomplete send");
        return ERR_IF;
    }

    return ERR_OK;
}

static err_t wifi_init_netif_callback(struct netif* netif) {
    furi_assert(netif);

    netif->mtu = WIRELESS_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;
    network_netif_assign_name(netif, NetworkNetifWifi);

    netif->output = etharp_output;
    netif->linkoutput = wifi_link_output_callback;

    return ERR_OK;
}

static void wifi_netif_status_callback(struct netif* netif) {
    Wifi* instance = netif->state;
    furi_assert(instance);

    if(dhcp_supplied_address(netif)) {
        furi_check(furi_semaphore_release(instance->dhcp_semaphore) == FuriStatusOk);
        netif_set_status_callback(netif, NULL);
    }
}

static void wifi_net_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    Wifi* instance = context;

    size_t size = data_size;
#if(ETH_PAD_SIZE != 0)
    size += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
    struct pbuf* p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);

    if(!p) {
        FURI_LOG_E(TAG, "cannot receive frame, pbuf_alloc failed");
        return;
    }
#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    const void* src = data;

    for(struct pbuf* q = p; q != NULL && size > 0; q = q->next) {
        /* Read enough bytes to fill this pbuf in the chain.
         * The available data in the pbuf is given by the q->len variable. */
        memcpy(q->payload, src, size < q->len ? size : q->len);
        src += q->len;
        size -= q->len;
    }

#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    struct netif* netif = &instance->netif;
    const err_t err = netif->input(p, netif);

    if(err != ERR_OK) {
        FURI_LOG_W(TAG, "netif->input failed with error: %d", err);
        pbuf_free(p); /* Free pbuf if input failed */
    }
}

void wifi_net_init(Wifi* instance, const uint8_t* hw_addr) {
    furi_record_open(RECORD_USB_NETWORK);

    struct netif* netif = &instance->netif;

    const ip_addr_t ip = {0};
    const ip_addr_t netmask = {0};
    const ip4_addr_t gateway = {0};

    LOCK_TCPIP_CORE();

    netif_add(netif, &ip, &netmask, &gateway, instance, wifi_init_netif_callback, tcpip_input);
    netif_set_default(netif);

    netif->hwaddr_len = ETH_HWADDR_LEN;
    memcpy(netif->hwaddr, hw_addr, ETH_HWADDR_LEN);

    UNLOCK_TCPIP_CORE();

    instance->intercom_ch_data = intercom_channel_open(
        instance->intercom, IntercomChannelIdWifiData, wifi_net_intercom_rx_callback, instance);
}

bool wifi_net_up(Wifi* instance, const WifiIpConfig* ip_config) {
    struct netif* netif = &instance->netif;

    LOCK_TCPIP_CORE();

    if(ip_config->mgmt == WifiIpManagementStatic) {
        const WifiIpv4Settings* ip4_settings = &ip_config->ip4;

        netif->ip_addr.addr = ip4_settings->address.value;
        netif->netmask.addr = ip4_settings->mask.value;
        netif->gw.addr = ip4_settings->gateway.value;

        const ip_addr_t dns_addr = {
            .addr = ip4_settings->dns.value,
        };

        dns_setserver(DNS_PRIMARY_SERVER_INDEX, &dns_addr);

    } else if(ip_config->mgmt == WifiIpManagementDynamic) {
        dns_setserver(DNS_PRIMARY_SERVER_INDEX, NULL);
    }

    netif_set_link_up(netif);
    netif_set_up(netif);

    if(ip_config->mgmt == WifiIpManagementDynamic) {
        furi_check(dhcp_start(netif) == ERR_OK);
    }

    UNLOCK_TCPIP_CORE();

    bool success = true;

    if(ip_config->mgmt == WifiIpManagementDynamic) {
        FURI_LOG_I(TAG, "Waiting for IP configuration");

        LOCK_TCPIP_CORE();
        netif_set_status_callback(netif, wifi_netif_status_callback);
        UNLOCK_TCPIP_CORE();

        if(furi_semaphore_acquire(instance->dhcp_semaphore, DHCP_WAIT_MS) != FuriStatusOk) {
            FURI_LOG_E(TAG, "Failed to receive IP configuration");
            success = false;
        }
    }

    return success;
}

void wifi_net_down(Wifi* instance) {
    struct netif* netif = &instance->netif;

    LOCK_TCPIP_CORE();

    dhcp_release_and_stop(netif);

    netif_set_down(netif);
    netif_set_link_down(netif);

    UNLOCK_TCPIP_CORE();
}

void wifi_net_get_ip_config(Wifi* instance, WifiIpConfig* ip_config) {
    const struct netif* netif = &instance->netif;

    LOCK_TCPIP_CORE();

    ip_config->type = WifiIpTypeV4;
    ip_config->mgmt = dhcp_supplied_address(netif) ? WifiIpManagementDynamic :
                                                     WifiIpManagementStatic;
    WifiIpv4Settings* ip4 = &ip_config->ip4;

    ip4->address.value = netif->ip_addr.addr;
    ip4->mask.value = netif->netmask.addr;
    ip4->gateway.value = netif->gw.addr;
    ip4->dns.value = dns_getserver(DNS_PRIMARY_SERVER_INDEX)->addr;

    UNLOCK_TCPIP_CORE();
}

void wifi_net_set_hostname(Wifi* instance, const char* hostname) {
    struct netif* netif = &instance->netif;

    LOCK_TCPIP_CORE();

    if(strncmp(hostname, DEVICE_NAME_DEFAULT, DEVICE_NAME_MAX_LENGTH) == 0) {
        furi_string_set(instance->hostname, DEVICE_NAME_DEFAULT);
    } else {
        furi_string_printf(instance->hostname, "%s %s", DEVICE_NAME_DEFAULT, hostname);
    }

    netif->hostname = furi_string_get_cstr(instance->hostname);
    // Retransmit new DHCP hostname (if applicable)
    dhcp_network_changed_link_up(netif);

    UNLOCK_TCPIP_CORE();
}
