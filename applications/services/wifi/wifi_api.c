#include "wifi_i.h"

#define WIFI_API_QUEUE_TIMEOUT_MS      (5000)
#define WIFI_PRIORITY_QUEUE_TIMEOUT_MS (10)

static bool wifi_api_send_message(
    FuriMessageQueue* queue,
    const WifiMessage* message,
    uint32_t timeout_ms) {
    bool success = true;

    const FuriStatus queue_status =
        furi_message_queue_put(queue, message, furi_ms_to_ticks(timeout_ms));

    if(queue_status != FuriStatusOk) {
        furi_check(
            (queue_status == FuriStatusErrorTimeout) || (queue_status == FuriStatusErrorResource));
        success = false;
    }

    return success;
}

static WifiStatus wifi_api_blocking_request(Wifi* instance, WifiMessage* message) {
    // ATTENTION: This initial value is REQUIRED for correct functioning of wifi_api_unlock().
    WifiStatus status = WifiStatusMax;

    message->status = &status;
    message->lock = api_lock_alloc_locked();

    if(wifi_api_send_message(instance->api_queue, message, WIFI_API_QUEUE_TIMEOUT_MS)) {
        api_lock_wait_unlock_and_free(message->lock);

    } else {
        FURI_LOG_E(TAG, "Request timed out");

        status = WifiStatusTimeout;
        api_lock_free(message->lock);
    }

    return status;
}

static void wifi_api_send_priority_request(Wifi* instance, WifiMessage* message) {
    message->is_priority = true;

    if(!wifi_api_send_message(instance->priority_queue, message, WIFI_PRIORITY_QUEUE_TIMEOUT_MS)) {
        FURI_LOG_E(TAG, "Priority request timed out");
    }
}

static void wifi_api_send_nonblocking_request(Wifi* instance, const WifiMessage* message) {
    if(!wifi_api_send_message(instance->api_queue, message, 0)) {
        FURI_LOG_W(TAG, "Failed to send nonblocking request");
    }
}

static void wifi_api_send_event(Wifi* instance, const WifiEvent* event) {
    const FuriStatus queue_status = furi_message_queue_put(instance->event_queue, event, 0);

    if(queue_status != FuriStatusOk) {
        furi_check(queue_status == FuriStatusErrorResource);
        FURI_LOG_W(TAG, "Failed to deliver event");
    }
}

static void wifi_api_schedule_pending_request(Wifi* instance) {
    if(furi_message_queue_get_count(instance->api_queue) != 0) {
        furi_event_loop_pend_callback(
            instance->event_loop, wifi_pending_request_callback, instance);
    }
}

static void wifi_api_unlock_api_queue(Wifi* instance) {
    furi_check(furi_message_queue_reset(instance->api_queue) == FuriStatusOk);
}

void wifi_api_unlock(Wifi* instance, WifiStatus status) {
    furi_assert(instance->is_processing);

    WifiMessage* message = &instance->api_message;

    if(message->lock) {
        furi_assert(message->status);
        if(*message->status == WifiStatusMax) {
            *message->status = status;
        }

        api_lock_unlock(message->lock);
    }

    if(message->is_priority) {
        wifi_api_schedule_pending_request(instance);
    } else {
        wifi_api_unlock_api_queue(instance);
    }

    instance->is_processing = false;
}

void wifi_schedule_init_request(Wifi* instance) {
    furi_assert(instance);

    WifiMessage msg = {
        .request_type = WifiRequestTypeInit,
    };

    wifi_api_send_priority_request(instance, &msg);
}

void wifi_schedule_deinit_request(Wifi* instance) {
    furi_assert(instance);

    WifiMessage msg = {
        .request_type = WifiRequestTypeDeinit,
    };

    wifi_api_send_priority_request(instance, &msg);
}

void wifi_schedule_connect_request(Wifi* instance, const WifiSettings* settings) {
    furi_assert(instance);

    const WifiMessage msg = {
        .request_type = WifiRequestTypeConnect,
        .connect_message =
            {
                .credentials = settings->credentials,
                .ip_config = settings->ip_config,
            },
    };

    wifi_api_send_nonblocking_request(instance, &msg);
}

void wifi_schedule_disconnect_request(Wifi* instance) {
    furi_assert(instance);

    const WifiMessage msg = {
        .request_type = WifiRequestTypeDisconnect,
    };

    wifi_api_send_nonblocking_request(instance, &msg);
}

void wifi_send_device_name_info_event(Wifi* instance, const DeviceNameInfo* device_name_info) {
    furi_assert(instance);

    const WifiEvent event = {
        .type = WifiEventTypeDeviceNameInfo,
        .device_name_info = *device_name_info,
    };

    wifi_api_send_event(instance, &event);
}

FuriState* wifi_get_state(Wifi* instance) {
    furi_check(instance);
    return instance->state;
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

    return wifi_api_blocking_request(instance, &msg);
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

    return wifi_api_blocking_request(instance, &msg);
}

WifiStatus wifi_disconnect(Wifi* instance) {
    furi_check(instance);

    WifiMessage msg = {
        .request_type = WifiRequestTypeDisconnect,
    };

    return wifi_api_blocking_request(instance, &msg);
}

WifiStatus wifi_forget(Wifi* instance) {
    furi_check(instance);

    WifiMessage msg = {
        .request_type = WifiRequestTypeForget,
    };

    return wifi_api_blocking_request(instance, &msg);
}

WifiStatus wifi_get_info(Wifi* instance, WifiInfo* info) {
    furi_check(instance);
    furi_check(info);

    furi_state_get(instance->state, info);
    return WifiStatusOk;
}
