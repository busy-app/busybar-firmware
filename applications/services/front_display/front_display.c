#include "front_display_i.h"

#include <furi.h>

#include <power/power_service/power.h>
#include <light_sensor/light_sensor.h>

#define TAG "FrontDisplaySrv"

#define REFRESH_PERIOD_MS (100)

#define AUTO_BRIGHTNESS_MIN_LEVEL (25)
#define AUTO_BRIGHTNESS_MAX_LEVEL (100)

typedef enum {
    FrontDisplaySrvEventMessage = 1UL << 0,
    FrontDisplaySrvEventUpdateDone = 1UL << 1,
} FrontDisplaySrvEvent;

typedef enum {
    FrontDisplaySrvEventFlagReady = 1UL << 0,
    FrontDisplaySrvEventFlagDone = 1UL << 1,
} FrontDisplaySrvEventFlag;

typedef enum {
    FrontDisplaySrvMessageTypeDraw,
    FrontDisplaySrvMessageTypeBrightness,
} FrontDisplaySrvMessageType;

typedef struct {
    FrontDisplaySrvMessageType type;
    union {
        uint8_t brightness;
        const uint8_t* frame_buffer;
    };
} FrontDisplaySrvMessage;

struct FrontDisplaySrv {
    uint8_t sensor_brightness, brightness_override;
    Power* power;
    FuriEventLoop* event_loop;
    FuriSemaphore* power_ready_sem;
    FuriEventFlag* event_flag;
    FuriPubSub* light_sensor_pubsub;
    const uint8_t* frame_buf_ptr;
    const FrontDisplaySrvMessage* message;
};

void front_display_reset(FrontDisplaySrv* instance) {
    furi_check(instance);
}

static void
    front_display_send_message(FrontDisplaySrv* instance, const FrontDisplaySrvMessage* message) {
    uint32_t flags;

    flags = furi_event_flag_wait(
        instance->event_flag, FrontDisplaySrvEventFlagReady, FuriFlagWaitAll, FuriWaitForever);
    furi_check(flags == FrontDisplaySrvEventFlagReady);

    instance->message = message;
    furi_event_loop_set_custom_event(instance->event_loop, FrontDisplaySrvEventMessage);

    flags = furi_event_flag_wait(
        instance->event_flag, FrontDisplaySrvEventFlagDone, FuriFlagWaitAll, FuriWaitForever);
    furi_check(flags == FrontDisplaySrvEventFlagDone);
}

void front_display_draw(FrontDisplaySrv* instance, const uint8_t* frame_buffer) {
    furi_check(instance);
    furi_check(frame_buffer);

    const FrontDisplaySrvMessage message = {
        .type = FrontDisplaySrvMessageTypeDraw,
        .frame_buffer = frame_buffer,
    };

    front_display_send_message(instance, &message);
}

static void front_display_apply_brightness_level(FrontDisplaySrv* instance) {
    const uint8_t brightness = (instance->brightness_override == FRONT_DISPLAY_BRIGHTNESS_AUTO) ?
                                   instance->sensor_brightness :
                                   instance->brightness_override;

    const FrontDisplaySrvMessage message = {
        .type = FrontDisplaySrvMessageTypeBrightness,
        .brightness = brightness,
    };
    front_display_send_message(instance, &message);
}

void front_display_set_brightness(FrontDisplaySrv* instance, uint8_t brightness) {
    furi_check(instance);

    instance->brightness_override = brightness;
    front_display_apply_brightness_level(instance);
}

static void front_display_update_done_callback(void* context) {
    FrontDisplaySrv* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, FrontDisplaySrvEventUpdateDone);
}

