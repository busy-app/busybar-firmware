#include <sl_net.h>
#include <sl_net_wifi_types.h>

#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include <sl_rsi_utility.h>
#include <sli_net_common_utility.h>
#include <sli_wifi_constants.h>

#include <lwip/tcpip.h>
#include <lwip/ethip6.h>
#include <lwip/prot/ethernet.h>

#include "wifi_backend_i.h"

#define LWIP_FRAME_ALIGNMENT (60U)
#define ETHERTYPE_IPV6       (0xDD86)
#define MAX_TRANSFER_UNIT    (1500U)

static Wifi* instance;

// Tcpip thread synchronisation

static void wifi_net_tcpip_unlock(void) {
    furi_check(furi_semaphore_release(instance->tcpip_lock) == FuriStatusOk);
}

static void wifi_net_tcpip_wait_unlock(void) {
    furi_check(furi_semaphore_acquire(instance->tcpip_lock, FuriWaitForever) == FuriStatusOk);
}

static void wifi_net_tcpip_callback(tcpip_callback_fn callback, void* context) {
    tcpip_callback(callback, context);
    wifi_net_tcpip_wait_unlock();
}

// Hosted Lwip stack

static err_t wifi_net_tcpip_output_callback(struct netif* netif, struct pbuf* p) {
    UNUSED(netif);

    const sl_status_t status =
        sl_wifi_send_raw_data_frame(SL_WIFI_CLIENT_INTERFACE, p->payload, p->len);

    return status != SL_STATUS_OK ? ERR_IF : ERR_OK;
}

static err_t wifi_net_tcpip_netif_init_callback(struct netif* netif) {
    furi_assert(netif);

    netif->name[0] = 'W';
    netif->name[1] = 'L';

    netif->output_ip6 = ethip6_output;
    netif->linkoutput = wifi_net_tcpip_output_callback;

    netif->mtu = MAX_TRANSFER_UNIT;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;

    return ERR_OK;
}

static void wifi_net_tcpip_input(const uint8_t* data, uint16_t data_len) {
    furi_check(instance);

    struct netif* netif = &instance->netif;
    // Drop packets originated from the same interface and is not destined for the said interface
    // TODO: is this necessary?
    const uint8_t* src_mac = data + netif->hwaddr_len;
    const uint8_t* dst_mac = data;

    if(!(ip6_addr_ispreferred(netif_ip6_addr_state(netif, 0))) &&
       (memcmp(netif->hwaddr, src_mac, netif->hwaddr_len) == 0) &&
       (memcmp(netif->hwaddr, dst_mac, netif->hwaddr_len) != 0)) {
        FURI_LOG_W(TAG, "Dropping packet");
        return;
    }

    struct pbuf* p = pbuf_alloc(PBUF_RAW, data_len, PBUF_POOL);

    if(p != NULL) {
        struct pbuf* q;
        uint32_t offset;

        for(q = p, offset = 0; q != NULL; q = q->next) {
            memcpy((uint8_t*)q->payload, data + offset, q->len);
            offset += q->len;
        }

        if(netif->input(p, netif) != ERR_OK) {
            pbuf_free(p);
        }
    }
}

static void wifi_net_intercom_input(const uint8_t* data, uint16_t data_len) {
    furi_check(instance);

    const size_t tx_size =
        intercom_tx(instance->intercom, IntercomChannelWifiData, data, data_len, FuriWaitForever);
    furi_check(tx_size == data_len);
}

static void wifi_net_tcpip_netif_up_callback(void* context) {
    furi_assert(context);

    const sl_mac_address_t* mac_addr = context;
    struct netif* netif = &instance->netif;

    netif->hwaddr_len = ETH_HWADDR_LEN;
    memcpy(netif->hwaddr, mac_addr, ETH_HWADDR_LEN);

    netif_set_up(netif);
    netif_set_link_up(netif);

    netif_set_ip6_autoconfig_enabled(netif, 1);
    netif_create_ip6_linklocal_address(netif, 1);

    FURI_LOG_I(TAG, "IPv6 link-local addr: %s", ip6addr_ntoa(&netif->ip6_addr[0]));

    wifi_net_tcpip_unlock();
}

static void wifi_net_tcpip_netif_down_callback(void* context) {
    UNUSED(context);

    struct netif* netif = &instance->netif;

    netif_set_link_down(netif);
    netif_set_down(netif);

    wifi_net_tcpip_unlock();
}

