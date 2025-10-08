#include "sntp.h"
#include "sntp_time_update.h"

#include <wifi/wifi.h>

#include <api_lock.h>

#define TAG "SntpSvc"

#define SNTP_S_TO_MS(x) ((x) * 1000)

#define SNTP_MAX_MESSAGES 4

typedef enum {
    SntpCustomEventUpdateSuccess = 1 << 0,
    SntpCustomEventUpdateError = 1 << 1,
} SntpCustomEvent;

typedef enum {
    SntpMessageTypeGetSettings,
    SntpMessageTypeSetSettings,
} SntpMessageType;

typedef enum {
    SntpStateBoot,
    SntpStateInSync,
    SntpStateRetry,
} SntpState;

typedef struct {
    SntpMessageType type;
    FuriApiLock lock;
    bool* result;
    union {
        SntpSettings* get_settings;
        const SntpSettings* set_settings;
    };
} SntpMessage;

struct Sntp {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriMessageQueue* message_queue;
    SntpSettings settings;
    SntpState state;
    bool is_time_update_ongoing;
};

static bool sntp_is_wifi_connected(void) {
    bool is_wifi_connected = false;
    Wifi* wifi = furi_record_open(RECORD_WIFI);
    WifiInfo wifi_info;
    const WifiStatus status = wifi_get_info(wifi, &wifi_info);

    if(status != WifiStatusOk) {
        FURI_LOG_D(TAG, "Failed to get Wifi info: %d", status);
    } else if(wifi_info.state != WifiStateUp) {
        FURI_LOG_D(TAG, "Wifi is not connected");
    } else {
        FURI_LOG_D(TAG, "Wifi is connected");
        is_wifi_connected = true;
    }
    furi_record_close(RECORD_WIFI);
    return is_wifi_connected;
}

static void sntp_timer_callback(void* context) {
    Sntp* instance = context;

    if(!instance->is_time_update_ongoing && sntp_is_wifi_connected()) {
        instance->is_time_update_ongoing = true;
        sntp_time_update_startup(instance);
    }
}

void sntp_status_update(Sntp* instance, bool success) {
    furi_assert(instance);

    furi_event_loop_set_custom_event(
        instance->event_loop,
        (success) ? SntpCustomEventUpdateSuccess : SntpCustomEventUpdateError);
}

static void sntp_message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);
    Sntp* instance = context;

    SntpMessage message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    bool result = false;
    switch(message.type) {
    case SntpMessageTypeGetSettings:
        *message.get_settings = instance->settings;
        result = true;
        break;

    case SntpMessageTypeSetSettings:
        result = sntp_settings_save(message.set_settings) &&
                 sntp_settings_load(&instance->settings);

        if(result) {
            if(instance->settings.is_enabled) {
                furi_event_loop_pend_callback(instance->event_loop, sntp_timer_callback, instance);
                furi_event_loop_timer_start(
                    instance->timer, SNTP_S_TO_MS(instance->settings.retry_sync_interval));
            } else {
                furi_event_loop_timer_stop(instance->timer);
                instance->state = SntpStateRetry;
            }
        }
        break;

    default:
        furi_crash("Invalid message type");
    }

    if(message.result) {
        *message.result = result;
    }

    if(message.lock) {
        api_lock_unlock(message.lock);
    }
}

static void sntp_custom_event_callback(uint32_t events, void* context) {
    Sntp* instance = context;

    if(events & SntpCustomEventUpdateSuccess) {
        FURI_LOG_I(TAG, "SNTP time update successful");

        instance->is_time_update_ongoing = false;

        if(instance->settings.is_enabled && instance->state != SntpStateInSync) {
            furi_event_loop_timer_start(
                instance->timer, SNTP_S_TO_MS(instance->settings.background_sync_interval));
            instance->state = SntpStateInSync;
        }
    } else if(events & SntpCustomEventUpdateError) {
        FURI_LOG_E(TAG, "SNTP time update failed");

        instance->is_time_update_ongoing = false;

        if(instance->settings.is_enabled && instance->state != SntpStateRetry) {
            furi_event_loop_timer_start(
                instance->timer, SNTP_S_TO_MS(instance->settings.retry_sync_interval));
            instance->state = SntpStateRetry;
        }
    }
}

static void sntp_send_message(Sntp* instance, const SntpMessage* message) {
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    if(message->lock) {
        api_lock_wait_unlock_and_free(message->lock);
    }
}

Sntp* sntp_alloc() {
    Sntp* instance = malloc(sizeof(Sntp));

    instance->state = SntpStateBoot;
    instance->is_time_update_ongoing = false;
    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(SNTP_MAX_MESSAGES, sizeof(SntpMessage));
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, sntp_timer_callback, FuriEventLoopTimerTypePeriodic, instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        sntp_message_queue_callback,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, sntp_custom_event_callback, instance);

    sntp_settings_load(&instance->settings);

    if(instance->settings.is_enabled) {
        furi_event_loop_timer_start(instance->timer, SNTP_S_TO_MS(instance->settings.boot_delay));
    }

    furi_record_create(RECORD_SNTP, instance);

    return instance;
}

void sntp_get_settings(const Sntp* instance, SntpSettings* settings) {
    furi_check(instance);
    furi_check(settings);

    const SntpMessage message = {
        .type = SntpMessageTypeGetSettings,
        .get_settings = settings,
        .lock = api_lock_alloc_locked(),
    };

    sntp_send_message((Sntp*)instance, &message);
}

bool sntp_set_settings(Sntp* instance, const SntpSettings* settings) {
    furi_check(instance);
    furi_check(settings);

    bool result;
    const SntpMessage message = {
        .type = SntpMessageTypeSetSettings,
        .set_settings = settings,
        .lock = api_lock_alloc_locked(),
        .result = &result,
    };

    sntp_send_message(instance, &message);

    return result;
}

int32_t sntp_srv(void* p) {
    UNUSED(p);
    Sntp* instance = sntp_alloc();

    FURI_LOG_I(TAG, "Service started");
    furi_event_loop_run(instance->event_loop);

    furi_crash();
    return 0;
}
