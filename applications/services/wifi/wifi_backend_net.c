#include <sl_net.h>
#include <sl_net_wifi_types.h>

#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include <sl_rsi_utility.h>
#include <sli_net_common_utility.h>
#include <sli_wifi_constants.h>

#include <furi.h>
#include <intercom/intercom.h>
#include <wifi/wifi_common_i.h>

static Intercom* intercom;

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
    UNUSED(event_handler);

    sl_status_t status;

    do {
        status = sl_wifi_init(configuration, NULL, sl_wifi_default_event_handler);

        if(status != SL_STATUS_OK) {
            break;
        }

        intercom = context;

    } while(false);

    return status;
}

sl_status_t sl_net_wifi_client_deinit(sl_net_interface_t interface) {
    UNUSED(interface);
    return sl_wifi_deinit();
}

sl_status_t sl_net_wifi_client_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id) {
    UNUSED(interface);

    sl_status_t status;

    do {
        // Load profile and connect here
        sl_net_wifi_client_profile_t profile = {0};

        // Connect to the Wi-Fi network
        if(profile_id == SL_NET_AUTO_JOIN) {
            // status = sli_handle_auto_join(interface, &profile);
            furi_crash("SL_NET_AUTO_JOIN");
            break;
        }

        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, profile_id, &profile);
        if(status != SL_STATUS_OK) {
            break;
        }

        status =
            sl_wifi_connect(SL_WIFI_CLIENT_INTERFACE, &profile.config, SLI_WIFI_CONNECT_TIMEOUT);
        if(status != SL_STATUS_OK) {
            break;
        }

    } while(false);

    return status;
}

sl_status_t sl_net_wifi_client_down(sl_net_interface_t interface) {
    UNUSED(interface);
    return sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
}

sl_status_t
    sl_si91x_host_process_data_frame(sl_wifi_interface_t interface, sl_wifi_buffer_t* buffer) {
    UNUSED(interface);

    sl_wifi_system_packet_t* pkt = sl_si91x_host_get_buffer_data(buffer, 0, NULL);

    const size_t len = MAX(pkt->length, 60);

    const size_t tx_size =
        intercom_tx(intercom, IntercomChannelSockets, pkt->data, len, FuriWaitForever);
    furi_check(tx_size == len);

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
