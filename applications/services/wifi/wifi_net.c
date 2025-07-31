#include "wifi_i.h"

#include <lwip/tcpip.h>
#include <lwip/etharp.h>
#include <lwip/dhcp.h>

#include <usb_network/usb_network.h>

#define MAX_DATA_LEN (1019UL) // Limited by Intercom
#define WIRELESS_MTU (MAX_DATA_LEN - SIZEOF_ETH_HDR + ETH_PAD_SIZE)

static err_t wifi_link_output_callback(struct netif* netif, struct pbuf* p) {
    Wifi* instance = netif->state;
    furi_assert(instance);

#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    const size_t tx_size = intercom_tx(
        instance->intercom, IntercomChannelWifiData, p->payload, p->len, FuriWaitForever);
    furi_check(tx_size == p->len);

#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    return ERR_OK;
}

static err_t wifi_init_netif_callback(struct netif* netif) {
    furi_assert(netif);

    netif->mtu = WIRELESS_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;
    netif->name[0] = 'W';
    netif->name[1] = 'L';

    netif->output = etharp_output;
    netif->linkoutput = wifi_link_output_callback;

    return ERR_OK;
}

static void wifi_enable_netif_callback(void* arg) {
    furi_assert(arg);
    Wifi* instance = arg;

    struct netif* netif = &instance->netif;

    netif_set_link_up(netif);
    netif_set_up(netif);

    const err_t err = dhcp_start(netif);

    if(err != ERR_OK) {
        FURI_LOG_E(TAG, "DHCP error: %d", err);
    }
}

static void wifi_disable_netif_callback(void* arg) {
    furi_assert(arg);
    Wifi* instance = arg;

    struct netif* netif = &instance->netif;

    dhcp_stop(netif);

    netif_set_down(netif);
    netif_set_link_down(netif);
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

static void wifi_add_netif_callback(void* arg) {
    furi_assert(arg);
    Wifi* instance = arg;

    const ip_addr_t ip = {0};
    const ip_addr_t netmask = {0};
    const ip4_addr_t gateway = {0};

    struct netif* netif = &instance->netif;

    netif_add(netif, &ip, &netmask, &gateway, instance, wifi_init_netif_callback, tcpip_input);
    netif_set_default(netif);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifiData, wifi_net_intercom_rx_callback, instance);
}

void wifi_net_init(Wifi* instance) {
    furi_record_open(RECORD_USB_NETWORK);
    tcpip_callback(wifi_add_netif_callback, instance);
}

void wifi_net_set_hw_address(Wifi* instance, const WifiHardwareAddress* addr) {
    struct netif* netif = &instance->netif;

    netif->hwaddr_len = ETH_HWADDR_LEN;
    memcpy(netif->hwaddr, addr, ETH_HWADDR_LEN);
}

void wifi_net_up(Wifi* instance) {
    tcpip_callback(wifi_enable_netif_callback, instance);

    struct netif* netif = &instance->netif;

    while(!dhcp_supplied_address(netif)) {
        FURI_LOG_I(TAG, "Waiting for IP address...");
        furi_delay_ms(1000);
    }

    const uint8_t* addr = (void*)&netif->ip_addr;
    FURI_LOG_I(TAG, "IP address: %hhu.%hhu.%hhu.%hhu", addr[0], addr[1], addr[2], addr[3]);
}

void wifi_net_down(Wifi* instance) {
    tcpip_callback(wifi_disable_netif_callback, instance);
}
