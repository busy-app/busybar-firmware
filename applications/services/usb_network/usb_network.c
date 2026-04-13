#include "usb_i.h"
#include "usb_network.h"

#include <furi_hal_version.h>

#include <furi.h>

#include <tusb.h>

#include <lwip/api.h>
#include <lwip/init.h>
#include <lwip/udp.h>
#include <lwip/tcpip.h>
#include <lwip/apps/mdns.h>
#include <lwip/apps/lwiperf.h>

#include <dhserver.h>

#include <network/network.h>

#include "settings/usb_network_settings.h"

#define TAG "UsbNet"

#define USB_NET_IPERF
#define DHCP_ENTRIES_MAX   3
#define DHCP_LEASE_DEFAULT (24 * 60 * 60)

struct UsbNetwork {
    struct netif netif;
    dhcp_config_t dhcp_config;
    dhcp_entry_t dhcp_entries[DHCP_ENTRIES_MAX];
    UsbNetworkSettings settings;
};

static UsbNetwork* usb_network = NULL;

static err_t linkoutput_fn(struct netif* netif, struct pbuf* p) {
    (void)netif;

#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    if(!tud_ready()) {
        return ERR_USE;
    }

    if(!tud_network_can_xmit(p->tot_len)) {
        return ERR_USE;
    }
    tud_network_xmit(p, 0);

#if(ETH_PAD_SIZE != 0)
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    return ERR_OK;
}

static err_t ip4_output_fn(struct netif* netif, struct pbuf* p, const ip4_addr_t* addr) {
    return etharp_output(netif, p, addr);
}

#if LWIP_IPV6
static err_t ip6_output_fn(struct netif* netif, struct pbuf* p, const ip6_addr_t* addr) {
    return ethip6_output(netif, p, addr);
}
#endif

static err_t netif_init_cb(struct netif* netif) {
    LWIP_ASSERT("netif != NULL", (netif != NULL));
    netif->mtu = CFG_TUD_NET_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP |
    // TODO: Link control
                   NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;
    netif->state = NULL;
    netif->name[0] = 'E';
    netif->name[1] = 'X';
    netif->linkoutput = linkoutput_fn;
    netif->output = ip4_output_fn;
#if LWIP_IPV6
    netif->output_ip6 = ip6_output_fn;
#endif
    return ERR_OK;
}

static void mdns_srv_txt(struct mdns_service* service, void* txt_userdata) {
    UNUSED(txt_userdata);

    err_t res = mdns_resp_add_service_txtitem(service, "path=/", 6);
    if(res != ERR_OK) {
        FURI_LOG_E(TAG, "mdns add service txt failed");
    }
}

bool tud_network_recv_cb(const uint8_t* src, uint16_t size) {
    if(size != 0) {
#if(ETH_PAD_SIZE != 0)
        size += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
        struct pbuf* p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);

        if(!p) {
            FURI_LOG_T(TAG, "cannot receive frame, pbuf_alloc failed");
            tud_network_recv_renew();
            return true;
        }

#if(ETH_PAD_SIZE != 0)
        pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

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

        if(usb_network) { /* Check if netif is initialized */
            err_t err = usb_network->netif.input(p, &usb_network->netif);
            if(err != ERR_OK) {
                FURI_LOG_W(TAG, "netif->input failed with error: %d", err);
                pbuf_free(p); /* Free pbuf if input failed */
            }
        } else {
            FURI_LOG_E(TAG, "usb_network->netif is NULL in recv_cb");
            pbuf_free(p); /* Free pbuf as it cannot be processed */
        }
        tud_network_recv_renew();
    }

    return true;
}

uint16_t tud_network_xmit_cb(uint8_t* dst, void* ref, uint16_t arg) {
    struct pbuf* p = (struct pbuf*)ref;
    UNUSED(arg);

    uint16_t res = pbuf_copy_partial(p, dst, p->tot_len, 0);
    return res;
}

void tud_network_init_cb(void) {
}

static void usb_network_init_dhcp_entries(UsbNetwork* instance, UsbNetworkIpAddress start_addr) {
    UsbNetworkIpAddress entry_addr = start_addr;

    for(uint32_t i = 0; i < DHCP_ENTRIES_MAX; i++) {
        uint8_t device_octet = entry_addr.bytes[3];

        while(device_octet == start_addr.bytes[3] || device_octet == 0) {
            ++device_octet;
        }

        entry_addr.bytes[3] = device_octet;

        dhcp_entry_t* entry = &instance->dhcp_entries[i];
        entry->addr.addr = entry_addr.val;
        entry->lease = DHCP_LEASE_DEFAULT;
    }
}

static void usb_network_init_dhcp(UsbNetwork* instance, UsbNetworkIpAddress start_addr) {
    instance->dhcp_config.netif = &instance->netif;
    instance->dhcp_config.router.addr = 0;
    instance->dhcp_config.port = 67;
    instance->dhcp_config.dns.addr = 0;
    instance->dhcp_config.domain = "usb";
    instance->dhcp_config.num_entry = DHCP_ENTRIES_MAX;
    instance->dhcp_config.entries = usb_network->dhcp_entries;

    usb_network_init_dhcp_entries(instance, start_addr);

    while(dhserv_init(&instance->dhcp_config) != ERR_OK) {
        // Busy poll
    }
}

static void usb_network_init_mdns(UsbNetwork* instance) {
    // TODO: use device name as hostname
    struct netif* netif = &instance->netif;

    mdns_resp_init();
    mdns_resp_add_netif(netif, "busybar");
    mdns_resp_add_service(netif, "httpd", "_http", DNSSD_PROTO_TCP, 80, mdns_srv_txt, NULL);
    mdns_resp_announce(netif);
}

static void usb_network_init_netif(void* arg) {
    furi_assert(arg);
    UsbNetwork* instance = arg;

    struct netif* netif = &instance->netif;

    memcpy(netif->hwaddr, furi_hal_version_get_usb_mac(), 6);
    netif->hwaddr[5] ^= 0x01;
    netif->hwaddr_len = 6;

    const UsbNetworkIpConfig* ip_config = &instance->settings.ip_config;

    const ip4_addr_t ip = {ip_config->address.val};
    const ip4_addr_t gateway = {ip_config->gateway.val};
    const ip4_addr_t netmask = {ip_config->netmask.val};

    netif_add(netif, &ip, &netmask, &gateway, NULL, netif_init_cb, tcpip_input);
#if LWIP_IPV6
    netif_create_ip6_linklocal_address(netif, 1);
#endif

    usb_network_init_dhcp(instance, ip_config->address);
    usb_network_init_mdns(instance);

#ifdef USB_NET_IPERF
    lwiperf_start_tcp_server_default(NULL, NULL);
#endif
}

bool usb_network_is_dhcp_addr(UsbNetwork* instance, uint8_t* addr) {
    furi_assert(instance);
    furi_assert(addr);
    for(uint8_t i = 0; i < DHCP_ENTRIES_MAX; i++) {
        if(memcmp(&instance->dhcp_entries[i].addr.addr, addr, 4) == 0) {
            return true;
        }
    }
    return false;
}

static UsbNetwork* usb_network_alloc(void) {
    UsbNetwork* instance = malloc(sizeof(UsbNetwork));

    usb_network_settings_load(&instance->settings);

    tcpip_callback(usb_network_init_netif, instance);

    return instance;
}

void usb_network_init(void) {
    furi_record_open(RECORD_NETWORK);

    furi_check(usb_network == NULL);
    usb_network = usb_network_alloc();

    furi_record_create(RECORD_USB_NETWORK, usb_network);
}
