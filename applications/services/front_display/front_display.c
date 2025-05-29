#include "front_display_i.h"

#include <furi.h>
#include <toolbox/api_lock.h>
#include <power/power_service/power.h>
#include <light_sensor/light_sensor.h>

#define TAG "FrontDisplaySrv"

#define REFRESH_PERIOD_MS (100)

#define AUTO_BRIGHTNESS_MIN_LEVEL (25)
#define AUTO_BRIGHTNESS_MAX_LEVEL (100)

struct FrontDisplaySrv {
    uint8_t sensor_brightness, brightness_override;
    Power* power;
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    FuriPubSub* light_sensor_pubsub;
    FuriApiLock update_in_process_lock;
    bool is_on;
    bool needs_update;
};

// static void front_display_apply_brightness_level(FrontDisplaySrv* instance) {
//     const uint8_t brightness = (instance->brightness_override == FRONT_DISPLAY_BRIGHTNESS_AUTO) ?
//                                    instance->sensor_brightness :
//                                    instance->brightness_override;

//     const FrontDisplaySrvMessage message = {
//         .type = FrontDisplaySrvMessageTypeBrightness,
//         .brightness = brightness,
//     };
//     front_display_send_message(instance, &message);
// }

// void front_display_set_brightness(FrontDisplaySrv* instance, uint8_t brightness) {
//     furi_check(instance);

//     instance->brightness_override = brightness;
//     front_display_apply_brightness_level(instance);
// }

// static void front_display_update_done_callback(void* context) {
//     FrontDisplaySrv* instance = context;
//     furi_event_loop_set_custom_event(instance->event_loop, FrontDisplaySrvEventUpdateDone);
// }

// static void front_display_srv_custom_event_callback(uint32_t events, void* context) {
//     FrontDisplaySrv* instance = context;

//     if(events == FrontDisplaySrvEventMessage) {
//         const FrontDisplaySrvMessage* message = instance->message;
//         const FrontDisplaySrvMessageType message_type = message->type;

//         if(message_type == FrontDisplaySrvMessageTypeBrightness) {
//             front_display_driver_set_brightness(message->brightness);
//             if(instance->frame_buf_ptr) front_display_driver_send_frame(instance->frame_buf_ptr);
//             furi_event_flag_set(instance->event_flag, FrontDisplaySrvEventFlagDone);
//         } else if(message_type == FrontDisplaySrvMessageTypeDraw) {
//             front_display_driver_send_frame(message->frame_buffer);
//             furi_event_flag_set(instance->event_flag, FrontDisplaySrvEventFlagDone);
//             if(instance->frame_buf_ptr == NULL) instance->frame_buf_ptr = message->frame_buffer;
//         }

//     } else if(events == FrontDisplaySrvEventUpdateDone) {
//         furi_event_flag_set(instance->event_flag, FrontDisplaySrvEventFlagReady);

//     } else {
//         furi_crash(TAG ": Multiple events");
//     }
// }

// static void front_display_srv_power_event(const void* message, void* context) {
//     furi_assert(message);
//     furi_assert(context);

//     const PowerEvent* event = message;
//     FrontDisplaySrv* instance = context;

//     if(event->type == PowerEventReady) {
//         furi_semaphore_release(instance->power_ready_sem);
//     }
//     // TODO: React on overheat or low power budget by limiting brightness
// }

// static FrontDisplaySrv* front_display_srv_alloc(void) {
//     FrontDisplaySrv* instance = malloc(sizeof(FrontDisplaySrv));

//     // Must be first to ensure that power subsystem is OK
//     instance->power = furi_record_open(RECORD_POWER);
//     if(!power_is_battery_ready(instance->power)) {
//         instance->power_ready_sem = furi_semaphore_alloc(1, 0);
//         furi_pubsub_subscribe(
//             power_get_pubsub(instance->power), front_display_srv_power_event, instance);
//         furi_check(
//             furi_semaphore_acquire(instance->power_ready_sem, FuriWaitForever) == FuriStatusOk);
//         furi_semaphore_free(instance->power_ready_sem);
//     }

