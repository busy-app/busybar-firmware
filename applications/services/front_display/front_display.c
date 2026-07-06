#include "front_display_i.h"

#include <furi.h>
#include <furi_hal_display.h>
#include <toolbox/api_lock.h>
#include <power/power_service/power.h>

#define TAG "FrontDisplaySrv"

#define FRONT_DISPLAY_BRIGHTNESS_MIN (0)
#define FRONT_DISPLAY_BRIGHTNESS_MAX (100)

#define AUTO_BRIGHTNESS_MIN_LEVEL (25)
#define AUTO_BRIGHTNESS_MAX_LEVEL (100)

#define FRONT_DISPLAY_FRAME_SIZE (FRONT_DISPLAY_W * FRONT_DISPLAY_H * 3) // RGB888

#define FRONT_DISPLAY_TRANSITION_DURATION_MS      (200)
#define FRONT_DISPLAY_TRANSITION_STEP_DURATION_MS (16)
#define FRONT_DISPLAY_TRANSITION_STEP_COUNT \
    (FRONT_DISPLAY_TRANSITION_DURATION_MS / FRONT_DISPLAY_TRANSITION_STEP_DURATION_MS)

// #define FRONT_DISPLAY_DEBUG_ENABLE

#ifdef FRONT_DISPLAY_DEBUG_ENABLE
#define FRONT_DISPLAY_DEBUG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define FRONT_DISPLAY_DEBUG(...)
#endif

struct FrontDisplaySrv {
    Power* power;
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* transition_timer;
    FuriMessageQueue* message_queue;
    FuriPubSubSubscription* power_subscription;
    bool enabled;
    bool send_in_progress;
    bool need_update;
    bool is_blanked;
    bool power_off_pending;

    uint8_t last_frame[FRONT_DISPLAY_FRAME_SIZE];
    uint8_t brightness_override;
    uint8_t brightness_current; // For brightness transitions
};

typedef enum {
    FrontDisplayMessageTypeDraw,
    FrontDisplayMessageTypeDrawEnd,
    FrontDisplayMessageTypeBrightness,
    FrontDisplayMessageTypeBlanking,
    FrontDisplayMessageTypeSleep,
    FrontDisplayMessageTypeOn,
    FrontDisplayMessageTypeOff,
    FrontDisplayMessageTypeBatteryReady,
} FrontDisplayMessageType;

