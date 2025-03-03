#include "light_sensor_test.h"

#include <furi.h>

#define TAG "LightSensorTest"

static void light_sensor_test_app_draw(LightSensorTestApp* instance) {
    gui_lvgl_acquire(instance->gui);

    // Back screen
    lv_label_set_text_fmt(
        instance->label_light_raw,
        "600nm: %d, 840nm: %d",
        instance->raw_600nm,
        instance->raw_840nm);
    lv_label_set_text_fmt(instance->label_lux_instant, "Lux instant: %.2f", instance->lux_instant);
    lv_label_set_text_fmt(instance->label_lux_mean, "Lux mean: %.2f", instance->lux_mean);
    lv_label_set_text_fmt(instance->label_light_level, "Light level: %d", instance->light_level);

    gui_lvgl_release(instance->gui);
}

static void light_sensor_test_app_keypad_callback(lv_event_t* event) {
    LightSensorTestApp* instance = lv_event_get_user_data(event);

    const lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_KEY) {
        const uint32_t key = *((uint32_t*)lv_event_get_param(event));
        if(key == LV_KEY_ESC) {
            LightSensorTestAppEvent app_event = {
                .type = LightSensorTestAppEventExit,
            };
            furi_check(
                furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
                FuriStatusOk);
        }
    }
}

static void
    light_sensor_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    LightSensorTestApp* instance = context;
    furi_check(object == instance->event_queue);

    LightSensorTestAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(event.type == LightSensorTestAppEventLightLevelUpdate) {
        instance->light_level = event.light_level;
        light_sensor_test_app_draw(instance);
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
    light_sensor_test_app_draw(instance);
}

static void light_sensor_test_app_light_sensor_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    LightSensorTestApp* instance = context;
    const LightSensorEvent* event = message;
    LightSensorTestAppEvent app_event = {
        .type = LightSensorTestAppEventLightLevelUpdate,
        .light_level = event->light_level_current,
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

    instance->gui = furi_record_open(RECORD_GUI_LVGL);
    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdBack, GuiLayerIdActive);

    // Back screen
    instance->label_light_raw = lv_label_create(active);
    lv_obj_set_pos(instance->label_light_raw, 10, 0);
    lv_obj_set_style_text_color(instance->label_light_raw, lv_color_white(), LV_PART_MAIN);

    instance->label_lux_instant = lv_label_create(active);
    lv_obj_set_pos(instance->label_lux_instant, 10, 10);
    lv_obj_set_style_text_color(instance->label_lux_instant, lv_color_white(), LV_PART_MAIN);

    instance->label_lux_mean = lv_label_create(active);
    lv_obj_set_pos(instance->label_lux_mean, 10, 30);
    lv_obj_set_style_text_color(instance->label_lux_mean, lv_color_white(), LV_PART_MAIN);

    instance->label_light_level = lv_label_create(active);
    lv_obj_set_pos(instance->label_light_level, 10, 40);
    lv_obj_set_style_text_color(instance->label_light_level, lv_color_white(), LV_PART_MAIN);

    // Input events
    active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);
    instance->dummy_input = lv_label_create(active);
    lv_group_add_obj(lv_group_get_default(), instance->dummy_input);
    lv_obj_add_event_cb(
        instance->dummy_input, light_sensor_test_app_keypad_callback, LV_EVENT_KEY, instance);

    gui_lvgl_release(instance->gui);

    light_sensor_test_app_get_measurements(instance);
    light_sensor_test_app_draw(instance);

    furi_event_loop_timer_start(instance->timer, 1000 / 60);

    return instance;
}

static void light_sensor_test_app_free(LightSensorTestApp* instance) {
    furi_check(instance);

    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(instance->label_light_raw);
    lv_obj_delete(instance->label_lux_instant);
    lv_obj_delete(instance->label_lux_mean);
    lv_obj_delete(instance->label_light_level);
    lv_obj_delete(instance->dummy_input);

    gui_lvgl_release(instance->gui);

    furi_record_close(RECORD_GUI_LVGL);

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
