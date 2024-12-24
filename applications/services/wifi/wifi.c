#include "wifi.h"
#include "wifi_common_i.h"

#include <furi.h>
#include <intercom/intercom.h>

typedef enum {
    WifiEventRequest = 1UL << 0,
    WifiEventResponse = 1UL << 1,
} WifiEvent;

typedef struct {
    WifiRequestType request_type;
    WifiStatus status;
    union {
        struct {
            const FuriString* ssid;
            const FuriString* passphrase;
        } credentials;
        struct {
            WifiScanResult* data;
            uint8_t* count;
            uint8_t max_count;
        } scan_results;
    };
} WifiMessage;

struct Wifi {
    FuriEventLoop* event_loop;
    FuriEventFlag* event_flag;
    Intercom* intercom;
    WifiMessage* message;
    WifiRequest request;
    WifiResponse response;
};

static void wifi_send_message(Wifi* instance, WifiMessage* message) {
    uint32_t flags;
    // Wait until the Wifi system becomes ready for the next request
    flags = furi_event_flag_wait(
        instance->event_flag, WifiEventRequest, FuriFlagWaitAll, FuriWaitForever);
    furi_check(flags & WifiEventRequest);

    instance->message = message;
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventRequest);

    // Wait until the response is received
    flags = furi_event_flag_wait(
        instance->event_flag, WifiEventResponse, FuriFlagWaitAll, FuriWaitForever);
    furi_check(flags & WifiEventResponse);
}

WifiStatus wifi_init(Wifi* instance) {
    furi_check(instance);

    WifiMessage msg = {
        .request_type = WifiRequestTypeInit,
    };

    wifi_send_message(instance, &msg);
    return msg.status;
}

WifiStatus wifi_deinit(Wifi* instance) {
    furi_check(instance);

    WifiMessage msg = {
        .request_type = WifiRequestTypeDeinit,
    };

    wifi_send_message(instance, &msg);
    return msg.status;
}

WifiStatus wifi_scan(Wifi* instance, WifiScanResult* results, uint8_t* count, uint8_t max_count) {
    furi_check(instance);
    furi_check(results);
    furi_check(count);
    furi_check(max_count);

    WifiMessage msg = {
        .request_type = WifiRequestTypeScan,
        .scan_results =
            {
                .data = results,
                .count = count,
                .max_count = max_count,
            },
    };

    wifi_send_message(instance, &msg);
    return msg.status;
}

WifiStatus wifi_connect(Wifi* instance, const FuriString* ssid, const FuriString* passphrase) {
    furi_check(instance);
    furi_check(ssid);
    furi_check(passphrase);

    WifiMessage msg = {
        .request_type = WifiRequestTypeDeinit,
        .credentials =
            {
                .ssid = ssid,
                .passphrase = passphrase,
            },
    };

    wifi_send_message(instance, &msg);
    return msg.status;
}

WifiStatus wifi_disconnect(Wifi* instance) {
    furi_check(instance);

    WifiMessage msg = {
        .request_type = WifiRequestTypeDisconnect,
    };

    wifi_send_message(instance, &msg);
    return msg.status;
}

static void wifi_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data_size == sizeof(WifiResponse));
    furi_assert(context);

    Wifi* instance = context;

    memcpy(&instance->response, data, data_size);
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventResponse);
}

static void wifi_process_request(WifiRequest* request, const WifiMessage* message) {
    const WifiRequestType request_type = message->request_type;
    request->type = request_type;

    if(request_type == WifiRequestTypeConnect) {
        strncpy(
            request->credentials.ssid,
            furi_string_get_cstr(message->credentials.ssid),
            sizeof(request->credentials.ssid));
        strncpy(
            request->credentials.passphrase,
            furi_string_get_cstr(message->credentials.passphrase),
            sizeof(request->credentials.passphrase));
    }
}

static void wifi_process_response(const WifiResponse* response, WifiMessage* message) {
    const WifiRequestType request_type = message->request_type;
    const WifiStatus status = response->status;

    if(status == WifiStatusOk) {
        if(request_type == WifiRequestTypeScan) {
            const uint8_t results_count =
                MIN(message->scan_results.max_count, response->scan_results.count);

            const WifiScanResult* results_in = response->scan_results.data;
            WifiScanResult* results_out = message->scan_results.data;

            memcpy(results_out, results_in, results_count * sizeof(WifiScanResult));
            *message->scan_results.count = results_count;
        }
    }

    message->status = status;
}

static void wifi_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Wifi* instance = context;

    if(events == WifiEventRequest) {
        WifiRequest* request = &instance->request;
        wifi_process_request(request, instance->message);
        intercom_tx(
            instance->intercom, IntercomChannelWifi, request, sizeof(WifiRequest), FuriWaitForever);

    } else if(events == WifiEventResponse) {
        wifi_process_response(&instance->response, instance->message);
        furi_event_flag_set(instance->event_flag, WifiEventRequest | WifiEventResponse);

    } else {
        furi_crash("Multiple Wifi events");
    }
}

static Wifi* wifi_alloc(void) {
    Wifi* instance = malloc(sizeof(Wifi));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_flag = furi_event_flag_alloc();
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_custom_event_callback, instance);

    intercom_set_rx_callback(
        instance->intercom, IntercomChannelWifi, wifi_intercom_rx_callback, instance);

    // Start receiving requests
    furi_event_flag_set(instance->event_flag, WifiEventRequest);
    furi_record_create(RECORD_WIFI, instance);

    return instance;
}

int32_t wifi_srv(void* arg) {
    UNUSED(arg);

    Wifi* instance = wifi_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