static void wifi_net_tcpip_add_netif_callback(void* context) {
    UNUSED(context);

    struct netif* netif = &instance->netif;
    netif_add(netif, NULL, wifi_net_tcpip_netif_init_callback, tcpip_input);
    netif_set_default(netif);

    wifi_net_tcpip_unlock();
}

// Private API

void wifi_net_tcpip_init(Wifi* wifi) {
    // TODO: Is it possible to store the instance not in a global variable?
    instance = wifi;
    wifi_net_tcpip_callback(wifi_net_tcpip_add_netif_callback, NULL);
}

// Public API

sl_status_t sl_net_wifi_ap_init(
    sl_net_interface_t interface,
    const void* configuration,
    const void* workspace,
    sl_net_event_handler_t event_handler) {
    UNUSED(interface);
    UNUSED(configuration);
    UNUSED(workspace);
    UNUSED(event_handler);
    return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_net_wifi_ap_deinit(sl_net_interface_t interface) {
    UNUSED(interface);
    return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_net_wifi_ap_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id) {
    UNUSED(interface);
    UNUSED(profile_id);
    return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_net_wifi_ap_down(sl_net_interface_t interface) {
    UNUSED(interface);
    return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_net_wifi_client_init(
    sl_net_interface_t interface,
    const void* configuration,
    void* context,
    sl_net_event_handler_t event_handler) {
    UNUSED(interface);
    UNUSED(context);
    UNUSED(event_handler);

    return sl_wifi_init(configuration, NULL, sl_wifi_default_event_handler);
}

sl_status_t sl_net_wifi_client_deinit(sl_net_interface_t interface) {
    UNUSED(interface);
    return sl_wifi_deinit();
}

sl_status_t sl_net_wifi_client_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id) {
    furi_check(instance);

    UNUSED(interface);
    // TODO: What is SL_NET_AUTO_JOIN for?
    furi_check(profile_id != SL_NET_AUTO_JOIN);

    sl_status_t status;

    do {
        sl_net_wifi_client_profile_t profile = {0};

        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, profile_id, &profile);
        if(status != SL_STATUS_OK) {
            break;
        }

        status =
            sl_wifi_connect(SL_WIFI_CLIENT_INTERFACE, &profile.config, SLI_WIFI_CONNECT_TIMEOUT);
        if(status != SL_STATUS_OK) {
            break;
        }

        sl_mac_address_t mac_addr;
        status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
        if(status != SL_STATUS_OK) {
            break;
        }

        wifi_net_tcpip_callback(wifi_net_tcpip_netif_up_callback, &mac_addr);

    } while(false);

    return status;
}

sl_status_t sl_net_wifi_client_down(sl_net_interface_t interface) {
    furi_check(instance);
    UNUSED(interface);

    wifi_net_tcpip_callback(wifi_net_tcpip_netif_down_callback, NULL);

    return sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
}

sl_status_t
    sl_si91x_host_process_data_frame(sl_wifi_interface_t interface, sl_wifi_buffer_t* buffer) {
    UNUSED(interface);

    const sl_wifi_system_packet_t* pkt = sl_si91x_host_get_buffer_data(buffer, 0, NULL);

    const uint8_t* data = pkt->data;
    const uint16_t data_len = MAX(pkt->length, LWIP_FRAME_ALIGNMENT);

    const uint16_t ethertype = *((const uint16_t*)&data[12]);

    if(ethertype == ETHERTYPE_IPV6) {
        wifi_net_tcpip_input(data, data_len);
    } else {
        wifi_net_intercom_input(data, data_len);
    }

    return SL_STATUS_OK;
}

sl_status_t sl_net_configure_ip(
    sl_net_interface_t interface,
    const sl_net_ip_configuration_t* ip_config,
    uint32_t timeout) {
    UNUSED(interface);
    UNUSED(ip_config);
    UNUSED(timeout);
    return SL_STATUS_WIFI_UNSUPPORTED;
}

sl_status_t sl_net_get_ip_address(
    sl_net_interface_t interface,
    sl_net_ip_address_t* ip_address,
    uint32_t timeout) {
    UNUSED(interface);
    UNUSED(ip_address);
    UNUSED(timeout);
    return SL_STATUS_WIFI_UNSUPPORTED;
}
