#include "wifi_i.h"

#define WIFI_API_TIMEOUT_MS (5000)

static void wifi_api_send_message(Wifi* instance, const WifiMessage* message) {
    instance->api_message = *message;
    furi_event_loop_set_custom_event(instance->event_loop, WifiEventRequest);
}

static WifiStatus wifi_api_blocking_request(Wifi* instance, WifiMessage* message) {
    // ATTENTION: This initial value is REQUIRED for correct functioning of wifi_api_unlock().
    WifiStatus status = WifiStatusMax;

    message->status = &status;
    message->lock = api_lock_alloc_locked();

    const FuriStatus sem_status =
        furi_semaphore_acquire(instance->api_semaphore, furi_ms_to_ticks(WIFI_API_TIMEOUT_MS));

    if(sem_status == FuriStatusOk) {
        wifi_api_send_message(instance, message);
        api_lock_wait_unlock_and_free(message->lock);

    } else {
        FURI_LOG_E(TAG, "Request timed out");

        status = WifiStatusTimeout;
        api_lock_free(message->lock);
    }

    return status;
}

static void wifi_api_nonblocking_request(Wifi* instance, const WifiMessage* message) {
    if(wifi_api_try_lock(instance)) {
        wifi_api_send_message(instance, message);
    } else {
        FURI_LOG_W(TAG, "Failed to send nonblocking request");
    }
}

static void wifi_api_override_request(Wifi* instance, const WifiMessage* message) {
    const FuriStatus queue_status = furi_message_queue_put(
        instance->override_queue, message, furi_ms_to_ticks(WIFI_API_TIMEOUT_MS));

    if(queue_status != FuriStatusOk) {
        furi_check(queue_status == FuriStatusErrorTimeout);
        FURI_LOG_E(TAG, "Failed to override current request: timeout");
    }
}

bool wifi_api_is_locked(Wifi* instance) {
    return furi_semaphore_get_count(instance->api_semaphore) == 0;
}

bool wifi_api_try_lock(Wifi* instance) {
    return furi_semaphore_acquire(instance->api_semaphore, 0) == FuriStatusOk;
}

void wifi_api_unlock(Wifi* instance, WifiStatus status) {
    wifi_api_unlock_pending_request(instance, status);
    furi_check(furi_semaphore_release(instance->api_semaphore) == FuriStatusOk);
}

void wifi_api_unlock_pending_request(Wifi* instance, WifiStatus status) {
    WifiMessage* message = &instance->api_message;

    if(message->lock) {
        furi_assert(message->status);
        if(*message->status == WifiStatusMax) {
            *message->status = status;
        }

        api_lock_unlock(message->lock);
    }
}

void wifi_schedule_init_request(Wifi* instance) {
    furi_assert(instance);

    const WifiMessage msg = {
        .request_type = WifiRequestTypeInit,
    };

    wifi_api_override_request(instance, &msg);
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

    wifi_api_nonblocking_request(instance, &msg);
}

void wifi_schedule_disconnect_request(Wifi* instance) {
    furi_assert(instance);

    const WifiMessage msg = {
        .request_type = WifiRequestTypeDisconnect,
    };

    wifi_api_nonblocking_request(instance, &msg);
}

void wifi_schedule_deinit_request(Wifi* instance) {
    furi_assert(instance);

    const WifiMessage msg = {
        .request_type = WifiRequestTypeDeinit,
    };

    wifi_api_override_request(instance, &msg);
}

void wifi_schedule_set_hostname_request(Wifi* instance, const DeviceNameInfo* device_name_info) {
    furi_assert(instance);

    const WifiMessage msg = {
        .request_type = WifiRequestTypeSetHostname,
        .set_hostname_message = {.device_name_info = *device_name_info},
    };

    wifi_api_nonblocking_request(instance, &msg);
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
