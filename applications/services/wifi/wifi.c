#include "wifi_i.h"

#include <network/network.h>

#include "wifi_state.h"

#define API_QUEUE_SIZE            (1)
#define EVENT_QUEUE_SIZE          (4)
#define PRIORITY_QUEUE_SIZE       (1)
#define RESPONSE_QUEUE_SIZE       (4)
#define RESPONSE_QUEUE_TIMEOUT_MS (200)

#define WIFI_REQUEST_TIMEOUT_MS (5000)

#if(API_QUEUE_SIZE != 1)
#error "API logic will break with API_QUEUE_SIZE other than 1"
#endif

static void wifi_intercom_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    Wifi* instance = context;
    const IntercomStatus intercom_status = *(IntercomStatus*)item;

    if(intercom_status == IntercomStatusOk) {
        wifi_schedule_init_request(instance);
    } else if(intercom_status != IntercomStatusUnknown) {
        wifi_schedule_deinit_request(instance);
    }
}

static void wifi_device_name_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    Wifi* instance = context;
    const DeviceNameInfo* device_name_info = item;

    wifi_send_device_name_info_event(instance, device_name_info);
}

static void wifi_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data_size == sizeof(WifiResponse));
    furi_assert(context);

    Wifi* instance = context;

    const FuriStatus status = furi_message_queue_put(
        instance->response_queue, data, furi_ms_to_ticks(RESPONSE_QUEUE_TIMEOUT_MS));

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorTimeout);
        FURI_LOG_E(TAG, "BUG: response queue overrun");
    }
}

static void wifi_print_connection_info(Wifi* instance) {
    with_furi_state(instance->state, const WifiInfo* info, {
        const WifiIpv4Settings* addresses = &info->ip_config.ip4;

        FURI_LOG_I(
            TAG,
            "Connection success\r\n"
            "\tSSID:\t\t%s\r\n"
            "\tIPv4 address:\t" WIFI_IP4_ADDR_FORMAT "\r\n"
            "\tIPv4 gateway:\t" WIFI_IP4_ADDR_FORMAT "\r\n"
            "\tIPv4 mask:\t" WIFI_IP4_ADDR_FORMAT "\r\n"
            "\tDNS address:\t" WIFI_IP4_ADDR_FORMAT "\r\n",
            info->ssid,
            WIFI_IP4_ADDR_SPREAD(&addresses->address),
            WIFI_IP4_ADDR_SPREAD(&addresses->gateway),
            WIFI_IP4_ADDR_SPREAD(&addresses->mask),
            WIFI_IP4_ADDR_SPREAD(&addresses->dns));
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

static WifiStatus wifi_send_request(Wifi* instance, WifiRequestType request_type) {
    WifiRequest* request = &instance->request;

    request->type = request_type;

    const size_t tx_size = intercom_tx(
        instance->intercom_ch_control, request, sizeof(WifiRequest), WIFI_REQUEST_TIMEOUT_MS);
    return (tx_size == sizeof(WifiRequest)) ? WifiStatusOk : WifiStatusTimeout;
}

static void wifi_process_request(Wifi* instance) {
    furi_assert(instance->is_processing);

    const WifiMessage* message = &instance->api_message;

    WifiRequest* request = &instance->request;
    const WifiRequestType request_type = message->request_type;

    WifiStatus status;
    bool unlock_api = true;

    do {
        status = wifi_state_check_request_type(instance, request_type);

        if(status != WifiStatusOk) {
            break;
        }

        if(request_type == WifiRequestTypeInit) {
            FURI_LOG_I(TAG, "Initializing");
            instance->intercom_ch_control = intercom_channel_open(
                instance->intercom,
                IntercomChannelIdWifiControl,
                wifi_intercom_rx_callback,
                instance);

        } else if(request_type == WifiRequestTypeConnect) {
            const WifiConnectMessage* connect_message = &message->connect_message;
            const WifiCredentials* credentials = &connect_message->credentials;

            WifiConnectRequest* connect_request = &request->connect_request;
            connect_request->credentials = *credentials;

            wifi_state_transition(instance, WifiStateConnecting, credentials);
            FURI_LOG_I(TAG, "Connecting to \"%s\"", credentials->ssid);

        } else if(request_type == WifiRequestTypeDisconnect) {
            FURI_LOG_I(TAG, "Disconnecting");
            wifi_state_transition(instance, WifiStateDisconnecting);

        } else if(request_type == WifiRequestTypeForget) {
            FURI_LOG_I(TAG, "Forgetting saved network");
            wifi_settings_reset(NULL);
            break; // No backend request necessary

        } else if(request_type == WifiRequestTypeDeinit) {
            FURI_LOG_W(TAG, "Deinitializing due to error");
            wifi_net_down(instance);
            wifi_state_transition(instance, WifiStateUnknown);
            break; // No backend request necessary
        }

        status = wifi_send_request(instance, request_type);

        if(status != WifiStatusOk) {
            break;
        }

        unlock_api = false;

    } while(false);

    if(status != WifiStatusOk) {
        FURI_LOG_E(TAG, "Request type: %d failed with status: %d", request_type, status);
    }

    if(unlock_api) {
        wifi_api_unlock(instance, status);
    }
}

static void
    wifi_followup_request(Wifi* instance, WifiRequestType request_type, WifiStatus prev_status) {
    instance->api_message.request_type = request_type;
    *instance->api_message.status = prev_status;

    wifi_process_request(instance);
}

static void wifi_process_response(Wifi* instance, const WifiResponse* response) {
    furi_assert(instance->is_processing);
    WifiMessage* message = &instance->api_message;

    const WifiRequestType request_type = message->request_type;
    furi_assert(request_type == response->type);

    WifiStatus status = response->status;

    bool unlock_api = true;

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

                wifi_state_transition(instance, WifiStateConnected, &new_ip_config);
                wifi_settings_save(&(const WifiSettings){
                    .credentials = *credentials,
                    .ip_config = *ip_config,
                });

                wifi_print_connection_info(instance);

            } else {
                unlock_api = false;

                wifi_followup_request(
                    instance, WifiRequestTypeDisconnect, WifiStatusConfigurationFailed);
            }

        } else if(request_type == WifiRequestTypeDisconnect) {
            wifi_net_down(instance);
            wifi_state_transition(instance, WifiStateDisconnected);
        }

    } else {
        FURI_LOG_E(TAG, "Request type: %d failed with status: %d", request_type, status);

        if(request_type == WifiRequestTypeConnect) {
            wifi_state_transition(instance, WifiStateDisconnected);
        } else if(request_type == WifiRequestTypeDisconnect) {
            wifi_state_transition(instance, WifiStateDisconnected);
        }
    }

    if(unlock_api) {
        wifi_api_unlock(instance, status);
    }
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

static void wifi_process_event(Wifi* instance, const WifiEvent* event) {
    const WifiEventType event_type = event->type;

    if(event_type == WifiEventTypeDeviceNameInfo) {
        wifi_net_set_hostname(instance, event->device_name_info.name);
    } else {
        furi_crash("Invalid WifiEventType value");
    }
}

static void wifi_process_api_request(Wifi* instance) {
    if(instance->is_processing) {
        return;
    }

    furi_check(
        furi_message_queue_peek(instance->api_queue, &instance->api_message, 0) == FuriStatusOk);

    instance->is_processing = true;
    wifi_process_request(instance);
}

static void wifi_process_priority_request(Wifi* instance) {
    if(instance->is_processing) {
        furi_check(furi_message_queue_reset(instance->response_queue) == FuriStatusOk);
        wifi_api_unlock(instance, WifiStatusError);
    }

    furi_check(
        furi_message_queue_get(instance->priority_queue, &instance->api_message, 0) ==
        FuriStatusOk);

    instance->is_processing = true;
    wifi_process_request(instance);
}

static void wifi_api_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Wifi* instance = context;
    furi_assert(object == instance->api_queue);

    wifi_process_api_request(instance);
}

