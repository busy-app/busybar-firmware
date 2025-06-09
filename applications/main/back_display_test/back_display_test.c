#include "back_display_test.h"
#include "back_display_test_canvas.h"

#define TAG "BackDisplayTest"

typedef enum {
    BackDisplayTestAppEventNextPattern,
    BackDisplayTestAppEventPrevPattern,
    BackDisplayTestAppEventNextColor,
    BackDisplayTestAppEventPrevColor,
    BackDisplayTestAppEventTick,
    BackDisplayTestAppEventExit,
} BackDisplayTestAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* timer;
    Gui* gui;

    // Back display
    Widget* app_window;
    Label* pattern_label;

    // Front display
    Canvas* canvas;

    BackDisplayTestPattern current_pattern;
    BackDisplayTestColor current_color;
} BackDisplayTestApp;

static bool back_display_test_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BackDisplayTestApp* app = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        BackDisplayTestAppEvent app_event;

        if(event->key == InputKeyUp) {
            app_event = BackDisplayTestAppEventPrevColor;
            consumed = true;
        } else if(event->key == InputKeyDown) {
            app_event = BackDisplayTestAppEventNextColor;
            consumed = true;
        } else if(event->key == InputKeyBack) {
            app_event = BackDisplayTestAppEventExit;
            consumed = true;
        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            app_event = BackDisplayTestAppEventNextPattern;
            consumed = true;
        }

        if(consumed) {
            furi_check(
                furi_message_queue_put(app->event_queue, &app_event, FuriWaitForever) ==
                FuriStatusOk);
        }
    }

    return consumed;
}

static void back_display_test_app_update(BackDisplayTestApp* app) {
    with_gui(app->gui, {
        back_display_test_canvas_update(app->canvas, app->current_pattern, app->current_color);
        label_set_text_fmt(
            app->pattern_label,
            "Color: %.0f%%\nPattern: %s",
            100.0f / (BackDisplayTestColorMax - 1) *
                (BackDisplayTestColorMax - app->current_color - 1),
            back_display_test_pattern_to_string(app->current_pattern));
    });
}

static void
    back_display_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    BackDisplayTestApp* app = context;
    furi_check(object == app->event_queue);

    BackDisplayTestAppEvent event;
    furi_check(furi_message_queue_get(app->event_queue, &event, 0) == FuriStatusOk);

    switch(event) {
    case BackDisplayTestAppEventNextPattern:
        app->current_pattern = (app->current_pattern + 1) % BackDisplayTestPatternMax;
        break;
    case BackDisplayTestAppEventPrevPattern:
        app->current_pattern = (app->current_pattern == 0) ? BackDisplayTestPatternMax - 1 :
                                                             app->current_pattern - 1;
        break;
    case BackDisplayTestAppEventNextColor:
        app->current_color = (app->current_color + 1) % BackDisplayTestColorMax;
        break;
    case BackDisplayTestAppEventPrevColor:
        app->current_color = (app->current_color == 0) ? BackDisplayTestColorMax - 1 :
                                                         app->current_color - 1;
        break;
    case BackDisplayTestAppEventTick:
        break;
    case BackDisplayTestAppEventExit:
        furi_event_loop_stop(app->event_loop);
        break;
    }

    back_display_test_app_update(app);
}

static void back_display_test_app_timer_callback(void* context) {
    BackDisplayTestApp* app = context;
    back_display_test_app_update(app);
}

static BackDisplayTestApp* back_display_test_app_alloc() {
    BackDisplayTestApp* app = malloc(sizeof(BackDisplayTestApp));

    app->event_loop = furi_event_loop_alloc();
    app->event_queue = furi_message_queue_alloc(16, sizeof(BackDisplayTestAppEvent));
    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->event_queue,
        FuriEventLoopEventIn,
        back_display_test_app_event_queue_callback,
        app);
    app->timer = furi_event_loop_timer_alloc(
        app->event_loop, back_display_test_app_timer_callback, FuriEventLoopTimerTypePeriodic, app);

    // Create a single label for the back display
    app->gui = furi_record_open(RECORD_GUI);

    with_gui(app->gui, {
        GuiLayer* main_layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, back_display_test_app_input_callback, app);

        Widget* root;

        // Front display
        {
            root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);

            app->app_window = widget_alloc(root);

            app->pattern_label = label_alloc(app->app_window);
        }

        //  Back display
        {
            GuiLayer* system_layer = gui_get_layer(app->gui, GuiLayerIdSystem);

            root = gui_layer_get_root_widget(system_layer, GuiDisplayIdBack);
            app->canvas = canvas_alloc(root, widget_get_width(root), widget_get_height(root));
        }
    });

    back_display_test_app_update(app);
    furi_event_loop_timer_start(app->timer, 1000 / 60);

    return app;
}

static void back_display_test_app_free(BackDisplayTestApp* app) {
    furi_assert(app);

    with_gui(app->gui, {
        GuiLayer* main_layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, back_display_test_app_input_callback);

        widget_free(app->app_window);
        canvas_free(app->canvas);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(app->event_loop, app->event_queue);
    furi_message_queue_free(app->event_queue);
    furi_event_loop_timer_free(app->timer);
    furi_event_loop_free(app->event_loop);
    free(app);
}

int32_t back_display_test(void* args) {
    UNUSED(args);

    BackDisplayTestApp* app = back_display_test_app_alloc();
    furi_event_loop_run(app->event_loop);
    back_display_test_app_free(app);

    return 0;
}
