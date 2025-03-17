#include "input_test.h"

#include <furi.h>

#define TAG "InputTest"

static void input_test_app_update(InputTestApp* instance) {
    with_gui(instance->gui, { label_set_text_fmt(instance->label_text, "Hi Hui"); });
}

static void ligh_sensor_test_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    InputTestApp* instance = context;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        const InputTestAppEvent app_event = {
            .type = InputTestAppEventExit,
        };
        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    } else if((event->type == InputTypeShort)) {
        const InputTestAppEvent app_event = {
            .type = InputTestAppEventKeyStateChanged,
            .input_key = event->key,
        };
        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static void input_test_app_handle_input_short_event(InputTestApp* instance, InputKey key) {
    InputTestAppModel* model = &instance->input_model;
    if(key == InputKeyUp) {
        model->encoder++;
    } else if(key == InputKeyDown) {
        model->encoder--;
    } else if(key == InputKeyOk) {
        model->ok++;
    } else if(key == InputKeyBack) {
        memset(model, 0, sizeof(InputTestAppModel));
        model->switch_pos = -1;
    } else if(key == InputKeyStart) {
        model->start++;
    } else if(key >= InputKeyBusy && key < InputKeyMAX) {
        model->switch_pos = key;
    }
}

static void input_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    InputTestApp* instance = context;
    furi_check(object == instance->event_queue);

    InputTestAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(event.type == InputTestAppEventExit) {
        furi_event_loop_stop(instance->event_loop);
    } else if(event.type == InputTestAppEventKeyStateChanged) {
        with_gui(instance->gui, {
            input_test_app_handle_input_short_event(instance, event.input_key);
            input_test_app_update(instance);
        })
    }
}

static InputTestApp* input_test_app_alloc(void) {
    InputTestApp* instance = malloc(sizeof(InputTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(InputTestAppEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        input_test_app_event_queue_callback,
        instance);

    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        Widget* root = gui_get_root_widget(instance->gui, GuiDisplayIdBack, GuiLayerIdMain);

        instance->app_window = widget_alloc(root);

        // Back screen
        instance->label_text = label_alloc(instance->app_window);
        widget_set_pos((Widget*)instance->label_text, 10, 0);

        // Input events
        widget_set_input_callback(
            instance->app_window, ligh_sensor_test_app_input_callback, instance);

        gui_add_active_widget(instance->gui, instance->app_window);
        input_test_app_update(instance);
    });

    return instance;
}

static void input_test_app_free(InputTestApp* instance) {
    furi_check(instance);

    with_gui(instance->gui, { widget_free(instance->app_window); });

    furi_record_close(RECORD_GUI);

    furi_record_close(RECORD_LIGHT_SENSOR_EVENTS);

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_timer_free(instance->timer);
    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t input_test_app(void* args) {
    UNUSED(args);

    InputTestApp* instance = input_test_app_alloc();
    furi_event_loop_run(instance->event_loop);
    input_test_app_free(instance);

    return 0;
}
