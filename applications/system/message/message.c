#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/label.h>

typedef struct {
    FuriEventLoop* event_loop;
    Gui* gui;
    Label* label;
} MessageApp;

static void message_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    MessageApp* instance = context;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_stop(instance->event_loop);
        }
    }
}

static MessageApp* message_app_alloc(const char* message) {
    MessageApp* instance = malloc(sizeof(MessageApp));
    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        Widget* root = gui_get_root_widget(gui, GuiDisplayIdFront, GuiLayerIdActive);
        instance->label = label_alloc(root);
        label_set_text(instance->label, message ? message : "Hello There");

        widget_set_input_callback((Widget*)instance->label, message_app_input_callback, instance);
        gui_set_active_widget(gui, (Widget*)instance->label);
    });

    return instance;
}

static void message_app_free(MessageApp* instance) {
    with_gui(instance->gui, { label_free(instance->label); });

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
