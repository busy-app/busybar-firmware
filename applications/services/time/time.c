#include "settings/settings_i.h"
#include "time_update.h"

#include <wifi/wifi.h>
#include <utz/utz.h>

#include <furi_hal_rtc.h>
#include <api_lock.h>
#include <furi/core/state.h>

#define TAG "TimeSvc"

#define TIME_S_TO_MS(x) ((x) * 1000)
#define TIME_M_TO_S(x)  ((x) * 60)

#define TIME_MAX_MESSAGES 4

typedef enum {
    TimeCustomEventUpdateSuccess = 1 << 0,
    TimeCustomEventUpdateFailure = 1 << 1,
    TimeCustomEventWifiConnected = 1 << 2,
} TimeCustomEvent;

typedef enum {
    TimeMessageTypeGetSettings,
    TimeMessageTypeSetSettings,
    TimeMessageTypeGetLocalTime,

    TimeMessageTypesCount,
} TimeMessageType;

typedef enum {
    TimeStatusBoot,
    TimeStatusInSync,
    TimeStatusRetry,
} TimeStatus;

typedef struct {
    TimeMessageType type;
    FuriApiLock lock;
    bool* is_success;

    union {
        TimeSettings* get_settings;
        const TimeSettings* set_settings;
        LocalTime* local_time;
    };
} TimeMessage;

typedef bool (*TimeMessageHandler)(Time* instance, TimeMessage* message);

struct Time {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriMessageQueue* message_queue;
    FuriState* state;

    TimeSettings settings;
    TimeStatus status;
    bool is_time_update_ongoing;

    Wifi* wifi;
    WifiState last_state; //<! only accessed by external thread in `time_wifi_callback`
};

static const TimeMessageHandler message_handlers[];

static void time_update_callback(Time* instance, bool is_success) {
    furi_event_loop_set_custom_event(
        instance->event_loop,
        is_success ? TimeCustomEventUpdateSuccess : TimeCustomEventUpdateFailure);
}

static void time_do_update(Time* instance) {
#ifdef FW_CFG_unit_tests
    UNUSED(instance);

    UNUSED(time_update_callback);
#else /* FW_CFG_unit_tests */
    furi_assert(instance);

    if(instance->is_time_update_ongoing) return;
    instance->is_time_update_ongoing = true;
    time_update_run(instance, time_update_callback);
#endif /* FW_CFG_unit_tests */
}

static void time_update_timer_callback(void* context) {
    Time* instance = context;
    FURI_LOG_T(TAG, "updating: background timer fired");
    time_do_update(instance);
}

static void time_wifi_callback(const void* item, void* context) {
    Time* instance = context;
    const WifiInfo* wifi_info = item;

    if(wifi_info->state == instance->last_state) return;
    instance->last_state = wifi_info->state;

    if(wifi_info->state == WifiStateConnected) {
        FURI_LOG_T(TAG, "updating: wi-fi connected");
        furi_event_loop_set_custom_event(instance->event_loop, TimeCustomEventWifiConnected);
    }
}

static bool do_get_settings(Time* instance, TimeMessage* message) {
    *message->get_settings = instance->settings;

    return true;
}

static bool do_set_settings(Time* instance, TimeMessage* message) {
    if(!time_settings_save(message->set_settings)) return false;

    instance->settings = *message->set_settings;

    furi_state_set(instance->state, &instance->settings);

    if(instance->settings.is_enabled) {
        furi_event_loop_pend_callback(instance->event_loop, time_update_timer_callback, instance);
    } else {
        furi_event_loop_timer_stop(instance->timer);
    }

    return true;
}

static bool do_get_local_time(Time* instance, TimeMessage* message) {
    DateTimeMs dt = furi_hal_rtc_get_datetime();

    utz_offset_t offset;
    utz_get_current_offset(&instance->settings.timezone, &dt.dt, &offset);

    dt.dt = utz_udatetime_add(&dt.dt, &offset);

    message->local_time->dt = dt.dt;
    message->local_time->offset = offset;

    return true;
}

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    Time* instance = context;

    TimeMessage message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    bool handler_result = message_handlers[message.type](instance, &message);

    if(message.is_success) *message.is_success = handler_result;
    if(message.lock) api_lock_unlock(message.lock);
}

