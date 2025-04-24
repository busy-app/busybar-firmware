#include "input_test.h"

#define TAG "InputTest"

static void input_test_app_back_displaylay_update(InputTestApp* instance) {
    InputTestAppModel* m = &instance->input_model;

    with_gui(instance->gui, {
        label_set_text_fmt(
            instance->content_label,
            "OK: %lu   Start: %lu\n"
            "Encoder: %ld\n"
            "Switch: %s",
            m->ok,
            m->start,
            m->encoder,
            m->switch_pos < 0 ? "--" : input_get_key_name(m->switch_pos));
    });
}

static bool input_test_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    InputTestApp* instance = context;
    InputTestAppEvent app_event;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        app_event.type = InputTestAppEventExit;
        consumed = true;
    } else if((event->type == InputTypeShort)) {
        app_event.type = InputTestAppEventKeyStateChanged;
        app_event.input_key = event->key;
        consumed = true;
    }

    if(consumed) {
        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    }

    return consumed;
}

static void input_test_app_reset_model(InputTestAppModel* model) {
    memset(model, 0, sizeof(InputTestAppModel));
    model->switch_pos = -1;
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
        input_test_app_reset_model(model);
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
        input_test_app_handle_input_short_event(instance, event.input_key);
        input_test_app_back_displaylay_update(instance);
    }
}

static InputTestApp* input_test_app_alloc(void) {
    InputTestApp* instance = malloc(sizeof(InputTestApp));
    input_test_app_reset_model(&instance->input_model);

    instance->event_loop = furi_event_loop_alloc();

    instance->event_queue = furi_message_queue_alloc(16, sizeof(InputTestAppEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        input_test_app_event_queue_callback,
        instance);

    instance->desktop = furi_record_open(RECORD_DESKTOP);
    desktop_pin_current_app(instance->desktop, true);

    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, input_test_app_input_callback, instance);

        Widget* root;

        // Front display
        root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
        instance->front_label = label_alloc(root);
        label_set_text(instance->front_label, "Look at back display");
        widget_set_align(label_get_base(instance->front_label), AlignCenter);

        // Back display
        root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);

        instance->back_window = widget_alloc(root);

        instance->header_label = label_alloc(instance->back_window);
        label_set_text(instance->header_label, "Input Test");
        widget_set_align(label_get_base(instance->header_label), AlignTopMid);

        instance->content_label = label_alloc(instance->back_window);
        label_set_line_spacing(instance->content_label, 2);
        widget_set_align(label_get_base(instance->content_label), AlignLeftMid);

        instance->footer_label = label_alloc(instance->back_window);
        label_set_text(instance->footer_label, "Hold 'Back' to exit");
        widget_set_align(label_get_base(instance->footer_label), AlignBottomRight);
    });

    input_test_app_back_displaylay_update(instance);

    return instance;
}

static void input_test_app_free(InputTestApp* instance) {
    furi_check(instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, input_test_app_input_callback);
        label_free(instance->front_label);
        widget_free(instance->back_window);
    });

    furi_record_close(RECORD_GUI);

    desktop_pin_current_app(instance->desktop, false);
    furi_record_close(RECORD_DESKTOP);

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->event_queue);
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
