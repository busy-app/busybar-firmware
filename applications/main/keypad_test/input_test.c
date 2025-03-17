#include "input_test.h"

#include <furi.h>

#define TAG "InputTest"

static void input_test_app_update(InputTestApp* instance) {
    with_gui(instance->gui, {
        // Back screen
        label_set_text_fmt(
            instance->label_light_raw,
            "600nm: %d, 840nm: %d",
            instance->raw_600nm,
            instance->raw_840nm);
        label_set_text_fmt(
            instance->label_lux_instant, "Lux instant: %.2f", instance->lux_instant);
        label_set_text_fmt(instance->label_lux_mean, "Lux mean: %.2f", instance->lux_mean);
        label_set_text_fmt(instance->label_light_level, "Light level: %d", instance->light_level);
    });
}

static void ligh_sensor_test_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    InputTestApp* instance = context;

    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        const InputTestAppEvent app_event = {
            .type = InputTestAppEventExit,
        };
        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static void input_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    InputTestApp* instance = context;
    furi_check(object == instance->event_queue);

    InputTestAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(event.type == InputTestAppEventLightLevelUpdate) {
        instance->light_level = event.light_level;
        input_test_app_update(instance);
    } else if(event.type == InputTestAppEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
}

static void input_test_app_get_measurements(InputTestApp* instance) {
    instance->lux_instant = 10.0f;
    instance->lux_mean = 11.f;
    // input_get_raw_data(InputLightWavelength600nm, &instance->raw_600nm);
    // input_get_raw_data(InputLightWavelength840nm, &instance->raw_840nm);
}

static void input_test_app_timer_callback(void* context) {
    InputTestApp* instance = context;

    input_test_app_get_measurements(instance);
    input_test_app_update(instance);
}

static InputTestApp* input_test_app_alloc(void) {
    InputTestApp* instance = malloc(sizeof(InputTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(InputTestAppEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        input_test_app_event_queue_callback,
        instance);
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        input_test_app_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    // To check light level changes in pubsub, receive further light level value from events
    instance->light_level = 2;

    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        Widget* root = gui_get_root_widget(instance->gui, GuiDisplayIdBack, GuiLayerIdMain);

        instance->app_window = widget_alloc(root);

        // Back screen
        instance->label_light_raw = label_alloc(instance->app_window);
        widget_set_pos((Widget*)instance->label_light_raw, 10, 0);

        instance->label_lux_instant = label_alloc(instance->app_window);
        widget_set_pos((Widget*)instance->label_lux_instant, 10, 10);

        instance->label_lux_mean = label_alloc(instance->app_window);
        widget_set_pos((Widget*)instance->label_lux_mean, 10, 30);

        instance->label_light_level = label_alloc(instance->app_window);
        widget_set_pos((Widget*)instance->label_light_level, 10, 40);

        // Input events
        widget_set_input_callback(
            instance->app_window, ligh_sensor_test_app_input_callback, instance);

        gui_add_active_widget(instance->gui, instance->app_window);
    });

    input_test_app_get_measurements(instance);
    input_test_app_update(instance);

    furi_event_loop_timer_start(instance->timer, 1000 / 60);

    return instance;
}

static void input_test_app_free(InputTestApp* instance) {
    furi_check(instance);

    with_gui(instance->gui, { widget_free(instance->app_window); });

    furi_record_close(RECORD_GUI);

    furi_record_close(RECORD_LIGHT_SENSOR_EVENTS);

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_timer_free(instance->timer);
    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t input_test_app(void* args) {
    UNUSED(args);

    InputTestApp* instance = input_test_app_alloc();
    furi_event_loop_run(instance->event_loop);
    input_test_app_free(instance);

    return 0;
}