static void custom_event_callback(uint32_t events, void* context) {
    Time* instance = context;

    if(events & TimeCustomEventUpdateSuccess) {
        FURI_LOG_D(TAG, "SNTP time update successful");

        instance->is_time_update_ongoing = false;

        if(instance->settings.is_enabled) {
            furi_event_loop_timer_start(
                instance->timer, TIME_S_TO_MS(instance->settings.background_sync_interval));
            instance->status = TimeStatusInSync;
        }

    } else if(events & TimeCustomEventUpdateFailure) {
        FURI_LOG_E(TAG, "SNTP time update failed");

        instance->is_time_update_ongoing = false;

        if(instance->settings.is_enabled) {
            furi_event_loop_timer_start(
                instance->timer, TIME_S_TO_MS(instance->settings.retry_sync_interval));
            instance->status = TimeStatusRetry;
        }
    }

    if(events & TimeCustomEventWifiConnected) {
        time_do_update(instance);
    }
}

static void time_send_message(Time* instance, const TimeMessage* message) {
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    if(message->lock) api_lock_wait_unlock_and_free(message->lock);
}

Time* time_alloc() {
    Time* instance = malloc(sizeof(Time));

    instance->status = TimeStatusBoot;
    instance->is_time_update_ongoing = false;
    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(TIME_MAX_MESSAGES, sizeof(TimeMessage));
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

    time_settings_load(&instance->settings);
    instance->state = furi_state_alloc(sizeof(TimeSettings));
    furi_state_set(instance->state, &instance->settings);

    if(instance->settings.is_enabled)
        furi_event_loop_timer_start(instance->timer, TIME_S_TO_MS(instance->settings.boot_delay));

    instance->wifi = furi_record_open(RECORD_WIFI);
    furi_state_subscribe(wifi_get_state(instance->wifi), time_wifi_callback, instance);

    furi_record_create(RECORD_TIME, instance);

    return instance;
}

void time_get_settings(const Time* instance, TimeSettings* settings) {
    furi_check(instance);
    furi_check(settings);

    const TimeMessage message = {
        .type = TimeMessageTypeGetSettings,
        .lock = api_lock_alloc_locked(),
        .is_success = NULL,

        .get_settings = settings,
    };

    time_send_message((Time*)instance, &message);
}

bool time_set_settings(Time* instance, const TimeSettings* settings) {
    furi_check(instance);
    furi_check(settings);

    bool is_success;
    const TimeMessage message = {
        .type = TimeMessageTypeSetSettings,
        .lock = api_lock_alloc_locked(),
        .is_success = &is_success,

        .set_settings = settings,
    };

    time_send_message(instance, &message);

    return is_success;
}

time_t time_get_timestamp(void) {
    return furi_hal_rtc_get_timestamp();
}

time_t time_get_timestamp_ms(void) {
    return furi_hal_rtc_get_timestamp_ms();
}

LocalTime time_get_local_time(Time* instance) {
    furi_check(instance);

    LocalTime result;
    const TimeMessage message = {
        .type = TimeMessageTypeGetLocalTime,
        .lock = api_lock_alloc_locked(),
        .is_success = NULL,

        .local_time = &result,
    };

    time_send_message(instance, &message);

    return result;
}

FuriState* time_get_settings_state(Time* instance) {
    return instance->state;
}

int32_t time_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Service starting...");

    Time* instance = time_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const TimeMessageHandler message_handlers[] = {
    [TimeMessageTypeGetSettings] = do_get_settings,
    [TimeMessageTypeSetSettings] = do_set_settings,
    [TimeMessageTypeGetLocalTime] = do_get_local_time,
};

static_assert(COUNT_OF(message_handlers) == TimeMessageTypesCount);
