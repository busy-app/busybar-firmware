#include "led_display_test_app_i.h"

#include <furi.h>

#define TAG "LedDisplayTest"

static void led_display_test_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    LedDisplayTestApp* instance = context;

    if(event->type == InputTypeShort) {
        LedDisplayTestAppEvent app_event;

        if(event->key == InputKeyUp) {
            app_event = LedDisplayTestAppEventPrevColor;
        } else if(event->key == InputKeyDown) {
            app_event = LedDisplayTestAppEventNextColor;
        } else if(event->key == InputKeyBack) {
            app_event = LedDisplayTestAppEventExit;
        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            app_event = LedDisplayTestAppEventNextPattern;
        } else {
            return;
        }

        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static void led_display_test_app_update(LedDisplayTestApp* instance) {
    with_gui(instance->gui, {
        // Front display
        led_display_test_set(instance->canvas, instance->pattern, instance->color);
        // Back display
        label_set_text_fmt(
            instance->pattern_label,
            "Pattern: %s",
            led_display_get_pattern_str(instance->pattern));
        label_set_text_fmt(
            instance->color_label, "Color: %s", led_display_get_color_str(instance->color));
    });
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

    led_display_test_app_update(instance);
}

static void led_display_test_app_timer_callback(void* context) {
    LedDisplayTestApp* instance = context;
    led_display_test_app_update(instance);
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
    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        Widget* root;

        // Back display
        root = gui_get_root_widget(gui, GuiDisplayIdBack, GuiLayerIdActive);

        instance->app_window = widget_alloc(root);
        widget_set_input_callback(
            instance->app_window, led_display_test_app_input_callback, instance);

        instance->static_label = label_alloc(instance->app_window);
        widget_set_pos((Widget*)instance->static_label, 10, 0);
        label_set_text(
            instance->static_label, "Start/Ok - change pattern.\nEncoder - change color");

        instance->pattern_label = label_alloc(instance->app_window);
        widget_set_pos((Widget*)instance->pattern_label, 10, 30);

        instance->color_label = label_alloc(instance->app_window);
        widget_set_pos((Widget*)instance->color_label, 10, 40);

        // Front display
        root = gui_get_root_widget(gui, GuiDisplayIdFront, GuiLayerIdActive);
        instance->canvas = canvas_alloc(root, DOT_MATRIX_W, DOT_MATRIX_H);

        gui_set_active_widget(gui, instance->app_window);
    });

    instance->pattern = LedDisplayTestPatternChess;
    instance->color = LedDisplayTestColorRed;

    led_display_test_app_update(instance);
    furi_event_loop_timer_start(instance->timer, 1000 / 60);

    return instance;
}

static void led_display_test_app_free(LedDisplayTestApp* instance) {
    furi_assert(instance);

    with_gui(instance->gui, {
        widget_free(instance->app_window);
        canvas_free(instance->canvas);
    });

    furi_record_close(RECORD_GUI);

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
