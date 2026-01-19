#include "wifi_i.h"

#include <network/network.h>

#include "wifi_state.h"

static void wifi_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data_size == sizeof(WifiResponse));
    furi_assert(context);

    Wifi* instance = context;
    furi_check(
        furi_message_queue_put(instance->response_queue, data, FuriWaitForever) == FuriStatusOk);
}

static void wifi_print_connection_info(Wifi* instance) {
    with_furi_state(instance->state, const WifiInfo* info, {
        const WifiIpv4* addr = &info->ip_config.ip4.address;

        FURI_LOG_I(
            TAG,
            "Connection success\r\n"
            "\tSSID:\t\t%s\r\n"
            "\tIPv4 address:\t%hhu.%hhu.%hhu.%hhu",
            info->ssid,
            addr->bytes[0],
            addr->bytes[1],
            addr->bytes[2],
            addr->bytes[3]);
    });
}

static void wifi_apply_settings_pending_callback(void* context) {
    furi_assert(context);
    Wifi* instance = context;

    do {
        WifiSettings settings;
        wifi_settings_load(&settings);

        if(strnlen(settings.credentials.ssid, SSID_MAX_LEN) == 0) {
            FURI_LOG_I(TAG, "No SSID specified");
            break;
        }

        wifi_schedule_connect_request(instance, &settings);

    } while(false);
}

static void wifi_process_request(Wifi* instance) {
    const WifiMessage* message = &instance->api_message;
    WifiRequest* request = &instance->request;

    const WifiRequestType request_type = message->request_type;
    const WifiStatus status = wifi_state_check_request_type(instance, request_type);

    if(status == WifiStatusOk) {
        if(request_type == WifiRequestTypeConnect) {
            const WifiConnectMessage* connect_message = &message->connect_message;
            const WifiCredentials* credentials = &connect_message->credentials;

            WifiConnectRequest* connect_request = &request->connect_request;
            connect_request->credentials = *credentials;

            wifi_state_transition(instance, WifiStateConnecting, credentials);

            FURI_LOG_I(TAG, "Connecting to \"%s\"", credentials->ssid);

        } else if(request_type == WifiRequestTypeDisconnect) {
            wifi_state_transition(instance, WifiStateDisconnecting);
        }

        request->type = request_type;

        intercom_tx(instance->intercom_ch_control, request, sizeof(WifiRequest), FuriWaitForever);

    } else {
        FURI_LOG_E(TAG, "Request type: %d failed with status: %d", request_type, status);
        wifi_api_unlock(instance, status);
    }
}

static void wifi_process_response(Wifi* instance, const WifiResponse* response) {
    furi_assert(wifi_api_is_locked(instance));
    WifiMessage* message = &instance->api_message;

    const WifiRequestType request_type = message->request_type;
    furi_assert(request_type == response->type);

    WifiStatus status = response->status;

    if(status == WifiStatusOk) {
        if(request_type == WifiRequestTypeInit) {
            wifi_net_init(instance, response->hw_address);
            wifi_state_transition(instance, WifiStateDisconnected);
            // Asynchronously load and apply settings if needed
            furi_event_loop_pend_callback(
                instance->event_loop, wifi_apply_settings_pending_callback, instance);

        } else if(request_type == WifiRequestTypeScan) {
            WifiScanMessage* scan_message = &message->scan_message;
            const WifiScanResults* scan_results = &response->scan_results;

            const uint8_t results_count = MIN(scan_message->max_count, scan_results->count);

            const WifiScanResult* results_in = scan_results->data;
            WifiScanResult* results_out = scan_message->data;

            memcpy(results_out, results_in, results_count * sizeof(WifiScanResult));
            *scan_message->count = results_count;

        } else if(request_type == WifiRequestTypeConnect) {
            const WifiConnectMessage* connect_message = &message->connect_message;
            const WifiCredentials* credentials = &connect_message->credentials;
            const WifiIpConfig* ip_config = &connect_message->ip_config;

            if(wifi_net_up(instance, ip_config)) {
                WifiIpConfig new_ip_config;
                wifi_net_get_ip_config(instance, &new_ip_config);

                wifi_state_transition(instance, WifiStateConnected, credentials, &new_ip_config);
                wifi_settings_save(&(WifiSettings){
                    .credentials = *credentials,
                    .ip_config = *ip_config,
                });

                wifi_print_connection_info(instance);

            } else {
                status = WifiStatusTimeout;
                wifi_state_transition(instance, WifiStateDisconnected);
            }

        } else if(request_type == WifiRequestTypeDisconnect) {
            wifi_net_down(instance);

            wifi_state_transition(instance, WifiStateDisconnected);

            wifi_settings_reset(NULL);
        }

    } else {
        FURI_LOG_E(TAG, "Request type: %d failed with status: %d", request_type, status);

        if(request_type == WifiRequestTypeConnect) {
            wifi_state_transition(instance, WifiStateDisconnected);
            wifi_settings_reset(NULL);

        } else if(request_type == WifiRequestTypeDisconnect) {
            wifi_state_transition(instance, WifiStateDisconnected);
        }
    }

    wifi_api_unlock(instance, status);
}