//     instance->brightness_override = FRONT_DISPLAY_BRIGHTNESS_AUTO;
//     instance->sensor_brightness = BRIGHTNESS_VAL_MIN;
//     instance->frame_buf_ptr = NULL;

//     instance->event_loop = furi_event_loop_alloc();
//     instance->event_flag = furi_event_flag_alloc();

//     furi_event_loop_set_custom_event_callback(
//         instance->event_loop, front_display_srv_custom_event_callback, instance);

//     furi_hal_gpio_init_simple(&gpio_front_display_power_en, GpioModeOutputPushPull);
//     furi_hal_gpio_write(&gpio_front_display_power_en, true);
//     furi_delay_ms(50);

//     front_display_scan_init();
//     front_display_driver_init(instance->sensor_brightness);
//     front_display_driver_set_update_callback(front_display_update_done_callback, instance);

//     front_display_scan_start();
//     front_display_driver_start();

//     instance->light_sensor_pubsub = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
//     furi_pubsub_subscribe(
//         instance->light_sensor_pubsub, front_display_srv_light_sensor_event, instance);

//     furi_event_flag_set(instance->event_flag, FrontDisplaySrvEventFlagReady);

//     furi_record_create(RECORD_FRONT_DISPLAY, instance);
//     return instance;
// }

// int32_t front_display_srv(void* p) {
//     UNUSED(p);

//     FrontDisplaySrv* instance = front_display_srv_alloc();
//     furi_event_loop_run(instance->event_loop);

//     return 0;
// }

typedef enum {
    FrontDisplayMessageTypeDraw,
    FrontDisplayMessageTypeDrawEnd,
    FrontDisplayMessageTypeBrightness,
    FrontDisplayMessageTypeOn,
    FrontDisplayMessageTypeOff,
} FrontDisplayMessageType;

typedef struct {
    FuriApiLock api_lock;
    FrontDisplayMessageType type;
    union {
        const uint8_t* frame_buffer;
        uint8_t brightness; // Brightness value (0-100) or FRONT_DISPLAY_BRIGHTNESS_AUTO
    };
} FrontDisplayMessage;

void front_display_draw(FrontDisplaySrv* instance, const uint8_t* frame_buffer) {
    furi_check(instance);
    furi_check(frame_buffer);

    FrontDisplayMessage message = {
        .api_lock = api_lock_alloc_locked(),
        .type = FrontDisplayMessageTypeDraw,
        .frame_buffer = frame_buffer,
    };
    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
    api_lock_wait_unlock_and_free(message.api_lock);
}

