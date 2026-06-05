#include "back_display_factory.h"
#include "back_display_test_patterns.h"
#include <brightness_control/brightness_control.h>
#include <desktop/desktop.h>

#define TAG "BackDisplayFactory"

typedef enum {
    BackDisplayFactoryAppEventNextPattern,
    BackDisplayFactoryAppEventPrevPattern,
    BackDisplayFactoryAppEventTick,
    BackDisplayFactoryAppEventExit,
} BackDisplayFactoryAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* timer;
    Gui* gui;
    Desktop* desktop;

    // Back display
    Widget* app_window;
    Label* pattern_label;

    // Front display
    Canvas* canvas;

    BackDisplayPattern current_pattern;
} BackDisplayFactoryApp;

static bool back_display_factory_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BackDisplayFactoryApp* app = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        BackDisplayFactoryAppEvent app_event;

        if(event->key == InputKeyUp) {
            app_event = BackDisplayFactoryAppEventNextPattern;
            consumed = true;
        } else if(event->key == InputKeyDown) {
            app_event = BackDisplayFactoryAppEventPrevPattern;
            consumed = true;
        } else if(event->key == InputKeyBack) {
            app_event = BackDisplayFactoryAppEventExit;
            consumed = true;
        } else if(event->key == InputKeyOk || event->key == InputKeyStart) {
            app_event = BackDisplayFactoryAppEventNextPattern;
            consumed = true;
        }

        if(consumed) {
            furi_check(
                furi_message_queue_put(app->event_queue, &app_event, FuriWaitForever) ==
                FuriStatusOk);
        }
    }

    if((!consumed) && (event->key != InputKeyBack)) {
        consumed = true;
    }

    return consumed;
}

static void back_display_factory_app_update(BackDisplayFactoryApp* app) {
    with_gui(app->gui, {
        back_display_pattern_update(app->canvas, app->current_pattern);
        FuriString* pattern_str = furi_string_alloc();
        back_display_pattern_to_string(app->current_pattern, pattern_str);
        label_set_text(app->pattern_label, furi_string_get_cstr(pattern_str));
        furi_string_free(pattern_str);
    });
}

static void
    back_display_factory_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    BackDisplayFactoryApp* app = context;
    furi_check(object == app->event_queue);

    BackDisplayFactoryAppEvent event;
    furi_check(furi_message_queue_get(app->event_queue, &event, 0) == FuriStatusOk);

    switch(event) {
    case BackDisplayFactoryAppEventNextPattern:
        app->current_pattern = (app->current_pattern + 1) % BackDisplayPatternMax;
        break;
    case BackDisplayFactoryAppEventPrevPattern:
        app->current_pattern = (app->current_pattern == 0) ? BackDisplayPatternMax - 1 :
                                                             app->current_pattern - 1;
        break;
    case BackDisplayFactoryAppEventTick:
        break;
    case BackDisplayFactoryAppEventExit:
        furi_event_loop_stop(app->event_loop);
        break;
    }

    back_display_factory_app_update(app);
}

static void back_display_factory_app_timer_callback(void* context) {
    BackDisplayFactoryApp* app = context;
    back_display_factory_app_update(app);
}

static BackDisplayFactoryApp* back_display_factory_app_alloc() {
    BackDisplayFactoryApp* app = malloc(sizeof(BackDisplayFactoryApp));

    app->event_loop = furi_event_loop_alloc();
    app->event_queue = furi_message_queue_alloc(16, sizeof(BackDisplayFactoryAppEvent));
    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->event_queue,
        FuriEventLoopEventIn,
        back_display_factory_app_event_queue_callback,
        app);
    app->timer = furi_event_loop_timer_alloc(
        app->event_loop,
        back_display_factory_app_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        app);

    app->desktop = furi_record_open(RECORD_DESKTOP);
    desktop_pin_current_app(app->desktop, true);

    // Create a single label for the back display
    app->gui = furi_record_open(RECORD_GUI);

    with_gui(app->gui, {
        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdSystem);
        gui_layer_add_input_callback(layer, back_display_factory_app_input_callback, app);

        Widget* root;

        // Front display
        root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        app->app_window = widget_alloc(root);
        app->pattern_label = label_alloc(app->app_window);

        //  Back display
        root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        app->canvas = canvas_alloc(root, widget_get_width(root), widget_get_height(root));
    });

    back_display_factory_app_update(app);
    furi_event_loop_timer_start(app->timer, 1000 / 60);

    return app;
}

static void back_display_factory_app_free(BackDisplayFactoryApp* app) {
    furi_assert(app);

    with_gui(app->gui, {
        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdSystem);
        gui_layer_remove_input_callback(layer, back_display_factory_app_input_callback);

        widget_free(app->app_window);
        canvas_free(app->canvas);
    });

    furi_record_close(RECORD_GUI);

    desktop_pin_current_app(app->desktop, false);
    furi_record_close(RECORD_DESKTOP);

    furi_event_loop_unsubscribe(app->event_loop, app->event_queue);
    furi_message_queue_free(app->event_queue);
    furi_event_loop_timer_free(app->timer);
    furi_event_loop_free(app->event_loop);
    free(app);
}

int32_t back_display_factory_app(void* args) {
    UNUSED(args);

    BackDisplayFactoryApp* app = back_display_factory_app_alloc();

    BrightnessControl* brightness = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
    brightness_control_set_brightness_override(
        brightness, BrightnessControlModuleBackDisplay, BRIGHTNESS_MAX);

    furi_event_loop_run(app->event_loop);

    brightness_control_reset_brightness_override(brightness, BrightnessControlModuleBackDisplay);
    furi_record_close(RECORD_BRIGHTNESS_CONTROL);
    back_display_factory_app_free(app);

    return 0;
}