static void
    wifi_process_backend_info_response(Wifi* instance, const WifiBackendInfo* backend_info) {
    WifiInfo wifi_info;
    furi_state_get(instance->state, &wifi_info);

    if(wifi_info.state == WifiStateConnected) {
        if(backend_info->state == WifiBackendStateReconnecting) {
            FURI_LOG_W(TAG, "Disconnected from \"%s\", trying to reconnect", wifi_info.ssid);

            wifi_net_down(instance);
            wifi_state_transition(instance, WifiStateReconnecting);
        }

    } else if(wifi_info.state == WifiStateReconnecting) {
        if(backend_info->state == WifiBackendStateConnected) {
            FURI_LOG_I(TAG, "Reconnected to \"%s\"", wifi_info.ssid);

            if(wifi_net_up(instance, &wifi_info.ip_config)) {
                WifiIpConfig new_ip_config;
                wifi_net_get_ip_config(instance, &new_ip_config);

                wifi_state_transition(instance, WifiStateConnected, &new_ip_config);
                wifi_print_connection_info(instance);

            } else {
                wifi_schedule_disconnect_request(instance);
            }

        } else if(backend_info->state == WifiBackendStateDisconnected) {
            wifi_state_transition(instance, WifiStateDisconnected);
        }
    }

    wifi_state_update_backend_info(instance, backend_info);
}

static void wifi_process_async_response(Wifi* instance, const WifiResponse* response) {
    const WifiRequestType response_type = response->type;

    if(response_type == WifiRequestTypeBackendInfo) {
        wifi_process_backend_info_response(instance, &response->backend_info);
    } else {
        furi_crash("Invalid WifiRequestType");
    }
}

static void wifi_response_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Wifi* instance = context;
    furi_assert(object == instance->response_queue);

    WifiResponse response;
    while(furi_message_queue_get(instance->response_queue, &response, 0) == FuriStatusOk) {
        if(response.type != WifiRequestTypeBackendInfo) {
            wifi_process_response(instance, &response);
        } else {
            wifi_process_async_response(instance, &response);
        }
    }
}

static void wifi_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Wifi* instance = context;

    if(events == WifiEventRequest) {
        wifi_process_request(instance);
    } else {
        furi_crash("Multiple Wifi events");
    }
}

static Wifi* wifi_alloc(void) {
    Wifi* instance = malloc(sizeof(Wifi));

    instance->event_loop = furi_event_loop_alloc();
    instance->response_queue = furi_message_queue_alloc(3, sizeof(WifiResponse));
    instance->api_semaphore = furi_semaphore_alloc(1, 1);
    instance->dhcp_semaphore = furi_semaphore_alloc(1, 0);
    instance->state = furi_state_alloc(sizeof(WifiInfo));
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_record_open(RECORD_NETWORK);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_custom_event_callback, instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->response_queue,
        FuriEventLoopEventIn,
        wifi_response_queue_callback,
        instance);

    instance->intercom_ch_control = intercom_channel_open(
        instance->intercom, IntercomChannelIdWifiControl, wifi_intercom_rx_callback, instance);

    wifi_schedule_init_request(instance);

    furi_record_create(RECORD_WIFI, instance);

    return instance;
}

int32_t wifi_srv(void* arg) {
    UNUSED(arg);

    Wifi* instance = wifi_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
