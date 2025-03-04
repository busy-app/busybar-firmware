#include "led_display_test_app_i.h"

#include <input/input.h>
#include <lib/lvgl/src/widgets/canvas/lv_canvas.h>

#include <furi.h>

#define TAG "LedDisplayTest"

static void led_display_test_app_keypad_callback(lv_event_t* event) {
    LedDisplayTestApp* instance = lv_event_get_user_data(event);

    // TODO Fix focus and differ OK and START buttons
    const lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_KEY) {
        LedDisplayTestAppEvent app_event;
        const uint32_t key = *((uint32_t*)lv_event_get_param(event));
        FURI_LOG_I(TAG, "Key event: %ld", key);
        if(key == LV_KEY_LEFT) {
            app_event = LedDisplayTestAppEventPrevColor;
        } else if(key == LV_KEY_RIGHT) {
            app_event = LedDisplayTestAppEventNextColor;
        } else if(key == LV_KEY_ENTER) {
            app_event = LedDisplayTestAppEventNextPattern;
        } else if(key == LV_KEY_ESC) {
            app_event = LedDisplayTestAppEventExit;
        } else {
            furi_crash("Unknown key");
        }
        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static void led_display_test_app_draw(LedDisplayTestApp* instance) {
    gui_lock(instance->gui);

    memset(instance->canvas_buffer, 0, sizeof(instance->canvas_buffer));
    // Front screen
    led_display_test_set(instance->canvas_buffer, instance->pattern, instance->color);
    lv_obj_invalidate(instance->canvas);

    // Back screen
    lv_label_set_text_fmt(
        instance->pattern_label, "Pattern: %s", led_display_get_pattern_str(instance->pattern));
    lv_label_set_text_fmt(
        instance->color_label, "Color: %s", led_display_get_color_str(instance->color));

    gui_unlock(instance->gui);
}

static void led_display_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    LedDisplayTestApp* instance = context;
    furi_check(object == instance->event_queue);

    LedDisplayTestAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(event == LedDisplayTestAppEventNextPattern) {
        instance->pattern = (instance->pattern + 1) % LedDisplayTestPatternNum;
    } else if(event == LedDisplayTestAppEventPrevPattern) {
        instance->pattern = (instance->pattern == 0) ? LedDisplayTestPatternNum - 1 :
                                                       instance->pattern - 1;
    } else if(event == LedDisplayTestAppEventNextColor) {
        instance->color = (instance->color + 1) % LedDisplayTestColorNum;
    } else if(event == LedDisplayTestAppEventPrevColor) {
        instance->color = (instance->color == 0) ? LedDisplayTestColorNum - 1 :
                                                   instance->color - 1;
    } else if(event == LedDisplayTestAppEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
    led_display_test_app_draw(instance);
}

static void led_display_test_app_timer_callback(void* context) {
    LedDisplayTestApp* instance = context;
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
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        led_display_test_app_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    // Create a single label for the back display
    instance->gui = furi_record_open(RECORD_GUI_LVGL);
    gui_lock(instance->gui);

    lv_obj_t* active = gui_get_layer(instance->gui, GuiDisplayIdBack, GuiLayerIdActive);

    // Back screen
    instance->static_label = lv_label_create(active);
    lv_obj_set_pos(instance->static_label, 10, 0);
    lv_obj_set_style_text_color(instance->static_label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(
        instance->static_label, "Start/Ok - change pattern.\nEncoder - change color");

    instance->pattern_label = lv_label_create(active);
    lv_obj_set_pos(instance->pattern_label, 10, 30);
    lv_obj_set_style_text_color(instance->pattern_label, lv_color_white(), LV_PART_MAIN);

    instance->color_label = lv_label_create(active);
    lv_obj_set_pos(instance->color_label, 10, 40);
    lv_obj_set_style_text_color(instance->color_label, lv_color_white(), LV_PART_MAIN);

    // Front screen
    active = gui_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);
    instance->canvas = lv_canvas_create(active);
    lv_canvas_set_buffer(
        instance->canvas, instance->canvas_buffer, 72, 16, LV_COLOR_FORMAT_RGB888);

    lv_group_add_obj(lv_group_get_default(), instance->canvas);
    lv_group_focus_obj(instance->canvas);
    // Input events
    lv_obj_add_event_cb(
        instance->canvas, led_display_test_app_keypad_callback, LV_EVENT_KEY, instance);

    gui_unlock(instance->gui);

    instance->pattern = LedDisplayTestPatternChess;
    instance->color = LedDisplayTestColorRed;
    led_display_test_app_draw(instance);
    furi_event_loop_timer_start(instance->timer, 1000 / 60);

    return instance;
}

static void led_display_test_app_free(LedDisplayTestApp* instance) {
    furi_check(instance);

    gui_lock(instance->gui);

    lv_obj_delete(instance->static_label);
    lv_obj_delete(instance->color_label);
    lv_obj_delete(instance->pattern_label);
    lv_obj_delete(instance->canvas);

    gui_unlock(instance->gui);

    furi_record_close(RECORD_GUI_LVGL);

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_timer_free(instance->timer);
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
