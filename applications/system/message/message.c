#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/label.h>

typedef struct {
    FuriEventLoop* event_loop;
    Gui* gui;
    Label* label;
} MessageApp;

static bool message_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    MessageApp* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_stop(instance->event_loop);
            consumed = true;
        }
    }

    return consumed;
}

static MessageApp* message_app_alloc(const char* message) {
    MessageApp* instance = malloc(sizeof(MessageApp));
    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, message_app_input_callback, instance);
        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
        instance->label = label_alloc(root);
        label_set_text(instance->label, message ? message : "Hello There");
    });

    return instance;
}

static void message_app_free(MessageApp* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, message_app_input_callback);
        label_free(instance->label);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t message_app(void* arg) {
    MessageApp* instance = message_app_alloc(arg);
    furi_event_loop_run(instance->event_loop);
    message_app_free(instance);

    return 0;
}
