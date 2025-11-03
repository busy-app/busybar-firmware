#include "wifi_backend_i.h"

#include <sl_net.h>
#include <sl_wifi.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_callback_framework.h>

#include "wifi_config.h"
#include "wifi_backend_util.h"

#define NUM_CONNECTION_ATTEMPTS 3
#define SCAN_INTERVAL_S         5
#define BEACON_MISSED_COUNT     40

#define STATE_CODE_UPPER_MASK 0xF0
#define REASON_CODE_MSB_MASK  0x7F

#define STATE_CODE_ASSOCIATED   0x80
#define STATE_CODE_DEASSOCIATED 0x90

#define REASON_CODE_NO_RESPONSE  0x01
#define REASON_CODE_ASSOC_DENIAL 0x02
#define REASON_CODE_AP_NOT_FOUND 0x03
#define REASON_CODE_DEAUTH_USER  0x06
#define REASON_CODE_KEY_FAILURE  0x08
#define REASON_CODE_BEACON_LOSS  0x10

#define INFO_TIMER_PERIOD_MS (15 * 1000)

typedef enum {
    WifiEventTypeRequestReceived,
    WifiEventTypeScanFinished,
    WifiEventTypeModuleStats,
    WifiEventTypeMax,
} WifiEventType;

typedef struct {
    WifiRequest request;
} WifiRequestReceivedEvent;

typedef struct {
    sl_status_t status;
} WifiScanFinishedEvent;

typedef struct {
    uint8_t state_code;
    uint8_t reason_code;
} WifiModuleStatsEvent;

typedef struct {
    WifiEventType type;
    union {
        WifiRequestReceivedEvent request_received;
        WifiScanFinishedEvent scan_finished;
        WifiModuleStatsEvent module_stats;
    };
} WifiEvent;

typedef sl_status_t (
    *WifiRequestHandler)(Wifi* instance, const WifiRequest* request, WifiResponse* response);

static const WifiRequestHandler wifi_request_handlers[WifiRequestTypeMax];

static inline void wifi_send_response(Wifi* instance, const WifiResponse* response) {
    const size_t tx_size = intercom_tx(
        instance->intercom, IntercomChannelWifi, response, sizeof(WifiResponse), FuriWaitForever);
    furi_check(tx_size == sizeof(WifiResponse));
}

static void wifi_backend_info_callback(void* context) {
    furi_assert(context);
    Wifi* instance = context;

    WifiResponse response = {
        .type = WifiRequestTypeBackendInfo,
    };

    sl_status_t status;

    do {
        WifiBackendInfo* backend_info = &response.backend_info;
        backend_info->state = instance->state;

        if(instance->state != WifiBackendStateConnected) {
            status = SL_STATUS_OK;
            break;
        }

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

    response.status = wifi_decode_sl_status(status);

    wifi_send_response(instance, &response);
}

static inline void wifi_set_state(Wifi* instance, WifiBackendState state) {
    if(state != instance->state) {
        instance->state = state;
        furi_pubsub_publish(instance->event_pubsub, &instance->state);
    }
}

static sl_status_t
    wifi_init_request_handler(Wifi* instance, const WifiRequest* request, WifiResponse* response) {
    UNUSED(instance);
    UNUSED(request);

    FURI_LOG_D(TAG, "Init");

    const sl_status_t status =
        sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, (sl_mac_address_t*)response->hw_address);

    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to get MAC address: %lX", status);
    }

    return status;
}

static sl_status_t
    wifi_scan_request_handler(Wifi* instance, const WifiRequest* request, WifiResponse* response) {
    UNUSED(instance);
    UNUSED(request);
    UNUSED(response);

    FURI_LOG_D(TAG, "Scan");

    sl_wifi_scan_configuration_t wifi_scan_configuration = default_wifi_scan_configuration;
    wifi_scan_configuration.type = SL_WIFI_SCAN_TYPE_EXTENDED;

    const sl_status_t status =
        sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);

    if(status != SL_STATUS_IN_PROGRESS) {
        FURI_LOG_E(TAG, "Failed to initiate scan: %lX", status);
    }

    return status;
}

static sl_status_t wifi_connect_request_handler(
    Wifi* instance,
    const WifiRequest* request,
    WifiResponse* response) {
    UNUSED(response);

    FURI_LOG_D(TAG, "Connect");

    sl_status_t status;

    do {
        const WifiCredentials* credentials = &request->connect_request.credentials;

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

    return status;
}

static sl_status_t wifi_disconnect_request_handler(
    Wifi* instance,
    const WifiRequest* request,
    WifiResponse* response) {
    UNUSED(request);
    UNUSED(response);

    FURI_LOG_D(TAG, "Disconnect");

    sl_status_t status;

    do {
        status = sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);

        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to bring Wifi interface DOWN: %lX", status);
            break;
        }

        wifi_set_state(instance, WifiBackendStateDisconnected);

    } while(false);

    return status;
}

