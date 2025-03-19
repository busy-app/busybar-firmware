#include "light_sensor_test.h"

#include <furi.h>

#define TAG "LightSensorTest"

static void light_sensor_test_app_update(LightSensorTestApp* instance) {
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

static bool ligh_sensor_test_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    LightSensorTestApp* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        consumed = true;
        const LightSensorTestAppEvent app_event = {
            .type = LightSensorTestAppEventExit,
        };
        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    }

    return consumed;
}

static void
    light_sensor_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    LightSensorTestApp* instance = context;
    furi_check(object == instance->event_queue);

    LightSensorTestAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(event.type == LightSensorTestAppEventLightLevelUpdate) {
        instance->light_level = event.light_level;
        light_sensor_test_app_update(instance);
    } else if(event.type == LightSensorTestAppEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
}

static void light_sensor_test_app_get_measurements(LightSensorTestApp* instance) {
    instance->lux_instant = light_sensor_get_lux_instant();
    instance->lux_mean = light_sensor_get_lux();
    light_sensor_get_raw_data(LightSensorLightWavelength600nm, &instance->raw_600nm);
    light_sensor_get_raw_data(LightSensorLightWavelength840nm, &instance->raw_840nm);
}

static void light_sensor_test_app_timer_callback(void* context) {
    LightSensorTestApp* instance = context;

    light_sensor_test_app_get_measurements(instance);
    light_sensor_test_app_update(instance);
}

static void light_sensor_test_app_light_sensor_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    LightSensorTestApp* instance = context;
    const LightSensorEvent* event = message;
    LightSensorTestAppEvent app_event = {
        .type = LightSensorTestAppEventLightLevelUpdate,
        .light_level = event->light_level,
    };

    furi_check(
        furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
        FuriStatusOk);
}

static LightSensorTestApp* light_sensor_test_app_alloc(void) {
    LightSensorTestApp* instance = malloc(sizeof(LightSensorTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(LightSensorTestAppEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        light_sensor_test_app_event_queue_callback,
        instance);
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        light_sensor_test_app_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    // Not optimal for this app, just for testing pubsub events from service
    instance->light_sensor_events = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
    // To check light level changes in pubsub, receive further light level value from events
    instance->light_level = light_sensor_get_light_level();
    instance->light_sensor_subscription = furi_pubsub_subscribe(
        instance->light_sensor_events, light_sensor_test_app_light_sensor_callback, instance);

    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        instance->input_events = gui_layer_subscribe_to_input_events(
            main_layer, ligh_sensor_test_app_input_callback, instance);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->app_window = widget_alloc(root);

        // Back screen
        instance->label_light_raw = label_alloc(instance->app_window);
        widget_set_pos(label_get_base(instance->label_light_raw), 10, 0);

        instance->label_lux_instant = label_alloc(instance->app_window);
        widget_set_pos(label_get_base(instance->label_lux_instant), 10, 10);

        instance->label_lux_mean = label_alloc(instance->app_window);
        widget_set_pos(label_get_base(instance->label_lux_mean), 10, 30);

        instance->label_light_level = label_alloc(instance->app_window);
        widget_set_pos(label_get_base(instance->label_light_level), 10, 40);
    });

    light_sensor_test_app_get_measurements(instance);
    light_sensor_test_app_update(instance);

    furi_event_loop_timer_start(instance->timer, 1000 / 60);

    return instance;
}

static void light_sensor_test_app_free(LightSensorTestApp* instance) {
    furi_check(instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_unsubscribe_from_input_events(main_layer, instance->input_events);
        widget_free(instance->app_window);
    });

    furi_record_close(RECORD_GUI);

    furi_pubsub_unsubscribe(instance->light_sensor_events, instance->light_sensor_subscription);
    furi_record_close(RECORD_LIGHT_SENSOR_EVENTS);

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_timer_free(instance->timer);
    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t light_sensor_test_app(void* args) {
    UNUSED(args);

    LightSensorTestApp* instance = light_sensor_test_app_alloc();
    furi_event_loop_run(instance->event_loop);
    light_sensor_test_app_free(instance);

    return 0;
}
