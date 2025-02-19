#include "led_display_test_app_i.h"

#include <input/input.h>
#include <lib/lvgl/src/widgets/canvas/lv_canvas.h>

#include <furi.h>

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

static uint8_t led_disp_buffer[72 * 16 * 3] = {};

static void led_display_test_app_draw(LedDisplayTestApp* instance) {
    gui_lvgl_acquire(instance->gui);

    // Back screen
    lv_label_set_text(instance->back_label, "Hui");

    // Front screen
    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);
    lv_obj_t* canvas = lv_canvas_create(active);
    lv_canvas_set_buffer(canvas, led_disp_buffer, 72, 16, LV_COLOR_FORMAT_RGB888);
    memset(led_disp_buffer, 0xff, sizeof(led_disp_buffer) / 3);

    gui_lvgl_release(instance->gui);
}

static LedDisplayTestApp* led_display_test_app_alloc(void) {
    LedDisplayTestApp* instance = malloc(sizeof(LedDisplayTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(LedDisplayTestAppEvent));
    instance->gui = furi_record_open(RECORD_GUI_LVGL);

    // FuriPubSub* input = furi_record_open(RECORD_INPUT_EVENTS);
    // instance->input_events =
    //     furi_pubsub_subscribe(input, led_display_test_app_input_callback, instance);

    // Create a single label for the back display
    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdBack, GuiLayerIdActive);
    instance->back_label = lv_label_create(active);
    lv_obj_center(instance->back_label);
    lv_obj_set_style_text_color(instance->back_label, lv_color_white(), LV_PART_MAIN);

    gui_lvgl_release(instance->gui);

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
