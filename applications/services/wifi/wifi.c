#include "wifi_i.h"

#include <network/network.h>

#define STARTUP_THREAD_STACK_SIZE (2048UL)

static void wifi_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data_size == sizeof(WifiResponse));
    furi_assert(context);

    Wifi* instance = context;

    memcpy(&instance->response, data, data_size);
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventResponse);
}

static void wifi_save_default_settings(Wifi* instance) {
    WifiSettings* settings = &instance->settings;
    wifi_settings_init_defaults(settings);
    wifi_settings_save(settings);
}

static void wifi_load_settings(Wifi* instance) {
    if(!wifi_settings_load(&instance->settings)) {
        FURI_LOG_W(TAG, "Failed to load settings, using defaults");
        wifi_save_default_settings(instance);
    }
}

static void wifi_update_settings(
    Wifi* instance,
    const WifiCredentials* credentials,
    const WifiIpConfig* ip_config) {
    WifiSettings* settings = &instance->settings;

    bool save_file = false;

    if(memcmp(&settings->credentials, credentials, sizeof(WifiCredentials)) != 0) {
        settings->credentials = *credentials;
        save_file = true;
    }

    if(memcmp(&settings->ip_config, ip_config, sizeof(WifiIpConfig)) != 0) {
        settings->ip_config = *ip_config;
        save_file = true;
    }

    if(save_file) {
        wifi_settings_save(settings);
    }
}

static void wifi_process_request(Wifi* instance) {
    const WifiMessage* message = instance->current_message;
    WifiRequest* request = &instance->request;

    const WifiRequestType request_type = message->request_type;
    request->type = request_type;

    if(request_type == WifiRequestTypeConnect) {
        const WifiConnectMessage* connect_message = &message->connect_message;
        WifiConnectRequest* connect_request = &request->connect_request;

        connect_request->credentials = *connect_message->credentials;
        connect_request->ip = *connect_message->ip_config;
    }

    intercom_tx(
        instance->intercom, IntercomChannelWifi, request, sizeof(WifiRequest), FuriWaitForever);
}

static void wifi_process_response(Wifi* instance) {
    WifiMessage* message = instance->current_message;

    if(message == NULL) {
        // BUG: Figure out where the rogue responses come from
        FURI_LOG_W(TAG, "BUG: Rogue response of type %d", instance->response.type);
        return;
    }

    const WifiResponse* response = &instance->response;
    const WifiRequestType request_type = message->request_type;
    furi_assert(request_type == response->type);

    const WifiStatus status = response->status;

    if(status == WifiStatusOk) {
        if(request_type == WifiRequestTypeScan) {
            const uint8_t results_count =
                MIN(message->scan_message.max_count, response->scan_results.count);

            const WifiScanResult* results_in = response->scan_results.data;
            WifiScanResult* results_out = message->scan_message.data;

            memcpy(results_out, results_in, results_count * sizeof(WifiScanResult));
            *message->scan_message.count = results_count;

        } else if(request_type == WifiRequestTypeConnect) {
            const WifiConnectMessage* connect_message = &message->connect_message;
            const WifiIpConfig* ip_config = connect_message->ip_config;

            wifi_update_settings(instance, connect_message->credentials, ip_config);
            wifi_net_up(instance);

        } else if(request_type == WifiRequestTypeDisconnect) {
            wifi_net_down(instance);
            wifi_save_default_settings(instance);

        } else if(request_type == WifiRequestTypeGetInfo) {
            WifiInfo* info = message->get_info_message.info;
            *info = response->info;

            wifi_net_get_ip_config(instance, &info->ip_config);

        } else if(request_type == WifiRequestTypeGetHwAddress) {
            WifiHardwareAddress* hw_address = message->get_hw_address_message.hw_address;
            *hw_address = response->hw_address;
        }
    }

    message->status = status;
    api_lock_unlock(message->lock);

    instance->current_message = NULL;
    furi_semaphore_release(instance->access_semaphore);
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

static int32_t wifi_startup_thread_callback(void* arg) {
    furi_assert(arg);
    Wifi* instance = arg;

    do {
        // TODO [FW-300]: Implement reliable Intercom channel opening
        furi_delay_ms(250); // Wait for the Wifi service to become ready on Si917

        WifiHardwareAddress hw_address;
        if(wifi_get_hw_address(instance, &hw_address) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to get hardware address");
            break;
        }

        wifi_net_init(instance, &hw_address);

        const WifiSettings* settings = &instance->settings;
        const char* ssid = settings->credentials.ssid;

        if(strlen(ssid) == 0) {
            FURI_LOG_I(TAG, "No SSID specified");
            break;
        }

        FURI_LOG_I(TAG, "Connecting to \"%s\"", ssid);

        if(wifi_connect(instance, &settings->credentials, &settings->ip_config) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to connect");
            break;
        }

        FURI_LOG_I(TAG, "Connected to \"%s\"", ssid);

        WifiInfo info;
        if(wifi_get_info(instance, &info) != WifiStatusOk) {
            FURI_LOG_E(TAG, "Failed to get info");
            break;
        }

        const WifiIpConfig* ip_config = &info.ip_config;
        const WifiIpv4* addr = &ip_config->ip4.address;
        FURI_LOG_I(
            TAG,
            "IP: %hhu.%hhu.%hhu.%hhu",
            addr->bytes[0],
            addr->bytes[1],
            addr->bytes[2],
            addr->bytes[3]);

    } while(false);

    return 0;
}

static void
    wifi_startup_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
    }
}

static void wifi_run_startup_thread(Wifi* instance) {
    FuriThread* startup_thread = furi_thread_alloc_ex(
        "WifiStartup", STARTUP_THREAD_STACK_SIZE, wifi_startup_thread_callback, instance);

    furi_thread_set_state_callback(startup_thread, wifi_startup_thread_state_callback);
    furi_thread_start(startup_thread);
}

static Wifi* wifi_alloc(void) {
    Wifi* instance = malloc(sizeof(Wifi));

    instance->event_loop = furi_event_loop_alloc();
    instance->access_semaphore = furi_semaphore_alloc(1, 1);
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_record_open(RECORD_NETWORK);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_custom_event_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifi, wifi_intercom_rx_callback, instance);

    wifi_load_settings(instance);
    wifi_run_startup_thread(instance);

    furi_record_create(RECORD_WIFI, instance);

    return instance;
}

int32_t wifi_srv(void* arg) {
    UNUSED(arg);

    Wifi* instance = wifi_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
