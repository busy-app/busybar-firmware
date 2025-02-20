#include "led_display_test_app_i.h"

#include <input/input.h>
#include <lib/lvgl/src/widgets/canvas/lv_canvas.h>

#include <furi.h>

#define TAG "LedDisplayTest"

static void led_display_test_app_keypad_callback(lv_event_t* event) {
    LedDisplayTestApp* instance = lv_event_get_user_data(event);

    const lv_event_code_t code = lv_event_get_code(event);
    const lv_indev_t* indev = lv_event_get_param(event);

    if(lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD) {
        if(code == LV_EVENT_SINGLE_CLICKED) {
            FURI_LOG_I(TAG, "SINGLE_CLICKED");
            instance->pattern = (instance->pattern + 1) % LedDisplayTestPatternNum;
            LedDisplayTestAppEvent event = {
                .type = LedDisplayTestAppEventTypeNextPattern,
            };
            furi_check(
                furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) ==
                FuriStatusOk);
        }
    }
}

static void led_display_test_app_draw(LedDisplayTestApp* instance) {
    gui_lvgl_acquire(instance->gui);

    memset(instance->canvas_buffer, 0, sizeof(instance->canvas_buffer));
    // Front screen
    led_display_test_set(instance->canvas_buffer, instance->pattern, instance->color);

    // Back screen
    lv_label_set_text(instance->back_label, led_display_get_pattern_str(instance->pattern));

    gui_lvgl_release(instance->gui);
}

static void led_display_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    LedDisplayTestApp* instance = context;
    furi_check(object == instance->event_queue);

    LedDisplayTestAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    led_display_test_app_draw(instance);
}

static LedDisplayTestApp* led_display_test_app_alloc(void) {
    LedDisplayTestApp* instance = malloc(sizeof(LedDisplayTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(LedDisplayTestAppEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        led_display_test_app_event_queue_callback,
        instance);

    // FuriPubSub* input = furi_record_open(RECORD_INPUT_EVENTS);
    // instance->input_events =
    //     furi_pubsub_subscribe(input, led_display_test_app_input_callback, instance);

    // Create a single label for the back display
    instance->gui = furi_record_open(RECORD_GUI_LVGL);
    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdBack, GuiLayerIdActive);

    // Back screen
    instance->back_label = lv_label_create(active);
    lv_obj_center(instance->back_label);
    lv_obj_set_style_text_color(instance->back_label, lv_color_white(), LV_PART_MAIN);

    // Front screen
    active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);
    instance->canvas = lv_canvas_create(active);
    FURI_LOG_E(
        TAG, "Color format pizel size: %d", lv_color_format_get_size(LV_COLOR_FORMAT_RGB888));
    lv_canvas_set_buffer(
        instance->canvas, instance->canvas_buffer, 72, 16, LV_COLOR_FORMAT_RGB888);

    // Input events
    lv_obj_add_event_cb(
        instance->canvas, led_display_test_app_keypad_callback, LV_EVENT_SINGLE_CLICKED, instance);
    lv_group_add_obj(lv_group_get_default(), instance->canvas);

    gui_lvgl_release(instance->gui);

    instance->pattern = LedDisplayTestPatternChess;
    instance->color = LedDisplayTestColorRed;
    led_display_test_app_draw(instance);

    return instance;
}

static void led_display_test_app_free(LedDisplayTestApp* instance) {
    furi_check(instance);

    gui_lvgl_acquire(instance->gui);
    lv_obj_delete(instance->back_label);
    gui_lvgl_release(instance->gui);
    furi_record_close(RECORD_GUI_LVGL);

    furi_message_queue_free(instance->event_queue);
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