static void wifi_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    furi_assert(data_size == sizeof(WifiRequest));

    Wifi* instance = context;

    WifiEvent wifi_event = {
        .type = WifiEventTypeRequestReceived,
    };

    memcpy(&wifi_event.request_received, data, data_size);

    furi_check(
        furi_message_queue_put(instance->event_queue, &wifi_event, FuriWaitForever) ==
        FuriStatusOk);
}

static void wifi_net_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    UNUSED(context);
    sl_wifi_send_raw_data_frame(SL_WIFI_CLIENT_INTERFACE, data, data_size);
}

static void
    wifi_request_received_event_handler(Wifi* instance, const WifiRequestReceivedEvent* event) {
    const WifiRequest* request = &event->request;
    const WifiRequestType request_type = request->type;
    furi_check(request_type < WifiRequestTypeBackendInfo);

    WifiResponse response = {0};
    const sl_status_t status = wifi_request_handlers[request_type](instance, request, &response);

    if(status != SL_STATUS_IN_PROGRESS) {
        response.type = request_type;
        response.status = wifi_decode_sl_status(status);

        wifi_send_response(instance, &response);
    }
}

static void wifi_scan_finished_event_handler(Wifi* instance, const WifiScanFinishedEvent* event) {
    WifiResponse response = {
        .type = WifiRequestTypeScan,
        .status = wifi_decode_sl_status(event->status),
    };

    if(response.status == WifiStatusOk) {
        uint16_t results_count = 0;

        const size_t results_size = sizeof(sl_wifi_extended_scan_result_t) * SCAN_MAX_RESULTS;
        sl_wifi_extended_scan_result_t* scan_results = malloc(results_size);

        sl_wifi_extended_scan_result_parameters_t params = {
            .scan_results = scan_results,
            .array_length = results_size,
            .result_count = &results_count,
        };

        sl_status_t status =
            sl_wifi_get_stored_scan_results(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &params);

        if(status == SL_STATUS_OK) {
            results_count = MIN(results_count, SCAN_MAX_RESULTS);

            for(uint16_t i = 0; i < results_count; ++i) {
                const sl_wifi_extended_scan_result_t* result_in = &scan_results[i];
                WifiScanResult* result_out = &response.scan_results.data[i];

                strncpy(result_out->ssid, (const char*)result_in->ssid, SSID_MAX_LEN);
                result_out->security_mode = wifi_decode_security_mode(result_in->security_mode);
                result_out->rssi = result_in->rssi;
            }
        }

        response.status = wifi_decode_sl_status(status);
        response.scan_results.count = results_count;

        sli_wifi_flush_scan_results_database();
        free(scan_results);

    } else {
        FURI_LOG_E(TAG, "Scan finished with error: 0x%lX", event->status);
    }

    wifi_send_response(instance, &response);
}

static void wifi_module_stats_event_handler(Wifi* instance, const WifiModuleStatsEvent* event) {
    const uint8_t state_code = event->state_code & STATE_CODE_UPPER_MASK;
    const uint8_t reason_code = event->reason_code & REASON_CODE_MSB_MASK;

    if(state_code == STATE_CODE_ASSOCIATED) {
        if(instance->state == WifiBackendStateReconnecting) {
            if(wifi_net_tcpip_netif_up(instance)) {
                wifi_set_state(instance, WifiBackendStateConnected);
            } else {
                wifi_set_state(instance, WifiBackendStateDisconnected);
            }

        } else if(instance->state == WifiBackendStateDisconnected) {
            FURI_LOG_W(TAG, "Association while disconnected");
        }

        furi_event_loop_timer_start(instance->info_timer, INFO_TIMER_PERIOD_MS);
        furi_event_loop_pend_callback(instance->event_loop, wifi_backend_info_callback, instance);

    } else if(state_code == STATE_CODE_DEASSOCIATED) {
        if(reason_code == REASON_CODE_DEAUTH_USER) {
            if(instance->state != WifiBackendStateDisconnected) {
                FURI_LOG_W(TAG, "Deassociation by user request while disconnected");
            }

        } else if(reason_code == REASON_CODE_NO_RESPONSE || reason_code == REASON_CODE_BEACON_LOSS) {
            if(instance->state == WifiBackendStateConnected) {
                wifi_set_state(instance, WifiBackendStateReconnecting);
                wifi_net_tcpip_netif_down(instance);
            }

            FURI_LOG_D(TAG, "No response from AP, current state: %d", instance->state);

        } else if(reason_code == REASON_CODE_AP_NOT_FOUND) {
            if(instance->state == WifiBackendStateReconnecting) {
                wifi_set_state(instance, WifiBackendStateDisconnected);
            }

            FURI_LOG_D(TAG, "AP not found, current state: %d", instance->state);

        } else if(reason_code == REASON_CODE_ASSOC_DENIAL || reason_code == REASON_CODE_KEY_FAILURE) {
            if(instance->state == WifiBackendStateReconnecting) {
                FURI_LOG_W(TAG, "AP credentials were changed, disconnecting");
                wifi_set_state(instance, WifiBackendStateDisconnected);

            } else {
                FURI_LOG_T(TAG, "Authentication error");
            }

        } else {
            FURI_LOG_E(TAG, "BUG: Unhandled reason code 0x%hhX, please report", reason_code);
        }

        furi_event_loop_timer_stop(instance->info_timer);
        furi_event_loop_pend_callback(instance->event_loop, wifi_backend_info_callback, instance);

    } else {
        FURI_LOG_T(TAG, "Module state: 0x%hhX, reason: 0x%hhX", state_code, reason_code);
    }
}

