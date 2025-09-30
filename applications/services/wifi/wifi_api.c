#include "wifi_i.h"

static void wifi_send_message(Wifi* instance, WifiMessage* message) {
    message->lock = api_lock_alloc_locked();

    furi_check(
        furi_semaphore_acquire(instance->access_semaphore, FuriWaitForever) == FuriStatusOk);

    instance->current_message = message;
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventRequest);

    api_lock_wait_unlock_and_free(message->lock);
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
                .credentials = credentials,
                .ip_config = ip_config,
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

    WifiMessage msg = {
        .request_type = WifiRequestTypeGetInfo,
        .get_info_message =
            {
                .info = info,
            },
    };

    wifi_send_message(instance, &msg);
    return msg.status;
}

WifiStatus wifi_get_hw_address(Wifi* instance, WifiHardwareAddress* hw_address) {
    furi_check(instance);
    furi_check(hw_address);

    WifiMessage msg = {
        .request_type = WifiRequestTypeGetHwAddress,
        .get_hw_address_message =
            {
                .hw_address = hw_address,
            },
    };

    wifi_send_message(instance, &msg);
    return msg.status;
}
