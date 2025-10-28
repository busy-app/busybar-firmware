#include "wifi_backend_i.h"

#include <sl_net.h>
#include <sl_wifi.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_callback_framework.h>

#include "wifi_config.h"
#include "wifi_backend_util.h"

#define NUM_CONNECTION_ATTEMPTS (3)

typedef enum {
    WifiEventRequest = 1UL << 0,
    WifiEventScanComplete = 1UL << 1,
} WifiEvent;

typedef void (*WifiRequestHandler)(Wifi* instance);

static const WifiRequestHandler wifi_request_handlers[WifiRequestTypeMax];

static inline void wifi_send_response(Wifi* instance) {
    const size_t tx_size = intercom_tx(
        instance->intercom,
        IntercomChannelWifi,
        &instance->response,
        sizeof(WifiResponse),
        FuriWaitForever);
    furi_check(tx_size == sizeof(WifiResponse));
}

static inline void wifi_set_state(Wifi* instance, WifiBackendState state) {
    if(state != instance->state) {
        instance->state = state;
        furi_pubsub_publish(instance->event_pubsub, &instance->state);
    }
}

static void wifi_init_request_handler(Wifi* instance) {
    FURI_LOG_D(TAG, "Init");

    WifiResponse* response = &instance->response;

    const sl_status_t status =
        sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, (sl_mac_address_t*)response->hw_address);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to get MAC address: %lX", status);
    }

    response->status = wifi_decode_sl_status(status);
    wifi_send_response(instance);
}

static void wifi_scan_request_handler(Wifi* instance) {
    FURI_LOG_D(TAG, "Scan");

    sl_wifi_scan_configuration_t wifi_scan_configuration = default_wifi_scan_configuration;
    wifi_scan_configuration.type = SL_WIFI_SCAN_TYPE_EXTENDED;

    sl_status_t status;
    status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);

    if(status != SL_STATUS_IN_PROGRESS) {
        WifiResponse* response = &instance->response;
        response->status = wifi_decode_sl_status(status);

        wifi_send_response(instance);
    }
}

static void wifi_connect_request_handler(Wifi* instance) {
    FURI_LOG_D(TAG, "Connect");

    WifiResponse* response = &instance->response;

    sl_status_t status;

    do {
        const WifiConnectRequest* request = &instance->request.connect_request;
        const WifiCredentials* credentials = &request->credentials;

        // Initialise client profile
        sl_net_wifi_client_profile_t profile = {
            .config =
                {
                    .security = wifi_encode_security_mode(credentials->security_mode),
                    .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
                },
        };

        wifi_encode_ssid(&profile.config.ssid, credentials->ssid);

        // Set profile
        status = sl_net_set_profile(
            SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to set Wifi profile: %lX", status);
            break;
        }

        if(credentials->security_mode != WifiSecurityModeOpen) {
            status = sl_net_set_credential(
                SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
                // TODO: Other passphrase types than PSK?
                SL_NET_WIFI_PSK,
                credentials->passphrase,
                strlen(credentials->passphrase));

            if(status != SL_STATUS_OK) {
                FURI_LOG_E(TAG, "Failed to set Wifi passphrase: %lX", status);
                break;
            }
        }

        // Connect to the network
        for(uint32_t i = 0; i < NUM_CONNECTION_ATTEMPTS; ++i) {
            status =
                sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);

            if(status == SL_STATUS_OK) {
                break;
            }

            if(i < NUM_CONNECTION_ATTEMPTS) {
                furi_delay_ms(250);
            }
        }

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to bring Wifi interface UP: %lX", status);
            break;
        }

        wifi_set_state(instance, WifiBackendStateConnected);

    } while(false);

    response->status = wifi_decode_sl_status(status);
    wifi_send_response(instance);
}

static void wifi_disconnect_request_handler(Wifi* instance) {
    FURI_LOG_D(TAG, "Disconnect");

    WifiResponse* response = &instance->response;

    sl_status_t status;

    do {
        status = sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to bring Wifi interface DOWN: %lX", status);
            break;
        }

        wifi_set_state(instance, WifiBackendStateDisconnected);

    } while(false);

    response->status = wifi_decode_sl_status(status);
    wifi_send_response(instance);
}

static void wifi_get_backend_info_request_handler(Wifi* instance) {
    FURI_LOG_D(TAG, "GetBackendInfo");

    WifiResponse* response = &instance->response;

    sl_status_t status;

    do {
        WifiBackendInfo* backend_info = &response->backend_info;

        sl_si91x_rsp_wireless_info_t wl_info;
        status = sl_wifi_get_wireless_info(&wl_info);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to get wireless info: %lX", status);
            break;
        }

        int32_t rssi;
        status = sl_wifi_get_signal_strength(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &rssi);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to get RSSI: %lX", status);
            break;
        }

        memcpy(backend_info->bssid, wl_info.bssid, HW_ADDRESS_LEN);
        backend_info->channel = wl_info.channel_number;
        backend_info->rssi = rssi;

    } while(false);

    response->status = wifi_decode_sl_status(status);
    wifi_send_response(instance);
}

