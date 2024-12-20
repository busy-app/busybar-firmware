#include "wifi_common_i.h"

#include <furi.h>
#include <intercom/intercom.h>

#include <sl_net.h>
#include <sl_wifi.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_callback_framework.h>

#define TAG "Wifi"

typedef enum {
    WifiEventRequest = 1UL << 0,
    WifiEventScanComplete = 1UL << 1,
} WifiEvent;

struct Wifi {
    FuriEventLoop* event_loop;
    Intercom* intercom;
    WifiRequest request;
    WifiResponse response;
};

typedef void (*WifiRequestHandler)(Wifi* instance);

extern const sl_wifi_device_configuration_t wifi_config;

static const WifiRequestHandler wifi_request_handlers[WifiRequestTypeMax];

static WifiStatus wifi_convert_sl_status(sl_status_t sl_status) {
    WifiStatus status;

    if(sl_status == SL_STATUS_OK) {
        status = WifiStatusOk;
    } else {
        // TODO: More error cases
        status = WifiStatusError;
    }

    return status;
}

static inline void wifi_send_response(Wifi* instance) {
    const size_t tx_size = intercom_tx(
        instance->intercom,
        IntercomChannelWifi,
        &instance->response,
        sizeof(WifiResponse),
        FuriWaitForever);
    furi_check(tx_size == sizeof(WifiResponse));
}

static void wifi_init_request_handler(Wifi* instance) {
    FURI_LOG_D(TAG, "Init");

    sl_status_t status;
    status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_config, NULL, NULL);

    FURI_LOG_D(TAG, "Init %s", status == SL_STATUS_OK ? "OK" : "FAIL");

    WifiResponse* response = &instance->response;
    response->status = wifi_convert_sl_status(status);

    wifi_send_response(instance);
}

static void wifi_deinit_request_handler(Wifi* instance) {
    FURI_LOG_D(TAG, "Deinit");

    sl_status_t status;

    do {
        // TODO: only call this if the interface was up
        sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
        status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
    } while(false);

    WifiResponse* response = &instance->response;
    response->status = wifi_convert_sl_status(status);

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
        response->status = wifi_convert_sl_status(status);

        wifi_send_response(instance);

    } else {
        // TODO: Scan timeout
    }
}

static void wifi_connect_request_handler(Wifi* instance) {
    FURI_LOG_D(TAG, "Connect");

    WifiResponse* response = &instance->response;
    response->status = WifiStatusError;

    wifi_send_response(instance);
}

static void wifi_disconnect_request_handler(Wifi* instance) {
    FURI_LOG_D(TAG, "Disconnect");

    WifiResponse* response = &instance->response;
    response->status = WifiStatusError;

    wifi_send_response(instance);
}

static void wifi_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    furi_assert(data_size == sizeof(WifiRequest));

    Wifi* instance = context;
    memcpy(&instance->request, data, data_size);
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventRequest);
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
        uint16_t result_count = 0;

        const size_t results_size = sizeof(sl_wifi_extended_scan_result_t) * SCAN_MAX_RESULTS;
        sl_wifi_extended_scan_result_t* results = malloc(results_size);

        sl_wifi_extended_scan_result_parameters_t params = {
            .scan_results = results,
            .array_length = results_size,
            .result_count = &result_count,
        };

        sl_status_t status =
            sl_wifi_get_stored_scan_results(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &params);
        UNUSED(status);

        result_count = MIN(result_count, (uint16_t)SCAN_MAX_RESULTS);

        for(uint16_t i = 0; i < result_count; ++i) {
            // TODO: Add other network params
            strncpy(
                response->scan_results[i].ssid,
                (const char*)results[i].ssid,
                sizeof(response->scan_results[0].ssid));
        }

        sli_wifi_flush_scan_results_database();
        free(results);

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
    UNUSED(result);
    UNUSED(result_length);

    furi_assert(context);
    Wifi* instance = context;
    WifiResponse* response = &instance->response;

    sl_status_t status;

    if(event & SL_WIFI_EVENT_FAIL_INDICATION) {
        // TODO: Get actual status from result cast
        status = SL_STATUS_FAIL;
    } else {
        status = SL_STATUS_OK;
    }

    response->status = wifi_convert_sl_status(status);
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventScanComplete);

    return status;
}

static Wifi* wifi_alloc(void) {
    Wifi* instance = malloc(sizeof(Wifi));
    sl_wifi_set_scan_callback(wifi_scan_callback, instance);

    instance->event_loop = furi_event_loop_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_custom_event_callback, instance);
    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifi, wifi_intercom_rx_callback, instance);

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
    [WifiRequestTypeDeinit] = wifi_deinit_request_handler,
    [WifiRequestTypeScan] = wifi_scan_request_handler,
    [WifiRequestTypeConnect] = wifi_connect_request_handler,
    [WifiRequestTypeDisconnect] = wifi_disconnect_request_handler,
};
