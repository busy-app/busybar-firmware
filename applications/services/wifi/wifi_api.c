#include "wifi_i.h"

static void wifi_send_message(Wifi* instance, WifiMessage* message) {
    message->lock = api_lock_alloc_locked();

    furi_check(furi_semaphore_acquire(instance->api_semaphore, FuriWaitForever) == FuriStatusOk);

    instance->api_message = *message;
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventRequest);

    api_lock_wait_unlock_and_free(message->lock);
}

static void wifi_api_nonblocking_request(Wifi* instance, const WifiMessage* message) {
    if(furi_semaphore_acquire(instance->api_semaphore, 0) == FuriStatusOk) {
        instance->api_message = *message;
        furi_event_loop_set_custom_event(instance->event_loop, WifiEventRequest);
    }
}

bool wifi_api_is_locked(Wifi* instance) {
    return furi_semaphore_get_count(instance->api_semaphore) == 0;
}

void wifi_api_unlock(Wifi* instance, WifiStatus status) {
    WifiMessage* message = &instance->api_message;

    message->status = status;

    if(message->lock) {
        api_lock_unlock(message->lock);
    }

    furi_check(furi_semaphore_release(instance->api_semaphore) == FuriStatusOk);
}

void wifi_schedule_init_request(Wifi* instance) {
    furi_assert(instance);

    WifiMessage msg = {
        .request_type = WifiRequestTypeInit,
    };

    wifi_api_nonblocking_request(instance, &msg);
}

void wifi_schedule_connect_request(Wifi* instance, const WifiSettings* settings) {
    furi_assert(instance);

    WifiMessage msg = {
        .request_type = WifiRequestTypeConnect,
        .connect_message =
            {
                .credentials = settings->credentials,
                .ip_config = settings->ip_config,
            },
    };

    wifi_api_nonblocking_request(instance, &msg);
}

void wifi_schedule_backend_info_request(Wifi* instance) {
    furi_assert(instance);

    const WifiMessage msg = {
        .request_type = WifiRequestTypeGetBackendInfo,
    };

    wifi_api_nonblocking_request(instance, &msg);
}

WifiStatus wifi_scan(Wifi* instance, WifiScanResult* results, uint8_t* count, uint8_t max_count) {
    furi_check(instance);
    furi_check(results);
    furi_check(count);
    furi_check(max_count);

    WifiMessage msg = {
        .request_type = WifiRequestTypeScan,
        .scan_message =
            {
                .data = results,
                .count = count,
                .max_count = max_count,
            },
    };

    wifi_send_message(instance, &msg);
    return msg.status;
}

WifiStatus wifi_connect(
    Wifi* instance,
    const WifiCredentials* credentials,
    const WifiIpConfig* ip_config) {
    furi_check(instance);
    furi_check(credentials);

    WifiMessage msg = {
        .request_type = WifiRequestTypeConnect,
        .connect_message =
            {
                .credentials = *credentials,
                .ip_config = *ip_config,
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

WifiStatus wifi_get_info(Wifi* instance, WifiInfo* info) {
    furi_check(instance);
    furi_check(info);

    // TODO: Protect with mutex / use FuriState
    *info = instance->info;

    return WifiStatusOk;
}
