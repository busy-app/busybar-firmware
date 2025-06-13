#include "front_display_i.h"

#include <furi.h>
#include <toolbox/api_lock.h>
#include <power/power_service/power.h>
#include <light_sensor/light_sensor.h>

#define TAG "FrontDisplaySrv"

#define REFRESH_PERIOD_MS (100)

#define AUTO_BRIGHTNESS_MIN_LEVEL (25)
#define AUTO_BRIGHTNESS_MAX_LEVEL (100)

#define FRONT_DISPLAY_FRAME_SIZE (FRONT_DISPLAY_W * FRONT_DISPLAY_H * 3) // RGB888

// #define FRONT_DISPLAY_DEBUG_ENABLE

#ifdef FRONT_DISPLAY_DEBUG_ENABLE
#define FRONT_DISPLAY_DEBUG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define FRONT_DISPLAY_DEBUG(...)
#endif

struct FrontDisplaySrv {
    uint32_t sensor_level, brightness_override;
    Power* power;
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    FuriPubSub* light_sensor_pubsub;
    bool enabled;
    bool send_in_progress;
    bool need_update;

    uint8_t last_frame[FRONT_DISPLAY_FRAME_SIZE];
};

typedef enum {
    FrontDisplayMessageTypeDraw,
    FrontDisplayMessageTypeDrawEnd,
    FrontDisplayMessageTypeBrightness,
    FrontDisplayMessageTypeLightSensor,
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

static void front_display_light_sensor_event(const void* event_message, void* context) {
    furi_assert(event_message);
    furi_assert(context);

    FrontDisplaySrv* instance = context;

    const LightSensorEvent* event = event_message;
    if(event->type != LightSensorEventTypeLightLevelChanged) {
        return;
    }

    FrontDisplayMessage message = {
        .api_lock = NULL, // No need for API lock here
        .type = FrontDisplayMessageTypeLightSensor,
        .brightness = event->light_level,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
}

static void front_display_update_done_callback(void* context) {
    FrontDisplaySrv* instance = context;

    // try to force a new frame to be sent
    FrontDisplayMessage message = {
        .api_lock = NULL, // No need for API lock here
        .type = FrontDisplayMessageTypeDrawEnd,
    };

    furi_check(furi_message_queue_put(instance->message_queue, &message, 0) == FuriStatusOk);
}

static void front_display_power_irq_callback(void* context) {
    FrontDisplaySrv* instance = context;
    UNUSED(instance);

    bool power_state = furi_hal_gpio_read(&gpio_front_display_power_en);

    FrontDisplayMessage message = {
        .api_lock = NULL, // No need for API lock here
        .type = power_state ? FrontDisplayMessageTypeOn : FrontDisplayMessageTypeOff,
    };

    furi_check(furi_message_queue_put(instance->message_queue, &message, 0) == FuriStatusOk);
}

static void front_display_power_pin_init(FrontDisplaySrv* instance) {
    // Open-drain output with pull-up
    furi_hal_gpio_init(
        &gpio_front_display_power_en, GpioModeInterruptRiseFall, GpioPullUp, GpioSpeedLow);
    LL_GPIO_SetPinOutputType(
        gpio_front_display_power_en.port,
        gpio_front_display_power_en.pin,
        LL_GPIO_OUTPUT_PUSHPULL); // TODO: open drain for target f21
    LL_GPIO_SetPinMode(
        gpio_front_display_power_en.port, gpio_front_display_power_en.pin, LL_GPIO_MODE_OUTPUT);

    furi_hal_gpio_write(&gpio_front_display_power_en, true);

    furi_hal_gpio_add_int_callback(
        &gpio_front_display_power_en, front_display_power_irq_callback, instance);

    furi_delay_ms(50); // Stabilize power state
}

static void front_display_power_reset(void) {
    // mask the GPIO interrupt to prevent firing disable again
    furi_hal_gpio_disable_int_callback(&gpio_front_display_power_en);

    furi_hal_gpio_write(&gpio_front_display_power_en, false);
    furi_delay_ms(50);
    furi_hal_gpio_write(&gpio_front_display_power_en, true);
    furi_delay_ms(50); // Allow time for the display to power up

    // re-enable the GPIO interrupt
    furi_hal_gpio_enable_int_callback(&gpio_front_display_power_en);
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

static uint32_t front_display_get_brightness(uint32_t brightness_override, uint8_t sensor_level) {
    if(brightness_override == FRONT_DISPLAY_BRIGHTNESS_AUTO) {
        return front_display_light_sensor_level_to_brightness(sensor_level);
    } else {
        return brightness_override;
    }
}

static void front_display_start(FrontDisplaySrv* display) {
    front_display_scan_init();
    uint32_t brightness =
        front_display_get_brightness(display->brightness_override, display->sensor_level);
    front_display_driver_init(brightness);
    front_display_driver_set_update_callback(front_display_update_done_callback, display);
    front_display_scan_start();
    front_display_driver_start();

    // We are sending initializaion sequence via DMA, so we need to wait a bit
    // TODO: replace with a proper synchronization mechanism
    furi_delay_ms(5);
}

static void front_display_stop(void) {
    front_display_scan_deinit();
    front_display_driver_deinit();
}

static void front_display_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_check(object);
    furi_check(context);

    FrontDisplaySrv* display = context;
    FrontDisplayMessage message;

    if(furi_message_queue_get(display->message_queue, &message, 0) != FuriStatusOk) {
        furi_crash(TAG ": Failed to get message from queue");
    }

    switch(message.type) {
    case FrontDisplayMessageTypeDraw:
        furi_check(message.frame_buffer);
        FRONT_DISPLAY_DEBUG("Front display draw request");
        memcpy(display->last_frame, message.frame_buffer, FRONT_DISPLAY_FRAME_SIZE);

        if(display->enabled && !display->send_in_progress) {
            display->need_update = false;
            display->send_in_progress = true;
            front_display_driver_send_frame(display->last_frame);
            FRONT_DISPLAY_DEBUG("Front display frame sent");
        } else {
            display->need_update = true;
            FRONT_DISPLAY_DEBUG("Front display frame queued for later");
        }

        break;
    case FrontDisplayMessageTypeDrawEnd:
        display->send_in_progress = false;
        FRONT_DISPLAY_DEBUG("Front display draw end");
        break;

    case FrontDisplayMessageTypeLightSensor:
        display->sensor_level = message.brightness;
        {
            uint32_t brightness =
                front_display_get_brightness(display->brightness_override, display->sensor_level);
            FRONT_DISPLAY_DEBUG(
                "Updating front display brightness to %ld from light sensor", brightness);
            front_display_driver_set_brightness(brightness);
        }
        display->need_update = true;
        break;
    case FrontDisplayMessageTypeBrightness:
        display->brightness_override = message.brightness;
        {
            uint32_t brightness =
                front_display_get_brightness(display->brightness_override, display->sensor_level);
            FRONT_DISPLAY_DEBUG("Updating front display brightness to %ld", brightness);
            front_display_driver_set_brightness(brightness);
        }
        display->need_update = true;
        break;
    case FrontDisplayMessageTypeOn:
        if(!display->enabled) {
            FRONT_DISPLAY_DEBUG("Turning on front display");

            front_display_power_reset();
            front_display_start(display);

            display->enabled = true;
            display->need_update = true; // Force an update after enabling
        }

        break;
    case FrontDisplayMessageTypeOff:
        FRONT_DISPLAY_DEBUG("Turning off front display");
        front_display_stop();
        display->enabled = false;
        display->send_in_progress = false;
        break;
    }

    if(message.api_lock) {
        api_lock_unlock(message.api_lock);
    }

    if(display->enabled && !display->send_in_progress && display->need_update) {
        FRONT_DISPLAY_DEBUG("Sending queued front display frame");
        display->need_update = false;
        display->send_in_progress = true;
        front_display_driver_send_frame(display->last_frame);
    }
}

static FrontDisplaySrv* front_display_alloc(void) {
    FrontDisplaySrv* instance = malloc(sizeof(FrontDisplaySrv));

    instance->brightness_override = FRONT_DISPLAY_BRIGHTNESS_AUTO;
    instance->sensor_level = 0;

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(8, sizeof(FrontDisplayMessage));

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

    front_display_power_pin_init(instance);
    instance->enabled = true;

    front_display_start(instance);

#if defined(SRV_LIGHT_SENSOR)
    instance->light_sensor_pubsub = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
    furi_pubsub_subscribe(
        instance->light_sensor_pubsub, front_display_light_sensor_event, instance);
#else
    UNUSED(front_display_light_sensor_event);
#endif

    furi_record_create(RECORD_FRONT_DISPLAY, instance);
    return instance;
}

int32_t front_display_srv(void* p) {
    UNUSED(p);

    FrontDisplaySrv* instance = front_display_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
