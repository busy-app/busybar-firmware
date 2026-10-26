#include "low_power.h"
#include "./settings/settings.h"

#include <furi.h>

#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <light_sensor/light_sensor.h>
#include <power/power_service/power.h>

#include <toolbox/api_lock.h>

#define TAG "LowPower"

#define API_QUEUE_SIZE         (16)
#define API_QUEUE_TIMEOUT_MS   (1000)
#define SETTINGS_SAVE_DELAY_MS (5000)

typedef enum {
    LowPowerApiMessageTypeLock,
    LowPowerApiMessageTypeUnlock,
    LowPowerApiMessageTypeGetSleepSettings,
    LowPowerApiMessageTypeSetSleepSettings,
    LowPowerApiMessageTypeMAX,
} LowPowerApiMessageType;

typedef struct {
    LowPowerApiMessageType type;
    FuriApiLock api_lock;
    union {
        LowPowerSleepSettings* sleep_settings;
    };
} LowPowerApiMessage;

struct LowPower {
    LightSensor* light_sensor;
    BackDisplaySrv* back_display;
    FrontDisplaySrv* front_display;
    Power* power;

    FuriEventLoop* event_loop;
    FuriMessageQueue* api_queue;
    FuriEventLoopTimer* sleep_holdoff_timer;
    FuriEventLoopTimer* setting_save_timer;

    bool settings_dirty;
    LowPowerSettings settings;
    uint32_t lock_count;
};

// =================
// Service internals
// =================

static void low_power_setting_flush(LowPower* instance) {
    furi_assert(instance);
    if(!instance->settings_dirty) return;
    if(!low_power_settings_save(&instance->settings)) {
        FURI_LOG_E(TAG, "Failed to save settings");
    } else {
        FURI_LOG_D(TAG, "Settings saved");
        instance->settings_dirty = false;
    }
}

static void low_power_enter(LowPower* instance) {
    FURI_LOG_D(TAG, "Enter");
    front_display_sleep_mode(instance->front_display, true);
    back_display_sleep_mode(instance->back_display, true);
    light_sensor_sleep(instance->light_sensor, true);

    // TODO: make it so that calls to the above services are synchronous
    furi_delay_ms(500);

    low_power_setting_flush(instance);

    power_deep_sleep(instance->power);
}

static void low_power_exit(LowPower* instance) {
    FURI_LOG_D(TAG, "Exit");
    front_display_sleep_mode(instance->front_display, false);
    back_display_sleep_mode(instance->back_display, false);
    light_sensor_sleep(instance->light_sensor, false);
}

static void low_power_sleep_holdoff_timer_expired(void* context) {
    LowPower* instance = context;
    furi_assert(instance);
    low_power_enter(instance);
}

static void low_power_setting_save_timer_expired(void* context) {
    LowPower* instance = context;
    furi_assert(instance);
    low_power_setting_flush(instance);
}

typedef enum {
    LowPowerConditionLockActivated,
    LowPowerConditionLockDeactivated,
    LowPowerConditionSettingsUpdated,
    LowPowerConditionMAX,
} LowPowerCondition;

static void low_power_update_sleep_timer(LowPower* instance, LowPowerCondition condition) {
    furi_assert(instance);
    furi_assert(condition < LowPowerConditionMAX);

    FuriEventLoopTimer* const timer = instance->sleep_holdoff_timer;

    if(condition == LowPowerConditionLockActivated) {
        FURI_LOG_D(TAG, "Holdoff stop");
        furi_event_loop_timer_stop(timer);

    } else if(condition == LowPowerConditionLockDeactivated) {
        FURI_LOG_D(TAG, "Locks deactivated");

        if(instance->settings.sleep.mode == LowPowerModeSleepAfterHoldoff) {
            FURI_LOG_D(TAG, "Holdoff start");
            if(!furi_event_loop_timer_is_running(timer)) {
                furi_event_loop_timer_start(timer, instance->settings.sleep.holdoff_duration);
            }
        }

    } else if(condition == LowPowerConditionSettingsUpdated) {
        FURI_LOG_D(TAG, "Settings updated");

        if(instance->settings.sleep.mode == LowPowerModeSleepAfterHoldoff) {
            if(furi_event_loop_timer_is_running(timer)) {
                FURI_LOG_D(TAG, "Holdoff restart");
                uint32_t elapsed = furi_event_loop_timer_get_interval(timer) -
                                   furi_event_loop_timer_get_remaining_time(timer);
                uint32_t new_duration = instance->settings.sleep.holdoff_duration;
                uint32_t remaining_duration = (new_duration > elapsed) ? (new_duration - elapsed) :
                                                                         0;
                furi_event_loop_timer_start(timer, remaining_duration);
            }

        } else {
            FURI_LOG_D(TAG, "Holdoff stop");
            furi_event_loop_timer_stop(timer);
        }
    }
}

