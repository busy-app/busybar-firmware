#include <furi.h>

#include <sl_si91x_driver.h>

#include <sl_net.h>
#include <sl_net_wifi_types.h>

#include <sl_wifi_callback_framework.h>

#define TAG "DemoWifiTestAp"

#define CHANNEL_NUMBER       6
#define WIFI_AP_PROFILE_SSID "FlipperBsbAp"
#define WIFI_AP_CREDENTIAL   "12345678"

//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_MODULE_IP_ADDRESS 0x0A0AA8C0

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define DEFAULT_WIFI_SN_MASK_ADDRESS 0x00FFFFFF

//! IP address of Gateway
//! E.g: 0x0A0AA8C0 == 192.168.10.10
#define DEFAULT_WIFI_GATEWAY_ADDRESS 0x0A0AA8C0

static sl_net_wifi_ap_profile_t wifi_ap_profile = {
    .config =
        {
            .ssid.value = WIFI_AP_PROFILE_SSID,
            .ssid.length = sizeof(WIFI_AP_PROFILE_SSID) - 1,
            .channel.channel = SL_WIFI_AUTO_CHANNEL,
            .channel.band = SL_WIFI_AUTO_BAND,
            .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
            .security = SL_WIFI_WPA2,
            .encryption = SL_WIFI_CCMP_ENCRYPTION,
            .rate_protocol = SL_WIFI_RATE_PROTOCOL_AUTO,
            .options = 0,
            .credential_id = SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
            .keepalive_type = SL_SI91X_AP_NULL_BASED_KEEP_ALIVE,
            .beacon_interval = 100,
            .client_idle_timeout = 0xFF,
            .dtim_beacon_count = 3,
            .maximum_clients = 3,
            .beacon_stop = 0,
            .is_11n_enabled = 0,
        },

    .ip =
        {
            .mode = SL_IP_MANAGEMENT_STATIC_IP,
            .type = SL_IPV4,
            .host_name = NULL,
            .ip =
                {
                    .v4.ip_address.value = DEFAULT_WIFI_MODULE_IP_ADDRESS,
                    .v4.gateway.value = DEFAULT_WIFI_GATEWAY_ADDRESS,
                    .v4.netmask.value = DEFAULT_WIFI_SN_MASK_ADDRESS,
                },
        },
};

sl_net_wifi_psk_credential_entry_t wifi_ap_credential = {
    .type = SL_NET_WIFI_PSK,
    .data_length = sizeof(WIFI_AP_CREDENTIAL) - 1,
    .data = WIFI_AP_CREDENTIAL,
};

typedef struct {
    FuriEventLoop* event_loop;
} DemoWifiTest;

static sl_status_t
    ap_connected_event_handler(sl_wifi_event_t event, void* data, uint32_t data_length, void* arg) {
    UNUSED_PARAMETER(data_length);
    UNUSED_PARAMETER(arg);
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(data);

    printf("Remote Client connected: ");
    print_mac_address((sl_mac_address_t*)data);
    printf("\r\n");

    return SL_STATUS_OK;
}

static sl_status_t ap_disconnected_event_handler(
    sl_wifi_event_t event,
    void* data,
    uint32_t data_length,
    void* arg) {
    UNUSED_PARAMETER(data_length);
    UNUSED_PARAMETER(arg);
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(data);
    printf("Remote Client disconnected: ");
    print_mac_address((sl_mac_address_t*)data);
    printf("\r\n");

    return SL_STATUS_OK;
}

void demo_wifi_test_ap(void* p) {
    UNUSED(p);
    FURI_LOG_I(TAG, "Starting AP");

    DemoWifiTest* instance = malloc(sizeof(DemoWifiTest));
    instance->event_loop = furi_event_loop_alloc();

    sl_status_t status;
    do {
        status = sl_net_init(
            SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to start Wi-Fi AP interface: 0x%lx", status);
            break;
        }
        FURI_LOG_I(TAG, "Wi-Fi AP interface init");

        sl_wifi_set_callback(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
        sl_wifi_set_callback(
            SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);

        wifi_ap_profile.config.channel.channel = CHANNEL_NUMBER;
        status =
            sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to set AP profile: 0x%lx", status);
            break;
        }
        FURI_LOG_I(TAG, "Success to set AP profile");

        status = sl_net_set_credential(
            SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
            wifi_ap_credential.type,
            &wifi_ap_credential.data,
            wifi_ap_credential.data_length);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to set credentials: 0x%lx", status);
            break;
        }
        FURI_LOG_I(TAG, "Wi-Fi set credential success");

        status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to bring Wi-Fi AP interface up: 0x%lx", status);
            break;
        }
        FURI_LOG_I(TAG, "AP started");
    } while(0);

    furi_event_loop_run(instance->event_loop);
}
