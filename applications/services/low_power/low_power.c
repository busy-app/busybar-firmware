#include "low_power.h"

#include <furi.h>

#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <light_sensor/light_sensor.h>

#define TAG "LowPower"

#define API_QUEUE_SIZE       (16)
#define API_QUEUE_TIMEOUT_MS (1000)

typedef enum {
    LowPowerApiMessageLock,
    LowPowerApiMessageUnlock,
} LowPowerApiMessage;

struct LowPower {
    FuriMessageQueue* api_queue;
    LightSensor* light_sensor;
    BackDisplaySrv* back_display;
    FrontDisplaySrv* front_display;

    uint32_t lock_count;
    bool in_low_power;
};

static void low_power_send_api_message(LowPower* instance, LowPowerApiMessage message) {
    const FuriStatus status = furi_message_queue_put(
        instance->api_queue, &message, furi_ms_to_ticks(API_QUEUE_TIMEOUT_MS));

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorTimeout);
        FURI_LOG_E(TAG, "Service is not responding, dropping request");
    }
}

static void low_power_enter(LowPower* instance) {
    front_display_sleep_mode(instance->front_display, true);
    back_display_sleep_mode(instance->back_display, true);
    light_sensor_sleep(instance->light_sensor, true);
}

static void low_power_exit(LowPower* instance) {
    front_display_sleep_mode(instance->front_display, false);
    back_display_sleep_mode(instance->back_display, false);
    light_sensor_sleep(instance->light_sensor, false);
}

static LowPower* low_power_alloc(void) {
    LowPower* instance = malloc(sizeof(LowPower));
    instance->api_queue = furi_message_queue_alloc(API_QUEUE_SIZE, sizeof(LowPowerApiMessage));
    instance->lock_count = 1; // Locked by default
    instance->in_low_power = false;

    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->light_sensor = furi_record_open(RECORD_LIGHT_SENSOR);

    return instance;
}

int32_t low_power_srv(void* arg) {
    UNUSED(arg);
    LowPower* instance = low_power_alloc();
    furi_record_create(RECORD_LOW_POWER, instance);

    while(1) {
        LowPowerApiMessage message;
        furi_check(
            furi_message_queue_get(instance->api_queue, &message, FuriWaitForever) ==
            FuriStatusOk);

        if(message == LowPowerApiMessageLock) {
            instance->lock_count++;

            if((instance->lock_count > 0) && (instance->in_low_power)) {
                low_power_exit(instance);
                instance->in_low_power = false;
            }

        } else if(message == LowPowerApiMessageUnlock) {
            if(instance->lock_count > 0) {
                instance->lock_count--;
            }

            if((instance->lock_count == 0) && (!instance->in_low_power)) {
                low_power_enter(instance);
                instance->in_low_power = true;
            }

        } else {
            furi_crash("Invalid LowPowerApiMessage value");
        }
    }

    return 0;
}

void low_power_lock(LowPower* instance) {
    furi_assert(instance);
    low_power_send_api_message(instance, LowPowerApiMessageLock);
}

void low_power_unlock(LowPower* instance) {
    furi_assert(instance);
    low_power_send_api_message(instance, LowPowerApiMessageUnlock);
}
