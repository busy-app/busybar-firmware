#include "light_sensor_test.h"

#include <furi.h>

#define TAG "LightSensorTest"

#define LIGHT_SENSOR_TEST_QUEUE_TIMEOUT_TICK (10)

static void light_sensor_test_app_update(LightSensorTestApp* instance) {
    const LightSensorState* light_sensor_state = &instance->light_sensor_state;

    with_gui(instance->gui, {
        // Back screen
        label_set_text_fmt(instance->label_light_raw_600nm, "600nm: %d", instance->raw_600nm);
        label_set_text_fmt(instance->label_light_raw_840nm, "840nm: %d", instance->raw_840nm);
        label_set_text_fmt(
            instance->label_lux_instant, "Lux instant: %.2f", light_sensor_state->lux.instant);
        label_set_text_fmt(
            instance->label_lux_mean, "Lux mean: %.2f", light_sensor_state->lux.mean);
        label_set_text_fmt(
            instance->label_light_level, "Light level: %hhu", light_sensor_state->level.val);
    });
}

static bool light_sensor_test_app_send_event(
    LightSensorTestApp* instance,
    const LightSensorTestAppEvent* event) {
    bool success = true;

    const FuriStatus status =
        furi_message_queue_put(instance->event_queue, event, LIGHT_SENSOR_TEST_QUEUE_TIMEOUT_TICK);

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorTimeout);
        success = false;
    }

    return success;
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

        if(!light_sensor_test_app_send_event(instance, &app_event)) {
            FURI_LOG_W(TAG, "Input event dropped");
        }
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
        instance->light_sensor_state = event.lighth_sensor_state;
        light_sensor_test_app_update(instance);
    } else if(event.type == LightSensorTestAppEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
}

static void light_sensor_test_app_get_measurements(LightSensorTestApp* instance) {
    LightSensor* light_sensor = instance->light_sensor;
    light_sensor_get_raw_data(light_sensor, LightSensorLightWavelength600nm, &instance->raw_600nm);
    light_sensor_get_raw_data(light_sensor, LightSensorLightWavelength840nm, &instance->raw_840nm);
}

static void light_sensor_test_app_timer_callback(void* context) {
    LightSensorTestApp* instance = context;

    light_sensor_test_app_get_measurements(instance);
    light_sensor_test_app_update(instance);
}

static void light_sensor_test_app_light_sensor_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    LightSensorTestApp* instance = context;
    const LightSensorState* light_sensor_state = item;

    const LightSensorTestAppEvent app_event = {
        .type = LightSensorTestAppEventLightLevelUpdate,
        .lighth_sensor_state = *light_sensor_state,
    };

    if(!light_sensor_test_app_send_event(instance, &app_event)) {
        FURI_LOG_W(TAG, "Light sensor event dropped");
    }
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

    instance->light_sensor = furi_record_open(RECORD_LIGHT_SENSOR);
    instance->light_sensor_events = furi_state_subscribe(
        light_sensor_get_state(instance->light_sensor),
        light_sensor_test_app_light_sensor_callback,
        instance);

    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, ligh_sensor_test_app_input_callback, instance);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);

        instance->flex = flex_layout_alloc(root, FlexLayoutTypeColumn);
        Widget* flex_base = flex_layout_get_base(instance->flex);

        instance->label_light_raw_600nm = label_alloc(flex_base);
        instance->label_light_raw_840nm = label_alloc(flex_base);

        instance->label_lux_instant = label_alloc(flex_base);
        flex_layout_set_child_widget_grow(
            instance->flex, label_get_base(instance->label_lux_instant), 2);

        instance->label_lux_mean = label_alloc(flex_base);
        instance->label_light_level = label_alloc(flex_base);
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
        gui_layer_remove_input_callback(main_layer, ligh_sensor_test_app_input_callback);
        flex_layout_free(instance->flex);
    });

    furi_record_close(RECORD_GUI);

    furi_state_unsubscribe(instance->light_sensor_events);
    furi_record_close(RECORD_LIGHT_SENSOR);

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
