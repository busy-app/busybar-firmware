#include <furi.h>
#include <lwip/api.h>
#include <lwip/init.h>
#include <lwip/udp.h>
#include <lwip/tcpip.h>
#include <lwip/apps/mdns.h>
#include <lwip/apps/lwiperf.h>
#include <dhserver.h>
#include <tusb.h>
#include "usb_i.h"
#include "usb_network.h"

#define TAG "USB NET"

#define USB_NET_IPERF
#define DHCP_ENTRIES_MAX   3
#define DHCP_LEASE_DEFAULT (24 * 60 * 60)

#define INIT_IP4(a, b, c, d) {PP_HTONL(LWIP_MAKEU32(a, b, c, d))}

struct UsbNetwork {
    struct netif netif_data;
    struct netif* netif;

    dhcp_config_t dhcp_config;
    dhcp_entry_t dhcp_entries[DHCP_ENTRIES_MAX];
};

static UsbNetwork* usb_network = NULL;

static const ip4_addr_t ipaddr = {PP_HTONL(USB_NETWORK_IP)};
static const ip4_addr_t netmask = INIT_IP4(255, 0, 0, 0);
static const ip4_addr_t gateway = INIT_IP4(0, 0, 0, 0);

const uint8_t* usb_network_get_mac_address(void) {
    return (const uint8_t[6])USB_NETWORK_MAC;
}

static err_t linkoutput_fn(struct netif* netif, struct pbuf* p) {
    (void)netif;

#if (ETH_PAD_SIZE != 0)
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    if(!tud_ready()) {
        return ERR_USE;
    }

    if(!tud_network_can_xmit(p->tot_len)) {
        return ERR_USE;
    }
    tud_network_xmit(p, 0);

#if (ETH_PAD_SIZE != 0)
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

// static void mdns_srv_txt(struct mdns_service* service, void* txt_userdata) {
//     UNUSED(txt_userdata);

//     err_t res = mdns_resp_add_service_txtitem(service, "path=/", 6);
//     if(res != ERR_OK) {
//         FURI_LOG_E(TAG, "mdns add service txt failed");
//     }
// }

bool tud_network_recv_cb(const uint8_t* src, uint16_t size) {
    if(size != 0) {
#if (ETH_PAD_SIZE != 0)
        size += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
        struct pbuf* p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);

        if(!p) {
            FURI_LOG_T(TAG, "cannot receive frame, pbuf_alloc failed");
            return false;
        }

#if (ETH_PAD_SIZE != 0)
        pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

        for(struct pbuf* q = p; q != NULL && size > 0; q = q->next) {
            /* Read enough bytes to fill this pbuf in the chain. 
             * The available data in the pbuf is given by the q->len variable. */
            memcpy(q->payload, src, size < q->len ? size : q->len);
            src += q->len;
            size -= q->len;
        }

#if (ETH_PAD_SIZE != 0)
        pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

        usb_network->netif->input(p, usb_network->netif);
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

static void usb_network_lwip_start_callback(void* arg) {
    furi_assert(arg);
    FuriSemaphore* lwip_start_sem = arg;
    furi_semaphore_release(lwip_start_sem);
}

void usb_network_thread_init(UsbNetwork* usb_network) {
    UNUSED(usb_network);
    netconn_thread_init();
}

void usb_network_thread_cleanup(UsbNetwork* usb_network) {
    UNUSED(usb_network);
    netconn_thread_cleanup();
}

void usb_network_init(void) {
    FuriSemaphore* lwip_start_sem = furi_semaphore_alloc(1, 0);
    tcpip_init(usb_network_lwip_start_callback, lwip_start_sem);
    furi_check(furi_semaphore_acquire(lwip_start_sem, FuriWaitForever) == FuriStatusOk);

    usb_network = malloc(sizeof(UsbNetwork));
    usb_network->netif = &(usb_network->netif_data);

    usb_network->netif_data.hwaddr_len = 6;
    memcpy(usb_network->netif_data.hwaddr, usb_network_get_mac_address(), 6);
    usb_network->netif_data.hwaddr[5] ^= 0x01;

    usb_network->netif = netif_add(
        &(usb_network->netif_data), &ipaddr, &netmask, &gateway, NULL, netif_init_cb, tcpip_input);
#if LWIP_IPV6
    netif_create_ip6_linklocal_address(usb_network->netif, 1);
#endif
    netif_set_default(usb_network->netif);

    furi_record_create(RECORD_USB_NETWORK, usb_network);

    while(!netif_is_up(usb_network->netif))
        ;

    // Prepare DHCP configuration
    for(uint8_t i = 0; i < DHCP_ENTRIES_MAX; i++) {
        usb_network->dhcp_entries[i].addr.addr = PP_HTONL(USB_NETWORK_IP + i + 1);
        usb_network->dhcp_entries[i].lease = DHCP_LEASE_DEFAULT;
    }
    usb_network->dhcp_config.router.addr = PP_HTONL(LWIP_MAKEU32(0, 0, 0, 0));
    usb_network->dhcp_config.port = 67;
    usb_network->dhcp_config.dns.addr = 0; //PP_HTONL(USB_NETWORK_IP);
    usb_network->dhcp_config.domain = "usb";
    usb_network->dhcp_config.num_entry = DHCP_ENTRIES_MAX;
    usb_network->dhcp_config.entries = usb_network->dhcp_entries;

    while(dhserv_init(&(usb_network->dhcp_config)) != ERR_OK)
        ;

    // mdns_resp_init();
    // mdns_resp_add_netif(netif_default, USB_NETWORK_HOSTNAME);
    // mdns_resp_add_service(
    //     netif_default, "httpd", "_http", DNSSD_PROTO_TCP, 80, mdns_srv_txt, NULL);
    // mdns_resp_announce(netif_default);

#ifdef USB_NET_IPERF
    lwiperf_start_tcp_server_default(NULL, NULL);
#endif
}
