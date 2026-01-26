#include "settings/settings_i.h"
#include "time_update.h"

#include <wifi/wifi.h>
#include <utz/utz.h>

#include <furi_hal_rtc.h>
#include <api_lock.h>

#define TAG "SntpSvc"

#define SNTP_S_TO_MS(x) ((x) * 1000)
#define SNTP_M_TO_S(x)  ((x) * 60)

#define SNTP_MAX_MESSAGES 4

typedef enum {
    SntpCustomEventUpdateSuccess = 1 << 0,
    SntpCustomEventUpdateFailure = 1 << 1,
} SntpCustomEvent;

typedef enum {
    SntpMessageTypeGetSettings,
    SntpMessageTypeSetSettings,
    SntpMessageTypeGetLocalTime,

    SntpMessageTypesCount,
} SntpMessageType;

typedef enum {
    SntpStateBoot,
    SntpStateInSync,
    SntpStateRetry,
} SntpState;

typedef struct {
    SntpMessageType type;
    FuriApiLock lock;
    bool* is_success;

    union {
        SntpSettings* get_settings;
        const SntpSettings* set_settings;
        LocalTime* local_time;
    };
} SntpMessage;

typedef bool (*SntpMessageHandler)(Sntp* instance, SntpMessage* message);

struct Sntp {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriMessageQueue* message_queue;

    SntpSettings settings;
    SntpState state;
    bool is_time_update_ongoing;
};

static const SntpMessageHandler message_handlers[];

static void time_update_callback(Sntp* instance, bool is_success) {
    furi_event_loop_set_custom_event(
        instance->event_loop,
        is_success ? SntpCustomEventUpdateSuccess : SntpCustomEventUpdateFailure);
}

static void time_update_timer_callback(void* context) {
    Sntp* instance = context;

    if(!instance->is_time_update_ongoing) {
        instance->is_time_update_ongoing = true;
        sntp_time_update_run(instance, time_update_callback);
    }
}

static bool do_get_settings(Sntp* instance, SntpMessage* message) {
    *message->get_settings = instance->settings;

    return true;
}

static bool do_set_settings(Sntp* instance, SntpMessage* message) {
    if(!sntp_settings_save(message->set_settings)) return false;

    instance->settings = *message->set_settings;

    if(instance->settings.is_enabled) {
        furi_event_loop_pend_callback(instance->event_loop, time_update_timer_callback, instance);
    } else {
        furi_event_loop_timer_stop(instance->timer);
    }

    return true;
}

static bool do_get_local_time(Sntp* instance, SntpMessage* message) {
    DateTime dt;
    furi_hal_rtc_get_datetime(&dt);

    udatetime_t udt = datetime_to_udatetime(&dt);

    uoffset_t offset;
    utz_get_current_offset(&instance->settings.timezone, &udt, &offset);

    udt = utz_udatetime_add(&udt, &offset);

    message->local_time->dt = datetime_from_udatetime(&udt);
    message->local_time->offset = offset;

    return true;
}

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    Sntp* instance = context;

    SntpMessage message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    bool handler_result = message_handlers[message.type](instance, &message);

    if(message.is_success) *message.is_success = handler_result;
    if(message.lock) api_lock_unlock(message.lock);
}

static void custom_event_callback(uint32_t events, void* context) {
    Sntp* instance = context;

    if(events & SntpCustomEventUpdateSuccess) {
        FURI_LOG_D(TAG, "SNTP time update successful");

        instance->is_time_update_ongoing = false;

        if(instance->settings.is_enabled) {
            furi_event_loop_timer_start(
                instance->timer, SNTP_S_TO_MS(instance->settings.background_sync_interval));
            instance->state = SntpStateInSync;
        }
    } else if(events & SntpCustomEventUpdateFailure) {
        FURI_LOG_E(TAG, "SNTP time update failed");

        instance->is_time_update_ongoing = false;

        if(instance->settings.is_enabled) {
            furi_event_loop_timer_start(
                instance->timer, SNTP_S_TO_MS(instance->settings.retry_sync_interval));
            instance->state = SntpStateRetry;
        }
    }
}

static void sntp_send_message(Sntp* instance, const SntpMessage* message) {
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    if(message->lock) api_lock_wait_unlock_and_free(message->lock);
}

Sntp* sntp_alloc() {
    Sntp* instance = malloc(sizeof(Sntp));

    instance->state = SntpStateBoot;
    instance->is_time_update_ongoing = false;
    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(SNTP_MAX_MESSAGES, sizeof(SntpMessage));
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, time_update_timer_callback, FuriEventLoopTimerTypeOnce, instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, custom_event_callback, instance);

    sntp_settings_load(&instance->settings);

    if(instance->settings.is_enabled)
        furi_event_loop_timer_start(instance->timer, SNTP_S_TO_MS(instance->settings.boot_delay));

    furi_record_create(RECORD_SNTP, instance);

    return instance;
}

void sntp_get_settings(const Sntp* instance, SntpSettings* settings) {
    furi_check(instance);
    furi_check(settings);

    const SntpMessage message = {
        .type = SntpMessageTypeGetSettings,
        .lock = api_lock_alloc_locked(),
        .is_success = NULL,

        .get_settings = settings,
    };

    sntp_send_message((Sntp*)instance, &message);
}

bool sntp_set_settings(Sntp* instance, const SntpSettings* settings) {
    furi_check(instance);
    furi_check(settings);

    bool is_success;
    const SntpMessage message = {
        .type = SntpMessageTypeSetSettings,
        .lock = api_lock_alloc_locked(),
        .is_success = &is_success,

        .set_settings = settings,
    };

    sntp_send_message(instance, &message);

    return is_success;
}

time_t sntp_get_timestamp(void) {
    return furi_hal_rtc_get_timestamp();;
}

time_t sntp_get_timestamp_ms(void) {
    return furi_hal_rtc_get_timestamp_ms();
}

LocalTime sntp_get_local_time(Sntp* instance) {
    furi_check(instance);

    LocalTime result;
    const SntpMessage message = {
        .type = SntpMessageTypeGetLocalTime,
        .lock = api_lock_alloc_locked(),
        .is_success = NULL,

        .local_time = &result,
    };

    sntp_send_message(instance, &message);

    return result;
}


int32_t sntp_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Service starting...");

    Sntp* instance = sntp_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const SntpMessageHandler message_handlers[] = {
    [SntpMessageTypeGetSettings] = do_get_settings,
    [SntpMessageTypeSetSettings] = do_set_settings,
    [SntpMessageTypeGetLocalTime] = do_get_local_time,
};

static_assert(COUNT_OF(message_handlers) == SntpMessageTypesCount);