void front_display_set_brightness(FrontDisplaySrv* instance, uint8_t brightness) {
    FrontDisplayMessage message = {
        .api_lock = NULL, // No need for API lock here
        .type = FrontDisplayMessageTypeBrightness,
        .brightness = brightness,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
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

static void front_display_light_sensor_event(const void* event_message, void* context) {
    furi_assert(event_message);
    furi_assert(context);

    FrontDisplaySrv* instance = context;

    const LightSensorEvent* event = event_message;
    if(event->type != LightSensorEventTypeLightLevelChanged) {
        return;
    }

    uint8_t brightness = front_display_light_sensor_level_to_brightness(event->light_level);
    front_display_set_brightness(instance, brightness);
}

// static void front_display_power_enable(FrontDisplaySrv* instance, bool enable) {
//     instance->is_on = enable;
//     furi_hal_gpio_write(&gpio_front_display_power_en, enable);
//     if(enable) {
//         furi_delay_ms(50); // Allow time for the display to power up
//         front_display_driver_send_cmd_init();
//         front_display_scan_start();
//         front_display_driver_start();
//     }
// }

static void front_display_update_done_callback(void* context) {
    FrontDisplaySrv* instance = context;
    api_lock_unlock(instance->update_in_process_lock);

    // try to force a new frame to be sent
    FrontDisplayMessage message = {
        .api_lock = NULL, // No need for API lock here
        .type = FrontDisplayMessageTypeDrawEnd,
    };

    furi_message_queue_put(instance->message_queue, &message, 0);
}

static void front_display_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_check(object);
    furi_check(context);

    FrontDisplaySrv* display = context;
    FrontDisplayMessage message;

    if(furi_message_queue_get(display->message_queue, &message, 0) != FuriStatusOk) {
        furi_crash(TAG ": Failed to get message from queue");
    }

    FURI_LOG_I(TAG, "Processing message type: %d", message.type);

    switch(message.type) {
    case FrontDisplayMessageTypeDraw:
        // if(!api_lock_is_locked(display->update_in_process_lock)) {
        furi_check(message.frame_buffer);
        front_display_driver_send_frame(message.frame_buffer);
        furi_delay_ms(50);
        // display->needs_update = true;
        // }
        FURI_LOG_I(TAG, "Draw frame");
        break;
    case FrontDisplayMessageTypeDrawEnd:
        // do nothing, this is just to send new frame if needed
        FURI_LOG_I(TAG, "Draw end");
        break;

    case FrontDisplayMessageTypeBrightness:
        FURI_LOG_I(TAG, "Set brightness: %d", message.brightness);
        break;
    case FrontDisplayMessageTypeOn:
        FURI_LOG_I(TAG, "Turn display ON");
        break;
    case FrontDisplayMessageTypeOff:
        FURI_LOG_I(TAG, "Turn display OFF");
        break;
    }

    // if(!api_lock_is_locked(display->update_in_process_lock)) {
    //     if(display->is_on && display->needs_update) {
    //         api_lock_relock(display->update_in_process_lock);
    //         display->needs_update = false;
    //         front_display_driver_send_frame();
    //     }
    // }

    if(message.api_lock) {
        api_lock_unlock(message.api_lock);
    }
}

static FrontDisplaySrv* front_display_alloc(void) {
    FrontDisplaySrv* instance = malloc(sizeof(FrontDisplaySrv));

    instance->brightness_override = FRONT_DISPLAY_BRIGHTNESS_AUTO;
    instance->sensor_brightness = BRIGHTNESS_VAL_MIN;

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(8, sizeof(FrontDisplayMessage));
    instance->update_in_process_lock = api_lock_alloc_locked();
    api_lock_unlock(instance->update_in_process_lock);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        front_display_message_queue_callback,
        instance);

    instance->power = furi_record_open(RECORD_POWER);
    while(!power_is_battery_ready(instance->power)) {
        furi_delay_ms(10);
    }

    FURI_LOG_I(TAG, "Front Display Service started");

    //     furi_hal_gpio_init_simple(&gpio_front_display_power_en, GpioModeOutputPushPull);
    //     furi_hal_gpio_write(&gpio_front_display_power_en, true);
    //     furi_delay_ms(50);

    //     front_display_scan_init();
    //     front_display_driver_init(instance->sensor_brightness);
    //     front_display_driver_set_update_callback(front_display_update_done_callback, instance);

    //     front_display_scan_start();
    //     front_display_driver_start();

    furi_hal_gpio_init_simple(&gpio_front_display_power_en, GpioModeOutputPushPull);
    furi_hal_gpio_write(&gpio_front_display_power_en, true);
    instance->is_on = true;

    furi_delay_ms(50); // Allow time for the display to power up

    front_display_scan_init();
    front_display_driver_init(instance->sensor_brightness);
    front_display_driver_set_update_callback(front_display_update_done_callback, instance);

    front_display_scan_start();
    front_display_driver_start();

    instance->light_sensor_pubsub = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
    furi_pubsub_subscribe(
        instance->light_sensor_pubsub, front_display_light_sensor_event, instance);

    furi_record_create(RECORD_FRONT_DISPLAY, instance);
    return instance;
}

int32_t front_display_srv(void* p) {
    UNUSED(p);

    FrontDisplaySrv* instance = front_display_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