static void low_power_sleep_api_message_received(FuriEventLoopObject* object, void* context) {
    LowPower* instance = context;
    furi_assert(instance);
    furi_assert(object == instance->api_queue);

    LowPowerApiMessage* message;
    furi_check(
        furi_message_queue_get(instance->api_queue, &message, FuriWaitForever) == FuriStatusOk);

    if(message->type == LowPowerApiMessageTypeLock) {
        instance->lock_count++;
        low_power_update_sleep_timer(instance, LowPowerConditionLockActivated);

    } else if(message->type == LowPowerApiMessageTypeUnlock) {
        if(instance->lock_count > 0) {
            instance->lock_count--;
        }

        if(instance->lock_count == 0) {
            low_power_update_sleep_timer(instance, LowPowerConditionLockDeactivated);
        }

    } else if(message->type == LowPowerApiMessageTypeGetSleepSettings) {
        *message->sleep_settings = instance->settings.sleep;

    } else if(message->type == LowPowerApiMessageTypeSetSleepSettings) {
        instance->settings.sleep = *message->sleep_settings;
        instance->settings_dirty = true;
        low_power_update_sleep_timer(instance, LowPowerConditionSettingsUpdated);
        furi_event_loop_timer_start(instance->setting_save_timer, SETTINGS_SAVE_DELAY_MS);

    } else {
        furi_crash("Invalid LowPowerApiMessage value");
    }

    api_lock_unlock(message->api_lock);
}

static LowPower* low_power_alloc(void) {
    LowPower* instance = malloc(sizeof(LowPower));
    instance->api_queue = furi_message_queue_alloc(API_QUEUE_SIZE, sizeof(LowPowerApiMessage*));
    instance->lock_count = 1; // Locked by default

    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->light_sensor = furi_record_open(RECORD_LIGHT_SENSOR);
    instance->power = furi_record_open(RECORD_POWER);

    low_power_exit(instance);

    instance->event_loop = furi_event_loop_alloc();
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_queue,
        FuriEventLoopEventIn,
        low_power_sleep_api_message_received,
        instance);
    instance->sleep_holdoff_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        low_power_sleep_holdoff_timer_expired,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->setting_save_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        low_power_setting_save_timer_expired,
        FuriEventLoopTimerTypeOnce,
        instance);

    if(!low_power_settings_load(&instance->settings)) {
        FURI_LOG_E(TAG, "Failed to load settings");
    }

    return instance;
}

int32_t low_power_srv(void* arg) {
    UNUSED(arg);
    LowPower* instance = low_power_alloc();
    furi_record_create(RECORD_LOW_POWER, instance);
    furi_event_loop_run(instance->event_loop);

    return 0;
}

// ===========
// Service API
// ===========

static void low_power_send_api_message(LowPower* instance, LowPowerApiMessage* message) {
    message->api_lock = api_lock_alloc_locked();

    const FuriStatus status = furi_message_queue_put(
        instance->api_queue, &message, furi_ms_to_ticks(API_QUEUE_TIMEOUT_MS));

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorTimeout);
        FURI_LOG_E(TAG, "Service is not responding, dropping request");
    }

    api_lock_wait_unlock_and_free(message->api_lock);
}

void low_power_lock(LowPower* instance) {
    furi_assert(instance);
    LowPowerApiMessage message = {
        .type = LowPowerApiMessageTypeLock,
    };
    low_power_send_api_message(instance, &message);
}

void low_power_unlock(LowPower* instance) {
    furi_assert(instance);
    LowPowerApiMessage message = {
        .type = LowPowerApiMessageTypeUnlock,
    };
    low_power_send_api_message(instance, &message);
}

void low_power_get_sleep_settings(LowPower* instance, LowPowerSleepSettings* settings) {
    furi_assert(instance);
    LowPowerApiMessage message = {
        .type = LowPowerApiMessageTypeGetSleepSettings,
        .sleep_settings = settings,
    };
    low_power_send_api_message(instance, &message);
}

void low_power_set_sleep_settings(LowPower* instance, LowPowerSleepSettings* settings) {
    furi_assert(instance);
    LowPowerApiMessage message = {
        .type = LowPowerApiMessageTypeSetSleepSettings,
        .sleep_settings = settings,
    };
    low_power_send_api_message(instance, &message);
}
