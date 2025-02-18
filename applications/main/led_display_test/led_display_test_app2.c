#include "led_display_test_app_i.h"

#include <input/input.h>

#include <furi.h>

static void led_display_test_app_input_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    LedDisplayTestApp* instance = context;
    const InputEvent* event = message;

    if(event->type == InputTypeShort) {
        LedDisplayTestAppEventType event_type;

        if(event->key == InputKeyStart) {
            event_type = LedDisplayTestAppEventTypeNextPattern;
        } else if(event->key == InputKeyBack) {
            event_type = BusyEventTypeBack;
        } else if(event->key == InputKeyOk) {
            event_type = LedDisplayTestAppEventTypeUpdateColor;
        } else {
            return;
        }

        const LedDisplayTestAppEvent event = {
            .type = event_type,
        };

        furi_check(
            furi_message_queue_put(instance->event_queue, &busy_event, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static LedDisplayTestApp* led_display_test_app_alloc(void) {
    LedDisplayTestApp* instance = malloc(sizeof(LedDisplayTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(LedDisplayTestAppEvent));
    instance->gui = furi_record_open(RECORD_GUI_LVGL);

    FuriPubSub* input = furi_record_open(RECORD_INPUT_EVENTS);
    instance->input_events =
        furi_pubsub_subscribe(input, led_display_test_app_input_callback, instance);

    return instance;
}

static void led_display_test_app_free(LedDisplayTestApp* instance) {
    furi_check(instance);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t led_display_test_app(void* args) {
    UNUSED(args);

    LedDisplayTestApp* instance = led_display_test_app_alloc();
    furi_event_loop_run(instance->event_loop);
    led_display_test_app_free(instance);

    return 0;
}