static void wifi_priority_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Wifi* instance = context;
    furi_assert(object == instance->priority_queue);

    wifi_process_priority_request(instance);
}

static void wifi_event_queue_callback(FuriEventLoopObject* object, void* context) {
    Wifi* instance = context;
    furi_assert(object == instance->event_queue);

    WifiEvent event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        wifi_process_event(instance, &event);
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

static Wifi* wifi_alloc(void) {
    Wifi* instance = malloc(sizeof(Wifi));

    instance->event_loop = furi_event_loop_alloc();
    instance->api_queue = furi_message_queue_alloc(API_QUEUE_SIZE, sizeof(WifiMessage));
    instance->event_queue = furi_message_queue_alloc(EVENT_QUEUE_SIZE, sizeof(WifiEvent));
    instance->priority_queue = furi_message_queue_alloc(PRIORITY_QUEUE_SIZE, sizeof(WifiMessage));
    instance->response_queue = furi_message_queue_alloc(RESPONSE_QUEUE_SIZE, sizeof(WifiResponse));
    instance->dhcp_semaphore = furi_semaphore_alloc(1, 0);
    instance->state = furi_state_alloc(sizeof(WifiInfo));
    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->hostname = furi_string_alloc();

    wifi_power_init(instance);

    furi_record_open(RECORD_NETWORK);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_queue,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        wifi_api_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        wifi_event_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->priority_queue,
        FuriEventLoopEventIn,
        wifi_priority_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->response_queue,
        FuriEventLoopEventIn,
        wifi_response_queue_callback,
        instance);

    DeviceName* device_name = furi_record_open(RECORD_DEVICE_NAME);
    furi_state_subscribe(
        device_name_get_state(device_name), wifi_device_name_state_callback, instance);

    furi_state_subscribe(
        intercom_get_state(instance->intercom), wifi_intercom_state_callback, instance);

    furi_record_create(RECORD_WIFI, instance);

    return instance;
}

void wifi_pending_request_callback(void* context) {
    furi_assert(context);
    Wifi* instance = context;

    wifi_process_api_request(instance);
}

int32_t wifi_srv(void* arg) {
    UNUSED(arg);

    Wifi* instance = wifi_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
