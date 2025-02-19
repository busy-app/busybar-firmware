#include "led_display_test_app_i.h"

#include <input/input.h>
#include <lib/lvgl/src/widgets/canvas/lv_canvas.h>

#include <furi.h>

#define TAG "LedDisplayTest"

// static void led_display_test_app_input_callback(const void* message, void* context) {
//     furi_assert(message);
//     furi_assert(context);

//     LedDisplayTestApp* instance = context;
//     const InputEvent* event = message;

//     if(event->type == InputTypeShort) {
//         LedDisplayTestAppEventType event_type;

//         if(event->key == InputKeyStart) {
//             event_type = LedDisplayTestAppEventTypeNextPattern;
//         } else if(event->key == InputKeyBack) {
//             event_type = LedDisplayTestAppEventTypeExit;
//         } else if(event->key == InputKeyOk) {
//             event_type = LedDisplayTestAppEventTypeUpdateColor;
//         } else {
//             return;
//         }

//         const LedDisplayTestAppEvent event = {
//             .type = event_type,
//         };

//         furi_check(
//             furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) ==
//             FuriStatusOk);
//     }
// }

static void led_display_test_app_keypad_callback(lv_event_t* event) {
    LedDisplayTestApp* instance = lv_event_get_user_data(event);

    const lv_event_code_t code = lv_event_get_code(event);
    const lv_indev_t* indev = lv_event_get_param(event);

    if(lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD) {
        if(code == LV_EVENT_SINGLE_CLICKED) {
            FURI_LOG_I(TAG, "SINGLE_CLICKED");
            instance->pattern = (instance->pattern + 1) % LedDisplayTestAppPatternNum;
            LedDisplayTestAppEvent event = {
                .type = LedDisplayTestAppEventTypeNextPattern,
            };
            furi_check(
                furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) ==
                FuriStatusOk);
        }
    }
}

static uint8_t led_disp_buffer[72 * 16 * 3] = {};

static void full_fill_on_enter(LedDisplayTestApp* instance) {
    gui_lvgl_acquire(instance->gui);

    // Front screen
    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);
    instance->canvas = lv_canvas_create(active);

    lv_obj_add_event_cb(
        instance->canvas, led_display_test_app_keypad_callback, LV_EVENT_SINGLE_CLICKED, instance);
    lv_group_add_obj(lv_group_get_default(), instance->canvas);

    lv_canvas_set_buffer(instance->canvas, led_disp_buffer, 72, 16, LV_COLOR_FORMAT_RGB888);
    // memset(led_disp_buffer, 0xff, sizeof(led_disp_buffer) / 3);
    lv_canvas_set_px(instance->canvas, 10, 10, lv_color_make(12, 123, 52), 100);

    // Back screen
    lv_label_set_text(instance->back_label, "Rect");

    gui_lvgl_release(instance->gui);
}

static void full_fill_on_exit(LedDisplayTestApp* instance) {
    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(instance->canvas);

    gui_lvgl_release(instance->gui);
}

const lv_point_precise_t points[] = {
    [0] = {.x = 0, .y = 0},
    [1] = {.x = 71, .y = 15},
};

const lv_point_precise_t points2[] = {
    [0] = {.x = 72, .y = 0},
    [1] = {.x = 0, .y = 15},
};

static void cross_lines_on_enter(LedDisplayTestApp* instance) {
    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 1);
    lv_style_set_line_color(&style_line, lv_color_make(255, 255, 255));
    lv_style_set_line_rounded(&style_line, true);

    instance->lines[0] = lv_line_create(active);
    lv_line_set_points(instance->lines[0], points, 2);
    lv_obj_add_style(instance->lines[0], &style_line, 0);

    instance->lines[1] = lv_line_create(active);
    lv_line_set_points(instance->lines[1], points2, 2);
    lv_obj_add_style(instance->lines[1], &style_line, 0);

    lv_obj_add_event_cb(
        instance->lines[0],
        led_display_test_app_keypad_callback,
        LV_EVENT_SINGLE_CLICKED,
        instance);
    lv_group_add_obj(lv_group_get_default(), instance->lines[0]);

    gui_lvgl_release(instance->gui);
}

static void cross_lines_on_exit(LedDisplayTestApp* instance) {
    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(instance->lines[0]);
    // lv_obj_delete(instance->lines[1]);

    gui_lvgl_release(instance->gui);
}

static void led_display_test_app_draw(LedDisplayTestApp* instance) {
    cross_lines_on_enter(instance);
}

static void
    led_display_test_app_draw_on_event(LedDisplayTestAppEvent event, LedDisplayTestApp* instance) {
    if(event.type == LedDisplayTestAppEventTypeNextPattern) {
        if(instance->pattern == LedDisplayTestAppPatternFullFill) {
            cross_lines_on_exit(instance);
            full_fill_on_enter(instance);
        } else if(instance->pattern == LedDisplayTestAppPatternCrossLines) {
            full_fill_on_exit(instance);
            cross_lines_on_enter(instance);
        }
    }
}

static void led_display_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    LedDisplayTestApp* instance = context;
    furi_check(object == instance->event_queue);

    LedDisplayTestAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    led_display_test_app_draw_on_event(event, instance);
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
    instance->back_label = lv_label_create(active);
    lv_obj_center(instance->back_label);
    lv_obj_set_style_text_color(instance->back_label, lv_color_white(), LV_PART_MAIN);

    gui_lvgl_release(instance->gui);

    instance->pattern = LedDisplayTestAppPatternFullFill;
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
