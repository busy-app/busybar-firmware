#include <furi_hal.h>
#include <furi_hal_usb_interface.h>

#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/ethip6.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "dhserver.h"
#include "lwip/tcpip.h"
#include "mdns.h"

#define TAG "USB NET"

#define INIT_IP4(a, b, c, d) {PP_HTONL(LWIP_MAKEU32(a, b, c, d))}

/* shared between tud_network_recv_cb() and service_traffic() */
// TODO: USB ctx store
static struct pbuf* received_frame = NULL;
static FuriSemaphore* frame_sem;

// TODO: furi_hal_version_get_network_mac()
static const uint8_t network_mac_address[6] = {0x0C, 0xFA, 0x22, 0x01, 0x23, 0x45};

/* network parameters of this MCU */
static const ip4_addr_t ipaddr = INIT_IP4(192, 168, 7, 1);
static const ip4_addr_t netmask = INIT_IP4(255, 255, 255, 0);
static const ip4_addr_t gateway = INIT_IP4(0, 0, 0, 0);

/* database IP addresses that can be offered to the host; this must be in RAM to store assigned MAC addresses */
static dhcp_entry_t entries[] = {
    {{0}, INIT_IP4(192, 168, 7, 2), 24 * 60 * 60},
    {{0}, INIT_IP4(192, 168, 7, 3), 24 * 60 * 60},
    {{0}, INIT_IP4(192, 168, 7, 4), 24 * 60 * 60},
};

static const dhcp_config_t dhcp_config = {
    .router = INIT_IP4(0, 0, 0, 0), /* router address (if any) */
    .port = 67, /* listen port */
    .dns = INIT_IP4(192, 168, 7, 1), /* dns server (if any) */
    "usb", /* dns suffix */
    COUNT_OF(entries), /* num entry */
    entries /* entries */
};

static err_t linkoutput_fn(struct netif* netif, struct pbuf* p) {
    (void)netif;

    for(;;) {
        /* if TinyUSB isn't ready, we must signal back to lwip that there is nothing we can do */
        // TODO: furi_hal_usb_ready??
        if(!tud_ready()) return ERR_USE;

        /* if the network driver can accept another packet, we make it happen */
        if(furi_hal_usb_eth_can_xmit(p->tot_len)) {
            furi_hal_usb_eth_xmit(p, 0 /* unused for this example */);
            return ERR_OK;
        }
    }
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
    netif->mtu = USB_ETH_MTU;
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

static void mdns_srv_txt(struct mdns_service* service, void* txt_userdata) {
    UNUSED(txt_userdata);

    err_t res = mdns_resp_add_service_txtitem(service, "path=/", 6);
    if(res != ERR_OK) {
        FURI_LOG_E(TAG, "mdns add service txt failed");
    }
}

bool tud_network_recv_cb(const uint8_t* src, uint16_t size) {
    if(received_frame) return false;

    if(size) {
        struct pbuf* p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
        furi_check(p);

        /* pbuf_alloc() has already initialized struct; all we need to do is copy the data */
        memcpy(p->payload, src, size);

        /* store away the pointer for service_traffic() to later handle */
        received_frame = p;
        furi_semaphore_release(frame_sem);
    }

    return true;
}

uint16_t tud_network_xmit_cb(uint8_t* dst, void* ref, uint16_t arg) {
    struct pbuf* p = (struct pbuf*)ref;

    (void)arg; /* unused for this example */

    return pbuf_copy_partial(p, dst, p->tot_len, 0);
}

void tud_network_init_cb(void) {
    if(received_frame) {
        pbuf_free(received_frame);
        received_frame = NULL;
    }
}

static int32_t network_thread(void* ctx) {
    UNUSED(ctx);
    frame_sem = furi_semaphore_alloc(1, 0);

    /* lwip context */
    struct netif netif_data;
    struct netif* netif = &netif_data;

    tcpip_init(NULL, NULL);

    /* the lwip virtual MAC address must be different from the host's; to ensure this, we toggle the LSbit */
    // TODO: ???
    netif->hwaddr_len = sizeof(network_mac_address);
    memcpy(netif->hwaddr, network_mac_address, sizeof(network_mac_address));
    netif->hwaddr[5] ^= 0x01;

    netif = netif_add(netif, &ipaddr, &netmask, &gateway, NULL, netif_init_cb, ip_input);
#if LWIP_IPV6
    netif_create_ip6_linklocal_address(netif, 1);
#endif
    netif_set_default(netif);

    while(!netif_is_up(&netif_data))
        ;
    while(dhserv_init(&dhcp_config) != ERR_OK)
        ;

    mdns_resp_init();
    mdns_resp_add_netif(netif_default, "lwip");
    mdns_resp_add_service(
        netif_default, "httpd", "_http", DNSSD_PROTO_TCP, 80, mdns_srv_txt, NULL);
    mdns_resp_announce(netif_default);

    while(1) {
        furi_semaphore_acquire(frame_sem, 10);
        if(received_frame) {
            ethernet_input(received_frame, &netif_data);
            received_frame = NULL;
            furi_hal_usb_eth_recv_renew();
        }
    }

    return 0;
}

void furi_hal_network_init(void) {
    FuriThread* thread = furi_thread_alloc_service("UsbNet", 4096, network_thread, NULL);
    furi_thread_start(thread);
}