static void wifi_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    furi_assert(data_size == sizeof(WifiRequest));

    Wifi* instance = context;
    memcpy(&instance->request, data, data_size);
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventRequest);
}

static void wifi_net_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    UNUSED(context);
    sl_wifi_send_raw_data_frame(SL_WIFI_CLIENT_INTERFACE, data, data_size);
}

static void wifi_prepare_scan_response(WifiResponse* response) {
    uint16_t results_count = 0;

    const size_t results_size = sizeof(sl_wifi_extended_scan_result_t) * SCAN_MAX_RESULTS;
    sl_wifi_extended_scan_result_t* scan_results = malloc(results_size);

    sl_wifi_extended_scan_result_parameters_t params = {
        .scan_results = scan_results,
        .array_length = results_size,
        .result_count = &results_count,
    };

    sl_status_t status = sl_wifi_get_stored_scan_results(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &params);

    if(status == SL_STATUS_OK) {
        results_count = MIN(results_count, SCAN_MAX_RESULTS);

        for(uint16_t i = 0; i < results_count; ++i) {
            const sl_wifi_extended_scan_result_t* result_in = &scan_results[i];
            WifiScanResult* result_out = &response->scan_results.data[i];

            strncpy(result_out->ssid, (const char*)result_in->ssid, SSID_MAX_LEN);
            result_out->security_mode = wifi_decode_security_mode(result_in->security_mode);
            result_out->rssi = result_in->rssi;
        }

    } else {
        response->status = wifi_decode_sl_status(status);
    }

    response->scan_results.count = results_count;

    sli_wifi_flush_scan_results_database();
    free(scan_results);
}

static void wifi_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);

    Wifi* instance = context;
    WifiResponse* response = &instance->response;

    if(events == WifiEventRequest) {
        const WifiRequestType request_type = instance->request.type;
        furi_check(request_type < WifiRequestTypeMax);

        response->type = request_type;
        wifi_request_handlers[request_type](instance);

    } else if(events == WifiEventScanComplete) {
        wifi_prepare_scan_response(response);
        wifi_send_response(instance);

    } else {
        furi_crash("Multiple Wifi events");
    }
}

static sl_status_t wifi_scan_callback(
    sl_wifi_event_t event,
    sl_wifi_scan_result_t* result,
    uint32_t result_length,
    void* context) {
    UNUSED(result_length);

    furi_assert(context);
    Wifi* instance = context;
    WifiResponse* response = &instance->response;

    sl_status_t ret, status;

    if(event & SL_WIFI_EVENT_FAIL_INDICATION) {
        status = *((sl_status_t*)result);
        ret = SL_STATUS_FAIL;

    } else {
        ret = status = SL_STATUS_OK;
    }

    response->status = wifi_decode_sl_status(status);
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventScanComplete);

    return ret;
}

static sl_status_t wifi_init_driver(Wifi* instance) {
    sl_status_t status;

    do {
        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_config_client, NULL, NULL);

        if(status != SL_STATUS_OK) {
            break;
        }

        status = sl_wifi_set_scan_callback(wifi_scan_callback, instance);

    } while(false);

    return status;
}

static Wifi* wifi_alloc(void) {
    Wifi* instance = malloc(sizeof(Wifi));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_pubsub = furi_pubsub_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->tcpip_lock = furi_semaphore_alloc(1, 0);
    instance->ip6_addr_valid = furi_semaphore_alloc(1, 0);

    furi_record_open(RECORD_NETWORK);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_custom_event_callback, instance);
    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifi, wifi_intercom_rx_callback, instance);
    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifiData, wifi_net_intercom_rx_callback, instance);

    wifi_net_tcpip_init(instance);

    const sl_status_t status = wifi_init_driver(instance);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to initialise Wifi: %lX", status);
    }

    furi_record_create(RECORD_WIFI, instance->event_pubsub);

    return instance;
}

int32_t wifi_srv(void* arg) {
    UNUSED(arg);

    Wifi* instance = wifi_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const WifiRequestHandler wifi_request_handlers[WifiRequestTypeMax] = {
    [WifiRequestTypeInit] = wifi_init_request_handler,
    [WifiRequestTypeScan] = wifi_scan_request_handler,
    [WifiRequestTypeConnect] = wifi_connect_request_handler,
    [WifiRequestTypeDisconnect] = wifi_disconnect_request_handler,
    [WifiRequestTypeGetBackendInfo] = wifi_get_backend_info_request_handler,
};
