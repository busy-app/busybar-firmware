#include "wifi_i.h"

#include <network/network.h>

#include "wifi_state.h"

#define WIFI_POLL_INTERVAL_MS (15 * 1000)

static void wifi_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data_size == sizeof(WifiResponse));
    furi_assert(context);

    Wifi* instance = context;

    memcpy(&instance->response, data, data_size);
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventResponse);
}

static void wifi_save_settings(const WifiCredentials* credentials, const WifiIpConfig* ip_config) {
    const WifiSettings settings = {
        .credentials = *credentials,
        .ip_config = *ip_config,
    };

    wifi_settings_save(&settings);
}

static void wifi_save_default_settings(void) {
    WifiSettings default_settings;
    wifi_settings_init_defaults(&default_settings);
    wifi_settings_save(&default_settings);
}

static void wifi_print_connection_info(Wifi* instance) {
    const WifiInfo* info = &instance->info;
    const WifiIpv4* addr = &info->ip_config.ip4.address;

    FURI_LOG_I(TAG, "Connected to \"%s\"", info->ssid);

    FURI_LOG_I(
        TAG,
        "IP: %hhu.%hhu.%hhu.%hhu",
        addr->bytes[0],
        addr->bytes[1],
        addr->bytes[2],
        addr->bytes[3]);
}

static void wifi_schedule_backend_info_request(void* context) {
    furi_assert(context);

    Wifi* instance = context;

    const WifiMessage message = {
        .request_type = WifiRequestTypeGetBackendInfo,
    };

    wifi_api_nonblocking_request(instance, &message);
}

static void wifi_process_request(Wifi* instance) {
    const WifiMessage* message = &instance->api_message;
    WifiRequest* request = &instance->request;

    const WifiRequestType request_type = message->request_type;
    request->type = request_type;

    if(request_type == WifiRequestTypeConnect) {
        const WifiConnectMessage* connect_message = &message->connect_message;
        const WifiCredentials* credentials = connect_message->credentials;

        WifiConnectRequest* connect_request = &request->connect_request;
        connect_request->credentials = *credentials;

        wifi_state_transition(instance, WifiStateConnecting);

        FURI_LOG_I(TAG, "Connecting to \"%s\"", credentials->ssid);

    } else if(request_type == WifiRequestTypeDisconnect) {
        wifi_state_transition(instance, WifiStateDisconnecting);
    }

    intercom_tx(
        instance->intercom, IntercomChannelWifi, request, sizeof(WifiRequest), FuriWaitForever);
}

static void wifi_process_response(Wifi* instance) {
    WifiMessage* message = &instance->api_message;

    if(!wifi_api_is_locked(instance)) {
        // BUG: Figure out where the rogue responses come from
        FURI_LOG_W(TAG, "BUG: Rogue response of type %d", instance->response.type);
        return;
    }

    const WifiResponse* response = &instance->response;
    const WifiRequestType request_type = message->request_type;
    furi_assert(request_type == response->type);

    WifiStatus status = response->status;

    if(status == WifiStatusOk) {
        if(request_type == WifiRequestTypeInit) {
            const WifiHardwareAddress* hw_address = &response->hw_address;

            wifi_net_init(instance, hw_address);
            wifi_state_transition(instance, WifiStateDisconnected, hw_address);

        } else if(request_type == WifiRequestTypeScan) {
            const uint8_t results_count =
                MIN(message->scan_message.max_count, response->scan_results.count);

            const WifiScanResult* results_in = response->scan_results.data;
            WifiScanResult* results_out = message->scan_message.data;

            memcpy(results_out, results_in, results_count * sizeof(WifiScanResult));
            *message->scan_message.count = results_count;

        } else if(request_type == WifiRequestTypeConnect) {
            const WifiConnectMessage* connect_message = &message->connect_message;
            const WifiCredentials* credentials = connect_message->credentials;
            const WifiIpConfig* ip_config = connect_message->ip_config;

            if(wifi_net_up(instance, ip_config)) {
                WifiIpConfig new_ip_config;
                wifi_net_get_ip_config(instance, &new_ip_config);

                wifi_state_transition(instance, WifiStateConnected, credentials, &new_ip_config);
                wifi_save_settings(credentials, ip_config);

                wifi_print_connection_info(instance);

                furi_event_loop_pend_callback(
                    instance->event_loop, wifi_schedule_backend_info_request, instance);

                furi_event_loop_timer_start(instance->poll_timer, WIFI_POLL_INTERVAL_MS);

            } else {
                status = WifiStatusError;
                wifi_state_transition(instance, WifiStateDisconnected);
            }

        } else if(request_type == WifiRequestTypeDisconnect) {
            wifi_net_down(instance);

            wifi_state_transition(instance, WifiStateDisconnected);

            wifi_save_default_settings();

            furi_event_loop_timer_stop(instance->poll_timer);

        } else if(request_type == WifiRequestTypeGetBackendInfo) {
            wifi_state_update_backend_info(instance, &response->backend_info);
        }

    } else {
        // TODO: Transitions from error conditions
        FURI_LOG_E(TAG, "Request type: %d failed with status: %d", request_type, status);
    }

    wifi_api_unlock(instance, status);
}

static void wifi_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Wifi* instance = context;

    if(events == WifiEventRequest) {
        wifi_process_request(instance);
    } else if(events == WifiEventResponse) {
        wifi_process_response(instance);
    } else {
        furi_crash("Multiple Wifi events");
    }
}

static Wifi* wifi_alloc(void) {
    Wifi* instance = malloc(sizeof(Wifi));

    instance->event_loop = furi_event_loop_alloc();
    instance->poll_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        wifi_schedule_backend_info_request,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->api_semaphore = furi_semaphore_alloc(1, 1);
    // instance->state = furi_state_alloc(sizeof(WifiInfo));
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_record_open(RECORD_NETWORK);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_custom_event_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifi, wifi_intercom_rx_callback, instance);

    furi_record_create(RECORD_WIFI, instance);

    return instance;
}

int32_t wifi_srv(void* arg) {
    UNUSED(arg);

    Wifi* instance = wifi_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