typedef struct {
    FuriApiLock api_lock;
    FrontDisplayMessageType type;
    union {
        const uint8_t* frame_buffer;
        uint8_t brightness; // Brightness value (0-100) or FRONT_DISPLAY_BRIGHTNESS_AUTO
        bool bool_param;
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

void front_display_set_brightness(FrontDisplaySrv* instance, FrontDisplayBrightness brightness) {
    FrontDisplayMessage message = {
        .api_lock = NULL, // No need for API lock here
        .type = FrontDisplayMessageTypeBrightness,
        .brightness = brightness.val,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
}

void front_display_set_blanked(FrontDisplaySrv* instance, bool is_blanked) {
    const FrontDisplayMessage message = {
        .api_lock = NULL, // No need for API lock here
        .type = FrontDisplayMessageTypeBlanking,
        .bool_param = is_blanked,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
}

void front_display_sleep_mode(FrontDisplaySrv* instance, bool sleep) {
    const FrontDisplayMessage message = {
        .api_lock = NULL, // No need for API lock here
        .type = FrontDisplayMessageTypeSleep,
        .bool_param = sleep,
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

    bool power_state = furi_hal_display_power_pin_read();

    FrontDisplayMessage message = {
        .api_lock = NULL, // No need for API lock here
        .type = power_state ? FrontDisplayMessageTypeOn : FrontDisplayMessageTypeOff,
    };

    furi_check(furi_message_queue_put(instance->message_queue, &message, 0) == FuriStatusOk);
}

static void front_display_power_event_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const PowerEvent* event = message;
    FrontDisplaySrv* instance = context;

    if(event->type == PowerEventBatteryPresent) {
        FrontDisplayMessage msg = {
            .api_lock = NULL,
            .type = FrontDisplayMessageTypeBatteryReady,
        };
        furi_check(furi_message_queue_put(instance->message_queue, &msg, 0) == FuriStatusOk);
    }
}

static void front_display_power_pin_init(FrontDisplaySrv* instance) {
    furi_hal_display_power_pin_init();

    furi_hal_display_power_enable();

    furi_hal_display_power_pin_attach_callback(front_display_power_irq_callback, instance);

    furi_delay_ms(50); // Stabilize power state
}

static void front_display_power_reset(void) {
    // mask the GPIO interrupt to prevent firing disable again
    furi_hal_display_power_pin_interrupt_disable();

    furi_hal_display_power_disable();
    furi_delay_ms(50);
    furi_hal_display_power_enable();
    furi_delay_ms(50); // Allow time for the display to power up

    // re-enable the GPIO interrupt
    furi_hal_display_power_pin_interrupt_enable();
}

static uint8_t front_display_get_brightness(const FrontDisplaySrv* instance) {
    return instance->is_blanked ? instance->brightness_current : instance->brightness_override;
}

static void front_display_start(FrontDisplaySrv* display) {
    front_display_scan_init();
    front_display_driver_init(front_display_get_brightness(display));
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

static void front_display_handle_set_blanked(FrontDisplaySrv* instance, bool is_blanked) {
    if(instance->is_blanked != is_blanked) {
        instance->is_blanked = is_blanked;
        if(!furi_event_loop_timer_is_running(instance->transition_timer)) {
            instance->brightness_current = is_blanked ? instance->brightness_override :
                                                        FRONT_DISPLAY_BRIGHTNESS_MIN;
            furi_event_loop_timer_start(
                instance->transition_timer, FRONT_DISPLAY_TRANSITION_STEP_DURATION_MS);
        }
    }
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

        if(display->is_blanked) {
            break;
        }

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
        if(display->power_off_pending) {
            furi_hal_display_power_disable();
            display->power_off_pending = false;
        }
        FRONT_DISPLAY_DEBUG("Front display draw end");
        break;
    case FrontDisplayMessageTypeBrightness:
        display->brightness_override = message.brightness;
        {
            const uint32_t brightness = front_display_get_brightness(display);
            FRONT_DISPLAY_DEBUG("Updating front display brightness to %ld", brightness);
            front_display_driver_set_brightness(brightness);
        }
        display->need_update = true;
        break;
    case FrontDisplayMessageTypeBlanking:
        front_display_handle_set_blanked(display, message.bool_param);
        break;
    case FrontDisplayMessageTypeSleep:
        if((message.bool_param == true) && (display->enabled == true)) {
            memset(display->last_frame, 0, FRONT_DISPLAY_FRAME_SIZE);
            display->need_update = true;

            front_display_scan_output_enable(false);
            display->power_off_pending = true;
        } else if(message.bool_param == false) {
            furi_hal_display_power_enable();
            if(display->power_off_pending) {
                display->power_off_pending = false;
            }
        }
        break;
    case FrontDisplayMessageTypeOn:
        if(!display->enabled) {
            FRONT_DISPLAY_DEBUG("Power turned on, reinitializing");

            front_display_power_reset();
            front_display_start(display);

            display->enabled = true;
            display->need_update = true; // Force an update after enabling
        }
        break;
    case FrontDisplayMessageTypeOff:
        FRONT_DISPLAY_DEBUG("Power turned off");
        front_display_stop();
        display->enabled = false;
        display->send_in_progress = false;
        break;
    case FrontDisplayMessageTypeBatteryReady:
        FRONT_DISPLAY_DEBUG("Battery ready, initializing display");

        if(display->power_subscription) {
            furi_pubsub_unsubscribe(power_get_pubsub(display->power), display->power_subscription);
            display->power_subscription = NULL;
        }

        if(!display->enabled) {
            front_display_power_reset();
            front_display_start(display);
            display->enabled = true;
            display->need_update = true;
        }
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

static void front_display_transition_timer_callback(void* context) {
    furi_assert(context);
    FrontDisplaySrv* instance = context;

    const bool is_blanked = instance->is_blanked;
    const uint8_t brightness = instance->brightness_current;
    const uint8_t eff_brightness = instance->brightness_override;

    const bool is_stop_condition = is_blanked ? (brightness == 0) : (brightness >= eff_brightness);

    if(is_stop_condition) {
        furi_event_loop_timer_stop(instance->transition_timer);

    } else {
        const int32_t step_abs = MAX(eff_brightness / FRONT_DISPLAY_TRANSITION_STEP_COUNT, 1);
        const int32_t step = is_blanked ? -step_abs : step_abs;

        const uint8_t new_brightness = CLAMP(
            (int32_t)brightness + step,
            FRONT_DISPLAY_BRIGHTNESS_MAX,
            FRONT_DISPLAY_BRIGHTNESS_MIN);

        front_display_driver_set_brightness(new_brightness);

        instance->brightness_current = new_brightness;
        instance->need_update = true;
    }
}

static FrontDisplaySrv* front_display_alloc(void) {
    FrontDisplaySrv* instance = malloc(sizeof(FrontDisplaySrv));

    instance->brightness_override = FRONT_DISPLAY_BRIGHTNESS_MAX;

    instance->event_loop = furi_event_loop_alloc();
    instance->transition_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        front_display_transition_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->message_queue = furi_message_queue_alloc(8, sizeof(FrontDisplayMessage));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        front_display_message_queue_callback,
        instance);

    instance->power = furi_record_open(RECORD_POWER);

    front_display_power_pin_init(instance);

    instance->power_subscription = furi_pubsub_subscribe(
        power_get_pubsub(instance->power), front_display_power_event_callback, instance);

    if(power_is_battery_ready(instance->power)) {
        front_display_start(instance);
        instance->enabled = true;
        furi_pubsub_unsubscribe(power_get_pubsub(instance->power), instance->power_subscription);
        instance->power_subscription = NULL;
    }

    furi_record_create(RECORD_FRONT_DISPLAY, instance);
    return instance;
}

int32_t front_display_srv(void* p) {
    UNUSED(p);

    FrontDisplaySrv* instance = front_display_alloc();
    FURI_LOG_I(TAG, "Front Display Service started");
    furi_event_loop_run(instance->event_loop);

    return 0;
}
