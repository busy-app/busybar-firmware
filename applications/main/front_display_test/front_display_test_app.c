#include "front_display_test_app_i.h"

#include <furi.h>

#define TAG "FrontDisplayTest"

static bool front_display_test_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    FrontDisplayTestApp* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        FrontDisplayTestAppEvent app_event;

        if(event->key == InputKeyUp) {
            app_event = FrontDisplayTestAppEventPrevColor;
            consumed = true;
        } else if(event->key == InputKeyDown) {
            app_event = FrontDisplayTestAppEventNextColor;
            consumed = true;
        } else if(event->key == InputKeyBack) {
            app_event = FrontDisplayTestAppEventExit;
            consumed = true;
        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            app_event = FrontDisplayTestAppEventNextPattern;
            consumed = true;
        }

        if(consumed) {
            furi_check(
                furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
                FuriStatusOk);
        }
    }

    return consumed;
}

static void front_display_test_app_update(FrontDisplayTestApp* instance) {
    with_gui(instance->gui, {
        // Front display
        front_display_test_set(instance->canvas, instance->pattern, instance->color);
        // Back display
        label_set_text_fmt(
            instance->pattern_label,
            "Pattern: %s",
            front_display_get_pattern_str(instance->pattern));
        label_set_text_fmt(
            instance->color_label, "Color: %s", front_display_get_color_str(instance->color));
    });
}

static void
    front_display_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    FrontDisplayTestApp* instance = context;
    furi_check(object == instance->event_queue);

    FrontDisplayTestAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(event == FrontDisplayTestAppEventNextPattern) {
        instance->pattern = (instance->pattern + 1) % FrontDisplayTestPatternNum;
    } else if(event == FrontDisplayTestAppEventPrevPattern) {
        instance->pattern = (instance->pattern == 0) ? FrontDisplayTestPatternNum - 1 :
                                                       instance->pattern - 1;
    } else if(event == FrontDisplayTestAppEventNextColor) {
        instance->color = (instance->color + 1) % FrontDisplayTestColorNum;
    } else if(event == FrontDisplayTestAppEventPrevColor) {
        instance->color = (instance->color == 0) ? FrontDisplayTestColorNum - 1 :
                                                   instance->color - 1;
    } else if(event == FrontDisplayTestAppEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }

    front_display_test_app_update(instance);
}

static void front_display_test_app_timer_callback(void* context) {
    FrontDisplayTestApp* instance = context;
    front_display_test_app_update(instance);
}

static FrontDisplayTestApp* front_display_test_app_alloc(void) {
    FrontDisplayTestApp* instance = malloc(sizeof(FrontDisplayTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(FrontDisplayTestAppEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        front_display_test_app_event_queue_callback,
        instance);
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        front_display_test_app_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    // Create a single label for the back display
    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, front_display_test_app_input_callback, instance);

        Widget* root;

        // Back display
        root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);

        instance->app_window = widget_alloc(root);
        instance->static_label = label_alloc(instance->app_window);
        widget_set_pos(label_get_base(instance->static_label), 10, 0);
        label_set_text(
            instance->static_label, "Start/Ok - change pattern.\nEncoder - change color");

        instance->pattern_label = label_alloc(instance->app_window);
        widget_set_pos(label_get_base(instance->pattern_label), 10, 30);

        instance->color_label = label_alloc(instance->app_window);
        widget_set_pos(label_get_base(instance->color_label), 10, 40);

        // Front display
        root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
        instance->canvas = canvas_alloc(root, FRONT_DISPLAY_W, FRONT_DISPLAY_H);
    });

    instance->pattern = FrontDisplayTestPatternChess;
    instance->color = FrontDisplayTestColorRed;

    front_display_test_app_update(instance);
    furi_event_loop_timer_start(instance->timer, 1000 / 60);

    return instance;
}

static void front_display_test_app_free(FrontDisplayTestApp* instance) {
    furi_assert(instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, front_display_test_app_input_callback);

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

int32_t front_display_test_app(void* args) {
    UNUSED(args);

    FrontDisplayTestApp* instance = front_display_test_app_alloc();
    furi_event_loop_run(instance->event_loop);
    front_display_test_app_free(instance);

    return 0;
}