static void front_display_srv_custom_event_callback(uint32_t events, void* context) {
    FrontDisplaySrv* instance = context;

    if(events == FrontDisplaySrvEventMessage) {
        const FrontDisplaySrvMessage* message = instance->message;
        const FrontDisplaySrvMessageType message_type = message->type;

        if(message_type == FrontDisplaySrvMessageTypeBrightness) {
            if(instance->frame_buf_ptr) {
                front_display_driver_set_brightness(message->brightness);
                front_display_driver_send_frame(instance->frame_buf_ptr);
            }
            furi_event_flag_set(instance->event_flag, FrontDisplaySrvEventFlagDone);
        } else if(message_type == FrontDisplaySrvMessageTypeDraw) {
            front_display_driver_send_frame(message->frame_buffer);
            furi_event_flag_set(instance->event_flag, FrontDisplaySrvEventFlagDone);
            if(instance->frame_buf_ptr == NULL) instance->frame_buf_ptr = message->frame_buffer;
        }

    } else if(events == FrontDisplaySrvEventUpdateDone) {
        furi_event_flag_set(instance->event_flag, FrontDisplaySrvEventFlagReady);

    } else {
        furi_crash(TAG ": Multiple events");
    }
}

static uint8_t front_display_light_sensor_level_to_brightness(uint8_t light_level) {
    uint8_t constrained_light = MIN(light_level, LIGHT_SENSOR_LIGHT_LEVEL_MAX);

    // Apply a non-linear mapping to better match human perception
    uint8_t brightness = AUTO_BRIGHTNESS_MIN_LEVEL +
                         ((AUTO_BRIGHTNESS_MAX_LEVEL - AUTO_BRIGHTNESS_MIN_LEVEL) *
                          constrained_light * constrained_light) /
                             (LIGHT_SENSOR_LIGHT_LEVEL_MAX * LIGHT_SENSOR_LIGHT_LEVEL_MAX);

    return MIN(MAX(brightness, AUTO_BRIGHTNESS_MIN_LEVEL), AUTO_BRIGHTNESS_MAX_LEVEL);
}

static void front_display_srv_light_sensor_event(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    FrontDisplaySrv* instance = context;

    const LightSensorEvent* event = message;
    if(event->type != LightSensorEventTypeLightLevelChanged) {
        return;
    }

    instance->sensor_brightness =
        front_display_light_sensor_level_to_brightness(event->light_level);
    front_display_apply_brightness_level(instance);
}

static void front_display_srv_power_event(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const PowerEvent* event = message;
    FrontDisplaySrv* instance = context;

    if(event->type == PowerEventReady) {
        furi_semaphore_release(instance->power_ready_sem);
    }
    // TODO: React on overheat or low power budget by limiting brightness
}

static FrontDisplaySrv* front_display_srv_alloc(void) {
    FrontDisplaySrv* instance = malloc(sizeof(FrontDisplaySrv));

    // Must be first to ensure that power subsystem is OK
    instance->power = furi_record_open(RECORD_POWER);
    if(!power_is_battery_ready(instance->power)) {
        instance->power_ready_sem = furi_semaphore_alloc(1, 0);
        furi_pubsub_subscribe(
            power_get_pubsub(instance->power), front_display_srv_power_event, instance);
        furi_check(
            furi_semaphore_acquire(instance->power_ready_sem, FuriWaitForever) == FuriStatusOk);
        furi_semaphore_free(instance->power_ready_sem);
    }

    instance->brightness_override = FRONT_DISPLAY_BRIGHTNESS_AUTO;
    instance->sensor_brightness = BRIGHTNESS_VAL_MIN;
    instance->frame_buf_ptr = NULL;

    instance->event_loop = furi_event_loop_alloc();
    instance->event_flag = furi_event_flag_alloc();

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, front_display_srv_custom_event_callback, instance);

    furi_hal_gpio_init_simple(&gpio_front_display_power_en, GpioModeOutputPushPull);
    furi_hal_gpio_write(&gpio_front_display_power_en, true);
    furi_delay_ms(50);

    front_display_scan_init();
    front_display_driver_init(instance->sensor_brightness);
    front_display_driver_set_update_callback(front_display_update_done_callback, instance);

    front_display_scan_start();
    front_display_driver_start();

    instance->light_sensor_pubsub = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
    furi_pubsub_subscribe(
        instance->light_sensor_pubsub, front_display_srv_light_sensor_event, instance);

    furi_event_flag_set(instance->event_flag, FrontDisplaySrvEventFlagReady);

    furi_record_create(RECORD_FRONT_DISPLAY, instance);
    return instance;
}

int32_t front_display_srv(void* p) {
    UNUSED(p);

    FrontDisplaySrv* instance = front_display_srv_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