static void wifi_event_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Wifi* instance = context;
    furi_assert(object == instance->event_queue);

    WifiEvent event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        const WifiEventType event_type = event.type;

        if(event_type == WifiEventTypeRequestReceived) {
            wifi_request_received_event_handler(instance, &event.request_received);
        } else if(event_type == WifiEventTypeScanFinished) {
            wifi_scan_finished_event_handler(instance, &event.scan_finished);
        } else if(event_type == WifiEventTypeModuleStats) {
            wifi_module_stats_event_handler(instance, &event.module_stats);
        } else {
            furi_crash("Invalid WifiEventType");
        }
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

    sl_status_t ret = SL_STATUS_OK;
    sl_status_t status = SL_STATUS_OK;

    if(event & SL_WIFI_EVENT_FAIL_INDICATION) {
        event &= ~SL_WIFI_EVENT_FAIL_INDICATION;
        status = *((sl_status_t*)result);
        ret = SL_STATUS_FAIL;
    }

    if(event == SL_WIFI_SCAN_RESULT_EVENT) {
        const WifiEvent wifi_event = {
            .type = WifiEventTypeScanFinished,
            .scan_finished =
                {
                    .status = status,
                },
        };

        furi_check(
            furi_message_queue_put(instance->event_queue, &wifi_event, FuriWaitForever) ==
            FuriStatusOk);
    }

    return ret;
}

static sl_status_t
    wifi_stats_callback(sl_wifi_event_t event, void* data, uint32_t data_length, void* context) {
    furi_assert(context);
    Wifi* instance = context;

    sl_status_t ret = SL_STATUS_OK;

    if(event & SL_WIFI_EVENT_FAIL_INDICATION) {
        ret = SL_STATUS_FAIL;

    } else if(event == SL_WIFI_STATS_MODULE_STATE_EVENT) {
        furi_assert(data);
        furi_assert(data_length = sizeof(sl_wifi_module_state_stats_response_t));

        const sl_wifi_module_state_stats_response_t* response = data;

        const WifiEvent wifi_event = {
            .type = WifiEventTypeModuleStats,
            .module_stats =
                {
                    .state_code = response->state_code,
                    .reason_code = response->reason_code,
                },
        };

        furi_check(
            furi_message_queue_put(instance->event_queue, &wifi_event, FuriWaitForever) ==
            FuriStatusOk);
    }

    return ret;
}

static sl_status_t wifi_init_driver(Wifi* instance) {
    sl_status_t status;

    do {
        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_config_client, NULL, NULL);

        if(status != SL_STATUS_OK) {
            break;
        }

        static const sl_wifi_advanced_client_configuration_t adv_cfg = {
            .max_retry_attempts = UINT32_MAX, // Try reconnecting infinitely if connection was lost
            .scan_interval = SCAN_INTERVAL_S,
            .beacon_missed_count = BEACON_MISSED_COUNT,
            .first_time_retry_enable = 0, // Initial retry count handled separately
        };
        status =
            sl_wifi_set_advanced_client_configuration(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &adv_cfg);

        if(status != SL_STATUS_OK) {
            break;
        }

        status = sl_wifi_set_scan_callback(wifi_scan_callback, instance);

        if(status != SL_STATUS_OK) {
            break;
        }

        status = sl_wifi_set_stats_callback(wifi_stats_callback, instance);

        if(status != SL_STATUS_OK) {
            break;
        }

        sl_mac_address_t mac_addr;
        status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &mac_addr);

        if(status != SL_STATUS_OK) {
            break;
        }

        wifi_net_tcpip_init(instance, &mac_addr);

    } while(false);

    return status;
}

static Wifi* wifi_alloc(void) {
    Wifi* instance = malloc(sizeof(Wifi));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(3, sizeof(WifiEvent));
    instance->event_pubsub = furi_pubsub_alloc();
    instance->info_timer = furi_event_loop_timer_alloc(
        instance->event_loop, wifi_backend_info_callback, FuriEventLoopTimerTypePeriodic, instance);
    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->tcpip_lock = furi_semaphore_alloc(1, 0);
    instance->ip6_addr_valid = furi_semaphore_alloc(1, 0);

    furi_record_open(RECORD_NETWORK);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        wifi_event_queue_callback,
        instance);
    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifi, wifi_intercom_rx_callback, instance);
    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifiData, wifi_net_intercom_rx_callback, instance);

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
};
